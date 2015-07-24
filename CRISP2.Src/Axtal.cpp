
#include "stdafx.h"

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

#include "HelperFn.h"

#pragma optimize ("",off)

typedef struct
{
	SGINFO	S;				// Space group info (DMAP holder)
	POINT	p[4];			// Coordinates of the points
	float	a[4];			// Angles
	float	l[4];			// Length

} AXTAL;

typedef AXTAL*	LPAXTAL;

static	UINT	ids[4] = { IDC_AX_POINT1, IDC_AX_POINT2, IDC_AX_POINT3, IDC_AX_POINT4 };

/****************************************************************************/
static void	ExtractAXTAL(LPBYTE src, LPBYTE dst, long stride, long iw, int xsize, int ysize,
						 LPAXTAL A, PIXEL_FORMAT pix, double imina, double imaxa, int neg)
{
	double x0, y0, x1, y1;
	double dx0, dy0, dx1, dy1;
	int		i;
	std::vector<double> vData(256);
	std::vector<long> vWeight(256);
	LPBYTE	ddd;

	double dScale = 255.0 / (imaxa - imina);
	if( imaxa == imina )
	{
		dScale = 1.0;
	}

	double* data = &*vData.begin();
	LPLONG	wgt = &*vWeight.begin();

	if( data == NULL || wgt == NULL )
	{
		Trace("ExtractAXTAL() -> failed to allocate memory");
		return;
	}
	x0 = A->p[0].x;
	x1 = A->p[1].x;
	dx0 = (A->p[3].x - A->p[0].x)/256.;
	dx1 = (A->p[2].x - A->p[1].x)/256.;
	// changed by Peter on 5 Nov 2001 - the ysize added in 2 eq
// 	y0 = ysize - A->p[0].y;
// 	y1 = ysize - A->p[1].y;
	// changed by Peter on 5 Nov 2001 - the order of y in brackets in 2 eq
// 	dy0 = (A->p[0].y - A->p[3].y)/256.;
// 	dy1 = (A->p[1].y - A->p[2].y)/256.;
	y0 = A->p[0].y;
	y1 = A->p[1].y;
	dy0 = (A->p[3].y - A->p[0].y)/256.;
	dy1 = (A->p[2].y - A->p[1].y)/256.;
	memset( dst, 0, 0xFFFF );
	for(i = 0; i < 256; i++)
	{
		memset( data, 0, sizeof(double)*256);
		memset( wgt, 0, sizeof(long)*256);
		switch( pix )
		{
		case PIX_BYTE:
			im8GetProfile( (LPBYTE)src, stride, (int)iw, ysize, neg, x0, y0, x1, y1, 256, data, wgt);
			break;
		case PIX_CHAR:
			im8sGetProfile( (LPSTR)src, stride, (int)iw, ysize, neg, x0, y0, x1, y1, 256, data, wgt);
			break;
		case PIX_WORD:
			im16GetProfile( (LPWORD)src, stride, (int)iw, ysize, neg, x0, y0, x1, y1, 256, data, wgt);
			break;
		case PIX_SHORT:
			im16sGetProfile( (SHORT*)src, stride, (int)iw, ysize, neg, x0, y0, x1, y1, 256, data, wgt);
			break;
		case PIX_DWORD:
			im32GetProfile( (LPDWORD)src, stride, (int)iw, ysize, neg, x0, y0, x1, y1, 256, data, wgt);
			break;
		case PIX_LONG:
			im32sGetProfile( (LPLONG)src, stride, (int)iw, ysize, neg, x0, y0, x1, y1, 256, data, wgt);
			break;
		case PIX_FLOAT:
			imFGetProfile( (LPFLOAT)src, stride, (int)iw, ysize, neg, x0, y0, x1, y1, 256, data, wgt);
			break;
		case PIX_DOUBLE:
			imDGetProfile( (LPDOUBLE)src, stride, (int)iw, ysize, neg, x0, y0, x1, y1, 256, data, wgt);
			break;
		default:
			{
				::OutputDebugString("ExtractAXTAL() -> unsupported pixel format\n");
				return;
				break;
			}
		}
		x0+=dx0; y0+=dy0;
		x1+=dx1; y1+=dy1;
		// changed by Peter on 5 Nov 2001 - the destination writing is changed (mirrored)
// 		ddd = dst + 256L*(255-i);
		ddd = dst + 256L * i;
		for(int pos = 0; pos < 256; pos++)
		{
			double dValue = (data[pos] - imina) * dScale;
			if( dValue > 255.0 )
			{
				dValue = 255.0;
			}
			if( dValue < 0.0 )
			{
				dValue = 0.0;
			}
			data[pos] = dValue;
		}
		im8Descale( ddd, data, wgt, 256 );
	}
}
/****************************************************************************/
static	void PaintAXTAL( HWND hDlg, OBJ* O, LPAXTAL A )
{
	HDC		hdc = NULL;
	HWND	hw;
	RECT	re;
	LPSGINFO S = &A->S;
	long	xs, ys;
	LPBYTE	aa;
	std::vector<BYTE> vBuffer;

	hw = GetDlgItem( hDlg, IDC_AX_PLOT );
	GetClientRect( hw, &re );
	InvalidateRect( hw, NULL, FALSE );
	InflateRect( &re, -1, -1);
	xs = re.right  - re.left;
	ys = re.bottom - re.top;
	vBuffer.resize(xs*(ys+3));
	aa = &*vBuffer.begin();//(LPBYTE)calloc(1, (long)xs*(long)(ys+3));

	if( aa )
	{
		// It seems that I can not use 'bimem' memory here, since it becomes corrupted!!!!
		DMap_To_Img256( &A->S, aa, xs, ys);
		// create a temp object (we always paint an 8 bit bitmap on artificial crystals)
		OBJ tmpObj;
		tmpObj.npix = PIX_BYTE;
		tmpObj.imin = 0;
		tmpObj.imax = 255;
		tmpObj.bright = 128;
		tmpObj.contrast = 128;
		pntBitmapToScreen( hw, aa, xs, ys, 0, 0, 1, 1, &tmpObj, 0x100 );
		//free( aa );
	}

	MapWindowPoints( hw, hDlg, (LPPOINT) &re, 2);
	ValidateRect( hDlg, &re );
}
/****************************************************************************/
static	void	DrawAXTAL( OBJ* O, LPAXTAL A )
{
	OBJ*		pr = OBJ::GetOBJ(O->ParentWnd);
	HDC			hdc;
	HPEN		hop, hp = CreatePen(PS_SOLID,1,RGB(64,64,255));
	POINT		p[4];
	int			i;

	for(i = 0; i < 4; i++)
	{
		p[i].x = round((A->p[i].x - pr->xo)*(float)pr->scu/(float)pr->scd     + (int)(pr->scu/2));
		// updated by Peter on 29 June 2004
		p[i].y = round((A->p[i].y - pr->yo)*(float)pr->scu/(float)pr->scd - 1 + (int)(pr->scu/2));
//		p[i].y = round(pr->yw - (A->p[i].y - pr->yo)*(float)pr->scu/(float)pr->scd - 1 - (int)(pr->scu/2));
	}

	hdc = GetDC( O->ParentWnd );
	hop = (HPEN)SelectObject( hdc, RedPen );
	_MoveTo( hdc, p[0].x, p[0].y);
	LineTo( hdc, p[1].x, p[1].y);
	SelectObject( hdc, hp );
	LineTo( hdc, p[2].x, p[2].y);
	SelectObject( hdc, RedPen );
	LineTo( hdc, p[3].x, p[3].y);
	SelectObject( hdc, hp );
	LineTo( hdc, p[0].x, p[0].y);
	SelectObject(hdc, hop);
	ReleaseDC( O->ParentWnd, hdc );
	DeleteObject(hp);
}
/****************************************************************************/
static float CalcDist( LPPOINT p, int a, int b)
{
	double	x, y;

	x = (p[a].x - p[b].x); y = (p[a].y - p[b].y);
	return sqrt(x*x+y*y);
}
/****************************************************************************/
static void UpdateCoordinatesText( HWND hDlg, LPAXTAL A)
{
	int		i;

	for(i = 0; i < 4; i++)
	{
		sprintf(TMP, "%4d %4d", A->p[i].x, A->p[i].y);
		SetDlgItemText( hDlg, ids[i], TMP);
		A->l[i] = CalcDist(A->p, i, (i+1)&3);
	}
	A->S.ral = (A->l[0]+A->l[2])/2;
	A->S.rbl = (A->l[1]+A->l[3])/2;
	sprintf(TMP, "a1=%5.2f", A->l[0]); SetDlgItemText( hDlg, IDC_AX_A1, TMP);
	sprintf(TMP, "b1=%5.2f", A->l[1]); SetDlgItemText( hDlg, IDC_AX_B1, TMP);
	sprintf(TMP, "a2=%5.2f", A->l[2]); SetDlgItemText( hDlg, IDC_AX_A2, TMP);
	sprintf(TMP, "b2=%5.2f", A->l[3]); SetDlgItemText( hDlg, IDC_AX_B2, TMP);

//	sprintf(TMP, "a=%5.2f", A->S.ral); SetDlgItemText( hDlg, IDC_AX_RAL, TMP);
//	sprintf(TMP, "b=%5.2f", A->S.rbl); SetDlgItemText( hDlg, IDC_AX_RBL, TMP);
//	sprintf(TMP, "\201=%-.1f", R2D(A->S.rgamma)); SetDlgItemText( hDlg, IDC_AX_RGAMMA, TMP);
	TextToDlgItemW(hDlg, IDC_AX_RAL, "a=%5.2f%c", A->S.ral, ANGSTR_CHAR);
	TextToDlgItemW(hDlg, IDC_AX_RBL, "b=%5.2f%c", A->S.rbl, ANGSTR_CHAR);
	TextToDlgItemW(hDlg, IDC_AX_RGAMMA, "%c=%-.1f%c", GAMMA_CHAR, R2D(FastAbs(A->S.rgamma)), DEGREE_CHAR);
}
/****************************************************************************/
static void UpdateAXTAL( HWND hDlg, OBJ* O, LPAXTAL A, BOOL bAutoUp )
{
	OBJ*	pr = OBJ::GetOBJ( O->ParentWnd );	// Image Object

	UpdateCoordinatesText( hDlg, A );
	InvalidateRect( O->ParentWnd, NULL, FALSE);							// To redraw crosses
	if (bAutoUp)
	{
		// TODO: negative (inverse) image must store the flag somewhere and use it as the last parameter
		ExtractAXTAL( pr->dp, O->bp, R4(pr->x * IMAGE::PixFmt2Bpp(O->npix))/*(pr->x+3)&0xFFFC*/, pr->x, pr->x, pr->y, A, pr->npix, pr->imin, pr->imax, 0/*O->neg*/);
		PaintAXTAL( hDlg, O, A );
	}
}
/****************************************************************************/
static float CalcAng3( LPPOINT p, int a, int b, int c)
{
	int		ph, ph1;

	ph = R2P(ATAN2(p[c].y-p[b].y, p[c].x-p[b].x));
	ph1 = R2P(ATAN2(p[a].y-p[b].y, p[a].x-p[b].x));
	ph = ph1-ph;
	return P2D(ph);
}
/****************************************************************************/
static BOOL DragPoints( HWND hDlg, LPARAM lParam, OBJ* O, LPAXTAL A )
{
	OBJ*	I = OBJ::GetOBJ( O->ParentWnd );	// Image Object
	int		x = (LOWORD(lParam)) * I->scd / I->scu  + I->xo;
	// updated by Peter on 29 June 2004
	int		y = (HIWORD(lParam) - 1) * I->scd / I->scu + I->yo;
//	int		y = (I->yw -  HIWORD(lParam) - 1) * I->scd / I->scu + I->yo;
	float	dx, dy;
	int		i, d, d0, n=-1;
	POINT	pp[4];
	float a0,a1,a2,a3;

	// Search for nearest point
	for(i = 0; i < 4; i++)
	{
		dx = A->p[i].x-x;
		dy = A->p[i].y-y;
		d = round(sqrt(dx*dx+dy*dy));
		if (n<0 || d<d0)
		{
			d0=d; n=i;
		}
	}

	if (d==0) return FALSE; // nothing to change

	// Checking angles
	memcpy(pp, A->p, sizeof(pp));
	A->p[n].x=x; A->p[n].y=y;
	a0=CalcAng3(A->p, 0,1,2);
	a1=CalcAng3(A->p, 1,2,3);
	a2=CalcAng3(A->p, 2,3,0);
	a3=CalcAng3(A->p, 3,0,1);

	if (a0>179.||a0<1.) goto errex;
	if (a1>179.||a1<1.) goto errex;
	if (a2>179.||a2<1.) goto errex;
	if (a3>179.||a3<1.) goto errex;
	A->a[0] = a0; A->a[1] = a1; A->a[2] = a2; A->a[3] = a3;
	// changed by Peter on 2 Dec 2001 - the '180 -' added to reverse y-ccord.
//	A->S.rgamma = D2R( 180 + (a1+a3)*.5 );
	A->S.rgamma = D2R( 180-(a1+a3)*.5 );
	return TRUE;
errex:
	memcpy(A->p, pp, sizeof(pp));
	return FALSE;
}
/****************************************************************************/
static LRESULT ChangeItemColors( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC		hdc = (HDC)wParam;
	HWND	hwnd = (HWND)lParam;
	int		idc = GetDlgCtrlID(hwnd);
	LRESULT	ret;
	static	BOOL bRec = FALSE;

	if (bRec)
		return FALSE;
	bRec = TRUE;

	ret = DefDlgProc(hDlg, msg, wParam, lParam);
	switch ( idc )
	{
		case IDC_AX_A1:
		case IDC_AX_A2:
		case IDC_AX_RAL:
			SelectObject(hdc, RedPen);  SetTextColor(hdc, RGB(255,0,0));
			break;

		case IDC_AX_B1:
		case IDC_AX_B2:
		case IDC_AX_RBL:
			SelectObject(hdc, BluePen); SetTextColor(hdc, RGB(0,0,255));
			break;
	}
	bRec = FALSE;

	return ret;
}
/****************************************************************************/
static	BOOL	InitAXTALObject( HWND hWnd, OBJ* O, LPAXTAL A )
{
	OBJ*	pr  = OBJ::GetOBJ(O->ParentWnd);
//	LPAXTAL A = (LPAXTAL) &O->dummy;
	LPSGINFO S = &A->S;
	int		x, y;

//	SetOBJ( hWnd, O );			// Remember Object ptr
	// TODO:
//	CopyOBJ ( O, pr );			// Make a copy of parent Obj

	x = pr->x>>1;
	y = pr->y>>1;
	A->p[0].x = x-10; A->p[0].y = y-10;
	A->p[1].x = x+10; A->p[1].y = y-10;
	A->p[2].x = x+10; A->p[2].y = y+10;
	A->p[3].x = x-10; A->p[3].y = y+10;
	// TODO check dxs is the same as ds at CRISP1
	S->dxs = 256;
// DMAP data memory
	O->bp = (LPBYTE)calloc(1, (long) S->dxs*S->dxs );
	if ( !O->bp )
		return FALSE;
	O->dp = NULL;
	O->bright = 128;
	O->contrast = 128;
	O->palno = -1;
	O->maxcontr = TRUE;
	S->dmap = O->bp;
	S->rgamma = M_PI * .5;
	S->rot = 0;
	S->shx = 0;
	S->shy = 0;
	S->ral = 1;
	S->rbl = 1;
	S->flg = 0;
	S->scale = 1.5;

	return	TRUE;
}
/****************************************************************************/
static void	InitAXTALDialog( HWND hDlg, OBJ* O, LPAXTAL A )
{
	int		i;

#ifdef USE_FONT_MANAGER
	HFONT hFont = FontManager::GetFont(FM_MEDIUM_FONT);
	HFONT hFontNormal = FontManager::GetFont(FM_NORMAL_FONT);
	HFONT hFontGreek = FontManager::GetFont(FM_GREEK_FONT);
#else
	HFONT hFont = hfMedium;
	HFONT hFontNormal = hfNormal;
	HFONT hFontGreek = hfGreek;
#endif
	for(i = 0; i < 4; i++)
		SendDlgItemMessage(hDlg, ids[i],  WM_SETFONT, (WPARAM)hFont, 0L);

	SendDlgItemMessage(hDlg, IDC_AX_TEXT1,WM_SETFONT, (WPARAM)hFont, 0L);
	SendDlgItemMessage(hDlg, IDC_AX_A1,   WM_SETFONT, (WPARAM)hFont, 0L);
	SendDlgItemMessage(hDlg, IDC_AX_A2,   WM_SETFONT, (WPARAM)hFont, 0L);
	SendDlgItemMessage(hDlg, IDC_AX_B1,   WM_SETFONT, (WPARAM)hFont, 0L);
	SendDlgItemMessage(hDlg, IDC_AX_B2,   WM_SETFONT, (WPARAM)hFont, 0L);

	SendDlgItemMessage(hDlg, IDC_AX_RAL,  WM_SETFONT, (WPARAM)hFontNormal, 0L);
	SendDlgItemMessage(hDlg, IDC_AX_RBL,  WM_SETFONT, (WPARAM)hFontNormal, 0L);
	SendDlgItemMessage(hDlg, IDC_AX_RGAMMA,WM_SETFONT, (WPARAM)hFontGreek, 0L);
//	SendDlgItemMessage(hDlg, IDC_AX_RAL,  WM_SETFONT, (WPARAM)hfMedium, 0L);
//	SendDlgItemMessage(hDlg, IDC_AX_RBL,  WM_SETFONT, (WPARAM)hfMedium, 0L);
//	SendDlgItemMessage(hDlg, IDC_AX_RGAMMA,WM_SETFONT, (WPARAM)hfMedium, 0L);

	InitAXTALObject( hDlg, O, A );
}
/***************************************************************************/
LRESULT CALLBACK AXtalWndProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	try
	{
		OBJ*	O = OBJ::GetOBJ(hDlg);
		LPAXTAL A = (LPAXTAL) &O->dummy;

		switch (message)
		{
		case WM_INITDIALOG:
			// ADDED BY PETER ON 29 JUNE 2004
			// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
			// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
			O = (OBJ*)lParam;
			A = (LPAXTAL) &O->dummy;
			InitializeObjectDialog(hDlg, O);

			InitAXTALDialog( hDlg, O, A );
			UpdateAXTAL( hDlg, O, A, TRUE );
//			objModifySysMenu( hDlg );
			return TRUE;

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORSCROLLBAR:
			return ChangeItemColors( hDlg, message, wParam, lParam );

		case WM_NCDESTROY:
			if (O->bp)  free( O->bp ); O->bp = NULL;
			if (O->ParentWnd) InvalidateRect( O->ParentWnd, NULL, FALSE );
			objFreeObj( hDlg );
			break;

		case WM_PAINT:
			PaintAXTAL( hDlg, O, A );
			break;

		case FM_ParentInspect:
			if (!DragPoints( hDlg, lParam, O, A )) break;

		case FM_Update:
			UpdateAXTAL( hDlg, O, A, IsDlgButtonChecked( hDlg, IDC_AX_AUPDATE) );
			break;

		case FM_UpdateNI:
			O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
			UpdateCoordinatesText( hDlg, A );
			PaintAXTAL( hDlg, O, A );
    		objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
    		break;

		case FM_ParentPaint:
			DrawAXTAL( O, A );
			break;

		case WM_MOUSEACTIVATE:
		case WM_ACTIVATE: if (wParam == WA_INACTIVE) break;
		case WM_QUERYOPEN:
			ActivateObj(hDlg);
			break;

		case WM_COMMAND:
//			if (wndDefSysMenuProc(hDlg, wParam, lParam)) break;
			switch (LOWORD(wParam))
			{
				case IDM_C_DMAP:
				case IDC_AX_CREATE:
					PostMessage( MainhWnd, WM_COMMAND, IDM_C_DMAP, 0 );
					break;

				case IDC_AX_AUPDATE:
					if (!IsDlgButtonChecked( hDlg, IDC_AX_AUPDATE) )
						break;

				case IDC_AX_UPDATE:
					UpdateAXTAL( hDlg, O, A, TRUE );
					break;

				case IDCANCEL:
					DestroyWindow( hDlg );	// To die -- kill parent
					break;
			}
			break;
		}
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("AXtalWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
