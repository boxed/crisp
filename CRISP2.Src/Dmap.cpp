/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.DensityMap * by ML ************************************************/
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

#include "HelperFn.h"

#pragma optimize ("",off)
/****************************************************************************/
/* Density map window *******************************************************/
/****************************************************************************/
extern void img8_CreateCtrMap( LPVOID lpimg, LPVOID lptmp, int xs, int ys, int steps );

/****************************************************************************/
static	void	UpdateNumbers( HWND hDlg, OBJ* O, LPSGINFO S )
{
	double xx = O->NI.Scale * 1e10;

	SetDlgItemDouble( hDlg, IDC_DM_AON, S->shx, -2);
	SetDlgItemDouble( hDlg, IDC_DM_BON, S->shy, -2);
	SetDlgItemDouble( hDlg, IDC_DM_ZMN, S->scale, -2);
	SetDlgItemDouble( hDlg, IDC_DM_RON, R2D(S->rot), -1);

//	sprintf( TMP, "a=%-.2fÅ", S->ral*xx);   SetDlgItemText( hDlg, IDC_DM_A, TMP);
//	sprintf( TMP, "b=%-.2fÅ", S->rbl*xx);   SetDlgItemText( hDlg, IDC_DM_B, TMP);
//	sprintf( TMP, "\201=%-.1f", R2D(S->rgamma));SetDlgItemText( hDlg, IDC_DM_G, TMP);
//	sprintf( TMP, "a-x angle=%5.1f", R2D(S->aw));SetDlgItemText( hDlg, IDC_DM_AXANGLE, TMP);
	TextToDlgItemW(hDlg, IDC_DM_A, "a=%6.2f%c", FastAbs(S->ral*xx), ANGSTR_CHAR);
	TextToDlgItemW(hDlg, IDC_DM_B, "b=%6.2f%c", FastAbs(S->rbl*xx), ANGSTR_CHAR);
	TextToDlgItemW(hDlg, IDC_DM_G, "%c=%6.1f%c", GAMMA_CHAR, FastAbs(R2D(S->rgamma)), DEGREE_CHAR);
	TextToDlgItemW(hDlg, IDC_DM_AXANGLE, "a-x angle=%5.1f%c", R2D(S->aw), DEGREE_CHAR);

	sprintf( TMP, "%d", O->x);	SetDlgItemText( hDlg, IDC_DM_XSIZE, TMP);
	sprintf( TMP, "%d", O->y);	SetDlgItemText( hDlg, IDC_DM_YSIZE, TMP);
	CheckDlgButton ( hDlg, IDC_DM_NEG, S->flg & DM_NEG );
	CheckDlgButton ( hDlg, IDC_DM_CELLEDGE, S->edge );
	SetDlgItemInt( hDlg, IDC_DM_CMSTEPS, S->cmsteps, FALSE );
	CheckDlgButton ( hDlg, IDC_DM_CONTMAP, S->contmap );
}

/****************************************************************************/
static	BOOL	ScanAll( HWND hDlg, OBJ* O, LPSGINFO S, HWND hWnd )
{
	double	ff;
	BOOL	ret = FALSE;
	int		zz;
	BOOL	it;
	BOOL	bChangeSize=FALSE;

	if ( GetDlgItemDouble( hDlg, IDC_DM_AON, &ff, -2) && S->shx   != ff ) { S->shx   = ff; ret = TRUE; }
	if ( GetDlgItemDouble( hDlg, IDC_DM_BON, &ff, -2) && S->shy   != ff ) { S->shy   = ff; ret = TRUE; }
	if ( GetDlgItemDouble( hDlg, IDC_DM_ZMN, &ff, -2) && S->scale != ff ) { S->scale = ff; ret = TRUE; }
	if ( GetDlgItemDouble( hDlg, IDC_DM_RON, &ff, -1) && S->rot   != D2R(ff) ) { S->rot = D2R(ff); ret = TRUE; }

	if (S->scale<0.01) S->scale = 0.01;
	zz = GetDlgItemInt( hDlg, IDC_DM_CMSTEPS, &it, FALSE);
	if (it && S->cmsteps != zz) {
		if (zz<0) zz=2;
		if (zz>64) zz=64;
		zz = (int)pow(2.0,round(log((double)zz)/log(2.0)));
		S->cmsteps = zz;
		ret = TRUE;
	}

	zz = GetDlgItemInt( hDlg, IDC_DM_XSIZE, &it, FALSE);
	if (it && O->x!=zz) {
		if (zz<0)    zz = 64;
		if (zz>2048) zz = 2048;
		O->x = zz; bChangeSize = TRUE;

	}

	zz = GetDlgItemInt( hDlg, IDC_DM_YSIZE, &it, FALSE);
	if (it && O->y!=zz) {
		if (zz<0)    zz = 64;
		if (zz>2048) zz = 2048;
		O->y = zz; bChangeSize = TRUE;
	}

	if (bChangeSize) wndSetWindowSize( hWnd, O->x, O->y, FALSE );
	if (ret) UpdateNumbers( hDlg, O, S );
	return ret;
}
/****************************************************************************/
static	BOOL	SetDefault( HWND hDlg, OBJ* O, LPSGINFO S )
{
	BOOL	ret = FALSE;
	if ( S->shx   != 0. ) { S->shx   = 0.;  ret = TRUE; }
	if ( S->shy   != 0. ) { S->shy   = 0.;  ret = TRUE; }
	if ( S->scale != 1.5) { S->scale = 1.5; ret = TRUE; }
	if ( S->rot   != 0. ) { S->rot   = 0.;  ret = TRUE; }

	if (ret) UpdateNumbers( hDlg, O, S );
	return ret;
}
/****************************************************************************/
void ImgToCnrMap( LPVOID lpimg, int xs, int ys, int steps )
{
LPVOID lptmp;

	if ((lptmp=calloc(1, xs*ys)) == NULL ) return;

	im8CreateCtrMap( lpimg, lptmp, xs, ys, 256/steps );

	memcpy( lpimg, lptmp, xs*ys);

	free( lptmp );
}
/****************************************************************************/
static	void	ChangeBitMap( HWND hWnd, OBJ* O, LPSGINFO S )
{
	HDC		hdc;
	static	myBITMAPINFO	bi= { 40, 0, 0, 1, 8, BI_RGB, 0, 0, 0, 0, 0 };
	HPALETTE	ohpal;
	int		i, xs, ys, ofs = 0;
	int		li, ls;
	RECT	r;

	if( ::IsIconic(hWnd) == TRUE )
		return;
	GetClientRect( hWnd, &r );
	if( (r.bottom - r.top) == 0 ||
		(r.right  - r.left) == 0 )
	{
		return;
	}

	xs = r.right;
	ys = r.bottom;
	xs = R4(xs);
	O->x = xs;
	O->y = ys;
#ifdef USE_XB_LENGTH
	O->xb = xs;
#else
	O->stride = R4(xs);
#endif
	if (O->dp)
		free(O->dp);

	if (O->hDlg)
		UpdateNumbers( O->hDlg, O, S );

	O->dp = (LPBYTE)calloc( 1, xs*ys );
	bi.biBitCount = 8;
	bi.biClrUsed  = COLORS;
	bi.biClrImportant = COLORS;
	bi.biSizeImage = xs * ys;
	bi.biWidth  = xs;
	bi.biHeight = -ys;

	if (O->palno < 0)
	{
		ofs = 0;
		ls = GRAYSIZ;
	}
	else
	{
		ofs = GRAYSIZ+O->palno*USERSIZ;
		ls = USERSIZ;
	}
	for(i = 0; i < 256; i++)
	{
		// changed by Peter on 19 Sep 2001
//		li = ((i*ls)/256 + O->bright - 128)*O->contrast/128;
		li = ( (((i*ls)>>8) + (O->bright*ls)/255 - (ls>>1))*O->contrast)>>7;
		if (li<0) li=0;
		if (li>=ls) li=ls-1;
		li += ofs;
		bi.index[i] = (BYTE)li;
	}

	hdc = GetDC( hWnd );
	if (O->hbit)
		DeleteObject( O->hbit );

	DMap_To_Img256( S, O->dp, xs, ys);

	if (S->contmap) ImgToCnrMap( O->dp, xs, ys, S->cmsteps );

	ohpal = SelectPalette(hdc, hMyPal, FALSE);
	RealizePalette(hdc);

	O->hbit = CreateDIBitmap( hdc, (LPBITMAPINFOHEADER)&bi, CBM_INIT, O->dp,
							 (LPBITMAPINFO)&bi, DIB_PAL_COLORS);

	if (ohpal) SelectPalette( hdc, ohpal, FALSE );

	ReleaseDC( hWnd, hdc );
	InvalidateRect( hWnd, NULL, FALSE );
}
/****************************************************************************/
static	void	PaintDMAP( HWND hWnd, OBJ* O, LPSGINFO S )
{
	RECT	r;
	HDC		hdc, memdc;
	POINT	pt[5];
	int		orop;
	HPEN	open, w3_pen;
	HBITMAP	obmp;

	if (O->hbit == NULL) ChangeBitMap( hWnd, O, S );

	hdc = GetDC(hWnd);
	GetClientRect( hWnd, &r );

	memdc = CreateCompatibleDC( hdc );
	obmp = SelectBitmap( memdc, O->hbit );
	BitBlt(hdc, 0, 0, r.right, r.bottom, memdc, 0, 0, SRCCOPY);
	SelectObject( memdc, obmp );
	DeleteDC( memdc );

	// Now draw Cell box
	if ( S->edge )
	{
		Fract2Client(S, r.right, r.bottom, 0., 1., &pt[0].x, &pt[0].y);
		Fract2Client(S, r.right, r.bottom, 1., 1., &pt[1].x, &pt[1].y);
		Fract2Client(S, r.right, r.bottom, 1., 0., &pt[2].x, &pt[2].y);
		Fract2Client(S, r.right, r.bottom, 0., 0., &pt[3].x, &pt[3].y);
//		for (i=0;i<4;i++) pt[i].y = r.bottom - pt[i].y - 1;
		pt[4]=pt[0];

		w3_pen	= CreatePen(PS_SOLID, 3, RGB(255,255,255));
		orop = SetROP2(hdc, R2_COPYPEN);
		open = SelectPen(hdc, w3_pen );

		Polyline( hdc, pt, 5);
		SetROP2(hdc, R2_BLACK);	SelectObject(hdc, open);
		Polyline( hdc, pt, 5);

		DeleteObject( w3_pen );	SetROP2( hdc, orop );
	}
	ReleaseDC( hWnd, hdc );
	ValidateRect( hWnd, NULL );
}
/****************************************************************************/
static	BOOL	Fit2Image( HWND hDlg, OBJ* O, LPSGINFO S, HWND hWnd )
{
	BOOL	ret = FALSE;
	RECT	r;
	double	a, b, scale, scale_x, scale_y, cos_g, sin_g, sr, cr, rgamma;
	HWND	hWndIMG;
	OBJ*	pr;
	int		mins;

	// find parent window with the image
	hWndIMG = objFindParent( O, OT_IMG );
	if( !hWndIMG )
		return FALSE;

	GetClientRect(hWnd, &r);

	// TODO: do we need this?
	rgamma = S->rgamma;
	sr = sin(rgamma);
	cr = cos(rgamma);
	// TODO: do we need fabs here?
	cos_g = fabs(cos(S->rgamma));
	sin_g = fabs(sin(S->rgamma));
	// minimal image size
	mins = __min(r.right,r.bottom);

	a = S->ral;
	b = S->rbl * sin_g;

	// TODO: use rotation angle for the image: S->rot!!!
	if( cos_g > 1e-4 )
		scale = (double)r.right / (S->rbl * cos_g);
	else
		scale = 1000;

	// Changed by Peter on 28 Aug 2001
//	scale_x = (double)r.right  / S->ral;
//	scale_y = (double)r.bottom / (S->rbl * sin_g);
	scale_x = (double)mins / a;
	scale_y = (double)mins / b;
	scale   = __min(scale_y,__min(scale,scale_x));

	// get the image
	pr = OBJ::GetOBJ( hWndIMG );
	scale *= (double)pr->scd / (double)pr->scu;
	if ( S->scale !=  scale )
	{
		S->scale = scale;
		ret = TRUE;
	}
	if ( S->rot != -M_PI - S->aw )
	{
		S->rot = -M_PI - S->aw;
		ret = TRUE;
	}

	if (ret)
		UpdateNumbers( hDlg, O, S );
	return ret;
}
/****************************************************************************/
BOOL CALLBACK DMapDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND ParentWnd = (HWND)GetWindowLong( hDlg, DWL_USER );
	OBJ*	O;
	LPSGINFO S = NULL;

	if (ParentWnd)
	{
		O = OBJ::GetOBJ(ParentWnd);
		S = (LPSGINFO)&O->dummy;
	}

	switch (message)
	{
		case WM_INITDIALOG:
			{
				// ADDED BY PETER ON 29 JUNE 2004
				// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
				// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
				//			O = (OBJ*)lParam;
				//			S = (LPEDRD) &O->dummy;
				O = OBJ::GetOBJ((HWND)lParam);
				S = (LPSGINFO)&O->dummy;
				InitializeObjectDialog(hDlg, O);
				
				//			O = OBJ::GetOBJ((HWND)lParam);
				//			S = (LPSGINFO)&O->dummy;
				S->cmsteps = 8;
				UpdateNumbers( hDlg, O, S );
				SetWindowLong( hDlg, DWL_USER, lParam);
				// 			SendDlgItemMessage( hDlg, IDC_DM_A, WM_SETFONT, (WPARAM)hfMedium, 0);
				// 			SendDlgItemMessage( hDlg, IDC_DM_B, WM_SETFONT, (WPARAM)hfMedium, 0);
				// 			SendDlgItemMessage( hDlg, IDC_DM_G, WM_SETFONT, (WPARAM)hfMedium, 0);
				// 			SendDlgItemMessage( hDlg, IDC_DM_AXANGLE, WM_SETFONT, (WPARAM)hfMedium, 0);
				wndPlaceToolWindow( (HWND)lParam, hDlg );
				
#ifdef USE_FONT_MANAGER
				HFONT hFont = FontManager::GetFont(FM_NORMAL_FONT);
				HFONT hFontGreek = FontManager::GetFont(FM_GREEK_FONT);
#else
				HFONT hFont = hfNormal;
				HFONT hFontGreek = hfGreek;
#endif
				SendDlgItemMessage(hDlg, IDC_DM_A, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_DM_B, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_DM_G, WM_SETFONT, (WPARAM)hFontGreek, 0);
				SendDlgItemMessage(hDlg, IDC_DM_AXANGLE, WM_SETFONT, (WPARAM)hFont, 0);
			}

			return (TRUE);

		case WM_DESTROY:
			O->hDlg = 0;
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					DestroyWindow(hDlg);
					O->hDlg = 0;
					return (TRUE);

				case IDC_DM_DEFAULT:
					if ( SetDefault( hDlg, O, S ) )
						ChangeBitMap( ParentWnd, O, S );
					break;

				case IDC_DM_FIT:
					if ( Fit2Image( hDlg, O, S, ParentWnd ) )
						ChangeBitMap( ParentWnd, O, S );
					break;

				case IDC_DM_APPLAY:
					if ( ScanAll( hDlg, O, S, ParentWnd ) )
						ChangeBitMap( ParentWnd, O, S );
					break;

				case IDC_DM_CELLEDGE:
					S->edge = IsDlgButtonChecked( hDlg, IDC_DM_CELLEDGE );
					InvalidateRect( ParentWnd, NULL, FALSE );
					break;
				case IDC_DM_CONTMAP:
					S->contmap = IsDlgButtonChecked( hDlg, IDC_DM_CONTMAP );
					ChangeBitMap( ParentWnd, O, S );
					break;
				case IDC_DM_NEG:
					if ( IsDlgButtonChecked( hDlg, IDC_DM_NEG ) ) S->flg |= DM_NEG;
					else										  S->flg &=~DM_NEG;
					ChangeBitMap( ParentWnd, O, S );
					break;
			}
			break;
		case WM_ACTIVATE:
			if (IsWindow(ParentWnd) && wParam==2) ActivateObj( ParentWnd );
			break;
	}
	return FALSE;
}
/****************************************************************************/
BOOL	InitDMAPObject( HWND hWnd, OBJ* O )
{
	OBJ*		pr = OBJ::GetOBJ(O->ParentWnd);
	LPSGINFO	S = (LPSGINFO) &O->dummy;
	LPLATTICE	P = (LPLATTICE) &pr->dummy;	// Parent is OREF type
	LPLATTICE	L = (LPLATTICE) malloc(sizeof(LATTICE));
	char		wtitle[256], sname[20];

	if (L == NULL) return FALSE;

	memcpy(L,P,sizeof(LATTICE));
	S->dxs = 256;
	O->bp = (LPBYTE)calloc(1, S->dxs*S->dxs);
	O->dp = NULL; // Parent' pointer no longer valid
	if ( ! O->bp ) return FALSE;

	if (pr->ID==OT_OREF)
	{
		*S = P->SI[P->Spg];				// Make a copy of parent SpgInfo structure
		S->dxs = DXS;

	// DMAP data memory
		S->dmap = O->bp;				// Old pointer no longer needed

		L->SI = (LPSGINFO)calloc(NUMSYMM+1, sizeof(SGINFO));
		if (L->SI==NULL) return FALSE;

		L->SI[L->Spg] = *S;

		Create_DMap( L,  1 );

		free( L->SI );
	}
	else if (pr->ID==OT_AXTL)
	{
		*S = *(LPSGINFO)( pr->dummy );
		S->dxs = DXS;
		memcpy(O->bp, S->dmap, S->dxs*S->dxs );
	}
	free( L );
	L = NULL;

	S->dmap = O->bp;					// Old pointer no longer needed
	S->edge = TRUE;						// Cell edge is visible

	SetWindowLong( hWnd, GWL_STYLE,  CS_DBLCLKS | (GetWindowLong(hWnd, GWL_STYLE) & ~WS_MAXIMIZEBOX) );
	wndSetWindowSize( hWnd, 256, 256, FALSE );

	GetWindowText( hWnd, wtitle, sizeof(wtitle));
	strcpy(sname, rflSymmName(P->Spg, 0));
	if (strchr(sname,' ')) *strchr(sname,' ') = 0;
	sprintf(TMP,"%s - %s", sname, wtitle);
	strcpy(O->title, sname);
	SetWindowText( hWnd,  TMP );

	ShowWindow( hWnd, SW_SHOW);

	PostMessage( hWnd, FM_Update, 0, 0);

	return	TRUE;
}
/****************************************************************************/
static void ChangeStyle( HWND hWnd )
{

#define	ws_ok	(WS_DLGFRAME | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX)

LONG	style   = GetWindowLong( hWnd, GWL_STYLE );
RECT	r;

	GetClientRect(hWnd, &r);
	ClientToScreen(hWnd, (LPPOINT)&r.left);
	ClientToScreen(hWnd, (LPPOINT)&r.right);

	if (style & WS_THICKFRAME) {
		style = ( style & ~ws_ok );
		SetParent( hWnd, MainhWnd );
	} else {
		style = ( style |  ws_ok );
		SetParent( hWnd, MDIhWnd );
	}
	SetWindowLong( hWnd, GWL_STYLE, style);
	AdjustWindowRectEx( &r, style, FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
	ScreenToClient( GetParent(hWnd), (LPPOINT)&r.left);
	ScreenToClient( GetParent(hWnd), (LPPOINT)&r.right);
	SetWindowPos( hWnd, NULL, r.left,r.top,r.right-r.left,r.bottom-r.top, SWP_NOZORDER|SWP_DRAWFRAME);
}
/****************************************************************************/
LRESULT CALLBACK DMapWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
	OBJ*		O = OBJ::GetOBJ(hWnd);
	LPSGINFO	S = (LPSGINFO)&O->dummy;

	switch(message)
	{
	case WM_CREATE:
		if ( ! InitDMAPObject( hWnd, O )) return -1;
		break;

	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
		if (O->hDlg)	UpdateNumbers( O->hDlg, O, S );
    	objInformChildren( hWnd, FM_UpdateNI, wParam, lParam);
    	break;

	case WM_DESTROY:
		if ( O->hDlg)   DestroyWindow( O->hDlg ); O->hDlg = NULL;
		if ( O->bp )    free( O->bp ); O->bp = NULL;
		if ( O->dp )    free( O->dp ); O->dp = NULL;
		if ( O->hbit)   DeleteObject( O->hbit ); O->hbit = NULL;
		break;

	case WM_NCDESTROY:
		objFreeObj( hWnd, TRUE );
		goto CallDCP;

	case WM_PAINT:
		if( ::IsIconic(hWnd) == FALSE )
		{
			PaintDMAP( hWnd, O, S );
			objInformChildren( hWnd, FM_ParentPaint, (WPARAM)O, 0 );
		}
		break;

	case WM_NCHITTEST:
		if (GetWindowLong(hWnd, GWL_STYLE) & WS_THICKFRAME) goto CallDCP;
		return HTCAPTION;

	case WM_NCLBUTTONDBLCLK:
		if (wParam != HTCAPTION) goto CallDCP;

	case WM_LBUTTONDBLCLK:
		if ( ::IsIconic( hWnd ) ) goto CallDCP;
		ChangeStyle( hWnd );
		goto CallDCP;

	case WM_RBUTTONUP:
		wndCreateObjMenu(hWnd, TRUE, FALSE, TRUE, lParam);
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_MOUSEMOVE:
		{	// Information printout
			double	dxy, dc;
			double	x0, y0, x, y, xc, yc, k, af, bf;
			RECT	r;

			GetClientRect( hWnd, &r );
			x0 = r.right*.5; y0 = r.bottom*.5;
			xc = x = LOWORD(lParam);
			yc = y = HIWORD(lParam);
			k = Get_DCenter( S, r.right, r.bottom, &xc, &yc, &af, &bf, &dxy, &dc, FALSE );
			sprintf(TMP, "%+5.3f,%+5.3f, Den=%3.0f", af, bf, dxy);
			tobSetInfoText( ID_INFO, TMP);
			// It means inspecting FFT map (care for FMAP and LENS)
			if ((wParam & (MK_LBUTTON | MK_SHIFT | MK_MBUTTON)))
				objInformChildren( hWnd, FM_ParentInspect, wParam, lParam);
		}
		break;

	case WM_GETMINMAXINFO:
		// Min track size
		if ( O )
		{
			RECT r;
			SetRect(&r, 0, 0, 64, 64);
			AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = r.right-r.left;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = r.bottom-r.top;
		}
		break;

	case WM_SIZE:
		ChangeBitMap( hWnd, O, S );
		break;

/*		case WM_SETFOCUS:
			if ( Mhwnd ) SetFocus( Mhwnd );
			break;

		case WM_INITMENU:
			InitColorMenu( O );
			break;

*/
	case WM_SYSCOMMAND:
	case WM_COMMAND:
		if (wndDefSysMenuProc(hWnd, wParam, lParam)) break;
		if (wParam==IDM_OM_TUNE) {
			if  (O->hDlg)  BringWindowToTop(O->hDlg);
			else O->hDlg = CreateDialogParam(hInst, "DMAP", hWnd, DMapDlgProc, (LPARAM)hWnd);
			break;
		}
		goto CallDCP;

	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
	case WM_QUERYOPEN:
		ActivateObj(hWnd);
		goto CallDCP;

	case WM_ERASEBKGND:
		return FALSE;

	case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS wp = (LPWINDOWPOS)lParam;
			// Setting this bit prevents(?) BitBlt while moving
			wp->flags |= SWP_NOCOPYBITS;
		}
		goto CallDCP;
/*
	case WM_COMMAND:
		switch (wParam) {
			case IDM_C_AMTR: CreateObj( OT_AMTR, hWnd, "Atomic Meter", 0, 0 ); break;
			case IDM_F_SAVE:
			{
				SaveImage si;
				RECT	r;
				LPSTR	lpImg;
				int		bufwidth;

				GetClientRect( hWnd, &r );
				bufwidth = (r.right+3)&0xFFFC;
				lpImg = calloc(bufwidth*(r.bottom+1));
				if (lpImg) {
					DMap_To_Img256( S, lpImg, bufwidth, r.bottom);
					si.fname = NULSTR;
					si.buf = lpImg;
					si.x0 = si.y0 = 0;
					si.x = bufwidth;
					si.cx = r.right;
					si.y = si.cy = r.bottom;
					si.NI = &O->NI;
					si.pix = 1;
					SaveImageFile( &si );
					free(lpImg);
				}
			}
			break;
		}
		break;
*/
	default:
CallDCP:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("DMapWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
