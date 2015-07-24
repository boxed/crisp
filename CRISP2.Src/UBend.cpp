/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.Ubend * by SK, ML *************************************************/
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

#pragma optimize ("",off)

/****************************************************************************/
static	void	ScanAll( HWND hDlg, OBJ* O,  LPSGINFO S)
{
/*
LPSGINFO SI = L->SI+L->Spg;
double	fd;

	GetDlgItemDouble( hDlg, IDC_ORT_THR, &L->Thr, 0);
	fd=P2D(SI->Sh); GetDlgItemDouble( hDlg, IDC_ORT_SH, &fd, -1 ); SI->Sh = D2P(fd);
	fd=P2D(SI->Sk); GetDlgItemDouble( hDlg, IDC_ORT_SK, &fd, -1 ); SI->Sk = D2P(fd);
	SetDlgItemDouble( hDlg, IDC_ORT_SH, P2D(SI->Sh), -1);
	SetDlgItemDouble( hDlg, IDC_ORT_SK, P2D(SI->Sk), -1);
*/
}
/****************************************************************************/
static void UpdateNumbers( HWND hDlg, OBJ* O, LPSGINFO S )
{
	sprintf( TMP, "a=%-.2f", S->ral);   SetDlgItemText( hDlg, IDC_UB_A, TMP);
	sprintf( TMP, "b=%-.2f", S->rbl);   SetDlgItemText( hDlg, IDC_UB_B, TMP);
	sprintf( TMP, "\201=%-.1f", R2D(fabs(S->rgamma)));SetDlgItemText( hDlg, IDC_UB_G, TMP);
	sprintf( TMP, "a-x angle=%5.1f", R2D(S->aw));SetDlgItemText( hDlg, IDC_UB_AXANGLE, TMP);
//	sprintf( TMP, "%d", O->x);	SetDlgItemText( hDlg, IDC_DM_XSIZE, TMP);
//	sprintf( TMP, "%d", O->y);	SetDlgItemText( hDlg, IDC_DM_YSIZE, TMP);
}
/****************************************************************************/
static BOOL DoUBend( HWND hDlg, OBJ* O, LPSGINFO S )
{
	double	scale, scx0, scx1, scy;
	HWND	hpar, himg, hpar_img;
	OBJ*	pr;
	OBJ*	pr_img;
	BMF		BFLT(64,1);

	SPOT	sp;
	int		size, i,j,s,n,rm,r,x,y,h,k;
	ICOMPLEX p,p1,pt,hk;
	FCOMPLEX ra, rb, fa, fb, u,v,w, as,bs;
	float	t;
	LPBYTE	bp;
	LPFLOAT	fp;
	int		aperture, aslope;		// Aperture size and slope
	int		average;				// Average value to fill area beyond aperture and slope
	static int imageserial = 1;		// serial number of image
	int		searchguard;			// Radius in which we have to find a spot on xcorr map
	int		maxspots = 32;			// Maximum number of spots along each exis on xcorr map

	OBJ		ODST;
	double	fcells = 2.5, fslope = 0.5;
	HCURSOR	hoc;

// Take the size of the Area and FFT
	if ((hpar = objFindParent( O, OT_AREA )) == NULL) return FALSE;
	size = OBJ::GetOBJ(hpar)->ui;	// Area size
	if ((hpar_img = objFindParent( OBJ::GetOBJ(hpar), OT_IMG )) == NULL) return FALSE;
	pr_img = OBJ::GetOBJ(hpar_img);

// Aperture and slope size in pixels calculated from their values in cells
	GetDlgItemDouble( hDlg, IDC_UB_APERTURE, &fcells, 0 );
	GetDlgItemDouble( hDlg, IDC_UB_ASLOPE, &fslope, 0 );
	aperture = fcells * __max(S->ral, S->rbl);
	aslope = fslope * __max(S->ral, S->rbl);
	if (aperture>=size/2) aperture = size/2-1;
	if (aperture+aslope>=size/2) aslope = size/2-aperture;

// Calculate the scale and rotation angle for DMAP -> pattern calculation
	scx0= (double)size / S->rbl / fabs(cos(S->rgamma));
	scx1= (double)size / S->ral;
	scy = (double)size / S->rbl / fabs(sin(S->rgamma));
	scale = __min(scy,__min(scx0,scx1));

	S->scale = scale;
	S->rot   = -S->aw;

// Maximum radius for searching a peak on Xcorr map
	searchguard = __min(S->ral, S->rbl)/2-2;	// two pixels less than half of the shortest cell dimension

// Alloc array of "spots on Xcorr map"
	maxspots = R4((int)(size/__min(S->ral, S->rbl)));
	SetDlgItemInt( hDlg, IDC_UB_SPOTS, maxspots, FALSE );
	BMSP Sp(maxspots*2,maxspots*2,maxspots,maxspots);

// initializing filter array
	for(i=0; i<BFLT.W; i++) BFLT[i] = 1. - cos((float)i * M_PI / BFLT.W / 2.);

// Creating a new FT2R with data pointer to the existing parent (original image) FT2R
	if ((hpar = objFindParent( O, OT_FFT )) == NULL) return FALSE;
	pr = OBJ::GetOBJ(hpar);
	FT2R FT1(size, GetFFT(pr)->ft2.BX.P );

// A new BMP where correlation image, and later resultant image will be stored
	BM8 B2(size,size);

// Creating pattern bitmap from DMAP data
	SetDlgItemText( hDlg, IDC_UB_INFO, "1: Creating pattern for Xcorrelation");
	DMap_To_Img256( S, B2.P, size, size );	// Convert from 256x256 DMAP data into size x size bitmap

// Making a copy of the original bitmap to be used in resampling
	if ((hpar = objFindParent( O, OT_IMG )) == NULL) return FALSE;
	pr = OBJ::GetOBJ(hpar);
	BM8 B1(size,size);	// an BMP object for storing an original bitmap
	for (i=0;i<size;i++)
	{
		memcpy(&B1.P[size*i], pr->dp + (i + pr->ya)*pr->stride + pr->xa, size );
	}

	// Components of the lattice vectors on original/pattern images
	ra.i = S->ral * cos(S->aw);
	ra.j = S->ral * sin(S->aw);
	rb.i = S->rbl * cos(S->rgamma + S->aw);
	rb.j = S->rbl * sin(S->rgamma + S->aw);

	hoc = SetCursor( hcWait );

	// Alloc an FT2R for FFT of pattern
	FT2R FT2(size);
	memset(FT2.BX.P, 0, size*size*4);

// Applying aperture on pattern ------------------------------------
	SetDlgItemText( hDlg, IDC_UB_INFO, "2: Applying apertute on pattern");
	for(fp=FT2.BX.P, bp=B2.P, average=128, y=-size/2; y<size/2; y++)
		for(x=-size/2; x<size/2; x++, fp++, bp++){
			r = ::sqrt((double)(x*x+y*y));
		if(r>=aperture+aslope) {*fp = average; continue;}	// point beyond aperture and slope
		if(r>=aperture)			*fp = average + (*bp-average) * ((aperture+aslope-(double)r)/aslope);	// point on slope
		else					*fp = *bp;					// point within aperture
	}

	// Xcorrelaton -----------------------------------------------------
	SetDlgItemText( hDlg, IDC_UB_INFO, "3: FFT of pattern");
	FT2.FFT();
	SetDlgItemText( hDlg, IDC_UB_INFO, "4: Compex conjugate multiplication");
	FT2 ^= FT1;
	FT2.MulBlob(BFLT, 0.02f);	// Applying a high pass filter to cut DC/low frequency features from Xcorr map
	SetDlgItemText( hDlg, IDC_UB_INFO, "5: IFFT -> Xcorrelation map");
	FT2.IFT();

	// Creating a destination window
	memset(&ODST, 0, sizeof(ODST));
	ODST.ID = OT_IMG;
	// changed by Peter on 20 Sep 2001
	ODST.x = size;
	ODST.y = size;
	ODST.xas = size;
	ODST.yas = size;
//	ODST.x = pr_img->x; ODST.y = pr_img->y;
//	ODST.xas = pr_img->xas; ODST.yas = pr_img->yas;
	ODST.npix = PIX_BYTE; // b816 ? 2 : 1;
	// changed by Peter on 20 Sep 2001
	ODST.stride = R4(size * IMAGE::PixFmt2Bpp(ODST.npix));
	ODST.dp = (LPBYTE)calloc(1, ODST.stride*ODST.y);	//
	if( NULL == ODST.dp )
		return FALSE;
	ODST.scu = 1;
	ODST.scd = 1;
	ODST.maxcontr = TRUE;
	ODST.bright = 128;
	ODST.contrast = 128;
	ODST.palno = -1;
	sprintf( ODST.title, "Unbended image #%d", imageserial ); imageserial++;
	strcpy( ODST.fname, "" );
	//	ODST.flags |= OF_someglags;
	himg = objCreateObject( IDM_F_OPEN, &ODST );

// Snapshot of Xcorr map into B2, and copy it for visualization in destination
	FT2.DisplayIFT(B2);
	memcpy( ODST.dp, B2.P, size*size );
	imgCalcMinMax( OBJ::GetOBJ(himg) );
	ActivateObj( himg );
	UpdateWindow( himg );

// Spot detection on Xcorrelation map -------------------------------
	SetDlgItemText( hDlg, IDC_UB_INFO, "6: Spot detection on Xcorrelation map");
	HDC hdc = GetDC( himg ); SelectBrush( hdc, GetStockObject(NULL_BRUSH) );
	B2.XC = B2.YC = size/2;
	for(rm=size/2-searchguard, k=0; k<maxspots; k++)	for(h=0; h<maxspots; h++){
		i=1; j=1;
		do{
			do{
				FCOMPLEX x, y, z;
				x = y + z;
				if(k==0){ if(h==0) w=0; else w = Sp(h-i, k).p + (FCOMPLEX)(ra*((float)i));}
				else	                     w = Sp(h, k-j).p + rb*((float)j);

				p = round(w);
				if((abs(p.i)>rm) || (abs(p.j)>rm)	) Sp(h,k).p = w;
				else{
					if( B2.CheckSpot(Sp(h,k), searchguard, p) )	// 9, p
						B2.RefineSpotCenter(Sp(h,k), 3, 1, 0.9f);
					else {Sp(h,k).p=w; Sp(h,k).a=0;}
				// Predicted peak position
				SelectPen( hdc, RedPen );
				Ellipse(hdc, p.i-2+size/2, p.j-2+size/2, p.i+2+1+size/2, p.j+2+1+size/2);
				// Detected peak position
				SelectPen( hdc, BluePen );
				Ellipse(hdc, round(Sp(h,k).p.i-2)+size/2, round(Sp(h,k).p.j-2)+size/2, round(Sp(h,k).p.i+2+1)+size/2, round(Sp(h,k).p.j+2+1)+size/2);
				}
				h = -h; i = -i;
			}while(h<0);
			k = -k;	j=-j;
		}while(k<0);
	}
	ReleaseDC(himg, hdc);

// Create unbended image (resampling) -------------------------------------------
	SetDlgItemText( hDlg, IDC_UB_INFO, "7: Creating unbended image");
	B1.XC = B1.YC = size/2;
	memset(B2.P, average, size*size);
	t = 1./(ra.i*rb.j - ra.j*rb.i);	n = size/2;
	for(s=size/2-1,bp=B2.P, y=-n; y<n; y++) for(x=-n; x<n; x++, bp++){

		w.i = t*(x*rb.j - y*rb.i);	w.j = t*(y*ra.i - x*ra.j);

		hk = round(w);
		if( (abs(hk.i)>32)||(abs(hk.j)>32)) {
			*bp = average;
			continue;
		}

		u = Sp(hk.i+1, hk.j).p - Sp(hk.i, hk.j).p;
		v = Sp(hk.i, hk.j+1).p - Sp(hk.i, hk.j).p;
		w -= hk;
		w = Sp(hk.i, hk.j).p + u*(double)w.i + v*(double)w.j;

		if((fabs(w.i)>s) || (fabs(w.j)>s))
			*bp = average;
		else
			*bp = B1[-w];
	}

	SetCursor( hoc );

// Copying unbended image into destination's buffer
	memcpy( ODST.dp, B2.P, size*size );
	imgCalcMinMax( OBJ::GetOBJ(himg) );
	ActivateObj( himg );
	InvalidateRect( himg, NULL, FALSE );
	UpdateWindow( himg );

// To avoid freeing of the parent FT2R data memory
	FT1.BX.P = NULL;

	return TRUE;
}
/****************************************************************************/
static BOOL InitUBend( HWND hDlg, OBJ* O )
{
	OBJ*		pr = OBJ::GetOBJ(O->ParentWnd);
	LPSGINFO	S = (LPSGINFO) &O->dummy;
	LPLATTICE	P = (LPLATTICE) &pr->dummy;	// Parent is OREF type
	LPLATTICE	L = (LPLATTICE) malloc(sizeof(LATTICE));
	//char		wtitle[256], sname[20];

#ifdef USE_FONT_MANAGER
	HFONT hFont = FontManager::GetFont(FM_MEDIUM_FONT);
#else
	HFONT hFont = hfMedium;
#endif
	SendDlgItemMessage( hDlg, IDC_UB_A, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage( hDlg, IDC_UB_B, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage( hDlg, IDC_UB_G, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage( hDlg, IDC_UB_AXANGLE, WM_SETFONT, (WPARAM)hFont, 0);

	SetDlgItemDouble( hDlg, IDC_UB_APERTURE, 2.5, 0 );
	SetDlgItemDouble( hDlg, IDC_UB_ASLOPE, 0.5, 0 );

	if (L == NULL) return FALSE;

	memcpy(L, P, sizeof(LATTICE));
	S->dxs = DXS;
	O->bp = (LPBYTE)calloc(1, S->dxs*S->dxs);
	if ( ! O->bp ) return FALSE;

	memcpy (S, P->SI+P->Spg, sizeof(SGINFO));	// Make a copy of parent SpgInfo structure

	S->dxs = DXS;					// 256

// DMAP data memory
	S->dmap = O->bp;				// Old pointer no longer needed

	L->SI = (LPSGINFO)calloc(NUMSYMM+1, sizeof(SGINFO));
	if (L->SI==NULL) return FALSE;

	L->SI[L->Spg] = *S;

	Create_DMap( L,  1 );

	free( L->SI );

	free( L );
	L = NULL;

	S->dmap = O->bp;					// Old pointer no longer needed
	S->edge = TRUE;						// Cell edge is visible
/*
	strcpy(sname, rflSymmName(P->Spg, 0));
	if (strchr(sname,' ')) *strchr(sname,' ') = 0;
	sprintf(TMP,"%s - %s", sname, wtitle);
	strcpy(O->title, sname);
	SetWindowText( hWnd,  TMP );
*/
	UpdateNumbers( hDlg, O, S );
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
static void UpdateUBend( OBJ* O, LPSGINFO S )
{
}
/****************************************************************************/
static LRESULT ChangeItemColors( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
HDC		hdc = (HDC)wParam;
HWND	hwnd = (HWND)lParam;
int		idc = GetDlgCtrlID(hwnd);
LRESULT	ret;
COLORREF	col  = RGB(0,0,0);
DWORD	bcol = GetSysColor(COLOR_WINDOW);
static	BOOL brec=FALSE;

	if (brec) return FALSE;
	brec = TRUE;
	ret = DefDlgProc(hDlg, msg, wParam, lParam);
	switch ( idc ) {
		case IDC_UB_A:
			col = RGB(255,0,0);	bcol = RGB(192,192,192);
			break;
		case IDC_UB_B:
			col = RGB(0,0,255);	bcol = RGB(192,192,192);
			break;
		default: goto xxx;
	}
	SetTextColor(hdc, col);
//	SetBkColor(hdc, bcol);
xxx:
	brec = FALSE;
	return	ret;
}
/****************************************************************************/
LRESULT CALLBACK UBendWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ( hDlg );
	LPSGINFO	S = (LPSGINFO) &O->dummy;

	switch (message)
	{
	case WM_INITDIALOG:
		// ADDED BY PETER ON 29 JUNE 2004
		// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
		// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
		O = (OBJ*)lParam;
		S = (LPSGINFO) &O->dummy;
		InitializeObjectDialog(hDlg, O);

 		if (!InitUBend(hDlg, O))
		{
			DestroyWindow( hDlg );
			break;
		}
		return FALSE;

	case FM_Update:
		UpdateUBend( O, S );
		break;

	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
    	objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
    	break;

//	case WM_NOTIFY:
//		return MsgNotify( hDlg, message, wParam, lParam);

	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
		return ChangeItemColors( hDlg, message, wParam, lParam );

	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
	case WM_QUERYOPEN:
		ActivateObj(hDlg);
		break;

	case WM_NCDESTROY:
		if (O->bp) free( O->bp ); O->bp = NULL;
		objFreeObj( hDlg );
		break;

	case WM_PAINT:
		break;

	case WM_COMMAND:
		if (wndDefSysMenuProc(hDlg, wParam, lParam)) break;

	switch(LOWORD(wParam)) {
		case IDC_UB_UNBEND:
		DoUBend( hDlg, O, S );
		break;

		case IDOK:
		case IDCANCEL:
			DestroyWindow(hDlg);
		return TRUE;
	}
	}
	return FALSE;
}
