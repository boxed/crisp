/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.OneD * by ML ******************************************************/
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

#include "HelperFn.h"

BOOL BrowseForFile(HWND hOwner, LPTSTR pszOutputFile, DWORD nMaxFName);

// To be used in Information panel
double	fDigit=-1., fAspectXY=-1.;

#define	MAXLEN	4096		// Maximum line length
#define	MRS	2				// Marker size on plot

typedef struct {
	int		x1, y1;			// Coordinates of the first  point
	int		x2, y2;			// Coordinates of the second point
	double	distance;		// Distance between points
	int		length;			// Length of line
	double	MaxAmp;			// FFT's Max Amp
	int		width;			// width of profile
//
	double	MaxPro;			// Max of refined profile
	double	MinPro;			// Min of refined profile
	double	PerPro;			// Period of profile
	int		nrefl;			// Number of reflections

	HFONT	m_hFont;		// only reference
	int		m_nFontH;

} ONED;

typedef ONED * LPONED;

ONED	DEFONED = { 0,0, 10,10, 0., 0, 0., 1, 0., 0., 0., 0 };

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

/****************************************************************************/
int	ExtractLine(LPVOID src, double far * dst, LPLONG wgt, int stride, int iw, int xsize, int ysize,
				double x1, double y1, double x2, double y2, int len, BOOL add, int neg,
				int width, PIXEL_FORMAT nFmt)//int pix)
{
	static double x, y;
	static double dx, dy;
	static int		i;

	x = x2 - x1;
	y = y2 - y1;
	len = round(sqrt(x*x + y*y));
	if (len<2) len = 2;
	if (len>MAXLEN) len = MAXLEN;
	dx = y/len;
	dy = -x/len;
	memset( dst, 0, sizeof(double)*MAXLEN);
	memset( wgt, 0, sizeof(long)*MAXLEN);
	if( width < 2 )
	{
		switch(nFmt)
		{
		case PIX_BYTE:
			im8GetProfile((LPBYTE)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
			break;
		case PIX_CHAR:
			im8sGetProfile((LPSTR)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
			break;
		case PIX_WORD:
			im16GetProfile((LPWORD)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
			break;
		case PIX_SHORT:
			im16sGetProfile((SHORT*)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
			break;
		case PIX_DWORD:
			im32GetProfile((LPDWORD)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
			break;
		case PIX_LONG:
			im32sGetProfile((LPLONG)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
			break;
		case PIX_FLOAT:
			imFGetProfile((LPFLOAT)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
			break;
		case PIX_DOUBLE:
			imDGetProfile((LPDOUBLE)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
			break;
		}
	}
	else
	{
		x1 -= dx*width*.5;
		y1 -= dy*width*.5;
		x2 -= dx*width*.5;
		y2 -= dy*width*.5;
		for(i=0;i<width;i++)
		{
			switch(nFmt)
			{
			case PIX_BYTE:
				im8GetProfile((LPBYTE)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_CHAR:
				im8sGetProfile((LPSTR)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_WORD:
				im16GetProfile((LPWORD)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_SHORT:
				im16sGetProfile((SHORT*)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_DWORD:
				im32GetProfile((LPDWORD)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_LONG:
				im32sGetProfile((LPLONG)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_FLOAT:
				imFGetProfile((LPFLOAT)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_DOUBLE:
				imDGetProfile((LPDOUBLE)src, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			}
			x1+=dx; y1+=dy;
			x2+=dx; y2+=dy;
		}
	}
	for(i=0;i<len;i++) if(wgt[i]) dst[i]/=(double)wgt[i]; else dst[i]=128.;
	i = len;
	while( i < MAXLEN )
	{
		dst[i] = 128.f;
		i++;
	}
	return len;
}
/****************************************************************************/
static	void	Draw1D( OBJ* O, LPONED S )
{
	OBJ*		pr = OBJ::GetOBJ(O->ParentWnd);
	HDC			hDC;
	HPEN		hop;
	int			x1, y1, x2, y2;

	hDC = GetDC( O->ParentWnd );
// First point
	x1 = round((S->x1 - pr->xo)*(double)pr->scu/(double)pr->scd + (int)(pr->scu/2));
	y1 = round((S->y1 - pr->yo)*(double)pr->scu/(double)pr->scd + (int)(pr->scu/2));
	hop = SelectPen( hDC, RedPen );
	_MoveTo( hDC, x1-1,  y1-5 ); LineTo( hDC, x1-1,  y1-1 ); LineTo( hDC, x1-6,  y1-1 );
	_MoveTo( hDC, x1+1,  y1-5 ); LineTo( hDC, x1+1,  y1-1 ); LineTo( hDC, x1+6,  y1-1 );
	_MoveTo( hDC, x1-5,  y1+1 ); LineTo( hDC, x1-1,  y1+1 ); LineTo( hDC, x1-1,  y1+6 );
	_MoveTo( hDC, x1+1,  y1+5 ); LineTo( hDC, x1+1,  y1+1 ); LineTo( hDC, x1+6,  y1+1 );
// Second point
	x2 = round((S->x2 - pr->xo)*(double)pr->scu/(double)pr->scd + (int)(pr->scu/2));
	y2 = round((S->y2 - pr->yo)*(double)pr->scu/(double)pr->scd + (int)(pr->scu/2));
	SelectPen( hDC, BluePen );
	_MoveTo( hDC, x2-1,  y2-5 ); LineTo( hDC, x2-1,  y2-1 ); LineTo( hDC, x2-6,  y2-1 );
	_MoveTo( hDC, x2+1,  y2-5 ); LineTo( hDC, x2+1,  y2-1 ); LineTo( hDC, x2+6,  y2-1 );
	_MoveTo( hDC, x2-5,  y2+1 ); LineTo( hDC, x2-1,  y2+1 ); LineTo( hDC, x2-1,  y2+6 );
	_MoveTo( hDC, x2+1,  y2+5 ); LineTo( hDC, x2+1,  y2+1 ); LineTo( hDC, x2+6,  y2+1 );
// Line between
	SelectPen( hDC, GreenPen );
	if (S->width < 2)
	{
		_MoveTo( hDC, x1, y1);	LineTo( hDC, x2, y2);
	}
	else
	{
		double x, y;
		double dx, dy, ww;
		int		len;

		x = x2 - x1;
		y = y2 - y1;
		len = round(sqrt(x*x + y*y));
		if (len<2) len = 2;
		if (len>MAXLEN) len = MAXLEN;
		ww = S->width*(double)pr->scu/(double)pr->scd + pr->scu*.5;
		dx = y/len*ww*.5;
		dy = -x/len*ww*.5;
		_MoveTo( hDC, round(x1-dx), round(y1-dy)); LineTo( hDC, round(x2-dx), round(y2-dy));
		LineTo( hDC, round(x2+dx), round(y2+dy));
		LineTo( hDC, round(x1+dx), round(y1+dy));
		LineTo( hDC, round(x1-dx), round(y1-dy));
	}

	SelectPen(hDC, hop);
	ReleaseDC( O->ParentWnd, hDC );
}
/****************************************************************************/
static	void	Get1AP( double far *fft, int n, double *a, double *p)
{
	double re, im;

	fft += n*2;	// Re.im
	re = *fft;
	im = *(fft+1);
	*a = sqrt(re*re+im*im);
	*p = ATAN2(im, re);
}
/****************************************************************************/
static	void	Put1AP( double far *fft, int n, double a, double p)
{
	fft += n*2;	// Re.im
	*fft     = a*cos(p);
	*(fft+1) = a*sin(p);
}
/****************************************************************************/
static	BOOL	RefineProfile( LPONED S, double * src, double * dst )
{
	double 	max, min, m, mw;
	double	x;
	double	a, p, p0;
	int		nrefl, r=6, i, j, j0, k;

	// 1D FFT
	memcpy ( dst, src, MAXLEN*sizeof(double));
	ft1CalcReal( dst, MAXLEN );

	// Find first reflection
	max = 0.; j = 0;
	for(i=20;i<MAXLEN;i+=2) {
		m = sqrt(dst[i]*dst[i] + dst[i+1]*dst[i+1]);
		if (m>max) { max = m; j = i; }
	}
	if (j<20) return FALSE;	// Refls not found

	// Refine preiod
	x = mw = 0.;
	for(i=j-r; i<=j+r; i+=2) {
		m = sqrt(dst[i]*dst[i] + dst[i+1]*dst[i+1]);
		x += (double)i*m; mw +=m;
	}
	S->PerPro = x = x/mw*.5;				// distance between refls, /2 due to (re.im)
	S->nrefl = nrefl = (int)(S->length/x);	// number of valuable refls
	if (nrefl<1) return FALSE;
	if (nrefl>=10) nrefl = 9;

	// Extract refls
	memset(dst, 0, sizeof(double)*10*2);	// space for refls
	for(i=1;i<=nrefl;i++) {
		j = round(x*2.*i) & ~1;				// must be even !!!
		j0=j; max = 0.;
		for(k=j-r; k<=j+r; k+=2) {
			m = sqrt(dst[k]*dst[k] + dst[k+1]*dst[k+1]);
			if (m>max) { max = m; j0 = k; }
		}
		dst[i*2]   = dst[j0];
		dst[i*2+1] = dst[j0+1];
	}
	memset(&dst[20], 0, sizeof(double)*(MAXLEN-10)*2);	// all others zero

	// Setting proper phase
	Get1AP( dst, 1, &a, &p0);
	for(i=0;i<=nrefl;i++) {
		Get1AP( dst, i, &a, &p);
		p = p - p0*i;
		Put1AP( dst, i, a, p);
	}

	// IFFT
	ft1CalcInverseReal( dst, MAXLEN );

	max = -1e30; min = 1e30;
	for(i=0;i<MAXLEN;i+=2) {
		m = dst[i];
		if ((max<m) && (i>0)) max = m;
		if ((min>m) && (i>0)) min = m;
	}
	S->MaxPro = max;
	S->MinPro = min;

	return TRUE;
}
/****************************************************************************/
// CHANGED by Peter on 21 Aug 2003
static void	DrawPlot( HWND hDlg, OBJ* O, LPONED S, FILE *pFile = NULL )
{
	HWND 	hw = GetDlgItem( hDlg, IDC_1D_PLOT);
	HDC 	shDC = GetDC( hw );
	HDC		hDC = CreateCompatibleDC( shDC );
	HBITMAP hbmp;
	HFONT	hof;
	int 	i, j, xs, ys;
	int		xsl, ysl;
	RECT 	r;
	int		x, y;
	double	*bdata, m, MaxAmp, ddd;
	int		*hdata;
	BOOL	res;

	GetClientRect( hw, &r );

	xs = r.right - r.left;
	ys = r.bottom - r.top;
    xsl = xs - MRS*2-1; ysl = ys - MRS*2-1;
	hbmp = CreateCompatibleBitmap( shDC, xs, ys);
	SelectObject( hDC, hbmp );

	PatBlt( hDC, 0, 0, xs, ys, BLACKNESS);
	SetROP2( hDC, R2_MERGEPEN );
//	PatBlt( hDC, 0, 0, xs, ys, WHITENESS);
//	SetROP2( hDC, R2_COPYPEN );

	// Prepare Refined profile
	if (IsDlgButtonChecked( hDlg, IDC_1D_IFFT ))
		res = RefineProfile(S, (double *)O->bp, (double *)O->dp );

	int nFontH = S->m_nFontH;
	// Grid
	SelectPen( hDC, GrayPen );
	SetTextColor( hDC, RGB(128,128,128));
	SetBkMode( hDC, TRANSPARENT );
// 	hof = SelectFont( hDC, hfSmall );
	hof = SelectFont( hDC, S->m_hFont );
	_MoveTo( hDC, 0, MRS );            LineTo( hDC, xs, MRS);
	_MoveTo( hDC, 0, MRS+ysl/2 ); LineTo( hDC, xs, MRS+ysl/2);
	_MoveTo( hDC, 0, MRS+ysl );   LineTo( hDC, xs, MRS+ysl);
	for(i=0;i<=10;i++) {
		x = i*xsl/10 + MRS;
		_MoveTo( hDC, x, 0 ); LineTo( hDC, x, ys );
		_MoveTo( hDC, x+1, ys/2-1);
		if (IsDlgButtonChecked( hDlg, IDC_1D_PROFILE )) {
			sprintf( TMP, "%-4.1f", (double)i/10.*S->distance);
			y = ys-MRS-nFontH;
		}
		if (IsDlgButtonChecked( hDlg, IDC_1D_FFT )) {
			sprintf( TMP, "%-4.2f", (double)i/10.*.5);
			y = ys-MRS-nFontH-ysl/2;
		}
		if (IsDlgButtonChecked( hDlg, IDC_1D_HISTO )) {
			sprintf( TMP, "%-3d ", (int)(i/10.*256.));
			y = ys-MRS-nFontH;
		}
		if (IsDlgButtonChecked( hDlg, IDC_1D_IFFT )) {
			if (res) sprintf( TMP, "%-4.2f", (double)MAXLEN*i/10./S->PerPro);
			else strcpy(TMP, "    ");
			y = ys-MRS-nFontH-ysl/2;
		}
		TextOut(hDC, x+1, y, TMP, 4);
	}
	SelectObject( hDC, hof );

// Drawing profile curve
	if (IsDlgButtonChecked( hDlg, IDC_1D_PROFILE ))
	{
		bdata = (LPDOUBLE) O->bp;
		SelectPen( hDC, RedPen );
		// commented by Peter on 16 Mar 2008
// 		if(O->npix==PIX_BYTE) ddd = 255.;
// 		if(O->npix==PIX_WORD) ddd = 65535.;
		// added by Peter on 16 Mar 2008
		switch(O->npix)
		{
		case PIX_BYTE:
		case PIX_CHAR:
			{
				double dMax = *max_element(bdata, bdata + S->length);
				ddd = dMax/*255.*/;
			}
			break;
		case PIX_WORD:
		case PIX_SHORT:
			{
				double dMax = *max_element(bdata, bdata + S->length);
				ddd = dMax/*65535.*/;
			}
			break;
		case PIX_DWORD:
		case PIX_LONG:
			{
				double dMax = *max_element(bdata, bdata + S->length);
				ddd = dMax/*65535.*/;
			}
			break;
		case PIX_FLOAT:
		case PIX_DOUBLE:
			{
				double dMax = *max_element(bdata, bdata + S->length);
				ddd = dMax/*65535.*/;
			}
			break;
		}
		// end of addition
		x = MRS;
		y = (ysl-ysl * (*(bdata))/ddd+MRS);
		_MoveTo( hDC, x, y );
		if( pFile )
			fprintf(pFile, "0\t%f\n", *bdata);
		for(i=0;i<S->length;i++)
		{
			x = (xsl*i/(S->length-1))+MRS;
			y = (ysl-ysl * (*(bdata+i))/ddd+MRS);
			LineTo( hDC, x, y );
			if( pFile )
				fprintf(pFile, "%d\t%f\n", i+1, *(bdata+i));
		}
	}
// Drawing FFT curve
	if (IsDlgButtonChecked( hDlg, IDC_1D_FFT ))
	{
		SelectPen( hDC, GreenPen );
		memcpy( O->dp, O->bp, MAXLEN*sizeof(double));
		bdata = (LPDOUBLE)O->dp;
		ft1CalcReal(bdata, MAXLEN);
		MaxAmp = 0.;
		for(i=0;i<MAXLEN;i+=2)
		{
			m = sqrt( (*(bdata))*(*(bdata)) + (*(bdata+1))*(*(bdata+1))); bdata+=2;
			if ((MaxAmp<m) && (i>0)) MaxAmp = m;
		}
		bdata = (LPDOUBLE)O->dp;
		S->MaxAmp = MaxAmp;
		for(i=0;i<MAXLEN;i+=2)
		{
			x = (xsl*i/MAXLEN)+MRS;
			m = sqrt( (*(bdata))*(*(bdata)) + (*(bdata+1))*(*(bdata+1))); bdata+=2;
			y = (int)(ysl-ysl * m/MaxAmp+MRS);
			if (i==0)
				_MoveTo( hDC, x, MRS );
			else
				LineTo( hDC, x, y );
			if( pFile )
				fprintf(pFile, "%d\t%f\n", i>>1, m);
		}
	}
// Drawing Histogram curve
	if (IsDlgButtonChecked( hDlg, IDC_1D_HISTO ))
	{
		bdata = (LPDOUBLE) O->bp;
		hdata = (LPINT) O->dp;
		memset( hdata, 0, 256*sizeof(int));
		for(i=0;i<S->length;i++)
		{
// 			if (O->npix==PIX_BYTE) *(hdata + (int)*(bdata++)) += 1;
// 			if (O->npix==PIX_WORD) *(hdata + (((WORD)*(bdata++))>>8)) += 1;
			switch(O->npix)
			{
			case PIX_BYTE:
				*(hdata + (int)*(bdata++)) += 1;
				break;
			case PIX_WORD:
				*(hdata + (((WORD)*(bdata++))>>8)) += 1;
				break;
			case PIX_DWORD:
				break;
			case PIX_FLOAT:
				break;
			}
		}
		j = 0;
		for(i=0;i<256;i++) if (j<*(hdata+i)) j = *(hdata+i);

		SelectPen( hDC, BluePen );
		for(i=0;i<256;i++)
		{
			x = (int)(xsl*i/256)+MRS;
			y = (int)(ysl-ysl * (*(hdata+i))/j+MRS);
			if (i==0) _MoveTo( hDC, x, y );
			else	  LineTo( hDC, x, y );
			if( pFile )
				fprintf(pFile, "%d\t%i\n", i, *(hdata+i));
		}
	}
// Drawing filtered profile
	if (IsDlgButtonChecked( hDlg, IDC_1D_IFFT ))
	{
		SelectPen( hDC, MagentaPen );
		bdata = (LPDOUBLE) O->dp;
		if ( res )
		{
			for(i=0;i<MAXLEN;i+=2)
			{
				x = (int)(xsl*i/MAXLEN)+MRS;
				m = *(bdata); bdata+=2;
				y = (int)(ysl-ysl * (m-S->MinPro)/(S->MaxPro-S->MinPro)+MRS);
				if (i==0) _MoveTo( hDC, x, y );
				else	  LineTo( hDC, x, y );
				if( pFile )
					fprintf(pFile, "%d\t%f\n", i>>1, m);
				m = (double)MAXLEN/S->PerPro;
		    	sprintf(TMP, "a=%-.3f, Ref#=%d", m, S->nrefl );
			}
		} else TMP[0]=0;
    	SetDlgItemText( hDlg, IDC_1D_INFO, TMP);
	}
	BitBlt( shDC, 0, 0, xs, ys, hDC, 0, 0, SRCCOPY);
	DeleteDC( hDC );
	DeleteObject( hbmp );

	ReleaseDC( hw, shDC );
	GetWindowRect( hw, &r );
	InflateRect( &r, -1, -1);
	MapWindowPoints( NULL, hDlg, (LPPOINT)&r, 2);
	ValidateRect( hDlg, &r);
}
/****************************************************************************/
static void UpdateCoordinatesText( HWND hDlg, OBJ* O, LPONED S)
{
	double x, y;
	sprintf(TMP, "1 %4d %4d", S->x1, S->y1);
	SetDlgItemText( hDlg, IDC_1D_POINT1, TMP);
	sprintf(TMP, "2 %4d %4d", S->x2, S->y2);
	SetDlgItemText( hDlg, IDC_1D_POINT2, TMP);
	x = (S->x2 - S->x1);
	y = (S->y2 - S->y1);
	S->distance = sqrt(x*x + y*y);
	if( 0 != (O->NI.Flags & NI_DIFFRACTION_PATTERN) )	// Default
	{
		// Diffraction pattern has pixels always
		TextToDlgItemW(hDlg, IDC_1D_DISTANCE, "Dist=%5.2f pix", S->distance);		// m -> A
		TextToDlgItemW(hDlg, IDC_1D_DISTANCE_2, "");
	}
	else
	{
		double dScale = O->NI.Scale * 1e10;
		TextToDlgItemW(hDlg, IDC_1D_DISTANCE, "Dist=%5.2f pix", S->distance);		// m -> A
		TextToDlgItemW(hDlg, IDC_1D_DISTANCE_2, "Dist=%5.2f%c", S->distance * dScale, ANGSTR_CHAR);		// m -> A
	}
// 	sprintf(TMP, "Dist=%5.2f", S->distance);		// m -> Å
// 	SetDlgItemText( hDlg, IDC_1D_DISTANCE, TMP);
}
/****************************************************************************/
static void Update1D( HWND hDlg, OBJ* O, LPONED S )
{
	OBJ*	pr = OBJ::GetOBJ( O->ParentWnd );	// Image Object
	BOOL	ff;

	UpdateCoordinatesText( hDlg, O, S );
	if(SendDlgItemMessage( hDlg, IDC_1D_WIDTH, EM_GETMODIFY, 0, 0 ))
	{
		S->width = GetDlgItemInt( hDlg, IDC_1D_WIDTH, &ff, FALSE );
		if (!ff || S->width<1 || S->width>32) S->width = 1;
		SetDlgItemInt( hDlg, IDC_1D_WIDTH, S->width, FALSE );
		SendDlgItemMessage( hDlg, IDC_1D_WIDTH, EM_SETMODIFY, FALSE, 0 );
	}
// 	S->length = ExtractLine( pr->dp, (LPDOUBLE)O->bp, (LPLONG)O->dp, R4(pr->x), pr->x, pr->y,
	S->length = ExtractLine( pr->dp, (LPDOUBLE)O->bp, (LPLONG)O->dp, pr->CalculateStride(), pr->x,  pr->x, pr->y,
							 S->x1, S->y1, S->x2, S->y2, 0, FALSE, 0, S->width, O->npix);
	DrawPlot( hDlg, O, S );
	InvalidateRect( O->ParentWnd, NULL, FALSE);							// To redraw crosses
}
/****************************************************************************/
static LRESULT ChangeItemColors( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC		hdc = (HDC)wParam;
	HWND	hwnd = (HWND)lParam;
	int		idc = GetDlgCtrlID(hwnd);
	LRESULT	ret;
	HPEN	pt = (HPEN)GetStockObject(BLACK_PEN);
	COLORREF	col = RGB(0,0,0);
	static	BOOL brec=FALSE;

	if (brec) return FALSE;
	brec = TRUE;
	ret = DefDlgProc(hDlg, msg, wParam, lParam);

	switch ( idc ) {
		case IDC_1D_POINT1:
			pt = RedPen;  col = RGB(255,0,0);
			break;

		case IDC_1D_POINT2:
			pt = BluePen; col = RGB(0,0,255);
			break;
	}
	if(!IsWindowEnabled(hwnd)) {
		if ( msg == WM_CTLCOLORSTATIC ) goto xxx;
		pt = GrayPen;	col = RGB(192,192,192);
	}
	SelectObject(hdc, pt);
	SetTextColor(hdc, col);
xxx:
	brec = FALSE;
	return	ret;
}
/****************************************************************************/
static void PrintInfo( HWND hDlg, OBJ* O, LPONED S, LPARAM lP )
{
POINT	p = {LOINT(lP), HIINT(lP)} ;
HWND	hw = GetDlgItem( hDlg, IDC_1D_PLOT);
RECT	r;
int		xs, ys, x;
int		xsl, ysl;
LPDOUBLE bdata = (LPDOUBLE)O->dp;
DOUBLE	f, a;

	MapWindowPoints( hDlg, hw, &p, 1);
	GetWindowRect( hw, &r );
	xs = r.right - r.left;
	ys = r.bottom - r.top;
    xsl = xs -  MRS*2; ysl = ys - MRS*2;

	p.y = ys-p.y-MRS;
	p.x = p.x-MRS;
	if ((p.x<0) || (p.x>=xsl) || (p.y<0) || (p.y>=ysl)) return;
    if (IsDlgButtonChecked( hDlg, IDC_1D_PROFILE )) {

    }
    if (IsDlgButtonChecked( hDlg, IDC_1D_FFT )) {
		x = p.x*MAXLEN/xsl;
//		f = x*.5/(O->NI.Scale*1e10)/(double)MAXLEN;
		f = x*.5/(double)MAXLEN;
		bdata += x;		// !!
		a = sqrt( (*(bdata))*(*(bdata)) + (*(bdata+1))*(*(bdata+1)))*100./S->MaxAmp;
		if (x==0) a = 100.;
    	sprintf(TMP, "F(%-.3f) = %4.1f%%", f, a);
    	SetDlgItemText( hDlg, IDC_1D_INFO, TMP);
    }
}
/****************************************************************************/
static BOOL	InitONEDDialog( HWND hDlg , OBJ* O, LPONED S)
{
// data memory
	O->bp = (LPBYTE)calloc( 1, MAXLEN*sizeof(double)*2 );
	if ( O->bp==NULL ) return FALSE;
// FFT, HIST memory
	O->dp = (LPBYTE)calloc( 1, MAXLEN*sizeof(double)*2 );
	if ( O->dp==NULL ) return FALSE;

#ifdef USE_FONT_MANAGER
	HFONT hFont = FontManager::GetFont(FM_MEDIUM_FONT_CW);
	HFONT hFontNormal = FontManager::GetFont(FM_NORMAL_FONT);
	int nFontW, nFontH;
	S->m_hFont = FontManager::GetFont(FM_SMALL_FONT, nFontW, nFontH);
	S->m_nFontH = nFontH;
#else
	HFONT hFont = hfMedium;
	HFONT hFontNormal = hfNormal;
	S->m_hFont = hfSmall;
	S->m_nFontH = fnts_h;
#endif

	SendDlgItemMessage(hDlg, IDC_1D_COORTITLE,  WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_1D_POINT1,     WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_1D_POINT2,     WM_SETFONT, (WPARAM)hFont, 0);
//	SendDlgItemMessage(hDlg, IDC_1D_DISTANCE,   WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_1D_DISTANCE,   WM_SETFONT, (WPARAM)hFontNormal, 0);

	SetDlgItemDouble( hDlg, IDC_1D_XEXT, 20., 0);
	SetDlgItemDouble( hDlg, IDC_1D_YEXT, 20., 0);
	SetDlgItemInt  ( hDlg, IDC_1D_WIDTH, 1, FALSE);

	CheckDlgButton( hDlg, IDC_1D_PROFILE, TRUE);

	*S = DEFONED;
	return TRUE;
}
/****************************************************************************/
static void CalculateCalibration(  HWND hDlg , OBJ* O, LPONED S )
{
double	xex=20., yex=20.;
int		dx = abs(S->x1-S->x2), dy=abs(S->y1-S->y2);

	if ((dx>10) && (dy>10)) {
		if (GetDlgItemDouble( hDlg, IDC_1D_XEXT, &xex, 0 )) {
			if (xex<1.) xex = 1.;
		}
		if (GetDlgItemDouble( hDlg, IDC_1D_YEXT, &yex, 0 )) {
			if (yex<1.) yex = 1.;
		}
		SetDlgItemDouble(hDlg, IDC_1D_XEXT, xex, 0);
		SetDlgItemDouble(hDlg, IDC_1D_YEXT, yex, 0);
		fDigit = (double)dx/xex*1e3;
		fAspectXY = (double)dx/xex*yex/(double)dy;
		sprintf(TMP, "%5.2f pixel/mm", fDigit*1e-3);
		SetDlgItemText( hDlg, IDC_1D_DIG, TMP);
		sprintf(TMP, "X/Y = %7.5f", fAspectXY);
		SetDlgItemText( hDlg, IDC_1D_ASPECT, TMP);
	}
}

// ADDED by Peter on 19 Feb 2003
void Save1DProfile( HWND hDlg, OBJ* O, LPONED S )
{
	static TCHAR szBuffer[_MAX_PATH];
	if( TRUE == BrowseForFile(hDlg, szBuffer, _MAX_PATH) )
	{
		TCHAR drv[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_tsplitpath(szBuffer, drv, dir, fname, ext);
		_tmakepath(szBuffer, drv, dir, fname, _T(".txt"));
		FILE *s_FileOut;
		if( NULL != (s_FileOut = _tfopen(szBuffer, _T("wt"))) )
		{
			DrawPlot( hDlg, O, S, s_FileOut );
			fclose(s_FileOut);
		}
	}
}
/***************************************************************************/
LRESULT CALLBACK OneDWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ(hDlg);
	LPONED	S = (LPONED) &O->dummy;

	switch (message)
	{
	case WM_INITDIALOG:
		// ADDED BY PETER ON 29 JUNE 2004
		// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
		// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
		O = (OBJ*)lParam;
		S = (LPONED) &O->dummy;
		InitializeObjectDialog(hDlg, O);

		InitONEDDialog( hDlg, O, S);
		Update1D( hDlg, O, S);
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
		if (O->bp) free( O->bp ); O->bp = NULL;
		if (O->dp) free( O->dp ); O->dp = NULL;
		if (O->ParentWnd)  InvalidateRect( O->ParentWnd, NULL, FALSE );
		objFreeObj( hDlg );
		break;

	case WM_PAINT:
		DrawPlot( hDlg, O, S );
		break;

	case FM_ParentInspect:
		{
			OBJ*	I = OBJ::GetOBJ( O->ParentWnd );	// Image Object
			int		x = (LOINT(lParam)) * I->scd / I->scu + I->xo;
			int		y = (HIINT(lParam)) * I->scd / I->scu + I->yo;

			if ((wParam&(MK_SHIFT|MK_MBUTTON)) != 0)
			{ S->x2 = x; S->y2 = y; }
			else
			{ S->x1 = x; S->y1 = y; }
		}
	case FM_Update:
		Update1D( hDlg, O, S );
		break;

	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
		UpdateCoordinatesText( hDlg, O, S );
		DrawPlot( hDlg, O, S );
		objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
		break;

	case FM_ParentPaint:
		Draw1D( O, S );
		break;

	case WM_MOUSEMOVE:
		PrintInfo( hDlg, O, S, lParam );
		break;

	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
	case WM_QUERYOPEN:
		ActivateObj(hDlg);
		break;

		// ADDED by Peter 06 Aug 2003
	case IDM_F_SAVE:
	case IDM_F_SSAVE:
		Save1DProfile( hDlg, O, S );
		break;
		// END ADD

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_1D_FFT:
		case IDC_1D_PROFILE:
		case IDC_1D_HISTO:
		case IDC_1D_IFFT:
			DrawPlot( hDlg, O, S );
			break;

		case IDC_1D_CALCAL:
			CalculateCalibration( hDlg, O, S );
			break;

		case IDC_SAVE:
			Save1DProfile( hDlg, O, S );
			break;

		case IDCANCEL:
			DestroyWindow( hDlg );	// To die -- kill parent
			break;
		}
		break;
	}
	return FALSE;
}
