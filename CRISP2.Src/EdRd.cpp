/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.EDRD * by ML ******************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"
#include "rgconfig.h"
// Added by Peter on 21 Feb 2006
#include "EDRD.h"

// Added by Peter on 24 February 2006
#include "Crash.h"

#pragma warning(disable:4305)

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

#ifdef _USE_LOGGING
FILE *s_pLog = NULL;
#endif

#define	WMUSER_ADD_RINGS		(WM_USER+100)

#define TMP_End (&TMP[strlen(TMP)])

// Added by Peter on 01 Mar 2006
extern BOOL EstimateBackground_1D(double *pProfile, int nSize, double *pOutProfile);
extern bool LocateMaximum(OBJ* pObj, LPEDRD pEDRD, double &x, double &y, double dErr);
extern void CorrectEllipticalDistortion(HWND hDlg, OBJ* pObj, LPEDRD pEDRD);
extern BOOL GetAutoconvCentre( HWND hDlg, OBJ* pObj, LPEDRD pEDRD );

//added by Peter on 27 Feb 2006
typedef BOOL (__stdcall *pfnPEAKFIT)(LPVOID, double);
typedef BOOL (__stdcall *pfnPEAKFIT_Free)(LPVOID);
extern pfnPEAKFIT		g_pfnPEAKFIT;
extern pfnPEAKFIT_Free	g_pfnPEAKFIT_Free;
//end of addition by Peter on 27 Feb 2006

/****************************************************************************/
/* ED measurements window * by ML *******************************************/
/****************************************************************************/

// To be used in Information pannel

#define	MAXLEN	4096		// Maximum line length
#define	MAXARC	100			// Maximum number of arcs
#define	MRS		2			// Marker size on plot

int _cdecl sort_arc_R(const void *pEl1, const void *pEl2)
{
	EDRD_REFL *pArc1 = (EDRD_REFL*)pEl1, *pArc2 = (EDRD_REFL*)pEl2;

	if( pArc1->dX0 < pArc2->dX0 )		return -1;
	else if( pArc1->dX0 == pArc2->dX0 )	return  0;
	return  1;
}

static void SetNegative(OBJ* pImg, IMAGE &Image, HWND hDlg)
{
	Image.neg = 0;
	BOOL bSet;

	if( NULL != pImg )
	{
		bSet = (TRUE == Image.IsNegative());
//		bSet = IsNegative(pImg);
	}
	else if( NULL != hDlg )
	{
		bSet = IsDlgButtonChecked( hDlg, IDC_EDRD_NEGAT );
	}
	else
	{
		return;
	}
	if( TRUE == bSet )
	{
		switch( Image.npix )
		{
		case PIX_BYTE:
		case PIX_CHAR:
			Image.neg = 0xFF;
			break;
		case PIX_WORD:
		case PIX_SHORT:
			Image.neg = 0xFFFF;
			break;
		case PIX_DWORD:
		case PIX_LONG:
			Image.neg = 0xFFFFFFFF;
			break;
		case PIX_FLOAT:
			Image.neg = 0xFFFFFFFF;
			break;
		case PIX_DOUBLE:
			Image.neg = 0xFFFFFFFF;
			break;
		default:
			MessageBox(NULL, "SetNegative - unsupported pixel format", "EDRD ERROR!", MB_OK);
			return;
			break;
		}
	}
	CheckDlgButton(hDlg, IDC_EDRD_NEGAT, Image.neg);
}

/****************************************************************************/
static void	DrawPlot( HWND hDlg, OBJ* O, LPEDRD S)
{
	HWND 	hw = GetDlgItem( hDlg, IDC_EDRD_PLOT);
	HDC 	shdc;
	HDC		hdc;
	HBITMAP hbmp, hobmp;
	HFONT	hof;
	int 	i, j, xs, ys;
	int		xsl, ysl;
	RECT 	r;
	int		x, y;
	double	scale;

	if (hw == NULL)
	{
		MessageBeep(-1);
		return;
	}
	GetClientRect( hw, &r );
	InvalidateRect(hw, NULL, FALSE);

	shdc = GetDC( hw );
	hdc = CreateCompatibleDC( shdc );
	if (hdc==NULL || shdc==NULL)
	{
		goto ex;
	}

	if (((fabs(O->NI.Scale-1e-10)>1e-12)) && (O->NI.Flags&NI_DIFFRACTION_PATTERN)!=0)
	{
		S->bCalibrated = TRUE;
		scale = O->NI.Scale * 1e10;
	}
	else
	{
		S->bCalibrated = FALSE;
		scale = 1.;
	}
	
	xs = r.right - r.left;
	ys = r.bottom - r.top;
	xsl = xs - MRS*2;
	ysl = ys - MRS*2;
	hbmp = CreateCompatibleBitmap( shdc, xs, ys );
	if (hbmp==NULL)
		goto ex;
	hobmp = SelectBitmap( hdc, hbmp );
	PatBlt( hdc, 0, 0, xs, ys, BLACKNESS);
	
	SetROP2( hdc, R2_MERGEPEN );
		
	{
#ifdef USE_FONT_MANAGER
		int nFntSizeW, nFntSizeH;
		HFONT hFont = FontManager::GetFont(FM_SMALL_FONT, nFntSizeW, nFntSizeH);
#else
		int nFntSizeH = fnts_h;
		HFONT hFont = hfSmall;
#endif
		if (S->flags & (RDFDONE|PRFDONE))
		{
			// Grid
			SelectPen( hdc, GrayPen );
			SetTextColor( hdc, RGB(128,128,128));
			SetBkMode( hdc, TRANSPARENT );
			hof = SelectFont( hdc, hFont );
			_MoveTo( hdc, 0, MRS );            LineTo( hdc, xs, MRS);
			_MoveTo( hdc, 0, MRS+ysl/2 ); LineTo( hdc, xs, MRS+ysl/2);
			_MoveTo( hdc, 0, MRS+ysl );   LineTo( hdc, xs, MRS+ysl);
			for(i = 0; i <= 10; i++)
			{
				x = i*xsl/10 + MRS;
				_MoveTo( hdc, x, 0 );
				LineTo( hdc, x, ys );
				_MoveTo( hdc, x+1, ys/2-1);
				if (S->bCalibrated)
				{
					if (i) sprintf( TMP, "%-2.3f", scale/((double)i*.1*S->r)); // A
					else   sprintf( TMP, "Inf ");
				} else	   sprintf( TMP, "%-4.1f", (double)i*.1*S->r);
				y = ys-MRS-nFntSizeH;
				TextOut(hdc, x+1, y, TMP, 4);
			}
			sprintf(TMP, "%-3.0f", S->rdmin);
			TextOut(hdc, MRS+2, ys-MRS-nFntSizeH*2, TMP, 3);
			sprintf(TMP, "%-3.0f", S->rdmax);
			TextOut(hdc, MRS+2, MRS+1, TMP, 3);
			SelectFont( hdc, hof );
		}
		if (S->flags & RDFDONE)
		{
			// Drawing RDF curve
			SelectPen( hdc, RedPen );
			x = MRS;
			y = ysl-ysl * (S->lpRDF[0]-S->rdmin)/(S->rdmax-S->rdmin)+MRS;
			_MoveTo( hdc, x, y );
			for(i = 0; i < S->r; i++)
			{
				x = xsl*i/(S->r-1) + MRS;
				y = ysl-ysl * (S->lpRDF[i]-S->rdmin)/(S->rdmax-S->rdmin)+MRS;
				LineTo( hdc, x, y );
			}
		}
		else if (S->flags & PRFDONE)
		{
			// Drawing profile curves
			SelectPen( hdc, RedPen );
			for(j = 0; j < S->profcnt; j++)
			{
				x = MRS;
				y = ysl-ysl * (S->lpProf[j][0]-S->rdmin)/(S->rdmax-S->rdmin)+MRS;
				_MoveTo( hdc, x, y );
				for(i = 0; i < S->r; i++)
				{
					x = xsl*i/(S->r-1)+MRS;
					y = ysl-ysl * (S->lpProf[j][i]-S->rdmin)/(S->rdmax-S->rdmin)+MRS;
					LineTo( hdc, x, y );
				}
			}
			SelectPen( hdc, GreenPen );
			x = MRS;
			y = ysl-ysl * (S->lpRDF[0]-S->rdmin)/(S->rdmax-S->rdmin)+MRS;
			_MoveTo( hdc, x, y );
			for(i = 0; i < S->r; i++)
			{
				x = xsl*i/(S->r-1) + MRS;
				y = ysl-ysl * (S->lpRDF[i]-S->rdmin)/(S->rdmax-S->rdmin)+MRS;
				LineTo( hdc, x, y );
			}
		}
	}

	if (!BitBlt( shdc, 0, 0, xs, ys, hdc, 0, 0, SRCCOPY))
	{
		// MessageBeep( -1 );
		OutputDebugString("DrawPlot() -> Cannot BitBlit\n");
	}
	if (hobmp) SelectBitmap( hdc, hobmp );
ex:
	if (hdc) DeleteDC( hdc );
	if (hbmp) DeleteObject( hbmp );
	if (shdc) ReleaseDC( hw, shdc );
	GetWindowRect( hw, &r );
	InflateRect( &r, -1, -1);
	MapWindowPoints( NULL, hDlg, (LPPOINT)&r, 2);
	ValidateRect( hDlg, &r);
}
/****************************************************************************/
static void DrawX( HDC hdc, int xx1, int yy1 )
{
	_MoveTo( hdc, xx1-1,  yy1-5 ); LineTo( hdc, xx1-1,  yy1-1 ); LineTo( hdc, xx1-6,  yy1-1 );
	_MoveTo( hdc, xx1+1,  yy1-5 ); LineTo( hdc, xx1+1,  yy1-1 ); LineTo( hdc, xx1+6,  yy1-1 );
	_MoveTo( hdc, xx1-5,  yy1+1 ); LineTo( hdc, xx1-1,  yy1+1 ); LineTo( hdc, xx1-1,  yy1+6 );
	_MoveTo( hdc, xx1+1,  yy1+5 ); LineTo( hdc, xx1+1,  yy1+1 ); LineTo( hdc, xx1+6,  yy1+1 );
}
/****************************************************************************/
static void cl2sc( OBJ* pr, int x, int y, int *xx, int *yy )
{
	double scale = (double)pr->scu/(double)pr->scd;
	int off = (int)(pr->scu >> 1);
	*xx = round((x - pr->xo)*scale + off);
	*yy = round((y - pr->yo)*scale + off);
}
/****************************************************************************/
static	void	DrawEDRD( HWND hDlg, OBJ* O, LPEDRD S )
{
	OBJ*		pr;
	HDC			hdc;
	HPEN		hop;
	HBRUSH		hob;
	int			xx1, yy1, r, i;
	int			xe0, ye0, xe1, ye1;
	double		x0, y0, x1, y1, scale;
	LPARC		lpa;
	int			xx, yy, off;

	pr = OBJ::GetOBJ(O->ParentWnd);
	lpa = S->lpArc;
	if ((hdc = GetDC( O->ParentWnd )) == NULL)
		return;
	scale = (double)pr->scu/(double)pr->scd;
	off = (int)(pr->scu >> 1);

// First point
	r   = round((S->r)*scale + off);

	hop = SelectPen( hdc, RedPen );
	cl2sc( pr, (int)S->xc, (int)S->yc, &xx1, &yy1 );
	DrawX( hdc, xx1, yy1 );

	// Commented by Peter on 07 Mar 2006
/*
// 4 points
	SelectPen( hdc, BluePen );
	for(i = 0; i < S->p4cnt; i++)
	{
		if (S->p4[i].x==0 && S->p4[i].y==0) break;
		cl2sc( pr, S->p4[i].x, S->p4[i].y, &xx, &yy );
		DrawX( hdc, xx, yy );
	}
*/
	// End of comments by Peter on 07 Mar 2006

	// Added by Peter on 21 Feb 2006
	std::vector<CVector2d>::iterator it;
	for(it = S->vUserClicks.begin(); it != S->vUserClicks.end(); ++it)
	{
		cl2sc( pr, it->x, it->y, &xx, &yy );
		DrawX( hdc, xx, yy );
	}
	if( true == S->bEstimated )
	{
		double dSin, dCos;
		FastSinCos(S->dDistortionDir, dSin, dCos);
		cl2sc( pr, S->xc - S->dDistA*dCos, S->yc + S->dDistA*dSin, &xx, &yy );
		_MoveTo(hdc, xx, yy);
		cl2sc( pr, S->xc + S->dDistA*dCos, S->yc - S->dDistA*dSin, &xx, &yy );
		SelectPen( hdc, BluePen );
		LineTo(hdc, xx, yy);
		FastSinCos(M_PI_2 + S->dDistortionDir, dSin, dCos);
		cl2sc( pr, S->xc - S->dDistB*dCos, S->yc + S->dDistB*dSin, &xx, &yy );
		_MoveTo(hdc, xx, yy);
		cl2sc( pr, S->xc + S->dDistB*dCos, S->yc - S->dDistB*dSin, &xx, &yy );
		hop = SelectPen( hdc, RedPen );
		LineTo(hdc, xx, yy);
//		Ellipse(hdc, , , , );
//		S->dDistA = S->dDistB = 0.0;
	}
	// End of addition by Peter on 21 Feb 2006

// Circle
	SelectPen( hdc, BluePen );
	hob = SelectBrush( hdc, GetStockObject(NULL_BRUSH));
	Ellipse( hdc, xx1-r, yy1-r, xx1+r+1, yy1+r+1 );

// Arc
	if (IsDlgButtonChecked( hDlg, IDC_EDRD_SARC ))
	{
		x0 = S->xc + S->r*cos(S->ae0);
		y0 = S->yc + S->r*sin(S->ae0);
		x1 = S->xc + S->r*cos(S->ae1);
		y1 = S->yc + S->r*sin(S->ae1);
		xe0 = round((x0 - pr->xo)*scale + off);
		ye0 = round((y0 - pr->yo)*scale + off);
		xe1 = round((x1 - pr->xo)*scale + off);
		ye1 = round((y1 - pr->yo)*scale + off);

		_MoveTo( hdc, xx1, yy1 ); LineTo( hdc, xe0, ye0 );
		_MoveTo( hdc, xx1, yy1 ); LineTo( hdc, xe1, ye1 );
		SelectPen( hdc, GreenPen );
		Arc( hdc, xx1-r, yy1-r, xx1+r+1, yy1+r+1, xe1, ye1, xe0, ye0 );
	}

	SelectPen( hdc, RedPen );
	lpa = S->lpArc;
	for(i = 0; i < S->arccnt; i++)
	{
		x0 = S->xc + lpa->rad*cos(lpa->a0);
		y0 = S->yc + lpa->rad*sin(lpa->a0);
		x1 = S->xc + lpa->rad*cos(lpa->a1);
		y1 = S->yc + lpa->rad*sin(lpa->a1);
		xe0 = round((x0 - pr->xo)*scale + off);
		ye0 = round((y0 - pr->yo)*scale + off);
		xe1 = round((x1 - pr->xo)*scale + off);
		ye1 = round((y1 - pr->yo)*scale + off);

		r  = round((lpa->rad)*scale    + off);
		if (xe0!=xe1 || ye0!=ye1 || fabs(lpa->a0-lpa->a1)>1.9*M_PI)
			Arc( hdc, xx1-r, yy1-r, xx1+r+1, yy1+r+1, xe1, ye1, xe0, ye0 );
		else
			SetPixel(hdc,xe0,ye0,RGB(255,0,0));

		x0 = (double)S->xc + ((double)lpa->rad-lpa->wid)*cos((lpa->a0+lpa->a1)*.5);
		y0 = (double)S->yc + ((double)lpa->rad-lpa->wid)*sin((lpa->a0+lpa->a1)*.5);
		x1 = (double)S->xc + ((double)lpa->rad+lpa->wid+1)*cos((lpa->a0+lpa->a1)*.5);
		y1 = (double)S->yc + ((double)lpa->rad+lpa->wid+1)*sin((lpa->a0+lpa->a1)*.5);
		xe0 = round((x0 - pr->xo)*scale + off);
		ye0 = round((y0 - pr->yo)*scale + off);
		xe1 = round((x1 - pr->xo)*scale + off);
		ye1 = round((y1 - pr->yo)*scale + off);
		_MoveTo(hdc,xe0,ye0); LineTo(hdc,xe1,ye1);
//		SetPixel(hdc,xe0,ye0,RGB(255,0,0));
		SetPixel(hdc,xe1,ye1,RGB(255,0,0));

		lpa++;
	}

	if (hop) SelectPen(hdc, hop);
	if (hob) SelectBrush(hdc, hob);
	ReleaseDC( O->ParentWnd, hdc );

}
/****************************************************************************/
void UpdateCoordinatesText( HWND hDlg, OBJ* O, LPEDRD S)
{
	sprintf(TMP, "X=%.1f Y=%.1f", S->xc, S->yc);
	SetDlgItemText( hDlg, IDC_EDRD_CENTER, TMP);

	if (S->bCalibrated && S->r!=0)
	{
		sprintf( TMP, "Rad=%-2.3fAng", O->NI.Scale * 1e10/((double)S->r)); // Å
	} else
		sprintf( TMP, "Rad=%dpix", S->r);
	if (IsDlgButtonChecked(hDlg,IDC_EDRD_SARC))
	{
		int a0, a1, da;
		a0 = round(R2D(S->ae0));
		a1 = round(R2D(S->ae1));
		da = a1 - a0;
		if (da<0) da+=360;
		sprintf(&TMP[strlen(TMP)],", Arc[%d°,%d°]=(%d°)",a0,a1,da);
	}
	if (S->np) sprintf(&TMP[strlen(TMP)],", %ld pixels used",S->np);

	SetDlgItemText( hDlg, IDC_EDRD_CREFRAD, TMP);

}
/****************************************************************************/
static void UpdateEDRD( HWND hDlg, OBJ* O, LPEDRD S )
{
	OBJ*	pr = OBJ::GetOBJ( O->ParentWnd );	// Image Object

	UpdateCoordinatesText( hDlg, O, S );
	InvalidateRect( O->ParentWnd, NULL, FALSE);							// To redraw crosses

}
/****************************************************************************/
static VOID AdjustRefineRadius( LPEDRD S )
{
//	if (round(S->xc) + S->r>=S->iw-1) S->r = S->iw-round(S->xc)-2;
//	if (round(S->yc) + S->r>=S->ih-1) S->r = S->ih-round(S->yc)-2;
//	if (round(S->xc) - S->r<=0)       S->r = round(S->xc)-1;
//	if (round(S->yc) - S->r<=0)       S->r = round(S->yc)-1;
	if (S->r<3) S->r=3;
	if (S->r>=MAXLEN) S->r = MAXLEN-1;

}
/****************************************************************************/
/*
int GetAXY8test2(LPBYTE img, int imw, int imh, int neg, int x, int y, int r,
				 int thr, LPFLOAT axs, LPFLOAT ays, LPFLOAT as)
{
	BYTE val;
	DWORD x0, y0;
	LPBYTE ptr = img;
	int i, j, count = 0;

	x0 = x - r;
	if(x0 < 0)
		return 0;
	ptr += x0;
	if(x+r >= imw)
		return 0;
	y0 = y - r;
	if(y0 < 0)
		return 0;
	ptr += imw*y0;
	if(y+r >= imh)
		return 0;
	r = 2*r+1;
	for(j = 0; j <= r; j++)
	{
		for(i = 0; i <= r; i++)
		{
			val = *ptr;
			ptr++;
			val ^= neg;
			if(val >= thr)
			{
				as += val;
				axs += val*x0;
				ays += val*y0;
				count++;
			}
			x0++;
		}
		y0++;
		ptr += imw - 2*r - 1;
	}
	return count;

}
*/
// Added by Peter on 07 Mar 2006
static BOOL ManualRefineCenter( HWND hDlg, OBJ* pObj, LPEDRD pEDRD )
{
	BOOL bRet = FALSE;
	CCircleFit cfit;
	CVector out;

	try
	{
		if( pEDRD->vUserClicks.size() < 4 )
		{
			MessageBox(hDlg, "Define ring with 4 points minimum.", "EDRD WARNING!", MB_OK);
		}
		else
		{
			size_t j, nSize = pEDRD->vUserClicks.size();
			vector<double> px(nSize), py(nSize);
			// prepare arrays of coordinates
			for(j = 0; j < nSize; j++)
			{
				px[j] = (double)pEDRD->vUserClicks[j].x;
				py[j] = (double)pEDRD->vUserClicks[j].y;
			}
			// solve for the circle
			if( cfit.Solve(&*px.begin(), &*py.begin(), nSize, out) != TRUE )
			{
				::OutputDebugString("ManualRefineCenter(): Bad points for circle refinement\n");
				MessageBeep( -1 );
			}
			// get solved data
			pEDRD->xc = out[0];	// Xc
			pEDRD->yc = out[1];	// Yc
			pEDRD->r  = out[2];	// R
			pEDRD->vUserClicks.clear();
			pEDRD->r = __min(__min(pEDRD->xc - 2, pEDRD->image.w - pEDRD->xc-2),
				__min(pEDRD->yc-2,pEDRD->image.h - pEDRD->yc-2));
			if( pEDRD->r < 10 )
			{
				pEDRD->r = 10;
			}
			bRet = TRUE;
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("ManualRefineCenter() -> %s"), e.what()));
	}
	return bRet;

}
// End of Peter's addition on 07 Mar 2006

static BOOL RefineCenter( HWND hDlg, OBJ* pObj, LPEDRD pEDRD )
{
	BOOL bRes = GetAutoconvCentre(hDlg, pObj, pEDRD);

	if( TRUE == bRes )
	{
		pEDRD->r = 
			__min(__min(pEDRD->xc-2,pEDRD->image.w-pEDRD->xc-2),
				  __min(pEDRD->yc-2,pEDRD->image.h-pEDRD->yc-2));
		if( pEDRD->r < 10 )
		{
			pEDRD->r = 10;
		}
	}
	return bRes;
// Commented by Peter on 21 Feb 2006
/*
	float	axs, ays, as;
	int		thr;

	thr = 0;
	ClrBits( S->flags, CRDONE );
	if (GetAXY8( S->lpImg, S->iw, S->ih, S->neg, round(S->xc), round(S->yc), S->r, thr, &axs, &ays, &as) < 100 || as<1.)
		return FALSE;
	if (GetAXY8test2( (LPBYTE)S->lpImg, S->iw, S->ih, S->neg, round(S->xc), round(S->yc), S->r, thr, &axs, &ays, &as) < 100 || as<1.)
		return FALSE;
	SetBits( S->flags, CRDONE );
	S->xc = round(axs/as);
	S->yc = round(ays/as);
	AdjustRefineRadius( S );

//	if (GetAXY8( S->lpImg, S->iw, S->ih, S->neg, round(S->xc), round(S->yc), S->r, thr, &axs, &ays, &as) < 100 || as<1.)
	if (GetAXY8test2( (LPBYTE)S->lpImg, S->iw, S->ih, S->neg, round(S->xc), round(S->yc), S->r, thr, &axs, &ays, &as) < 100 || as<1.)
		return TRUE;
	S->xc = round(axs/as);
	S->yc = round(ays/as);
	AdjustRefineRadius( S );
	return TRUE;
*/

}
/****************************************************************************/
static VOID	SubtractBkg(LPDOUBLE src, LPDOUBLE tmp, int len)
{
	EstimateBackground_1D(src, len, tmp);
	copy(tmp, tmp+len, src);

}
// commented by Peter on 06 Mar 2006
/*
static VOID	SubtractBkg(LPDOUBLE src, LPDOUBLE tmp, int len)
{
	double sx, sy, sxy, sxx, del, aa, bb;
	int		i0, i;
	double	a0, a1, as;

// search for first point (i0) which is 5% lower than maximum, and lies on \ slope
	i0 = 1;
	for(i=0,a0=1e10;i<len;i++)  if (src[i]<a0) a0 = src[i];
	for(i=0,a1=-1e10;i<len;i++) if (src[i]>a1) a1 = src[i];
	a0 = a1-(a1-a0)*.05;
	for(i=1;i<len/2;i++) if(src[i]<a0 && src[i]>src[i+1]) { i0=i; break; }
// fitting rdf to a line at interval (i0 - r)
	sx = 0.; sy = 0.; sxy = 0.; sxx = 0.; as=0.;
	for(i=i0;i<len;i++)
	{
		sx += i;
		sy += src[i];
		sxy += src[i]*i;
		sxx += (double)i*(double)i;
		as++;
	}
	del = as*sxx-sx*sx;
	if (del>1e-4)
	{
		aa = (sxx*sy-sx*sxy)/del;
		bb = (as*sxy-sx*sy)/del;
	}
	else
	{
		aa=0.;
		bb=0.;
	}
// subtract this line
	for(i=0;i<len;i++) src[i] = src[i]-aa-bb*i;

//	for(i=1;i<len;i++) tmp[i] = src[i]-src[i-1];
	for(i=1;i<len;i++) tmp[i] = src[i];
	tmp[0]=0;
// fitting local minimums of rdf to a line
	sx = 0.; sy = 0.; sxy = 0.; sxx = 0.; as=0.;
	for(i=i0;i<len;i++)
	{
		if(src[i]<src[i-1] && src[i]<src[i+1])
		{
			sx += i;
			sy += tmp[i];
			sxy += tmp[i]*i;
			sxx += (double)i*(double)i;
			as++;
		}
	}
	del = as*sxx-sx*sx;
	if (del>1e-6)
	{
		aa = (sxx*sy-sx*sxy)/del;
		bb = (as*sxy-sx*sy)/del;
	}
	else
	{
		aa=0.; bb=0.;
	}
// subtract also this line
	for(i=0;i<len;i++) tmp[i] = tmp[i]-aa-bb*i;
	for(i=0;i<i0;i++) tmp[i]=0;

//	tmp[0]=0;
//	for(i=1,d1=0;i<len;i++) { d1+=tmp[i]; tmp[i]=d1; }

	for(i=i0;i>=0;i--) tmp[i] = tmp[i0];
	memcpy( src, tmp, sizeof(double)*len);
}
*/
// end of comment by Peter on 06 Mar 2006
/****************************************************************************/
static VOID	LowPass( LPDOUBLE src, LPDOUBLE tmp, int len )
{
	int i;
	memcpy( tmp, src, sizeof(double)*len );
	src[0]     = src[1];
	src[len-1] = src[len-2];
	for(i=1;i<len-1;i++) src[i] = (tmp[i-1]+tmp[i]+tmp[i+1])/3.;
	src[0]     = src[1];
	src[len-1] = src[len-2];

}
/****************************************************************************/
template <typename _Tx>
_Tx make_negative(_Tx _val, int neg)
{
	return _val ^ neg;
}

template <>
float make_negative<float>(float _val, int neg)
{
	float res = _val;
	(*(DWORD*)&res) = (*(DWORD*)&res) ^ neg;
	return _val;
}

template <>
double make_negative<double>(double _val, int neg)
{
	double res = _val;
	(*(DWORD*)&res) = (*(DWORD*)&res) ^ neg;
	((DWORD*)&res)[1] = ((DWORD*)&res)[1] ^ neg;
	return _val;
}

// Added by Peter on 03 Mar 2006
template <typename _Tx>
//BOOL GetRDF_(_Tx *pData, size_t imgw, size_t imgh, size_t stride,
//			 int neg, double dx, double dy, double r,
//			 double *pProf, long *pWgt)
BOOL GetRDF_(_Tx *pData, IMAGE &Image, double dx, double dy, double r, double *pProf, long *pWgt)
{
	BOOL bRet = TRUE;
	double r2 = r*r, cur_r2, cur_r, dy2, sqrt_r;
	int min_x, max_x, min_y, max_y, int_r, min_cx, max_cx;
	_Tx *ptr;
	size_t imgw = Image.w, imgh = Image.h, stride = Image.stride;
	int neg = Image.neg;

	min_x = (int)floor(dx - r - 0.5);
	max_x = (int)floor(dx + r + 0.5);
	min_y = (int)floor(dy - r - 0.5);
	max_y = (int)floor(dy + r + 0.5);
	if( min_x < 0 ) min_x = 0;
	if( min_y < 0 ) min_y = 0;
	if( max_x >= (int)imgw ) max_x = imgw - 1;
	if( max_y >= (int)imgh ) max_y = imgh - 1;
	ptr = (_Tx*)(((LPBYTE)Image.pData) + min_y * stride);
	for(int y = min_y; y <= max_y; y++)
	{
		dy2 = SQR(y - dy);
		if( r2 < dy2 ) continue;
		sqrt_r = FastSqrt( r2 - dy2);
		min_cx = dx - sqrt_r;
		max_cx = dx + sqrt_r;
		min_cx = (min_cx < min_x) ? min_x : min_cx;
		max_cx = (max_cx > max_x) ? max_x : max_cx;
		for(int x = min_cx; x <= max_cx; x++)
//		for(int x = min_x; x <= max_x; x++)
		{
			cur_r2 = SQR(x - dx) + dy2;
			if( cur_r2 > r2 )
				continue;
			cur_r = FastSqrt(cur_r2);
			int_r = (int)floor(cur_r + 0.5);
//			pProf[int_r] += (ptr[x] ^ neg);
//			pProf[int_r] += (*((basic_type<_Tx>::type_)&ptr[x]) ^ neg);
			pProf[int_r] += make_negative(ptr[x], neg);
			pWgt[int_r]++;
		}
		ptr = (_Tx*)(((LPBYTE)ptr) + stride);
	}
	return bRet;

}
// End of addition by Peter on 03 Mar 2006
/*
// Added by Peter on 08 Mar 2006
template <typename _Tx>
BOOL GetArcRDF_(_Tx *pData, size_t imgw, size_t imgh, size_t stride, int neg, double dx, double dy,
				double r, double *pProf, long *pWgt, double dStartA, double dAngle)
{
	_Tx *ptr;
	BOOL bRet = TRUE;
	double r2 = r*r, cur_r2, tanA, tanB, tanC, d_x, d_y, dy2;
	int min_x, max_x, min_y, max_y, int_r;

	tanA = FastTan(dStartA);
	tanB = FastTan(dStartA - dAngle);
	if( tanA < tanB )
	{
		tanC = tanA; tanA = tanB; tanB = tanC;
	}
	min_x = (int)floor(dx - r - 0.5);
	max_x = (int)floor(dx + r + 0.5);
	min_y = (int)floor(dy - r - 0.5);
	max_y = (int)floor(dy + r + 0.5);
	if( min_x < 0 ) min_x = 0;
	if( min_y < 0 ) min_y = 0;
	if( max_x >= imgw ) max_x = imgw - 1;
	if( max_y >= imgh ) max_y = imgh - 1;
	ptr = (_Tx*)(((LPBYTE)pData) + min_y * stride);
	for(int y = min_y; y <= max_y; y++)
	{
		dy2 = SQR(d_y = y - dy);
		for(int x = min_x; x <= max_x; x++)
		{
			d_x = x - dx;
			cur_r2 = SQR(d_x) + dy2;
			if( cur_r2 > r2 )
				continue;
			if( 0 == d_x )
			{
				tanC = d_y * 1000;
			}
			else
			{
				tanC = d_y / d_x;
			}
			if( (tanC < tanB) || (tanC > tanA ) )
			{
				continue;
			}
//			cur_r = FastSqrt(cur_r2);
//			int_r = (int)floor(cur_r + 0.5);
			int_r = (int)floor(FastSqrt(cur_r2) + 0.5);
			pProf[int_r] += ptr[x];
			++pWgt[int_r];
		}
		ptr = (_Tx*)(((LPBYTE)ptr) + stride);
	}
	return bRet;
}
*/
// End of addition by Peter on 08 Mar 2006
/****************************************************************************/
static BOOL	CalculateRDF( HWND hDlg, OBJ* O, LPEDRD S )
{
	int		i;
	double	as, am;
	double	a0, a1;
	LPDOUBLE lprdf = (LPDOUBLE)calloc( sizeof(double), MAXLEN );
	LPLONG	 lpwgt = (LPLONG)calloc( sizeof(double), MAXLEN );	// double since it'll be used in filter which is double
	BOOL bRet = FALSE, bCircle, bArc;

	if( (NULL == lprdf) || (NULL == lpwgt) )
		goto Error_Exit;
	S->np = 0;
	ClrBits(S->flags, RDFDONE | PRFDONE);
	memset(lprdf, 0, sizeof(double)*MAXLEN);
	memset(lpwgt, 0, sizeof(double)*MAXLEN);

	bCircle = IsDlgButtonChecked(hDlg, IDC_EDRD_SCIRC);
	bArc = IsDlgButtonChecked(hDlg, IDC_EDRD_SARC);
	if( (FALSE == bCircle) && (FALSE == bArc) )
	{
		CheckDlgButton(hDlg, IDC_EDRD_AUTO_CENTRE, 0);
		CheckDlgButton(hDlg, IDC_EDRD_MANUAL_CENTRE, 0);
		CheckDlgButton(hDlg, IDC_EDRD_ELLIPTICAL_DISTORTION, 0);
		CheckDlgButton(hDlg, IDC_EDRD_SARC, 0);
		CheckDlgButton(hDlg, IDC_EDRD_SCIRC, 1);
		bCircle = TRUE;
	}
	// Circular RDF
	if( TRUE == bCircle )
	{
		switch( S->image.npix )
		{
		case PIX_BYTE:
			if( !GetRDF_<BYTE>((LPBYTE)S->image.pData, S->image, S->xc, S->yc, S->r, lprdf, lpwgt) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_CHAR:
			if( !GetRDF_<CHAR>((LPSTR)S->image.pData, S->image, S->xc, S->yc, S->r, lprdf, lpwgt) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_WORD:
			if( !GetRDF_<WORD>((LPWORD)S->image.pData, S->image, S->xc, S->yc, S->r, lprdf, lpwgt) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_SHORT:
			if( !GetRDF_<SHORT>((SHORT*)S->image.pData, S->image, S->xc, S->yc, S->r, lprdf, lpwgt) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_DWORD:
			if( !GetRDF_<DWORD>((LPDWORD)S->image.pData, S->image, S->xc, S->yc, S->r, lprdf, lpwgt) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_LONG:
			if( !GetRDF_<LONG>((LPLONG)S->image.pData, S->image, S->xc, S->yc, S->r, lprdf, lpwgt) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_FLOAT:
			if( !GetRDF_<FLOAT>((LPFLOAT)S->image.pData, S->image, S->xc, S->yc, S->r, lprdf, lpwgt) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_DOUBLE:
			if( !GetRDF_<DOUBLE>((LPDOUBLE)S->image.pData, S->image, S->xc, S->yc, S->r, lprdf, lpwgt) )
			{
				goto Error_Exit;
			}
			break;
		default:
			goto Error_Exit;
		}
	}
	else if( TRUE == bArc )
	{
// Arc RDF
//		if (S->ae1<S->ae0) S->ae1+=2*M_PI;
		a0 = S->ae0;
		a1 = S->ae1;
		if( a1 <= a0 ) a1 += M_2PI;

		as = -(a0 + a1)*.5;				// Shift angle
		am = FastAbs(a1 - a0)*.5;		// Maximum angle

		while(am > M_2PI) am -= M_2PI;	// maximum must be <2pi

		while(as > M_PI)  as -= M_2PI;	// maximum must be <2pi
		while(as <-M_PI)  as += M_2PI;	// maximum must be <2pi

		switch( S->image.npix )
		{
		case PIX_BYTE:
		case PIX_CHAR:
			if( !GetRDFangl8( S->image.pData, S->image.w, S->image.h, S->image.neg,
				round(S->xc), round(S->yc), S->r, lprdf, lpwgt, as, am ) )
//			if( !GetArcRDF_<BYTE>((LPBYTE)S->lpImg, S->iw, S->ih, R4(S->iw), S->neg,
//				S->xc, S->yc, S->r, lprdf, lpwgt, as, am) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_WORD:
		case PIX_SHORT:
			if( !GetRDFangl16( S->image.pData, S->image.w, S->image.h, S->image.neg,
				round(S->xc), round(S->yc), S->r, lprdf, lpwgt, as, am ) )
//			if( !GetArcRDF_<WORD>((LPWORD)S->lpImg, S->iw, S->ih, R4(S->iw*2), S->neg,
//				S->xc, S->yc, S->r, lprdf, lpwgt, as, am) )
			{
				goto Error_Exit;
			}
			break;
		case PIX_DWORD:
//			if( !GetArcRDF_<DWORD>((LPDWORD)S->lpImg, S->iw, S->ih, R4(S->iw*2), S->neg,
//				S->xc, S->yc, S->r, lprdf, lpwgt, as, am) )
			{
				MessageBox(NULL, "Arc mode doesn't support DWORD pixel format", "EDRD ERROR!", MB_OK);
				goto Error_Exit;
			}
			break;
		case PIX_FLOAT:
//			if( !GetArcRDF_<float>((float*)S->lpImg, S->iw, S->ih, R4(S->iw*2), S->neg,
//				S->xc, S->yc, S->r, lprdf, lpwgt, as, am) )
			{
				MessageBox(NULL, "Arc mode doesn't support FLOAT pixel format", "EDRD ERROR!", MB_OK);
				goto Error_Exit;
			}
			break;
		default:
			MessageBox(NULL, "Arc mode - unsupported pixel format", "EDRD ERROR!", MB_OK);
			goto Error_Exit;
			break;
		}
	}
	// added by Peter on 31 mar 2006
	else
	{
		MessageBox(hDlg, "Select Circle or Arc mode", "WARNING", MB_OK);
		goto Error_Exit;
	}
	// and of addition by Peter on 31 mar 2006

	// divide by number of pixels in each circle
	if( 0 == lpwgt[0] )
	{
		lpwgt[0] = 1;
		lprdf[0] = 128;
	}
	for(i = 0, S->np = 0; i < S->r; i++)
	{
		if (lpwgt[i])
		{
			S->lpRDF[i] = (double)lprdf[i] /(double)lpwgt[i];
			S->np+= lpwgt[i];
		}
		else lprdf[i] = lprdf[i-1];
	}

	// low pass filter, commented by Peter on 31 mar 2006
	//	LowPass( S->lpRDF, (LPDOUBLE)lpwgt, S->r );

	// subtract the background
	if( IsDlgButtonChecked( hDlg, IDC_EDRD_SUBBKG) )
	{
		SubtractBkg(S->lpRDF, (LPDOUBLE)lpwgt, S->r);
	}

	// find minimum and maximum
	S->rdmin = 1e30;
	S->rdmax = -1e30;
	for(i=0;i<S->r;i++)
	{
		if( S->lpRDF[i] > S->rdmax ) S->rdmax = S->lpRDF[i];
		if( S->lpRDF[i] < S->rdmin ) S->rdmin = S->lpRDF[i];
	}
	if (IsDlgButtonChecked( hDlg, IDC_EDRD_SUBBKG) )
	{
		for(i=0;i<S->r;i++) S->lpRDF[i] -= S->rdmin;
		S->rdmax -= S->rdmin; S->rdmin = 0;
	}
	if( S->rdmax - S->rdmin < 1. )
		S->rdmax = S->rdmin+2;
	SetBits(S->flags, RDFDONE);
	bRet = TRUE;
Error_Exit:
	if( NULL != lpwgt )
	{
		free(lpwgt);
	}
	if( NULL != lprdf )
	{
		free(lprdf);
	}
	return bRet;

}
/****************************************************************************/
static BOOL	CalculateNProf( HWND hDlg, OBJ* O, LPEDRD S )
{
	int		i, j;
	double	am, as;
	double	a0, a1;
	LPDOUBLE lprdf = (LPDOUBLE)calloc(sizeof(double),MAXLEN);
	LPLONG	 lpwgt = (LPLONG)calloc(sizeof(double),MAXLEN);
	int	na, np, len;
	double	x2, y2;
	BOOL bRet = FALSE;

	if (lprdf==NULL || lpwgt==NULL) return FALSE;
	S->np=0;
	ClrBits(S->flags, RDFDONE | PRFDONE);
// Set of profiles
	np = GetDlgItemInt( hDlg, IDC_EDRD_NBINS, &i, FALSE );
	if (np<1) np=1; if(np>NPROF) np=NPROF;
	S->profcnt=np; SetDlgItemInt( hDlg, IDC_EDRD_NBINS, np, FALSE );
	if (IsDlgButtonChecked(hDlg,IDC_EDRD_SCIRC))
	{
		a0 = 0.; a1 = 2*M_PI;
	}
	else
	{
		a0 = S->ae0; a1 = S->ae1;
	}
	if( a1 <= a0 ) a1 += M_2PI;
	am = FastAbs(a1-a0)/(double)np;
	len = S->r;
	as = 1./((double)len+4);	// Angular step
	na = round(am/as);
	if (na<2) na=2;

	for(i = 0; i < np; i++)		// All required profiles
	{
		memset( lprdf, 0, sizeof(LONG)*MAXLEN);
		memset( lpwgt, 0, sizeof(LONG)*MAXLEN);
		for(j=0;j<na;j++)
		{
			x2 = S->xc + (double)len*FastCos(a0+am*i+as*j);
			y2 = S->yc + (double)len*FastSin(a0+am*i+as*j);
			switch( S->image.npix )
			{
			case PIX_BYTE:
				im8GetProfile((LPBYTE)S->image.pData, S->image.stride, S->image.w, S->image.h,
					S->image.neg, S->xc, S->yc, x2,y2, len, lprdf, lpwgt);
				break;
			case PIX_CHAR:
				im8sGetProfile((LPSTR)S->image.pData, S->image.stride, S->image.w, S->image.h,
					S->image.neg, S->xc, S->yc, x2,y2, len, lprdf, lpwgt);
				break;
			case PIX_WORD:
				im16GetProfile((LPWORD)S->image.pData, S->image.stride, S->image.w, S->image.h,
					S->image.neg, S->xc, S->yc, x2,y2, len, lprdf, lpwgt);
				break;
			case PIX_SHORT:
				im16sGetProfile((SHORT*)S->image.pData, S->image.stride, S->image.w, S->image.h,
					S->image.neg, S->xc, S->yc, x2,y2, len, lprdf, lpwgt);
				break;
			case PIX_DWORD:
				im32GetProfile((LPDWORD)S->image.pData, S->image.stride, S->image.w, S->image.h,
					S->image.neg, S->xc, S->yc, x2,y2, len, lprdf, lpwgt);
				break;
			case PIX_LONG:
				im32sGetProfile((LONG*)S->image.pData, S->image.stride, S->image.w, S->image.h,
					S->image.neg, S->xc, S->yc, x2,y2, len, lprdf, lpwgt);
				break;
			case PIX_FLOAT:
				imFGetProfile((LPFLOAT)S->image.pData, S->image.stride, S->image.w, S->image.h,
					S->image.neg, S->xc, S->yc, x2,y2, len, lprdf, lpwgt);
				break;
			case PIX_DOUBLE:
				imDGetProfile((LPDOUBLE)S->image.pData, S->image.stride, S->image.w, S->image.h,
					S->image.neg, S->xc, S->yc, x2,y2, len, lprdf, lpwgt);
				break;
			default:
				MessageBox(NULL, "NProf mode - unsupported pixel format", "EDRD ERROR!", MB_OK);
				goto Error_Exit;
			}
		}
		for(j=0;j<len;j++)
		{
			if (lpwgt[j]) S->lpProf[i][j] = (double)lprdf[j] / (double)lpwgt[j];
			else          S->lpProf[i][j]=128.;
		}
// low pass filter
		LowPass( S->lpProf[i], (LPDOUBLE) lpwgt, len );
// subtrate background
		if ( IsDlgButtonChecked( hDlg, IDC_EDRD_SUBBKG) )
		{
			SubtractBkg(S->lpProf[i], (LPDOUBLE) lpwgt, len);
		}
	}
//	free(lprdf);
//	free(lpwgt);
// find minimum and maximum, and calculating average profile
	memset( S->lpRDF, 0, sizeof(long)*MAXLEN);
	S->rdmin = 1e10; S->rdmax = -1e10;
	for(j=0;j<np;j++)
	{
		for(i=0;i<S->r;i++)
		{
			if (S->lpProf[j][i] > S->rdmax) S->rdmax = S->lpProf[j][i];
			if (S->lpProf[j][i] < S->rdmin) S->rdmin = S->lpProf[j][i];
			S->lpRDF[i] += S->lpProf[j][i];
		}
	}
	for(i=0;i<S->r;i++) S->lpRDF[i] /= (double)np;
	if (IsDlgButtonChecked( hDlg, IDC_EDRD_SUBBKG) ) {
		for(j=0;j<np;j++) for(i=0;i<S->r;i++) S->lpProf[j][i] -= S->rdmin;
		for(i=0;i<S->r;i++) S->lpRDF[i] -= S->rdmin;
		S->rdmax -= S->rdmin; S->rdmin = 0;
	}
	if (S->rdmax-S->rdmin<1.) S->rdmax = S->rdmin+2;

	SetBits(S->flags, PRFDONE);
	bRet = TRUE;

Error_Exit:
	if( NULL != lprdf )
	{
		free(lprdf);
	}
	if( NULL != lpwgt )
	{
		free(lpwgt);
	}
	return bRet;

}
/****************************************************************************/
static VOID SaveRingList( HWND hDlg , OBJ* O, LPEDRD S)
{
	HWND	hw = GetDlgItem(hDlg, IDC_EDRD_LIST);
	int		i = 0;
	FILE	*out;

	_splitpath( O->fname, NULL, NULL, TMP, NULL );	// take file name only
	if( !fioGetFileNameDialog( MainhWnd, "Save ring list", &ofNames[OFN_LIST], TMP, TRUE) )
		return;

	if( NULL == (out = fopen(fioFileName(), "w")) )
		return;
	while (SendMessage(hw,CB_GETLBTEXT, i++, (LPARAM)(LPSTR)TMP) != CB_ERR)
		fprintf(out, "%s\n", TMP);
	fclose( out );

}
/****************************************************************************/
#define	LIMIT	20.	// from 10 Angstrom
#define	STEPS	200
#define	RDBEGIN	5	// start from point 5
static BOOL SaveRDFgrapher( HWND hDlg , OBJ* O, LPEDRD S)
{
	HWND	hw = GetDlgItem(hDlg, IDC_EDRD_LIST);
	int		i=0, indx;
	char	fname[MAX_PATH];
	FILE	*pOut, *pIn;
	double	minr,maxr, rad, val, scale;
	LPSTR	bimem;
	BOOL	bRet = FALSE;

	if( 0 == (S->flags & RDFDONE) )
	{
		MessageBox(MainhWnd, "There is nothing to save", "ED Measurements", MB_ICONSTOP);
		return bRet;
	}
	// commented by Peter on 23 Feb 2006
//	if (S->bCalibrated==0) {
//		MessageBox(MainhWnd, "Cannot save uncalibrated RDF into GRF file", "ED Measurements", MB_ICONSTOP);
//		return FALSE;
//	}
	// end of comment by Peter on 23 Feb 2006

	_splitpath( O->title, NULL, NULL, TMP, NULL );	// take file name only
	strcat(TMP, ".rdf");
	// Added by Peter on 23 Feb 2006
	if( 0 == S->bCalibrated )
	{
		if( !fioGetFileNameDialog(MainhWnd, "Save text list file",
			&ofNames[OFN_LIST], TMP, TRUE) )
		{
			return bRet;
		}
		if( NULL == (pOut = fopen(fioFileName(), "wt")) )
		{
			MessageBox(MainhWnd, "Cannot open output file",
				"ED Measurements", MB_ICONSTOP);
			return bRet;
		}
		for(i = 0; i < S->r - 1; i++)
		{
			fprintf(pOut, "%d\t%.5f\n", i, S->lpRDF[i]);
		}
		if( NULL != pOut )
		{
			fclose(pOut);
		}
	}
	else
	{
		if( !fioGetFileNameDialog(MainhWnd, "Save CERIUS Grapher file",
			&ofNames[OFN_CERIUS], TMP, TRUE) )
		{
			return bRet;
		}

		char grapherTemplate[] = "CERIUS Grapher File\n\
! Start of definition for XY graph RDF v3\n\
>PLOT XY DATA: \"RDF/RDF\" 3\n\
%s\n\
>PLOT XY METHOD: \"RDF/RDF\" 3\n\
 COLOR   DGR\n\
 STYLE   HISTOGRAM\n\
 YOFFSET   0.0000000E+00\n\
 LABEL   RDF\n\
>GRAPH XY METHOD: \"RDF\" 3\n\
 SCALE   FIXED\n\
 XMIN    %.5f\n\
 XMAX    %.5f\n\
 YMIN    %.5f\n\
 YMAX    %.5f\n\
 PLOT    \"RDF/RDF\" 3\n\
 SCALES  ON\n\
 KEY     ON\n\
 TITLE              4\n\
 RDF\n\
 Atomic RDF\n\
 Total function\n\
 rG(r)\n\
 TITLES  ON\n\
 XLABEL  r (Angstrom)\n\
 LABELS  ON\n\
! End of definition for XY graph RDF v3\n\
>GALLERY METHOD:\n\
 XLAYOUT AUTO\n\
 YLAYOUT AUTO\n\
 GRAPH   \"RDF\" 3\n\
>END\n\
";


		bimem = (LPSTR)calloc(S->r, 80);
		if( NULL == bimem )
		{
			goto Error_Exit_CERIUS;
//			return FALSE;
		}

		scale = O->NI.Scale * 1e10;
		for(i = S->r-1, indx = 0; i > RDBEGIN; i--)
		{
			if (indx) sprintf(&bimem[strlen(bimem)],"\n");
			indx = 1;
			rad = scale/i;
			val = S->lpRDF[i];
			sprintf(&bimem[strlen(bimem)]," %.5f     %.5f", rad, val);
			if (rad>LIMIT) break;
		}
		minr = scale/(S->r-1.);
		maxr = rad;

		if( NULL == (pOut = fopen( fioFileName(), "w")) )
		{
			goto Error_Exit_CERIUS;
//			free(bimem);
//			return FALSE;
		}
		fprintf(pOut, grapherTemplate, bimem, minr, maxr, S->rdmin, S->rdmax);
Error_Exit_CERIUS:
		if( NULL != bimem )
		{
			free(bimem);
		}
		if( NULL != pOut )
		{
			fclose( pOut );
		}
	}
	return bRet;
	// End of addition by Peter on 23 Feb 2006
	// Commented by Peter on 23 Feb 2006
/*
	if (!fioGetFileNameDialog( MainhWnd, "Save CERIUS Grapher file", &ofNames[OFN_CERIUS], TMP, TRUE)) return FALSE;

	GetModuleFileName( hInst, fname, sizeof(fname));
	strcpy(strrchr(fname,'\\'), "\\blank.rdf");

	if ((in=fopen( fname, "r")) == NULL)
	{
		MessageBox(MainhWnd, "Cannot open 'blank.rdf' file",
					"ED Measurements", MB_ICONSTOP);
		return FALSE;
	}

	memset( TMP, 0, sizeof(TMP));
	i = fread( TMP, 1, sizeof(TMP), in );
	TMP[i]=0;
	fclose( in );

	bimem = (LPSTR)calloc(S->r, 80);
	if (bimem==NULL) {
		return FALSE;
	}

	scale = O->NI.Scale * 1e10;
	for(i=S->r-1, indx=0;i>RDBEGIN;i--) {
		if (indx) sprintf(&bimem[strlen(bimem)],"\n");
		indx = 1;
		rad = scale/i;
		val = S->lpRDF[i];
		sprintf(&bimem[strlen(bimem)]," %.5f     %.5f", rad, val);
		if (rad>LIMIT) break;
	}
	minr = scale/(S->r-1.);
	maxr = rad;

	if ((out = fopen( fioFileName(), "w")) == NULL) {
		free(bimem);
		return FALSE;
	}
	fprintf(out, TMP, bimem, minr, maxr, S->rdmin, S->rdmax);
	fclose( out );
	free(bimem);
	return FALSE;
*/
	// End of comment by Peter on 23 Feb 2006

}
/****************************************************************************/
static VOID ClearRingList( HWND hDlg , OBJ* O, LPEDRD S)
{
	ClearProtocol();
	AddProtocolString(";");
	sprintf(TMP,"; File : %s ring/arc pattern analysis", O->fname);
	AddProtocolString(TMP);
	SendDlgItemMessage( hDlg, IDC_EDRD_LIST, CB_SETCURSEL, 1, 0);
	S->arccnt=0;
	InvalidateRect( O->ParentWnd, NULL, FALSE);

}
/****************************************************************************/
// Added by Peter on 23 Mar 2006
static VOID AddRingListHeader(HWND hDlg, OBJ* pObj, LPEDRD pEDRD)
{
	AddProtocolString(";");
	if( IsDlgButtonChecked(hDlg,IDC_EDRD_SARC) )
	{
		int a0, a1, da;
		a0 = round(R2D(pEDRD->ae0));
		a1 = round(R2D(pEDRD->ae1));
		da = a1 - a0;
		if( da < 0 )
			da += 360;
		sprintf(TMP,"; Arc[%d°,%d°]=(%d°)", a0, a1, da);
		AddProtocolString(TMP);
	}
	AddProtocolString(";");
	// numbering  01234567890123456789012345678901234567
	sprintf(TMP, "; Peak  Radius  d-value   Width   Width    Int.    Scaled");
	AddProtocolString(TMP);
	sprintf(TMP, ";number pixels    (Å)     pixels   (Å)             intens");
	AddProtocolString(TMP);
	SendDlgItemMessage(hDlg, IDC_EDRD_LIST, CB_SETCURSEL, 1, 0);

}
/****************************************************************************/
static double FindAmplScale(EDRD_REFL *pData, int nRefls)
{
	double dScale = 1.0;
	double dMax = 0.0;

	for(int i = 0; i < nRefls; i++)
	{
		if( pData[i].dIntens > dMax )
		{
			dMax = pData[i].dIntens;
		}
	}
	if( dMax > 0.0 )
	{
		dScale = 10000.0 / dMax;
	}

	return dScale;

}
// End of the addition by Peter on 23 Mar 2006
/****************************************************************************/
static void AddRingToList( HWND hDlg, OBJ* O, LPEDRD S, int n )
{
	LPARC	lpa = S->lpArc + n/*S->arccnt*/;
	double	width, radius, dScale, d_value;

	d_value = 0;
	radius = lpa->rad;
	if( S->bCalibrated && (lpa->rad > 0.1) )
	{
		dScale = O->NI.Scale * 1e10;
		d_value = dScale / lpa->rad;
		width = FastAbs(dScale * (1.0 / lpa->rad - 1.0 / (lpa->rad + lpa->wid)));
		sprintf(TMP, "%4d%c %6d  %8.4f %8.2f%8.2f  %6d  %8d",
			n+1, (lpa->flg) ? ' ' : '*', (int)radius, d_value, lpa->wid, width,
			(int)lpa->intens, (int)lpa->io); // Å
		// commented by Peter on 27 Mar 2006
//		sprintf( TMP, "Ring %2d%c, R=%6.3fÅ W=%6.3fÅ Io=%5.1f Ang=%.3g° at %.3g°",
//			n+1, lpa->flg?' ':'*', O->NI.Scale * 1e10/((double)lpa->rad), wid, lpa->io,
//			R2D(lpa->a1-lpa->a0), R2D(lpa->a0)); // Å
	}
	else
	{
		width = lpa->wid;
		sprintf(TMP, "%4d%c %6d          %8.2f           %6d  %8d",
			n+1, (lpa->flg) ? ' ' : '*', (int)radius, width,
			(int)lpa->intens, (int)lpa->io); // Å
		// commented by Peter on 27 Mar 2006
//		sprintf( TMP, "Ring %2d%c, R=%.1f W=%.1f Io=%5.1f Ang=%.3g° at %.3g°",
//			n+1, lpa->flg?' ':'*', lpa->rad, lpa->wid, lpa->io,
//			R2D(lpa->a1-lpa->a0),  R2D(lpa->a0));
	}
	AddProtocolString(TMP);
	SendDlgItemMessage( hDlg, IDC_EDRD_LIST, CB_SETCURSEL, S->arccnt+2, 0);	// +2 due to the header

}
/****************************************************************************/
static void AddProfileToList( HWND hDlg, OBJ* O, LPEDRD S, int n, int prf )
{
	LPARC	lpa = S->lpArc+S->arccnt;
	double	width, radius, dScale, d_value;

	d_value = 0;
	radius = lpa->rad;
	if( S->bCalibrated && (lpa->rad > 0.1) )
	{
		dScale = O->NI.Scale * 1e10;
		d_value = dScale / lpa->rad;
		width = FastAbs(dScale *(1.0 / lpa->rad - 1.0 /(lpa->rad + lpa->wid)));
		// commented by Peter on 27 Mar 2006
		sprintf(TMP, "%4d%c %6d  %8.4f  %8.5f %6d %8.2f Ang=%.3g° at %.3g°",
			n+1, (lpa->flg) ? ' ' : '*', (int)radius, d_value, width, (int)lpa->io, lpa->intens,
			R2D(lpa->a1-lpa->a0), R2D(lpa->a0)); // Å
//		sprintf( TMP, "Ring %2d:%d%c, R=%6.3fÅ W=%6.3fÅ Io=%5.1f Ang=%.3g° at %.3g°",
//					 n+1, prf+1, lpa->flg?' ':'*', O->NI.Scale * 1e10/((double)lpa->rad), wid, lpa->io,
//					 R2D(lpa->a1-lpa->a0), R2D(lpa->a0)); // Å
	}
	else
	{
		width = lpa->wid;
		sprintf(TMP, "%4d%c %6d            %8.5f %6d %8.2f Ang=%.3g° at %.3g°",
			n+1, (lpa->flg) ? ' ' : '*', (int)radius, d_value, width, (int)lpa->io, lpa->intens,
			R2D(lpa->a1-lpa->a0), R2D(lpa->a0)); // Å
		// commented by Peter on 27 Mar 2006
//		sprintf( TMP, "Ring %2d:%d%c, R=%.1f W=%.1f Io=%5.1f Ang=%.3g° at %.3g°",
//					 n+1, prf+1, lpa->flg?' ':'*', lpa->rad, lpa->wid, lpa->io,
//					 R2D(lpa->a1-lpa->a0),  R2D(lpa->a0));
	}
	AddProtocolString(TMP);
	SendDlgItemMessage( hDlg, IDC_EDRD_LIST, CB_SETCURSEL, S->arccnt+2, 0);	// +2 due to the header

}
/****************************************************************************/
static VOID AddRing( HWND hDlg , OBJ* O, LPEDRD S)
{
	LPARC	lpa = S->lpArc+S->arccnt;

	lpa->rad = S->r;
	lpa->a0 = 0.; lpa->a1 = 2*M_PI;
	lpa->wid = 1.;
	if (S->flags & RDFDONE)
	{
		lpa->io = S->lpRDF[S->r-1];
	}
	else
	{
		lpa->io = 0.;
	}
	AddRingToList( hDlg, O, S, S->arccnt );
	S->arccnt++;

}
/****************************************************************************/
static double Prop( double a, double b, double v )
{
	if((a<v && v<b) || (a>v && v>b))
	{
		if(fabs((v-a)/(b-a))>1.) MessageBeep( -1 );
		return(v-a)/(b-a);
	} else return 0.;

}
/****************************************************************************/
#define	AW	3	// width for CG
static BOOL FindRing( LPARC lpa, LPDOUBLE rdf, LPDOUBLE rad, int maxr, double athr, BOOL bRefine)
{
	double	fa, fr, bg, tval;
	int		r0,r1,rp,i;
	double	wl,wr;
	int		r = round(*rad);

	if (bRefine)
	{
		i=0;
		do
		{
			rp = r;
			r0 = r-AW;
			r1 = r+AW;
			if(r0 < 0)
				r0=0;
			if(r1 >= maxr)
				r1 = maxr-1;
			for(r=r0,bg=1e10;r<=r1;r++)
			{
				if (rdf[r] < bg)
					bg=rdf[r];
			}
	    	for(r=r0,fr=0.,fa=0.;r<=r1;r++)
			{
				fr += rdf[r]-bg;
				fa += (double)r * (rdf[r]-bg);
			}
			fa /= fr;
			r = round(fa);
			i++;
		} while (rp!=r && i<100);	// Iterate while r is moving

		*rad = fa;
	}
	lpa->rad = *rad;
	r = round(*rad);

	wl = wr = 0.;
	tval = rdf[r]*athr;
	if (rdf[r]>.1) for(i=1; i<r; i++)
	{
		if (wl<=0 && r-i-1>1)
		{
			if (rdf[r-i]<tval)  wl = i+Prop(rdf[r-i],rdf[r-i+1],tval);
			if (rdf[r-i-1]-rdf[r-i]>rdf[r]/10.) wl = -i;
		}
		if (wr<=0 && r+i+1<maxr-1)
		{
			if (rdf[r+i]<tval) wr = i-Prop(rdf[r+i],rdf[r+i-1],tval);
			if (rdf[r+i+1]-rdf[r+i]>rdf[r]/10.) wr = -i;
		}
		if (wl*wr) break;
	}
	if (wl>0 && wr>0)		lpa->wid = (wl+wr)*.5;
	else if (wl>0)			lpa->wid = wl;
	else if (wr>0)			lpa->wid = wr;
	else if (wl<0 && wr<0)  lpa->wid = -(wl+wr)*.5;
	else if (wl)			lpa->wid = -wl;
	else if (wr)			lpa->wid = -wr;
	else 					lpa->wid=1;

	if (wl<=0 || wr<=0) lpa->flg=FALSE;
	else			    lpa->flg=TRUE;

	wl=fabs(wl);
	wr=fabs(wr);
	for(fa = 0, fr = 0, i = r-round(wl); i <= r + round(wr); i++)
	{
		fa+=rdf[i];
		fr++;
	}
	lpa->io = fa/fr;
	return lpa->flg;

}
/****************************************************************************/
static VOID AddRingFromRDF( HWND hDlg , OBJ* O, LPEDRD S, int r)
{
	LPARC	lpa = S->lpArc+S->arccnt;
	LPDOUBLE	rdf = S->lpRDF;
	double	athr, rad, a0, a1, mr;
	int		i,j;
	int		acnt, pcnt;

	if (!GetDlgItemDouble(hDlg, IDC_EDRD_FWHM, &athr, 0))
	{
		athr = ATHR;
	}
	if(athr<0.1) athr=0.1;
	if(athr>0.9) athr=0.9;
	SetDlgItemDouble( hDlg, IDC_EDRD_FWHM, athr, 0);

	if (r>=S->r-AW) r = S->r-AW-1;
	if (r<AW) r=AW;

	if (IsDlgButtonChecked(hDlg,IDC_EDRD_SARC))
	{
		a0 = S->ae0;
		a1 = S->ae1;
		if (a1<=a0)
			a1+= 2*M_PI;
	}
	else
	{
		a0 = 0.;
		a1 = 2*M_PI;
	}
	rad = r;

	if (S->arccnt>=MAXARC)
	{
		MessageBox(MainhWnd, "The maximum number of measurements already has been done", "ED Measurements", MB_ICONINFORMATION);
		return;
	}
	if ( !IsDlgButtonChecked( hDlg, IDC_EDRD_NPROF))
	{
		if ((S->flags & RDFDONE)==0)
		{
			MessageBox(MainhWnd, "There is no RDF function", "ED Measurements", MB_ICONINFORMATION);
			return;
		}
		lpa->a0 = a0; lpa->a1 = a1;
		FindRing( lpa, rdf, &rad, S->r, athr, TRUE);
		for(i=0;i<S->arccnt;i++) if (fabs(r-S->lpArc[i].rad)<AW) {
			if (MessageBox(MainhWnd, "The ring with the same radius already exist.\nAdd anyway?",
						"ED Measurements", MB_ICONINFORMATION | MB_YESNO) == IDNO) return;
			break;
		}

		AddRingToList( hDlg, O, S, S->arccnt );
		S->arccnt++;
	}
	else
	{
		if ((S->flags & PRFDONE)==0)
		{
			MessageBox(MainhWnd, "There is no set of profiles",
						"ED Measurements", MB_ICONINFORMATION);
			return;
		}

// Looking for profile with maximum value at position [r]
		for(i=0,mr=0,j=-1;i<S->profcnt;i++)
		{
			if(S->lpProf[i][r]>mr)
			{
				mr=S->lpProf[i][r];
				j=i;
			}
		}
		if(j<0)
		{
			::OutputDebugString("AddRingFromRDF() -> Bad number\n");
			MessageBeep(-1);
			return;
		}
// refining radius at this profile
		FindRing( lpa, S->lpProf[j], &rad, S->r, athr, TRUE);

		a1 = (a1-a0)/(double)S->profcnt;
		acnt = S->arccnt?S->lpArc[S->arccnt-1].arcno+1:0;
		for(i=0,pcnt=0;i<S->profcnt;i++)
		{
			lpa = S->lpArc+S->arccnt;
			lpa->a0 = a0+a1*i; lpa->a1 = a0+a1*(i+1);
			if ( FindRing( lpa, S->lpProf[i], &rad, S->r, athr, FALSE) )
			{
				lpa->arcno=acnt; lpa->prfno=pcnt;
				AddProfileToList( hDlg, O, S, acnt, pcnt );
				S->arccnt++; pcnt++;
				if (S->arccnt>=MAXARC) break;
			}
		}
		if (pcnt!=S->profcnt)
		{
			sprintf(TMP,"Warning, from %d profiles only %d was estimated", S->profcnt, pcnt);
			MessageBox(MainhWnd, TMP, "ED Measurements", MB_ICONINFORMATION);
		}
	}
	InvalidateRect( O->ParentWnd, NULL, FALSE);

}
#undef AW
/****************************************************************************/
static void PrintInfo( HWND hDlg, OBJ* O, LPEDRD S, LPARAM lP, BOOL fAdd )
{
	POINT	p = {LOWORD(lP), HIWORD(lP)};
	HWND	hw;
	RECT	r;
	int		xs, ys, x, io;
	int		xsl, ysl;

	if( 0 == (S->flags & (RDFDONE|PRFDONE)) )
		return;

	hw = GetDlgItem(hDlg, IDC_EDRD_PLOT);
	if( FALSE == IsWindow(hw) )
	{
		return;
	}
	GetWindowRect( hw, &r );
	MapWindowPoints(hDlg, hw, &p, 1);
	xs = r.right - r.left;
	ys = r.bottom - r.top;
    xsl = xs - MRS*2;
	ysl = ys - MRS*2;

	p.x =	   p.x - MRS;
	p.y = ys - p.y - MRS;
	if( (p.x<0) || (p.x>=xsl) || (p.y<0) || (p.y>=ysl) )
		return;

	x = p.x*S->r/xsl;

	io = round(S->lpRDF[x]);

	if (S->bCalibrated && S->r!=0)
	{
		if (x==0) x=1;
		sprintf( TMP, "Rad=%-2.3fÅ, Io=%d", O->NI.Scale * 1e10/((double)x), io); // Å
	}
	else
		sprintf( TMP, "Rad=%dpix, Io=%d", x, io);

	SetDlgItemText( hDlg, IDC_EDRD_CREFRAD, TMP);

	if (fAdd)
		AddRingFromRDF( hDlg, O, S, x );

}
/****************************************************************************/
int		pxcmp(const void * x1, const void * x2)
{
	LPPOINT P1 = (LPPOINT)x1, P2 = (LPPOINT)x2;

	if(P1->x  > P2->x)  return( 1);
	if(P1->x  < P2->x)  return(-1);
	return(0);
}
/****************************************************************************/
int		pycmp(const void * x1, const void * x2)
{
	LPPOINT P1 = (LPPOINT)x1, P2 = (LPPOINT)x2;

	if(P1->y  > P2->y)  return( 1);
	if(P1->y  < P2->y)  return(-1);
	return(0);
}
/****************************************************************************/
static _inline double getr2( int x0, int y0, POINT a )
{
	return (double)(a.x-x0)*(double)(a.x-x0)+(double)(a.y-y0)*(double)(a.y-y0);
}
/****************************************************************************/
// Commented by Peter on 2 May 2008
/*
static BOOL IsNegative( OBJ* pObj )
{
	vector<long> vHisto(65536, 0);
//	LPLONG	his = (LPLONG)calloc( sizeof(long), 65536 );
	double as, an, an0, mean;
	int		min, max, k, l, nSize;
	BOOL	ret = FALSE;

	if( (NULL == pObj) || (true == vHisto.empty()) )
		return FALSE;

	LPLONG	his = &*vHisto.begin();
	an = 0;
	as = 0;
	an0 = round(0.005*(double)pObj->x * (double)pObj->y);
	if( an0 < 1 ) an0 = 1;
	// TODO: finish this
//	PIXEL_FORMAT pix = PIX_BYTE;
//	if( 2 == pObj->pix )
//	{
//		pix = PIX_WORD;
//	}
//	else
//	{
//		MessageBox(NULL, "IsNegative - unsupported pixel format", "EDRD ERROR!", MB_OK);
//	}
	switch( pObj->npix )
	{
	case PIX_BYTE:
		im8GetHisto(his, (LPBYTE)pObj->dp, pObj->xb, 8, 8, pObj->x-16, pObj->y-16, FALSE);
		nSize = 256;
		break;
//		for(i = 0; i < nSize; i++)
//		{
//			if( an < an0) min = i;
//			as += (double)i*(double)his[i];
//			an += (double)his[i];
//		}
//		mean = round(as / an);
//		an = 0;
//		for(i = nSize - 1; i > 0; i--)
//		{
//			if( an < an0 ) max = i;
//			an += (double)his[i];
//		}
//		k = (min/2 + max/2);
//		for(an = 0, l = min; l < k; l++)
//			an+=(double)his[l];
//		for(as = 0, l = k; l < max; l++)
//			as+=(double)his[l];
//		ret = ( as < an ) ? FALSE : TRUE;
	case PIX_WORD:
		im16GetHisto(his, (LPWORD)pObj->dp, pObj->xb, 8, 8, pObj->x-16, pObj->y-16, FALSE);
		nSize = 65536;
		break;
//		an = 0; as = 0; an0 = round(0.005*(double)pObj->x * (double)pObj->y);
//		if (an0<1) an0=1;
//		for(l = 0; l < nSize; l++)
//		{
//			if( an < an0 ) min = (WORD)l;
//			as + =(double)l*(double)his[l];
//			an + =(double)his[l];
//		}
//		mean = round(as / an);
//		an = 0;
//		for(l = nSize - 1; l > 0; l--)
//		{
//			if( an < an0 ) max = l;
//			an += (double)his[l];
//		}
//		k = (min/2 + max/2);
//		for(an = 0, l = min; l < k; l++)
//			an+=(double)his[l];
//		for(as = 0, l = k; l < max; l++)
//			as+=(double)his[l];
//		ret = ( as < an ) ? FALSE : TRUE;
	case PIX_DWORD:
		nSize = 65536;
		MessageBox(NULL, "IsNegative - doesn't support DWORD pixel format", "EDRD ERROR!", MB_OK);
		return FALSE;
		break;
	case PIX_FLOAT:
		nSize = 65536;
		MessageBox(NULL, "IsNegative - doesn't support FLOAT pixel format", "EDRD ERROR!", MB_OK);
		return FALSE;
		break;
	default:
		MessageBox(NULL, "IsNegative - unsupported pixel format", "EDRD ERROR!", MB_OK);
		return FALSE;
		break;
	}
	for(l = 0; l < nSize; l++)
	{
		if( an < an0 ) min = (WORD)l;
		as += (double)l*(double)his[l];
		an += (double)his[l];
	}
	mean = round(as / an);
	an = 0;
	for(l = nSize - 1; l > 0; l--)
	{
		if( an < an0 ) max = l;
		an += (double)his[l];
	}
	k = (min/2 + max/2);
	for(an = 0, l = min; l < k; l++)
		an += (double)his[l];
	for(as = 0, l = k; l < max; l++)
		as += (double)his[l];
	ret = ( as < an ) ? FALSE : TRUE;
//	free( his );
	return ret;

}
*/
// end of comment by Peter on 2 May 2008
/****************************************************************************/
static BOOL	InitEDRDDialog( HWND hDlg , OBJ* O, LPEDRD S)
{
	OBJ*	pr  = OBJ::GetOBJ(O->ParentWnd);
	int		thr;
	int		i;
	LPBYTE	lpData;

//	CopyObj ( O, pr );							// Make a copy of parent Obj

// data memory
	thr = MAXLEN*sizeof(double)			// RDF
		+ MAXARC*sizeof(ARC)			// ARC data
		+ NPROF*sizeof(double)*MAXLEN;	// Profiles data
	O->bp = (LPBYTE)calloc(thr,1);
	if ( ! O->bp ) return FALSE;

	lpData = O->bp;

	S->r = 10;		// Minimum radius
	S->xc = O->x/2;
	S->yc = O->y/2;
//	S->image.pData = pr->dp;
//	S->image.w = ((pr->x + 3)&0xFFFC);
//	S->image.h = pr->y;
//	S->image.npix = pr->pix;
	S->image.SetData(pr->dp, ((pr->x + 3)&0xFFFC), pr->y, pr->npix);
	S->flags=0;
	S->lpRDF = (LPDOUBLE)lpData;
	S->lpArc = (LPARC)(lpData + MAXLEN*sizeof(double));
	S->_handle = NULL;
	S->_id = NULL;
	lpData = (LPBYTE) S->lpArc;
	lpData += MAXARC*sizeof(ARC);

	S->ae0 = S->ae1 = 0.0;

	for(i = 0; i < NPROF; i++)
		S->lpProf[i]=(LPDOUBLE)(lpData+sizeof(double)*MAXLEN*i);

	SetNegative(pr, S->image, hDlg);
/*
	S->image.neg = 0;
	if( IsNegative(pr) )
	{
		switch( S->image.npix )
		{
		case PIX_BYTE:
			S->image.neg = 0xFF;
			break;
		case PIX_WORD:
			S->image.neg = 0xFFFF;
			break;
		case PIX_DWORD:
			S->image.neg = 0xFFFFFF;
			break;
		case PIX_FLOAT:
			S->image.neg = 0xFFFFFF;
			break;
		default:
			MessageBox(NULL, "EDRD initialization - unsupported pixel format", "EDRD ERROR!", MB_OK);
			goto Error_Exit;
			break;
		}
//		if (S->pix==1) S->neg=0xFF;
//		if (S->pix==2) S->neg=0xFFFF;
	}

	CheckDlgButton( hDlg, IDC_EDRD_NEGAT, S->image.neg );
*/
//	CheckDlgButton( hDlg, IDC_EDRD_MOVECENTRE, BST_CHECKED );
	CheckDlgButton( hDlg, IDC_EDRD_AUTO_CENTRE, BST_CHECKED );

	// Commented by Peter on 07 Mar 2006
//	memset(S->p4, 0, sizeof(S->p4));
//	S->p4cnt = 0;
	// End of comments by Peter on 07 Mar 2006

//	SendDlgItemMessage(hDlg, IDC_EDRD_CENTER,  WM_SETFONT, (WPARAM)hfMedium, 0);
//	SendDlgItemMessage(hDlg, IDC_EDRD_CREFRAD, WM_SETFONT, (WPARAM)hfMedium, 0);
#ifdef USE_FONT_MANAGER
	SendDlgItemMessage(hDlg, IDC_EDRD_LIST,    WM_SETFONT, (WPARAM)FontManager::GetFont(FM_MEDIUM_FONT), 0);
#else
	SendDlgItemMessage(hDlg, IDC_EDRD_LIST,    WM_SETFONT, (WPARAM)hfMedium, 0);
#endif
	SetDlgItemDouble( hDlg, IDC_EDRD_FWHM, ATHR, 0 );
	SetDlgItemInt  ( hDlg, IDC_EDRD_NBINS, NPROF, FALSE );
	CheckDlgButton( hDlg, IDC_EDRD_NEGAT, S->image.neg );
//	CheckDlgButton( hDlg, IDC_EDRD_SCIRC, 1 );
	ClearRingList( hDlg, O, S );

	return	TRUE;

}

// Added by Peter on 01 Mar 2006
static void EstimateEllipticalDistortion(HWND hDlg, OBJ* pObj, LPEDRD pEDRD)
{
	CEllipseFit efit;
	CVector X;
	std::vector<double> x, y;
	std::vector<CVector2d>::iterator it;
	int N, i;

	N = pEDRD->vUserClicks.size();
	if( N < 5 )
	{
		MessageBox(hDlg, "You must define at least 5 points", "EDRD ERROR!", MB_OK);
		return;
	}
	x.resize(N);
	y.resize(N);
	for(it = pEDRD->vUserClicks.begin(), i = 0; it != pEDRD->vUserClicks.end(); ++it, i++)
	{
		x[i] = it->x;
		y[i] = it->y;
	}
	if( TRUE == efit.Solve(&x[0], &y[0], N, CEllipseFit::ellArbitrary, X) )
	{
		// success
		double a, b, tilt, dShift;
		CVector2d _center;
		int nRes = IDYES;
		efit.GetEllipseParams(X, CEllipseFit::ellArbitrary, _center, a, b, tilt);
		// check if the center jumped too far away
		dShift = FastSqrt(SQR(_center.x - pEDRD->xc) + SQR(_center.y - pEDRD->yc));
		sprintf(TMP, "The refined center has moved %d pixels away. Accept?", (int)(dShift + 0.5));
		if( dShift > 0.0015*(pEDRD->image.w + pEDRD->image.h) )
		{
			nRes = MessageBox(hDlg, TMP, "EDRD warning", MB_ICONQUESTION | MB_YESNO);
		}
		if( IDYES == nRes )
		{
			sprintf(TMP, "e=%.1f%%, %.1f", 100*FastAbs(1.0 - b / a), R2D(tilt));
			HWND hWnd = GetDlgItem(hDlg, IDC_EDRD_ELL_DISTORT_TEXT);
			SetWindowText(hWnd, TMP);
			pEDRD->xc = _center.x;
			pEDRD->yc = _center.y;
			sprintf(TMP, "X=%.1f Y=%.1f", _center.x, _center.y);
			hWnd = GetDlgItem(hDlg, IDC_EDRD_CENTER);
			SetWindowText(hWnd, TMP);
			pEDRD->dDistortionDir = tilt;
			pEDRD->dDistA = a;
			pEDRD->dDistB = b;
			pEDRD->bEstimated = true;
		}
	}

}

// Added by Peter on 01 Mar 2006
static bool AutoEllipticalDistortion(HWND hDlg, OBJ* pObj, LPEDRD pEDRD, int x, int y)
{
	double dStartLen, dStartAng, dSin, dCos, xc, yc, len, _x, _y;
	int i, nErr;
	bool bRes = false;

	// the misfit distance
	nErr = 0.0015 * (pEDRD->image.h + pEDRD->image.w);
	if( nErr < 8 )
	{
		nErr = 8;
	}
	// reset all the points
	pEDRD->vUserClicks.clear();
	// locate the closest maximum as the starting point
	_x = x, _y = y;
	if( false == LocateMaximum(pObj, pEDRD, _x, _y, nErr) )
	{
		return false;
	}
	// add the current point
	pEDRD->vUserClicks.push_back(CVector2d(_x, _y));
	// the center
	xc = pEDRD->xc;
	yc = pEDRD->yc;
	// the starting angle
	dStartAng = FastAtan2(_x - xc, _y - yc);
	// the initial length
	len = dStartLen = FastSqrt(SQR(_x - xc) + SQR(_y - yc));
	// go around the circle
	for(i = 1; i < 36; i++)
	{
		FastSinCos(dStartAng + i*M_2PI/36.0, dSin, dCos);
		_x = xc + dCos * len;
		_y = yc + dSin * len;
		// locate the closest maximum
		if( true == LocateMaximum(pObj, pEDRD, _x, _y, nErr) )
		{
			pEDRD->vUserClicks.push_back(CVector2d(_x, _y));
//			UpdateEDRD( hDlg, pObj, pEDRD );
			DrawEDRD( hDlg, pObj, pEDRD );
			len = FastSqrt(SQR(_x - xc) + SQR(_y - yc));
		}
//		else
//		{
//			break;
//		}
//		len = FastSqrt(SQR(_x - xc) + SQR(_y - yc));
	}
	// go the other way in case if we met the beam stop
	if( i < 36 )
	{
		int nStop = i - 36;
		len = dStartLen;
		for(i = -1; i > nStop; i--)
		{
			FastSinCos(dStartAng + i*M_2PI/36.0, dSin, dCos);
			_x = xc + dCos * len;
			_y = yc + dSin * len;
			// locate the closest maximum
			if( true == LocateMaximum(pObj, pEDRD, _x, _y, nErr) )
			{
				pEDRD->vUserClicks.push_back(CVector2d(_x, _y));
//				UpdateEDRD( hDlg, pObj, pEDRD );
				DrawEDRD( hDlg, pObj, pEDRD );
				len = FastSqrt(SQR(_x - xc) + SQR(_y - yc));
			}
//			len = FastSqrt(SQR(_x - xc) + SQR(_y - yc));
		}
	}
	// finally estimate the elliptical distortion
	EstimateEllipticalDistortion(hDlg, pObj, pEDRD);
	return bRes;

}

// Added by Peter on 01 Mar 2006
bool LocateMaximum(OBJ* pObj, LPEDRD pEDRD, double &x, double &y, double dErr)
{
	double xc, yc, dx, dy, len, pos;
	bool bRet = false;
	vector<double> profile;
	vector<long> wgt;
	long nLen;

	try
	{
		xc = pEDRD->xc;
		yc = pEDRD->yc;
		dx = x - xc;
		dy = y - yc;
		len = round(FastSqrt(SQR(dx) + SQR(dy)));
		if( len < 2 )
			len = 2;
		nLen = len + 40;
		dx = dx * nLen / len;
		dy = dy * nLen / len;

		profile.resize(nLen+256);
		wgt.resize(nLen+256);

		int nWidth = len * D2R(7.0);
		if( nWidth < 1 )
		{			nWidth = 1;
		}
//		if( 0 < ExtractLine(pEDRD->image.pData, profile.begin(), wgt.begin(), pEDRD->image.w, pEDRD->image.w,
//			pEDRD->image.h, xc, yc, dx + xc, dy + yc, nLen-10, nWidth, pEDRD->pix, pEDRD->image.neg) )
		if( 0 < pEDRD->image.ExtractLine(&*profile.begin(), &*wgt.begin(),
			xc, yc, dx + xc, dy + yc, nLen-10, nWidth) )
		{
			// calculate the average
			double *ptr = &*profile.begin() + (int)(len - 10);
			double dAvg = accumulate(ptr, ptr + 21, 0) / 21.0;
			double dMax = *max_element(ptr, ptr + 21);
			double dMin = *min_element(ptr, ptr + 21);
			if( FastAbs(dAvg - dMax) > 0.05 * (dMax - dMin) )
			{
				// estimated position is 10 - just in the middle of the local profile
				pos = 10;
				if( true == Estimate1DPeakPosition(ptr, 20, pos) )
				{
					x = (x - xc) * (len + pos - 10) / len;
					y = (y - yc) * (len + pos - 10) / len;
					if( FastAbs(FastSqrt(x*x + y*y) - len) < dErr )
					{
						x += xc;
						y += yc;
						bRet = true;
					}
				}
			}
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("LocateMaximum() -> %s"), e.what()));
	}
	return bRet;
}

DWORD WINAPI EDRD_Fit(LPVOID lpParameter)
{
	DWORD	dwRet = -1;
	HWND	hDlg = NULL;
	OBJ*	pObj;
	LPEDRD	pEld;
	try
	{
		hDlg = (HWND)lpParameter;
		if( (NULL == lpParameter) || (FALSE == IsWindow(hDlg)) )
		{
			goto Finish_Thread;
		}
		if( NULL == (pObj = OBJ::GetOBJ(hDlg)) )
		{
			goto Finish_Thread;
		}
		if( NULL == (pEld = (LPEDRD)&pObj->dummy) )
		{
			goto Finish_Thread;
		}
		if( NULL == g_pfnPEAKFIT )
		{
			MessageBox(NULL, _T("PeakFit.dll not found"), _T("Error"), MB_OK);
			goto Finish_Thread;
		}
		{
//			EDRD_REFL *pData;
//			LONG nRefls = 0;
			double dScale = -1.0;
			if( pEld->bCalibrated )
			{
				dScale = pObj->NI.Scale * 1e10;
			}
//			if( NULL != (pData = (EDRD_REFL*)g_pfnPEAKFIT(pEld->lpRDF, &(pEld->r), nRefls, dScale)) )
			if( TRUE == g_pfnPEAKFIT(pEld, dScale) )
			{
				PostMessage(hDlg, WMUSER_ADD_RINGS, (WPARAM)pEld->arccnt, 0);
/*
				int nRefls = pEld->arccnt;
				ClearRingList(hDlg, pObj, pEld);
				// restore back
				pEld->arccnt = nRefls;
				AddRingListHeader(hDlg, pObj, pEld);
//				sprintf(TMP, "reflections: %d", pEld->arccnt);
//				MessageBox(NULL, TMP, "message", MB_OK);
//				LPARC lpa = pEld->lpArc;
//				pEld->arccnt = 0;
//				qsort(pData, nRefls, sizeof(EDRD_REFL), sort_arc_R);
//				double dScale = FindAmplScale(pData, nRefls);
				for(int i = 0; i < nRefls; i++)
				{
//					lpa[i].arcno = i;
//					lpa[i].intens = pData[i].dIntens;
//					lpa[i].io = pData[i].dIntens * dScale;
//					lpa[i].rad = pData[i].dX0;
//					lpa[i].wid = pData[i].dFWHM;
//					if( (0 == pEld->ae0) && (0 == pEld->ae1) )
//					{
//						lpa[i].a0 = 0;
//						lpa[i].a1 = M_2PI;
//					}
//					else
//					{
//						lpa[i].a0 = pEld->ae0;
//						lpa[i].a1 = pEld->ae1;
//					}
					AddRingToList(hDlg, pObj, pEld, i);
//					pEld->arccnt++;
				}
				// free memory
//				if( NULL != g_pfnPEAKFIT_Free )
//				{
//					g_pfnPEAKFIT_Free(pData);
//				}
*/
			}
			pEld->_handle = NULL;
			dwRet = 0;
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("EDRD_Fit() -> %s"), e.what()));
	}
Finish_Thread:
	if( NULL != pEld )
	{
		pEld->_handle = NULL;
	}
	::ExitThread(dwRet);
	return dwRet;
}

/****************************************************************************/
LRESULT CALLBACK EdRDWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		OBJ*		pImg;
		OBJ* pObj = OBJ::GetOBJ(hDlg);
		LPEDRD		pEld = (LPEDRD) &pObj->dummy;
		HCURSOR		hOldCur;

#ifdef _USE_LOGGING
		if( NULL == s_pLog )
		{
			s_pLog = fopen("d:\\EDRD_log.txt", "wt");
		}
		if( NULL != s_pLog )
		{
			fprintf(s_pLog, "msg: EDRD  %8X(%d)\n", message, message);
		}
#endif
		pImg = NULL;
		if( pObj )
		{
			pImg = OBJ::GetOBJ(pObj->ParentWnd);
		}
		switch (message)
		{
			case WM_INITDIALOG:
				// ADDED BY PETER ON 29 JUNE 2004
				// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
				// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
				pObj = (OBJ*)lParam;
				pEld = (LPEDRD) &pObj->dummy;
				InitializeObjectDialog(hDlg, pObj);

				if (!InitEDRDDialog( hDlg, pObj, pEld))
				{
					DestroyWindow( hDlg );
					break;
				}
				UpdateEDRD( hDlg, pObj, pEld);
				SetFocus( GetDlgItem(hDlg,IDC_EDRD_RDFCALC) );
				SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_EDRD_CREFINE, 0), 0);
				SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_EDRD_RDFCALC, 0), 0);
				return TRUE;

			case WM_NCDESTROY:
				if (pObj->bp) free( pObj->bp ); pObj->bp = NULL;
				if (pObj->ParentWnd)  InvalidateRect( pObj->ParentWnd, NULL, FALSE );
				objFreeObj( hDlg );
#ifdef _USE_LOGGING
				if( NULL != s_pLog )
				{
					fclose(s_pLog);
					s_pLog = NULL;
				}
#endif
				break;

//			case WM_CTLCOLOR:
//				return ChangeItemColors( hDlg, wParam, lParam );

			case WM_PAINT:
				DrawPlot( hDlg, pObj, pEld );
				break;

			case FM_ParentInspect:
			{
				OBJ*	I = OBJ::GetOBJ( pObj->ParentWnd );	// Image Object
				int		x = ((LOWORD(lParam)) * I->scd) / I->scu + I->xo;
				int		y = ((HIWORD(lParam)) * I->scd) / I->scu + I->yo;

				if (wParam & MK_LBUTTON)
				{
//					BOOL bMoveCentre = (IsDlgButtonChecked(hDlg, IDC_EDRD_MOVECENTRE) == BST_CHECKED);
//					BOOL bLocateOpposite = IsDlgButtonChecked(hDlg, IDC_EDRD_LOCATE_OPPOSITE);
					BOOL bManualCentre = (IsDlgButtonChecked(hDlg, IDC_EDRD_MANUAL_CENTRE) == BST_CHECKED);
					BOOL bEllipse    = (IsDlgButtonChecked(hDlg, IDC_EDRD_ELLIPTICAL_DISTORTION) == BST_CHECKED);
					UINT uMask = (wParam & MK_CONTROL) | (wParam & MK_SHIFT);
					BOOL bAlt = (::GetKeyState(VK_LMENU) < 0);
					if( (FALSE == bEllipse) && (uMask || bAlt) && (WM_LBUTTONDOWN == pImg->nMouseMessage) )
					{
						// Commented by Peter on 07 Mar 2006
//						pEld->p4cnt = 0;
						// End of comments by Peter on 07 Mar 2006
						if ( (uMask & MK_CONTROL) && (uMask & MK_SHIFT) )
						{
							pEld->r = round(fpCalcDistance(round(pEld->xc),round(pEld->yc),x,y));
						}
						else if (uMask & MK_CONTROL)	// first arc point
						{
							pEld->ae0 = ATAN2(y-pEld->yc, x-pEld->xc);
						}
						else if (uMask & MK_SHIFT)		// second arc point
						{
							pEld->ae1 = ATAN2(y-pEld->yc, x-pEld->xc);
						}
						else if ( bAlt /*|| && bMoveCentre*/ )	// move center
						{
							pEld->xc = x;
							pEld->yc = y;
						}
					}
					else if( (TRUE == bManualCentre) && (WM_LBUTTONDOWN == pImg->nMouseMessage) )
					{
						// set one of >= 4 points
						std::vector<CVector2d>::iterator it;
						for(it = pEld->vUserClicks.begin(); it != pEld->vUserClicks.end(); ++it)
						{
							if ( (SQR(it->x - x) + SQR(it->y - y)) < 16 )
								break;
						}
						// not found
						if( it == pEld->vUserClicks.end() )
						{
							pEld->vUserClicks.push_back(CVector2d(x, y));
						}
						// Commented by Peter on 07 Mar 2006
/*
						// check if some points are too close to each other
						for(i = 0; i < pEld->p4cnt; i++)
						{
							if ( (SQR(pEld->p4[i].x-x) + SQR(pEld->p4[i].y-y)) < 16 )
								  break;
						}
						// no such points found
						if (i == pEld->p4cnt)
						{
							pEld->p4[pEld->p4cnt].x = x;
							pEld->p4[pEld->p4cnt].y = y;
							pEld->p4cnt++;
							if (pEld->p4cnt == 4)	// we clicked on all 4 points
							{
								double px[4], py[4];
								// prepare arrays of coordinates
								for(int j = 0; j < 4; j++)
								{
									px[j] = (double)pEld->p4[j].x;
									py[j] = (double)pEld->p4[j].y;
								}
								// solve for the circle
								if( cfit.Solve(px, py, 4, out) != TRUE )
								{
									::OutputDebugString("FM_ParentInspect: Bad points for circle refinement\n");
									MessageBeep( -1 );
								}
								// get solved data
								pEld->xc = out[0];	// Xc
								pEld->yc = out[1];	// Yc
								pEld->r = out[2];	// R
								pEld->p4cnt = 0;	// no more points
							}
						}
*/
						// End of comments by Peter on 07 Mar 2006
					}
					else if( (TRUE == bEllipse) && (WM_LBUTTONDOWN == pImg->nMouseMessage) &&
						(uMask & MK_CONTROL) )
					{
						AutoEllipticalDistortion(hDlg, pObj, pEld, x, y);
/*
						// check if some points are too close to each other
						std::vector<CVector2d>::iterator it;
						double _x = x, _y = y;
						// locate the closest maximum
						if( false == LocateMaximum(pObj, pEld, _x, _y, 5) )
						{
							_x = x, _y = y;
						}
						for(it = pEld->vUserClicks.begin(); it != pEld->vUserClicks.end(); ++it)
						{
							if ( (SQR(it->x - _x) + SQR(it->y - _y)) < 16 )
								break;
						}
						// not found
						if( it == pEld->vUserClicks.end() )
						{
							pEld->vUserClicks.push_back(CVector2d(_x, _y));
							if( TRUE == bLocateOpposite )
							{
								int xc = pEld->xc, yc = pEld->yc;
								_x = 2*xc - _x, _y = 2*yc - _y;
								// locate the closest maximum
								if( true == LocateMaximum(pObj, pEld, _x, _y, 5) )
								{
									for(it = pEld->vUserClicks.begin(); it != pEld->vUserClicks.end(); ++it)
									{
										if ( (SQR(it->x - _x) + SQR(it->y - _y)) < 16 )
											break;
									}
									// not found
									if( it == pEld->vUserClicks.end() )
									{
										pEld->vUserClicks.push_back(CVector2d(_x, _y));
									}
								}
							}
						}
*/
					}
				}
				else if (wParam & MK_MBUTTON)
				{
					if (wParam & MK_SHIFT)
						pEld->ae1 = ATAN2(y-pEld->yc, x-pEld->xc);
					else if (wParam & MK_CONTROL)
						pEld->ae0 = ATAN2(y-pEld->yc, x-pEld->xc);
					else
						pEld->r = round(fpCalcDistance(round(pEld->xc),round(pEld->yc),x,y));
				}
				else
					break;
				AdjustRefineRadius( pEld );
			}

			case FM_Update:
				UpdateEDRD( hDlg, pObj, pEld );
				break;

			case FM_UpdateNI:
				pObj->NI = OBJ::GetOBJ(pObj->ParentWnd)->NI;
				UpdateCoordinatesText( hDlg, pObj, pEld );
				DrawPlot( hDlg, pObj, pEld );
	    		objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
    			break;

			case FM_ParentPaint:
				DrawEDRD( hDlg, pObj, pEld );
				break;

			case WMUSER_ADD_RINGS:
				{
					int nRefls = pEld->arccnt;
					ClearRingList(hDlg, pObj, pEld);
					// restore back
					pEld->arccnt = nRefls;
					AddRingListHeader(hDlg, pObj, pEld);
					for(int i = 0; i < nRefls; i++)
					{
						AddRingToList(hDlg, pObj, pEld, i);
					}
				}
				break;

			case WM_MOUSEMOVE:
				PrintInfo( hDlg, pObj, pEld, lParam, FALSE );
				break;

			case WM_LBUTTONDBLCLK:
				PrintInfo( hDlg, pObj, pEld, lParam, TRUE );
				break;

			// added by Peter on 30 Mar 2006
			case WM_SYSCOMMAND:
				if( (IDM_OM_INVERT == wParam) && (NULL != pImg) )
				{
					SetNegative(pImg, pEld->image, hDlg);
//					pEld->image.neg = 0;
//					if( IsNegative(pImg) )
//					{
//						if( 1 == pEld->pix ) pEld->neg = 0xFF;
//						if( 2 == pEld->pix ) pEld->neg = 0xFFFF;
//					}
//					CheckDlgButton( hDlg, IDC_EDRD_NEGAT, pEld->neg );
				}
				break;
			// end of addition by Peter on 30 Mar 2006

			case WM_COMMAND:
				switch( LOWORD(wParam) )
				{
					case IDC_EDRD_CREFINE:
					{
						BOOL bAutoCentre = (IsDlgButtonChecked(hDlg, IDC_EDRD_AUTO_CENTRE) == BST_CHECKED);
						BOOL bManualCentre = (IsDlgButtonChecked(hDlg, IDC_EDRD_MANUAL_CENTRE) == BST_CHECKED);
//						BOOL bEllipticalDist = (IsDlgButtonChecked(hDlg, IDC_EDRD_ELLIPTICAL_DISTORTION) == BST_CHECKED);
						hOldCur = SetCursor(hcWait);
						if( bAutoCentre )
						{
							if( !RefineCenter( hDlg, pObj, pEld ) )
							{
								MessageBeep(-1);
							}
						}
						else if( bManualCentre )
						{
							if( !ManualRefineCenter( hDlg, pObj, pEld ) )
							{
								MessageBeep(-1);
							}
						}
						UpdateEDRD( hDlg, pObj, pEld);
						SetCursor(hOldCur);
						break;
					}
					case IDC_EDRD_RDFCALC:
						hOldCur = SetCursor(hcWait);
						if (IsDlgButtonChecked( hDlg, IDC_EDRD_NPROF))
						{
							if ( ! CalculateNProf( hDlg, pObj, pEld ) ) MessageBeep(-1);
						}
						else
						{
							if ( ! CalculateRDF( hDlg, pObj, pEld ) ) MessageBeep(-1);
						}
						UpdateEDRD( hDlg, pObj, pEld);
						DrawPlot( hDlg, pObj, pEld);
						SetCursor(hOldCur);
						break;

					// Added by Peter on 21 Feb 2006
//					case IDC_EDRD_ELL_DIST_ESTIMATE:
//						EstimateEllipticalDistortion(hDlg, pObj, pEld);
//						UpdateEDRD( hDlg, pObj, pEld );
//						break;

					case IDC_EDRD_RESET_CENTRE:
						pEld->vUserClicks.clear();
						pEld->xc = pEld->image.w >> 1;
						pEld->yc = pEld->image.h >> 1;
						UpdateEDRD( hDlg, pObj, pEld );
						break;

					case IDC_EDRD_ELL_DIST_RESET:
						pEld->vUserClicks.clear();
						pEld->bEstimated = false;
						pEld->dDistortionDir = 0.0;
						pEld->dDistA = pEld->dDistB = 0.0;
						UpdateEDRD( hDlg, pObj, pEld );
						break;

					case IDC_EDRD_CORRECT_ELL_DIST:
						CorrectEllipticalDistortion(hDlg, pObj, pEld);
						UpdateEDRD( hDlg, pObj, pEld );
						break;
					// End of addition by Peter on 21 Feb 2006
					case IDC_EDRD_FIT_RDF:
						{
							SECURITY_ATTRIBUTES attrib;
							attrib.nLength = sizeof(attrib);
							attrib.lpSecurityDescriptor = NULL;
							attrib.bInheritHandle = FALSE;

							if( NULL != pEld->_handle )
							{
								MessageBox(hDlg, "Cannot start new Peak Fitting. Stop previous.",
									"EDRD warning", MB_OK);
							}
							else
							{
								pEld->_handle = ::CreateThread(&attrib, 0, EDRD_Fit,
									(LPVOID)hDlg, CREATE_SUSPENDED, &pEld->_id);
								::ResumeThread(pEld->_handle);
							}
						}
						break;

					case IDC_EDRD_ADDRING:
						AddRing( hDlg, pObj, pEld );
						break;
					case IDC_EDRD_CLEAR:
						ClearRingList( hDlg, pObj, pEld );
						break;
					case IDC_EDRD_SAVE:
						SaveRingList( hDlg, pObj, pEld );
						break;

					case IDC_EDRD_RDFSAVE:
						SaveRDFgrapher( hDlg, pObj, pEld );
						break;

					case IDC_EDRD_SCIRC:
					case IDC_EDRD_SARC:
						InvalidateRect( pObj->ParentWnd, NULL, FALSE);
						break;

					case IDC_EDRD_NEGAT:
						SetNegative(NULL, pEld->image, hDlg);
//						if( IsDlgButtonChecked( hDlg, IDC_EDRD_NEGAT ) )
//						{
//							if      (pEld->pix==1) pEld->neg = 0xFF;
//							else if (pEld->pix==2) pEld->neg = 0xFFFF;
//						}	else				pEld->neg = 0;
						break;

					case IDCANCEL:
						if( NULL != pEld->_handle )
						{
							MessageBox(hDlg, "Close Peak Fitting dialog first!",
								"EDRD warning", MB_OK);
						}
						else
						{
							DestroyWindow( hDlg );
#ifdef _USE_LOGGING
							if( NULL != s_pLog )
							{
								fclose(s_pLog);
								s_pLog = NULL;
							}
#endif
						}
						break;
				}
				break;

			case WM_MOUSEACTIVATE:
			case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
			case WM_QUERYOPEN:
				ActivateObj(hDlg);
				break;
		}
	}
	catch( std::exception& e)
	{
		Trace(_FMT(_T("Error -> in EdRDWndProc() with message: %s, %s"), message, e.what()));
	}
	return FALSE;
}
