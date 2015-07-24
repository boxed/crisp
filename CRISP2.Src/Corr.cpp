/****************************************************************************/
/* Copyright(c) 1998 SoftHard Technology, Ltd. ******************************/
/****************************************************************************/
/* CRISP2.CCDcorrection * by ML**********************************************/
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

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

#define STRSIZ	4096		// Maximum strip length
#define	MRS		2			// Marker size on plot

// CCD correction

typedef	struct {
	LPVOID		img;		// Input image
	int			ps;
	LPDOUBLE	acc;		// One accumulative string from selected area
	LPDOUBLE	fft;
	int			ca;			// Acc string length
	double		curve[22];	// Intetnsities, not more than 20 steps on strip
	double		mrkx[22];	// moveable marker X coor
	double		mrky[22];	// moveable marker Y coor
	int			cx;			// Plot window width
	int			cy;			// Plot window height
	double		step0;
	double		stepsz;
	int			steps;		// Real number of steps involved
	int			from;		// # of first band
	BOOL		neg;		// Positives or negatives
	BOOL		df;			// Drag flag
	int			di;			// dragged marker index
	int			maxval;		// Maximum possible value: either 255 (8bpp) or 65535 (16bpp)
} CCDCORR, * LPCCDCORR;


static	char	szCRVHead[] = "Calibration curve\n";
static	char	szCRVSteps[] = "Steps";

static	int		iCorrSerial = 0;	// serial number for corrected image

extern int ExtractLine(LPVOID src, double far * dst, LPLONG wgt, int stride, int iw,
					   int xsize, int ysize, double x1, double y1,
					   double x2, double y2, int len, BOOL add, int neg, int width,
					   PIXEL_FORMAT nFmt);//int pix);

/****************************************************************************/
static void	PrintXY( HWND hDlg, LPCCDCORR S )
{
	int y = (S->neg ? 1-S->mrky[S->di] : S->mrky[S->di]) * S->maxval;
	SetDlgItemInt( hDlg, IDC_LC_SX, S->mrkx[S->di]*S->maxval, FALSE );
	SetDlgItemInt( hDlg, IDC_LC_SY, y ,FALSE );
}

/****************************************************************************/
static BOOL	FindStrip( OBJ* O, LPCCDCORR S )
{
	int		x, y, xs, ys, x1, y1, x2, y2;
	int		i,j, width;
	int		bwid = (O->x+3)&0xFFFC;	// Image buffer width
	LPDOUBLE	ac;
	double	a, s, s0, re, im, xf, rs, is, xsf, as;

	double	blen;
	double	dd, ar, ofs, pos;
	int		cnt;
	int		tmp[20];
	BOOL	bGo;

	ys = O->yas;
	xs = O->xas;
	y  = O->ya;
	x  = O->xa;
	if (x+xs >= O->x) xs = O->x - x;
	if (y+ys >= O->y) ys = O->y - y;
	if (x < 0) { xs += x; x = 0; }
	if (y < 0) { ys += y; y = 0; }

	if (xs>ys) { // Horizontal strip
		x1 = x;	     x2 = x+xs;
		y1 = y+ys/2; y2 = y+ys/2;
		width = ys;
	} else {	// Vertical strip
		x1 = x+xs/2; x2 = x+xs/2;
		y1 = y;      y2 = y+ys;
		width = xs;
	}
	memset( S->acc, 0, STRSIZ*sizeof(double));
	memset( S->fft, 0, STRSIZ*sizeof(double));
// 	S->ca = ExtractLine( S->img, (LPDOUBLE)S->acc, (LPLONG)S->fft, R4(O->x), O->x, O->y,
	S->ca = ExtractLine( S->img, (LPDOUBLE)S->acc, (LPLONG)S->fft, O->CalculateStride(), O->x, O->x, O->y,
						  x1, y1, x2, y2, 0, FALSE, S->neg, width, O->npix);//S->ps);

#define RDIF 8
	ac = S->acc;
	for(i=RDIF; i<S->ca-RDIF; i++) {
		a = 0.;
		for(j=i-RDIF; j<i; j++) a += ac[j];
		for(j=i+RDIF; j>i; j--) a -= ac[j];
		S->fft[i ^ (STRSIZ/2)] = fabs(a);
	}

	ac = S->fft;
	ft1CalcReal (ac, STRSIZ);

#define	STEPWIDTH	20
	a = 0.; j=STEPWIDTH; s0=1e20; bGo = FALSE;
	for(i=STEPWIDTH; i<STRSIZ; i+=2) {
		s = sqrt(ac[i]*ac[i] + ac[i+1]*ac[i+1]);
		if (s0<s) bGo=TRUE;
		s0 = s;
		if(bGo && s > a) { j=i; a = s;}
	}
	xf = atan2(ac[j+1]+1e-10, ac[j]+1e-10)/M_PI*.5;

	rs = is = xsf = as = 1e-10;

#define RR 2
	for(i=j-RR; i<=j+RR; i+=2) {
		re = ac[i]; im = ac[i+1];
		rs += re; is += im;
		a = sqrt(re*re+im*im);
		xsf += (double)i*a; as +=a;
	}
	s = xsf/as*.5;
	s = (double)STRSIZ / s;

	xf = xf*s + STRSIZ*.5;	while(xf>1.75*s) xf -= s;
	xf -= s*.5;
	S->step0 = xf;
	S->stepsz= s;

	i = (int)( ((double)S->ca - xf - s/4.) / s ) + 1;
	if ((i<3)||(i>20)) i=0;
	S->steps = i;

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
	if (S->steps ==0) return FALSE;

	blen = S->stepsz;
	ar = S->stepsz/4.;
	ofs = S->step0;

	for(i=0; i<S->steps; i++) {
		dd = 0.; cnt = 0;
		pos = i*blen + ofs;
		for(j=(int)(pos-ar); j<=(int)(pos+ar); j++)
			if((j>=0)&&(j<S->ca)) { dd += *(S->acc+j); cnt++; }
		dd /= (double)cnt;
		tmp[i] = round(dd);
	}
	if (tmp[0] < tmp[S->steps-1])
		// Normal oder
		for(i=0; i<S->steps; i++) S->curve[i] = tmp[i]/(double)S->maxval;
	else
		// Reverse oder
		for(i=0; i<S->steps; i++) S->curve[i] = tmp[S->steps-1-i]/(double)S->maxval;
	return	TRUE;
}

/****************************************************************************/
static void MakeMarkerCoord( LPCCDCORR S )
{
int		i;

	if (S->steps == 0) return;			// Nothing to do
	for(i=0;i<S->steps;i++) {			// making markers coord
		S->mrkx[i] = (double)S->curve[i];
		S->mrky[i] = (double)i / (double)(S->steps-1);
	}

	for(i=1;i<S->steps;i++)				// Making ascending order of markers
		if (S->mrkx[i]<S->mrkx[i-1]) S->mrkx[i]=S->mrkx[i-1];
}

/****************************************************************************/
static void	UpdateCCD( HWND hWnd, OBJ* O)
{
	OBJ*	par = OBJ::GetOBJ( O->ParentWnd );		/* The parent should be AREA type */
	LPCCDCORR S = (LPCCDCORR) &O->dummy;

	O->x = par->x;
	O->y = par->y;
	O->xa = par->xa;
	O->ya = par->ya;
	O->xas = par->xas;
	O->yas = par->yas;
	O->dp = par->dp;
	O->npix = par->npix;

	S->img = O->dp;		// Input data
	S->ps = IMAGE::PixFmt2Bpp(O->npix);

	if (FindStrip( O, S ))	MakeMarkerCoord( S );
}

/****************************************************************************/
static void	DrawPlot( HWND hDlg, OBJ* O, LPCCDCORR S )
{
HWND 	hw = GetDlgItem( hDlg, IDC_LC_PLOT);
HDC 	shdc = GetDC( hw );
HDC		hdc = CreateCompatibleDC( shdc );
HBITMAP hbmp;
int 	i, xs, ys, xsl, ysl;
RECT 	r;
int		x, y;
HFONT	hof;

	sprintf(TMP,"Steps: %d", S->steps );
	SetDlgItemText( hDlg, IDC_LC_STEPS, TMP);

	GetClientRect( hw, &r );
	xs = r.right - r.left;
	ys = r.bottom - r.top;
    xsl = xs - MRS*2; ysl = ys - MRS*2;
	hbmp = CreateCompatibleBitmap( shdc, xs, ys);
	SelectObject( hdc, hbmp );
	PatBlt( hdc, 0, 0, xs, ys, BLACKNESS);

	if (S->steps == 0) {   // Strip not found
		HFONT	 ofnt = SelectFont(hdc, hfInfo);
		COLORREF otc  = SetTextColor(hdc, RGB(255,255,255));
		int		 obkm = SetBkMode(hdc, TRANSPARENT);
		RECT	 rc;

		SetRect( &rc, 0, 0, xs, ys);
		DrawText(hdc, "Calibration strip not recognized", -1, (LPRECT)&rc,
				 (DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_EXPANDTABS | DT_TABSTOP));
		SelectObject(hdc, ofnt);
		SetTextColor(hdc, otc);
		SetBkMode(hdc, obkm);
		goto ex0;
	}

	{
#ifdef USE_FONT_MANAGER
		int nFntSizeW, nFntSizeH;
		HFONT hFont = FontManager::GetFont(FM_SMALL_FONT, nFntSizeW, nFntSizeH);
#else
		int nFntSizeH = fnts_h;
		HFONT hFont = hfSmall;
#endif
		// Grid
		SelectPen( hdc, GrayPen );
		SetTextColor( hdc, RGB(128,128,128));
		SetBkMode( hdc, TRANSPARENT );
		hof = SelectFont( hdc, hFont );
		_MoveTo( hdc, 0, MRS );           LineTo( hdc, xs, MRS);
		_MoveTo( hdc, 0, MRS+(ysl+1)/2 ); LineTo( hdc, xs, MRS+(ysl+1)/2);
		_MoveTo( hdc, 0, MRS+ ysl );      LineTo( hdc, xs, MRS+ysl);
		for(i=0;i<=8;i++) {
			x = (i*xsl/7)+MRS;
			_MoveTo( hdc, x, 0 ); LineTo( hdc, x, ys );
			sprintf( TMP, "%d", i*(S->maxval/8));
			TextOut(hdc, x+1, ys-MRS-nFntSizeH, TMP, strlen(TMP));
		}
		SelectFont( hdc, hof );
		
		// Draw green crosses for expiremental points
		SelectPen(hdc, GreenPen);
		for(i=0; i<S->steps; i++) {
			x = S->curve[i]*xsl+MRS;
			if (S->neg)	y = (      i*ysl/(S->steps-1))+MRS;
			else		y = (ysl - i*ysl/(S->steps-1))+MRS;
			_MoveTo( hdc, x-MRS-1, y-MRS-1);  LineTo( hdc, x+MRS+2, y+MRS+2);
			_MoveTo( hdc, x+MRS+1, y-MRS-1);  LineTo( hdc, x-MRS-2, y+MRS+2);
		}
		
		SelectPen( hdc, RedPen );
		for(i=0; i<S->steps; i++) {
			x = S->mrkx[i]*xsl+MRS;
			if (S->neg)	y = (     S->mrky[i] *ysl)+MRS;
			else		y = ((1 - S->mrky[i])*ysl)+MRS;
			if (i==0) _MoveTo( hdc, x, y);
			else	  LineTo( hdc, x, y);
			Rectangle( hdc, x-MRS, y-MRS, x+MRS+1, y+MRS+1);
		}
	}
ex0:
	BitBlt( shdc, 0, 0, xs, ys, hdc, 0, 0, SRCCOPY);
	DeleteDC( hdc );
	DeleteObject( hbmp );

	ReleaseDC( hw, shdc );
	InflateRect( &r, -1, -1);
	MapWindowPoints( NULL, hDlg, (LPPOINT)&r, 2);
	ValidateRect( hDlg, &r);
}

/****************************************************************************/
static BOOL	TestClick( HWND hDlg, OBJ* O, LPCCDCORR S, LPARAM lP)
{
int		i, x, y;
POINT	p0 = {LOINT(lP), HIINT(lP)}, p1;
RECT	r;
HWND	hw = GetDlgItem( hDlg, IDC_LC_PLOT);
int		xs, ys;
int		xsl, ysl;
double	x0, y0, x1, y1;

	MapWindowPoints( hDlg, hw, &p0, 1);
	GetClientRect( hw, &r );
	xs = r.right - r.left;
	ys = r.bottom - r.top;
    xsl = xs -  MRS*2; ysl = ys - MRS*2;

	p0.x -= MRS;
	if ( !S->neg ) p0.y = ys-p0.y;
	p0.y -= MRS;

	for( i=0; i<S->steps; i++) {
		x = S->mrkx[i]*xsl;
		y = S->mrky[i]*ysl;
		if (abs(p0.x-x)<3 && abs(p0.y-y)<3) break;
	}

	if (i == S->steps)	return FALSE;

	// Save index
	S->di = i;

    p0.x =  x  + MRS;
	if (S->neg) p0.y =       y + MRS;
	else        p0.y = ysl - y + MRS;

    ClientToScreen(hw, &p0);
    SetCursorPos( p0.x, p0.y );

	if (i==0)
		{ x0 = 0; x1 = S->mrkx[1];   y0 = 0; y1 = S->mrky[i+1]; }
	else if (i==S->steps-1)
		{ x0 = S->mrkx[i-1]; x1 = 1; y0 = 1; y1 = S->mrky[i-1]; }
	else
		{ x0 = S->mrkx[i-1]; x1 = S->mrkx[i+1];
		  y0 = S->mrky[i-1]; y1 = S->mrky[i+1]; }

	if (!S->neg) { y0 = 1.-y0; y1= 1.-y1; }
	p0.x = x0 * xsl + MRS;
	p1.x = x1 * xsl + MRS;
	p0.y = y0 * ysl + MRS;
	p1.y = y1 * ysl + MRS;

	ClientToScreen(hw, &p0); ClientToScreen(hw, &p1);
	if (p0.x>p1.x) { x=p0.x; p0.x=p1.x; p1.x=x; }
	if (p0.y>p1.y) { y=p0.y; p0.y=p1.y; p1.y=y; }
	SetRect( &r, p0.x, p0.y+1, p1.x+1, p1.y+1);
	ClipCursor( &r );

	PrintXY( hDlg, S );
	return TRUE;
}

/****************************************************************************/
static void	DragMarker( HWND hDlg, OBJ* O, LPCCDCORR S, LPARAM lP)
{
POINT	p = { LOINT(lP), HIINT(lP)};
HWND	hw = GetDlgItem( hDlg, IDC_LC_PLOT);
RECT	r;
int		xs, ys;
int		xsl, ysl;

	MapWindowPoints( hDlg, hw, &p, 1);
	GetClientRect( hw, &r );
	xs = r.right - r.left;
	ys = r.bottom - r.top;
	xsl = xs - MRS*2; ysl = ys - MRS*2;

	p.x -= MRS;
	if ( !S->neg ) p.y = ys-p.y;
	p.y -= MRS;

	S->mrkx[S->di] = p.x / (double)xsl;
	S->mrky[S->di] = p.y / (double)ysl;

	PrintXY( hDlg, S );
}

/****************************************************************************/
static	BOOL	SaveCRV( HWND hDlg, OBJ* O, LPCCDCORR S )
{
FILE	*out;
int		i;
HCURSOR	hoc;

	_splitpath( O->fname, NULL, NULL, TMP, NULL );	// take file name only
	if (!fioGetFileNameDialog( MainhWnd, "Save Calibration Curve", &ofNames[OFN_CALIBR], TMP, TRUE)) return FALSE;

	if ((out = fopen( fioFileName(), "w")) == NULL) return dlgError(IDS_ERRCREATEFILE, hDlg);

	hoc = SetCursor(hcWait);
	fprintf(out,"%s", szCRVHead);
	fprintf(out,"%s\t%d\n", szCRVSteps, S->steps);
	fprintf(out,"; X\t  Y\n");
	fprintf(out,"--------------\n");
	for(i=0;i<S->steps;i++) fprintf(out,"%.4f\t%.4f\n", S->mrkx[i], S->mrky[i]);

	SetCursor(hoc);
	if (ferror(out)) { fclose(out); return dlgError(IDS_ERRWRITEFILE, hDlg); }
	fclose(out);
	return TRUE;
}

/****************************************************************************/
static	BOOL	LoadCRV( HWND hDlg, OBJ* O, LPCCDCORR S )
{
FILE	*inf;
int		steps = 0, i;
double	dd, dd1;
char	cmd[20];

	if (!fioGetFileNameDialog( MainhWnd, "Load Calibration Curve", &ofNames[OFN_CALIBR], "", FALSE)) return FALSE;
	if ((inf = fopen( fioFileName(), "r")) == NULL) return dlgError(IDS_ERROPENFILE, hDlg);
	fgets(TMP, 100, inf);
	if (strcmp(TMP, szCRVHead) != 0)
		return dlgError(IDS_ERRCRVHEADER, hDlg);

	while (feof(inf)==0)
	{
		if(fgets (TMP,80,inf)  == NULL) break;
		if(TMP[0] == ';') continue;
		if(strstr(TMP,"------")) break;
		if(sscanf(TMP, "%10s%d", cmd, &i) < 2) continue;

		if (strstr(cmd, szCRVSteps)) steps = i;
	}
	if (feof(inf))   { fclose(inf); return dlgError(IDS_ERRREADFILE, hDlg); }
	if (steps == 0)  { fclose(inf); return dlgError(IDS_ERRCRVSTEPS, hDlg); }
	S->steps = steps;
	for(i=0;i<steps;i++) {
		fgets(TMP, 80, inf);
		sscanf(TMP,"%lg%lg", &dd, &dd1);
		if ((dd<0) || (dd>1.) || (dd1<0) || (dd1>1))
			{ fclose(inf); return dlgError(IDS_ERRCRVDATA, hDlg); }
		S->curve[i] = S->mrkx[i] = dd;
					  S->mrky[i] = dd1;
	}

	if (feof(inf))
		{ S->steps = 0; fclose(inf); return dlgError(IDS_ERRREADFILE, hDlg); }

	for(i=1;i<S->steps;i++)				// Making ascending order of markers
		if (S->mrkx[i]<S->mrkx[i-1]) S->mrkx[i]=S->mrkx[i-1];

	fclose(inf);
	return TRUE;
}
/****************************************************************************/
static BOOL CreateCorrectedImage( HWND hDlg, OBJ* O, LPCCDCORR pCorr )
{
	OBJ		img;
	OBJ*	I;
	HWND	hwi = objFindParent( O, OT_IMG ), hwnew;
	HCURSOR	hoc = SetCursor(hcWait);
	double	xl, yl, xr, yr, ii;
	int		i, j;
	BYTE	bxlat[256];
	WORD	wxlat[65536];

	if (hwi==NULL) return FALSE;
	I = OBJ::GetOBJ(hwi);
	img = *I;

	// commented by Peter on 06 July 2006
//	for(i=0;i<MAXCHI;i++) img.Chi[i]=0;
	// end of comments by Peter on 06 July 2006
	img.ParentWnd = NULL;
	img.hDlg = NULL;
	img.hTree = NULL;
	img.dp = img.bp = img.ebp = img.edp = NULL;
	img.sysmenu = NULL;
	img.hWnd = NULL;

	sprintf( img.title, "Corrected image from %s #%d", I->title, ++iCorrSerial );
	strcpy( img.fname, "" );

#ifdef USE_XB_LENGTH
	if ((img.dp = (LPBYTE) calloc( img.xb*img.y, IMAGE::PixFmt2Bpp(img.npix) )) == NULL )
		return dlgError(IDS_ERRMEMFOROBJECT, hDlg);
#else
	if( NULL == (img.dp = (LPBYTE) calloc( img.stride*img.y, 1 )) )
		return dlgError(IDS_ERRMEMFOROBJECT, hDlg);
#endif

	// TODO: PIX_CHAR
// 	switch( pCorr->npix )
// 	{
// 	}
	if (pCorr->ps==1)
	{
		for(i=0;i<256;i++)
		{
			ii = i/255.;
			if (ii>pCorr->mrkx[pCorr->steps-1]) { xl = pCorr->mrkx[pCorr->steps-1]; yl = pCorr->mrky[pCorr->steps-1];
										xr = yr = 1.; }
			else if (ii<pCorr->mrkx[0])     { xl = yl = 0; xr = pCorr->mrkx[0]; yr = pCorr->mrky[0]; }
			else {
				for(j=0;j<pCorr->steps-1;j++) if(ii>=pCorr->mrkx[j] && ii<pCorr->mrkx[j+1]) break;
				xl=pCorr->mrkx[j]; xr = pCorr->mrkx[j+1];
				yl=pCorr->mrky[j]; yr = pCorr->mrky[j+1];
			}
			bxlat[i] = (BYTE)round((yl + (yr-yl)*(ii-xl)/(xr-xl))*255.);
		}
		if( pCorr->neg ) for(i=0;i<256;i++) bxlat[i] ^= 255;
#ifdef USE_XB_LENGTH
		im8Xlat( img.dp, I->dp, 0, bxlat, I->xb, I->y );
#else
		im8Xlat( img.dp, I->dp, 0, bxlat, I->stride, I->y );
#endif
	} else if (pCorr->ps==2) {
		for(i=0;i<65536;i++) {
			ii = i/65535.;
			if (ii>pCorr->mrkx[pCorr->steps-1]) { xl = pCorr->mrkx[pCorr->steps-1]; yl = pCorr->mrky[pCorr->steps-1];
										xr = yr = 1.; }
			else if (ii<pCorr->mrkx[0])     { xl = yl = 0; xr = pCorr->mrkx[0]; yr = pCorr->mrky[0]; }
			else {
				for(j=0;j<pCorr->steps-1;j++) if(ii>=pCorr->mrkx[j] && ii<pCorr->mrkx[j+1]) break;
				xl=pCorr->mrkx[j]; xr = pCorr->mrkx[j+1];
				yl=pCorr->mrky[j]; yr = pCorr->mrky[j+1];
			}
			wxlat[i] = (WORD)round((yl + (yr-yl)*(ii-xl)/(xr-xl))*65535.);
		}
		if( pCorr->neg ) for(i=0;i<65536;i++) wxlat[i] ^= 65535;
#ifdef USE_XB_LENGTH
		im16Xlat( (LPWORD)img.dp, (LPWORD)I->dp, 0, wxlat, I->xb, I->y );
#else
		im16Xlat( (LPWORD)img.dp, (LPWORD)I->dp, 0, wxlat, I->stride, I->y );
#endif
	}
	SetCursor( hoc );

	hwnew = objCreateObject( IDM_F_OPEN, &img );
	if (hwnew) ActivateObj(hwnew);
	return hwnew != NULL;
}
/****************************************************************************/
static BOOL InitCCD( HWND hDlg, OBJ* O )
{
	OBJ*	  pr  = OBJ::GetOBJ(O->ParentWnd);
	LPCCDCORR S   = (LPCCDCORR) &O->dummy;

	// data memory
	O->bp = (LPBYTE)malloc( STRSIZ*sizeof(double)*2*2 );	// x2 for ReIm, x2 for .acc and .fft
	if ( ! O->bp ) return FALSE;

	S->img = O->dp;
	S->ps = IMAGE::PixFmt2Bpp(O->npix);
	S->acc = (LPDOUBLE) O->bp;
	S->fft = S->acc+STRSIZ;
	S->steps = 7;
	S->from = 0;
	S->neg = FALSE;
	S->df = FALSE;
	S->di = 0;
	S->maxval = (S->ps==1) ? 255 : 65535;

	CheckDlgButton( hDlg, IDC_LC_NEG,  S->neg);
	sprintf(TMP,"Steps: %d", S->steps );
	SetDlgItemText( hDlg, IDC_LC_STEPS, TMP);

	return	TRUE;
}

/****************************************************************************/
LRESULT CALLBACK CorrWndProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ(hDlg);
	LPCCDCORR S = (LPCCDCORR)&O->dummy;

	switch (message)
	{
		case WM_INITDIALOG:
			// ADDED BY PETER ON 29 JUNE 2004
			// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
			// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
			O = (OBJ*)lParam;
			S = (LPCCDCORR) &O->dummy;
			InitializeObjectDialog(hDlg, O);

			InitCCD(hDlg, O);
			UpdateCCD(hDlg, O);
			S->df = FALSE;
			return TRUE;

		case WM_NCDESTROY:
			objFreeObj( hDlg );
			break;

		case WM_PAINT:
			DrawPlot( hDlg, O, S );
			break;

		case WM_LBUTTONUP:
			ClipCursor( NULL );
			S->df = FALSE;
			break;

		case WM_LBUTTONDOWN:
			S->df = TestClick( hDlg, O, S, lParam );
			break;

		case WM_MOUSEMOVE:
			if ((wParam & MK_LBUTTON) && S->df) {
				DragMarker( hDlg, O, S, lParam );
				DrawPlot( hDlg, O, S );
			}
			break;

		case FM_Update:
			UpdateCCD( hDlg, O );
			DrawPlot( hDlg, O, S );
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {

				case IDCANCEL:
					DestroyWindow( hDlg );
					break;

				case IDC_LC_CHANGE: {
					BOOL ff;
					int xx,yy, i = S->di;
					double x, y;

					xx = GetDlgItemInt( hDlg, IDC_LC_SX, &ff, FALSE );
					yy = GetDlgItemInt( hDlg, IDC_LC_SY, &ff, FALSE );
					x = xx/(double)S->maxval;
					y = yy/(double)S->maxval;

					if (i==0) {
						if (x<0) x=0; if (y<0) y=0;
						if (x>S->mrkx[i+1]) x = S->mrkx[i+1];
						if (y>S->mrky[i+1]) y = S->mrky[i+1];
					} else if (i==S->steps-1) {
						if (x<S->mrkx[i-1]) x = S->mrkx[i-1];
						if (y<S->mrky[i-1]) y = S->mrky[i-1];
						if (x>1.) x = 1;
						if (y>1.) y = 1;
					} else {
						if (x<S->mrkx[i-1]) x = S->mrkx[i-1];
						if (y<S->mrky[i-1]) y = S->mrky[i-1];
						if (x>S->mrkx[i+1]) x = S->mrkx[i+1];
						if (y>S->mrky[i+1]) y = S->mrky[i+1];
					}
					S->mrkx[i] = x;
					if (S->neg)	S->mrky[i] = 1-y;
					else		S->mrky[i] = y;
				}
					PrintXY( hDlg, S );
					DrawPlot( hDlg, O, S );
					break;

				case IDC_LC_LINEAR:
					S->neg = FALSE; S->from = 0;
					{
						int i;
						for(i=0;i<S->steps;i++) {
							S->mrkx[i] = S->mrky[i] = S->curve[i] =
								(double)i/(double)(S->steps-1);
						}
					}
					CheckDlgButton( hDlg, IDC_LC_NEG,  S->neg);
					DrawPlot( hDlg, O, S );
					break;

				case IDC_LC_NEG:
					S->neg = IsDlgButtonChecked(hDlg, IDC_LC_NEG);
					DrawPlot( hDlg, O, S );
					break;

				case IDC_LC_CREATE:
					CreateCorrectedImage( hDlg, O, S );
					break;

				case IDC_LC_SAVE:
					SaveCRV( hDlg, O, S );
					break;
				case IDC_LC_LOAD:
					if (LoadCRV( hDlg, O, S ) ) {
						DrawPlot( hDlg, O, S);
						objInformChildren( hDlg, FM_Update, 0, 0);
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
	return FALSE;
}
