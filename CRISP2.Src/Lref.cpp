/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.LatRef * by ML ****************************************************/
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
#include "testutils.h"

static int	iDefH[4] = {1,0,1,1},
			iDefK[4] = {0,1,1,1},
			iDefh[4] = {0,0,0,1},
			iDefk[4] = {0,0,1,0};

#define	Acolor	RGB( 192,   0,   0)
#define	Bcolor	RGB(   0,   0, 192)
#define	acolor	RGB( 192,   0, 192)
#define	bcolor	RGB(   0, 128,   0)

#pragma optimize ("",off)

/****************************************************************************/
static	LPSTR szColNamesAB[3] = {"Refl","H","K"};
static	int   iColWidthAB[3]  = { 50,    40, 40};
static	LPSTR szColNamesab[5] = {"Refl","H","K","h","k"};
static	int   iColWidthab[5]  = { 40,    27, 27, 27, 27};

static	UINT	uReflStates[4] = {LVIS_GCCHECK1, LVIS_GCCHECK2, LVIS_GCCHECK3, LVIS_GCCHECK4};

#define	IDC_ML_SPIN	0x1234

/****************************************************************************/
static void PaintRadiusOnFFT( HWND hDlg, OBJ* O, LPLATTICE L)
{
	HDC		hDC;
	double	x0,y0,x1,y1;
	int     a0,b0,a1,b1;
	int		orop;
	HBRUSH	hob;
	HPEN	hop;

	hDC = GetDC( L->hFFTwnd );

	x0 = -L->rad; y0 = -L->rad;	x1 = L->rad; y1 = L->rad;

// added by Peter on 13 Sep 2001
	OBJ* pFFT = OBJ::GetOBJ(L->hFFTwnd);
	Fc2Sc( &x0, &y0, pFFT );//L->F );
	Fc2Sc( &x1, &y1, pFFT );//L->F );

	a0 = round(x0); a1=round(x1); b0 = round(y0); b1 = round(y1);

	orop = SetROP2( hDC, R2_XORPEN );
	hob = SelectBrush( hDC, GetStockObject(NULL_BRUSH));
	hop = SelectPen( hDC, RedPen);	Ellipse( hDC, a0, 	b0,	  a1,   b1);
//	SelectObject( hDC, GreenPen);	Ellipse( hDC, a0-1, b0+1, a1+1, b1-1);
	SelectObject( hDC, BluePen); 	Ellipse( hDC, a0+1, b0-1, a1-1, b1+1);
	SelectObject( hDC, hop );
	SelectObject( hDC, hob );
	SetROP2( hDC, orop );

	ReleaseDC( L->hFFTwnd, hDC );
	sprintf(TMP, "%d", round(L->rad));
	SetDlgItemText( hDlg, IDC_ML_RAD, TMP );
}
/****************************************************************************/
static	void	PaintReflsOnFFT( OBJ* O, LPLATTICE L)
{
	HDC		hDC;
	double	fontWidth, fontHeight;
	int     x, y;
	HPEN	hop;
	int		i;
// added by Peter on 13 Sep 2001
	OBJ* pFFT = OBJ::GetOBJ(L->hFFTwnd);

	hDC = GetDC( L->hFFTwnd );
	hop = SelectPen( hDC, RedPen );

	for(i = 0; i < 4; i++)
	{
		if (!L->bDefRefl[i])
			continue;
		fontWidth = L->pDefRefl[i].x;
		fontHeight = L->pDefRefl[i].y;
// changed by Peter on 13 Sep 2001
//		Fc2Sc( &fontWidth, &fontHeight, L->F );
		Fc2Sc( &fontWidth, &fontHeight, pFFT );
		x = round(fontWidth);
		y = round(fontHeight);
		if(i < 2) SelectObject( hDC, i    ? BluePen	: RedPen );
		else	  SelectObject( hDC,(i-2) ? GreenPen : MagentaPen );
		_MoveTo( hDC, x-1,  y-5 ); LineTo( hDC, x-1,  y-1 ); LineTo( hDC, x-6,  y-1 );
		_MoveTo( hDC, x+1,  y-5 ); LineTo( hDC, x+1,  y-1 ); LineTo( hDC, x+6,  y-1 );
		_MoveTo( hDC, x-5,  y+1 ); LineTo( hDC, x-1,  y+1 ); LineTo( hDC, x-1,  y+6 );
		_MoveTo( hDC, x+1,  y+5 ); LineTo( hDC, x+1,  y+1 ); LineTo( hDC, x+6,  y+1 );
	}
	SelectPen(hDC, hop);
	ReleaseDC( L->hFFTwnd, hDC );
}
/****************************************************************************/
void	xLine( HDC hdc, OBJ* O, double x0, double y0, double x1, double y1)
{
	Fc2Sc( &x0, &y0, O ); _MoveTo( hdc, round(x0), round(y0) );
	Fc2Sc( &x1, &y1, O ); LineTo( hdc, round(x1), round(y1) );
	LineTo( hdc, round(x1)-1, round(y1) );
}
/****************************************************************************/
static	void	PaintAxesOnFFT( OBJ* O, LPLATTICE L )
{
HDC		hDC;
int		orop, i, k;
double	r, n, x, y, dx, dy, ax, ay, al, ta, tb;
double	x0 = 0, y0 = 0;
HPEN	op;
#define	AA	0.4
#define	AL	12
#define	ML	5
	if (L->hFFTwnd == NULL)
		return;

// added by Peter on 13 Sep 2001
	OBJ* pFFT = OBJ::GetOBJ(L->hFFTwnd);

	if(!Bit(L->LR_Mode, LR_DONE)) return;
	hDC = GetDC( L->hFFTwnd );
	orop = SetROP2( hDC, R2_COPYPEN );
	r = L->ft2.S*.47;

	for(k = 0; k < 4; k++)
	{
		switch (k)
		{
			case 0:	ax = L->ax; ay = L->ay; al = L->al;
					op = SelectPen(hDC, RedPen2);
					break;
			case 1:	ax = L->bx; ay = L->by; al = L->bl;
					op = SelectPen(hDC, BluePen2);
					break;

			case 2:	ax = L->cx; ay = L->cy; al = L->cl; r *= .5;
					op = SelectPen(hDC, MagentaPen);
					x0 = L->ax0*L->H[2] + L->bx0*L->K[2];
					y0 = L->ay0*L->H[2] + L->by0*L->K[2];
					break;
			case 3:	ax = L->dx; ay = L->dy; al = L->dl;
					op = SelectPen(hDC, GreenPen);

		}
		if(al < 3.)
			continue;
		ta = atan2(ax, ay);
		tb = ta + M_PI * .5;
// Axis
		n = fabs(r / al);
		x = n * ax;
		y = n * ay;
//		xLine( hDC, L->F, x0-x, y0-y, x0+x, y0+y);
		xLine( hDC, pFFT, x0-x, y0-y, x0+x, y0+y);
// Arrow
		x += x0;
		y += y0;
		dx = AL * sin(ta+AA);
		dy = AL * cos(ta+AA);
//		xLine( hDC, L->F, x, y, x-dx, y-dy);
		xLine( hDC, pFFT, x, y, x-dx, y-dy);
		dx = AL * sin(ta-AA);
		dy = AL * cos(ta-AA);
//		xLine( hDC, L->F, x, y, x-dx, y-dy);
		xLine( hDC, pFFT, x, y, x-dx, y-dy);
// Markers
		dx = ML * sin(tb);
		dy = ML * cos(tb);
		n = fabs ( (r-AL-3) / al );
		for (i = (int)n; i > 0; i--)
		{
			x = i * ax + x0;
			y = i * ay + y0;
//			xLine( hDC, L->F, x+dx, y+dy, x-dx, y-dy);
			xLine( hDC, pFFT, x+dx, y+dy, x-dx, y-dy);
		}
	}
	SetROP2( hDC, orop );
	SelectPen( hDC, op );
	ReleaseDC( L->hFFTwnd, hDC );
}
/****************************************************************************/
void UpdateLatticeInfo( HWND hDlg, LPLATTICE L)
{
	BOOL	f = (L->num_dimensions >= 1);

//	sprintf( TMP, "A*=%4.1f ", L->al ); SetDlgItemText( hDlg, IDC_ML_AS , TMP );
//	sprintf( TMP, "B*=%4.1f ", L->bl ); SetDlgItemText( hDlg, IDC_ML_BS , TMP );
//	sprintf( TMP, "\202*=%4.1f ", R2D(L->gamma) );	SetDlgItemText( hDlg, IDC_ML_GS , TMP );
	TextToDlgItemW( hDlg, IDC_ML_AS, "A*=%4.1f%c ", L->al, ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_ML_BS, "B*=%4.1f%c ", L->bl, ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_ML_GS, "%c*=%4.1f%c ", GAMMA_CHAR, R2D(L->gamma), DEGREE_CHAR );

	EnableWindow( GetDlgItem(hDlg, IDC_ML_AS), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_NEGA), f);
	f = (L->num_dimensions >=2);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_BS), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_GS), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_NEGB), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_SWAPAB), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_CENTAB), f);
	if(!f) CheckDlgButton( hDlg, IDC_ML_CENTAB, FALSE);

	f = (L->num_dimensions >= 3);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_ASS), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_NEGAS), f);
	if( f ) TextToDlgItemW( hDlg, IDC_ML_ASS, "a*=%4.1f%c ", L->cl, ANGSTR_CHAR );
//	if (f) { sprintf( TMP, "a*=%4.1f ", L->cl ); SetDlgItemText( hDlg, IDC_ML_ASS , TMP ); }

	f = (L->num_dimensions == 4);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_BSS), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_GSS), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_NEGBS), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_SWAPABS), f);
	EnableWindow( GetDlgItem(hDlg, IDC_ML_CENTABS), f);
	if( f ) TextToDlgItemW( hDlg, IDC_ML_BSS, "b*=%4.1f%c ", L->dl, ANGSTR_CHAR );
	if( f ) TextToDlgItemW( hDlg, IDC_ML_GSS, "%c*=%4.1f%c ", GAMMA_CHAR, R2D(L->cdgamma), DEGREE_CHAR );
//	if (f) { sprintf( TMP, "b*=%4.1f ", L->dl ); SetDlgItemText( hDlg, IDC_ML_BSS , TMP ); }
//	if (f) { sprintf( TMP, "\201*=%4.1f ", R2D(L->cdgamma) );SetDlgItemText( hDlg, IDC_ML_GSS , TMP ); }

	if(f==0) CheckDlgButton( hDlg, IDC_ML_CENTABS, FALSE);
}
/****************************************************************************/
static	BOOL	DoLatticeRefine( HWND hDlg, OBJ* O, LPLATTICE L, BOOL go )
{
int		ret, f, Mode=0;
HCURSOR hc;

// Update parameters from parent
//	L->xs = F->fs;			// FFT size
//	L->ffti = F->dst;		// FFT data
//	L->MaxAmp = F->MaxAmp;

	hc = SetCursor( hcWait );
//	InfoText( IDC_I_TMP, "Lattice Refinement");

	BitFromID( hDlg, IDC_ML_SWAPAB,  Mode, ML_SWAPAB);
	BitFromID( hDlg, IDC_ML_SWAPABS, Mode, ML_SWAPab);
	BitFromID( hDlg, IDC_ML_NEGA,	 Mode, ML_NEGA);
	BitFromID( hDlg, IDC_ML_NEGAS,   Mode, ML_NEGa);
	BitFromID( hDlg, IDC_ML_NEGB,	 Mode, ML_NEGB);
	BitFromID( hDlg, IDC_ML_NEGBS,	 Mode, ML_NEGb);
	BitFromID( hDlg, IDC_ML_CENTAB,	 Mode, ML_CENTAB);
	BitFromID( hDlg, IDC_ML_CENTABS, Mode, ML_CENTab);
	NotBitFromID( hDlg, IDC_ML_CN2,	 Mode, ML_CENAB0);
	NotBitFromID( hDlg, IDC_ML_CN2S, Mode, ML_CENab0);
	NotBitFromID( hDlg, IDC_ML_CN1,  Mode, ML_CENAB1);
	NotBitFromID( hDlg, IDC_ML_CN1S, Mode, ML_CENab1);

	BitFromID( hDlg, IDC_ML_AUTO, L->LR_Mode, LR_AUTO);

	if ((L->LR_Mode & LR_AUTO) && go)
	{
		f = ~L->LR_Mode; f &= LR_DONE;
		ret = rflLatticeRefine( L, go, Mode);
	}
	else
		ret = rflLatticeRefine( L, go, Mode );
	SetCursor( hc );
	if(ret)
	{
		f = ret & 3;
		if(Mode & ML_CENTAB)
		{
			CheckDlgButton( hDlg, IDC_ML_CN1, (f==1));
			CheckDlgButton( hDlg, IDC_ML_CN2, (f==2));
			CheckDlgButton( hDlg, IDC_ML_CN3, (f==3));
			if(f)
				f = (L->LR_Mode & (LR_CEXAB0 | LR_CEXAB1)) / LR_CEXAB0;
		}
		EnableWindow( GetDlgItem(hDlg, IDC_ML_CN1), (f >=1));
		EnableWindow( GetDlgItem(hDlg, IDC_ML_CN2), (f >=2));
		EnableWindow( GetDlgItem(hDlg, IDC_ML_CN3), (f >=3));
		if(f == 0)
			CheckDlgButton( hDlg, IDC_ML_CENTAB, FALSE);

		f = (ret >> 2) & 3;
		if(Mode & ML_CENTab)
		{
			CheckDlgButton( hDlg, IDC_ML_CN1S, (f==1));
			CheckDlgButton( hDlg, IDC_ML_CN2S, (f==2));
			CheckDlgButton( hDlg, IDC_ML_CN3S, (f==3));
			if(f)
				f = (L->LR_Mode & (LR_CEXab0 | LR_CEXab1)) / LR_CEXab0;
		}
		if(f == 0)
			CheckDlgButton( hDlg, IDC_ML_CENTABS, FALSE);
		EnableWindow( GetDlgItem(hDlg, IDC_ML_CN1S), (f >= 1));
		EnableWindow( GetDlgItem(hDlg, IDC_ML_CN2S), (f >= 2));
		EnableWindow( GetDlgItem(hDlg, IDC_ML_CN3S), (f >= 3));
		BitFromID( hDlg, IDC_ML_CENTAB,  L->LR_Mode, LR_CENTAB);
		BitFromID( hDlg, IDC_ML_CENTABS, L->LR_Mode, LR_CENTab);

		UpdateLatticeInfo( hDlg, L );
		ret = 1;
//		if(go) MessageBeep(MB_ICONASTERISK);
	}
	else if (go)
	{
//		MessageBeep(MB_ICONEXCLAMATION);
		myError(IDS_ERRREFINELATTICE);
	}
//	EnableOREF( O );
	InitializeMenu( MainhWnd );
	InvalidateRect( L->hFFTwnd, NULL, FALSE );
	PaintAxesOnFFT( O, L );
	return ret;
}
/****************************************************************************/
// Modified by Peter on 29 Aug 2001
static void InitLREFList( HWND hDlg, OBJ* O, LPLATTICE L )
{
	HWND lvhWndAB = GetDlgItem( hDlg, IDC_ML_LIST_AB);
	HWND lvhWndab = GetDlgItem( hDlg, IDC_ML_LIST_AB2);
	int		i,j;
	UINT	state;

#ifdef USE_FONT_MANAGER
	int nFntSizeW;
	L->m_hFont = FontManager::GetFont(FM_MEDIUM_FONT_CW, nFntSizeW, L->m_nFontH);
	SendDlgItemMessage(hDlg, IDC_HKE_TEXT, WM_SETFONT, (WPARAM)L->m_hFont, 0);
#else
	L->m_nFontH = fntm_h;
	L->m_hFont = hfMedium;
	SendDlgItemMessage(hDlg, IDC_HKE_TEXT, WM_SETFONT, (WPARAM)hfMedium, 0);
#endif
	// initialize 'AB' lattice controls
	SendMessage( lvhWndAB, WM_SETREDRAW, FALSE, 0 );
	for(i = 0; i < 2; i++)
	{
		sprintf(TMP, "%d", i+1);
		ListView_SetItemText(lvhWndAB, i, 0, TMP  );
		state = L->bDefRefl[i] ? (uReflStates[i]) : LVIS_GCNOCHECK;
		ListView_SetItemState(lvhWndAB, i, state, LVIS_STATEIMAGEMASK);
	}
	for(j = 0; j < 2; j++)
	{
		sprintf(TMP, "%d", L->H[j]); ListView_SetItemText(lvhWndAB, j, 1, TMP );
		sprintf(TMP, "%d", L->K[j]); ListView_SetItemText(lvhWndAB, j, 2, TMP );
	}
	SendMessage( lvhWndAB, WM_SETREDRAW, TRUE, 0 );
	InvalidateRect( lvhWndAB, NULL, FALSE );
	// initialize 'ab' lattice controls
	SendMessage( lvhWndab, WM_SETREDRAW, FALSE, 0 );
	for(i = 0; i < 2; i++)
	{
		sprintf(TMP, "%d", i+3);
		ListView_SetItemText(lvhWndab, i, 0, TMP  );
		state = L->bDefRefl[i+2] ? (uReflStates[i+2]) : LVIS_GCNOCHECK;
		ListView_SetItemState(lvhWndab, i, state, LVIS_STATEIMAGEMASK);
	}
	for(j = 0; j < 4; j++)
	{
		sprintf(TMP, "%d", L->H[j+2]); ListView_SetItemText(lvhWndab, j, 1, TMP );
		sprintf(TMP, "%d", L->K[j+2]); ListView_SetItemText(lvhWndab, j, 2, TMP );
		sprintf(TMP, "%d", L->h[j+2]); ListView_SetItemText(lvhWndab, j, 3, TMP );
		sprintf(TMP, "%d", L->k[j+2]); ListView_SetItemText(lvhWndab, j, 4, TMP );
	}
	SendMessage( lvhWndab, WM_SETREDRAW, TRUE, 0 );

	InvalidateRect( lvhWndab, NULL, FALSE );
}
/****************************************************************************/
static void InitLREF( HWND hDlg, OBJ* O, LPLATTICE L )
{
	LV_COLUMN	col;
	LV_ITEM		lvi;
	HWND lvhWndAB = GetDlgItem( hDlg, IDC_ML_LIST_AB);
	HWND lvhWndab = GetDlgItem( hDlg, IDC_ML_LIST_AB2);
	HIMAGELIST	himlState;
	int			i;
	LPFFT		fft;

	memset(L, 0, sizeof(LATTICE) );

	L->hFFTwnd = objFindParent( O, OT_FFT );
	L->F = OBJ::GetOBJ( O->ParentWnd );
//	L->F = OBJ::GetOBJ( L->hFFTwnd );	// added by Peter on 9 Sep 2001
	fft = GetFFT( L->F );
	L->ft2 = fft->ft2;
	L->rad = L->ft2.S >> 1;

	memcpy(L->H, iDefH, sizeof(iDefH));
	memcpy(L->K, iDefK, sizeof(iDefK));
	memcpy(L->h, iDefh, sizeof(iDefh));
	memcpy(L->k, iDefk, sizeof(iDefk));

	himlState = ImageList_Create(16, 16, ILC_COLORDDB | ILC_MASK, 5, 0);
	ImageList_AddMasked (himlState, LoadBitmap (hInst,"CHECKBOX"), RGB (255,255,0));
	ListView_SetImageList(lvhWndAB, himlState, LVSIL_STATE);
	ListView_SetImageList(lvhWndab, himlState, LVSIL_STATE);

	SetWindowLong( lvhWndAB, GWL_STYLE, GetWindowLong( lvhWndAB, GWL_STYLE) | LVS_SHOWSELALWAYS );
	SetWindowLong( lvhWndab, GWL_STYLE, GetWindowLong( lvhWndab, GWL_STYLE) | LVS_SHOWSELALWAYS );

	ListView_SetExtendedListViewStyle( lvhWndAB,
		ListView_GetExtendedListViewStyle( lvhWndAB ) |
		/*LVS_EX_GRIDLINES | */LVS_EX_FULLROWSELECT );
	ListView_SetExtendedListViewStyle( lvhWndab,
		ListView_GetExtendedListViewStyle( lvhWndab ) |
		/*LVS_EX_GRIDLINES | */LVS_EX_FULLROWSELECT );

	memset(&col, 0, sizeof(LV_COLUMN));
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt  = LVCFMT_LEFT;
	col.cchTextMax = 0;
	// 'AB' lattice
	for(i = 0; i < 3; i++)
	{
		col.iSubItem = i;
		col.cx = iColWidthAB[i];
		col.pszText = szColNamesAB[i];
		ListView_InsertColumn(lvhWndAB, i, &col);
	}
	// 'ab' lattice
	for(i = 0; i < 5; i++)
	{
		col.iSubItem = i;
		col.cx = iColWidthab[i];
		col.pszText = szColNamesab[i];
		ListView_InsertColumn(lvhWndab, i, &col);
	}

	memset(&lvi, 0, sizeof(LV_ITEM));
	lvi.mask = LVIF_TEXT;
	for(i = 0; i < 2; i++)
	{
		lvi.iItem = i;
		lvi.pszText = TMP;
		sprintf(TMP, "%d", i+1);
		ListView_InsertItem(lvhWndAB, &lvi );
		sprintf(TMP, "%d", i+3);
		ListView_InsertItem(lvhWndab, &lvi );
	}

	InitLREFList( hDlg, O, L );
	CheckDlgButton( hDlg, IDC_ML_AUTO, TRUE );
	EnableWindow( GetDlgItem( hDlg,IDC_ML_LIST_AB), !IsDlgButtonChecked(hDlg,IDC_ML_AUTO));
	EnableWindow( GetDlgItem( hDlg,IDC_ML_LIST_AB2), !IsDlgButtonChecked(hDlg,IDC_ML_AUTO));

#ifdef USE_FONT_MANAGER
	HFONT hFont = FontManager::GetFont(FM_NORMAL_FONT);
	HFONT hFontGreek = FontManager::GetFont(FM_GREEK_FONT);
#else
	HFONT hFont = hfNormal;
	HFONT hFontGreek = hfGreek;
#endif
//	SendDlgItemMessage(hDlg, IDC_ML_AS,  WM_SETFONT, (WPARAM)hfMedium, 0);
//	SendDlgItemMessage(hDlg, IDC_ML_BS,  WM_SETFONT, (WPARAM)hfMedium, 0);
//	SendDlgItemMessage(hDlg, IDC_ML_GS,  WM_SETFONT, (WPARAM)hfMedium, 0);
//	SendDlgItemMessage(hDlg, IDC_ML_ASS, WM_SETFONT, (WPARAM)hfMedium, 0);
//	SendDlgItemMessage(hDlg, IDC_ML_BSS, WM_SETFONT, (WPARAM)hfMedium, 0);
//	SendDlgItemMessage(hDlg, IDC_ML_GSS, WM_SETFONT, (WPARAM)hfMedium, 0);
	SendDlgItemMessage(hDlg, IDC_ML_AS, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ML_BS, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ML_GS, WM_SETFONT, (WPARAM)hFontGreek, 0);
	SendDlgItemMessage(hDlg, IDC_ML_ASS, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ML_BSS, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ML_GSS, WM_SETFONT, (WPARAM)hFontGreek, 0);

	TextToDlgItemW( hDlg, IDC_ML_AS, "A*=0.00%c ", L->al, ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_ML_BS, "B*=0.00%c ", L->bl, ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_ML_GS, "%c*=0.00%c ", GAMMA_CHAR, R2D(L->gamma), DEGREE_CHAR );

	TextToDlgItemW( hDlg, IDC_ML_ASS, "a*=0.00%c ", L->al, ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_ML_BSS, "b*=0.00%c ", L->bl, ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_ML_GSS, "%c*=0.00%c ", GAMMA_CHAR, R2D(L->gamma), DEGREE_CHAR );

	CheckDlgButton( hDlg, IDC_ML_BRAD, TRUE );

	InvalidateRect( L->hFFTwnd, NULL, FALSE );
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
		case IDC_ML_CENTAB:	case IDC_ML_CN1: case IDC_ML_CN2: case IDC_ML_CN3:
		case IDC_ML_AS:	 case IDC_ML_NEGA:
			pt = RedPen;		col = Acolor;	break;
		case IDC_ML_BS:  case IDC_ML_NEGB:
			pt = BluePen;		col = Bcolor;	break;
		case IDC_ML_ASS: case IDC_ML_NEGAS:
			pt = MagentaPen;	col = acolor;	break;
		case IDC_ML_CENTABS: case IDC_ML_CN1S: case IDC_ML_CN2S: case IDC_ML_CN3S:
		case IDC_ML_BSS: case IDC_ML_NEGBS:
			pt = GreenPen;		col = bcolor;	break;
		case IDC_ML_SWAPAB:	case IDC_ML_SWAPABS: break;
		default: goto xxx;
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
static void UpdateSpin( HWND hDlg, LPLATTICE L, int code )
{
	int d=0, *a, f, ee;

	switch( L->spinsubitem )
	{
		case 1: a=L->H; break;
		case 2: a=L->K; break;
		case 3: a=L->h; break;
		case 4: a=L->k; break;
		default: return;
	}
	a += L->spinitem;
	switch( code )
	{
		case JC_UP:   d= 1; break;
		case JC_DOWN: d=-1; break;
	}
	ee = GetDlgItemInt( hDlg, IDC_ML_SPIN, &f, TRUE );
	if (f) *a = ee;
	*a += d;
	SetDlgItemInt( hDlg, IDC_ML_SPIN, *a, TRUE );
}
/****************************************************************************/
// Changed by Peter on 29 Aug 2001
static void CreateSpin(  HWND hDlg, int IDC, OBJ* O, LPLATTICE L )
{
	HWND	hwndLV = GetDlgItem( hDlg, IDC );
	RECT	rc;

	ListView_GetSubItemRect( hwndLV, L->spinitem, L->spinsubitem, LVIR_LABEL, (LPARAM)&rc );
	MapWindowPoints( hwndLV, hDlg, (LPPOINT)&rc, 2 );
	L->hwSpin=CreateWindow("SHT_SPINEDIT","1",
		WS_VISIBLE | WS_CHILD/* | WS_OVERLAPPEDWINDOW  | WS_CLIPSIBLINGS*/,
		rc.left, rc.top, rc.right-rc.left+11, rc.bottom-rc.top,
		hDlg, (HMENU)IDC_ML_SPIN, hInst, O);
	SendDlgItemMessage( hDlg, IDC_ML_SPIN, WM_SETFONT,
		(WPARAM)SendMessage(hwndLV, WM_GETFONT, 0, 0), MAKELPARAM(TRUE,0));
	UpdateSpin( hDlg, L, 0 );
	EnableWindow( hwndLV, FALSE);
}
/****************************************************************************/
// Changed by Peter on 29 Aug 2001
static void CheckSpinCancel( HWND hDlg, int IDC, OBJ* O, LPLATTICE L, LPARAM lParam )
{
	POINT	p = {(int)LOWORD(lParam), (int)HIWORD(lParam) };

	if (L->hwSpin)
	{
		DestroyWindow( L->hwSpin );
		L->hwSpin = NULL;
		InitLREFList( hDlg, O, L );
		EnableWindow( GetDlgItem( hDlg, IDC), !IsDlgButtonChecked(hDlg, IDC_ML_AUTO));
	}
}
/****************************************************************************/
static LRESULT MsgNotify(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPNM_LISTVIEW	pnm    = (LPNM_LISTVIEW)lParam;
	HWND			hwndLV = pnm->hdr.hwndFrom;
	DWORD			dwpos;
	LV_HITTESTINFO	lvhti;
	int				item;
	OBJ*			O = OBJ::GetOBJ( hDlg );
	LPLATTICE		L = GetLATTICE( O );
	RECT			rc;
	BOOL			bUpdate = TRUE;

	if (wParam != IDC_ML_LIST_AB && wParam != IDC_ML_LIST_AB2)
		return 0;

	// Find out where the cursor was
	dwpos = GetMessagePos();
	lvhti.pt.x = LOWORD(dwpos);
	lvhti.pt.y = HIWORD(dwpos);
	MapWindowPoints(HWND_DESKTOP, hwndLV, &lvhti.pt, 1);
	// Now do a hittest with this point.
	item = ListView_SubItemHitTest(hwndLV, &lvhti);

	switch (pnm->hdr.code)
	{
	case LVN_ENDLABELEDIT:
		return TRUE;

	case NM_KILLFOCUS:
		item = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED );
		if (item >= 0) ListView_SetItemState( hwndLV, item, 0, LVIS_SELECTED);
		break;

	case NM_DBLCLK:
	case NM_CLICK:

		if (lvhti.flags & LVHT_ONITEMSTATEICON && pnm->hdr.code==NM_DBLCLK)
		{
			// Now lets get the state from the item and toggle it.
			switch(wParam)
			{
			case IDC_ML_LIST_AB:  L->bDefRefl[item] = FALSE; break;
			case IDC_ML_LIST_AB2: L->bDefRefl[item+2] = FALSE; break;
			}
		}
		if (item >= 0)
		{
			CheckDlgButton( hDlg, IDC_ML_BRAD, FALSE );
			ListView_SetItemState( hwndLV, item, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			InitLREFList( hDlg, O, L );
		}
		break;

	case NM_RCLICK:
		if (item >= 0 && lvhti.iSubItem > 0)
		{
			HMENU	hmenu;
			int		d=0, *a;

			ListView_GetSubItemRect( hwndLV, item, lvhti.iSubItem, LVIR_LABEL, (LPARAM)&rc );
			MapWindowPoints( hwndLV, NULL, (LPPOINT)&rc, 2 );
			switch(wParam)
			{
			case IDC_ML_LIST_AB:  L->spinitem = item;   break;
			case IDC_ML_LIST_AB2: L->spinitem = item+2; break;
			}
			L->spinsubitem = lvhti.iSubItem;
			hmenu = LoadMenu( hInst, "REFLINDEX" );
			switch(TrackPopupMenu( GetSubMenu( hmenu, 0 ),
							TPM_LEFTALIGN | TPM_VCENTERALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
							rc.right, rc.top, 0, hDlg, NULL )) {
			case IDM_RI_0:	d=0; break; case IDM_RI_1:	d=1; break; case IDM_RI_2:	d=2; break;
			case IDM_RI_3:	d=3; break; case IDM_RI_4:	d=4; break; case IDM_RI_5:	d=5; break;
			case IDM_RI_6:	d=6; break; case IDM_RI_7:	d=7; break; case IDM_RI_8:	d=8; break;
			case IDM_RI_9:	d=9; break; case IDM_RI_10:	d=10; break;
			case IDM_RI_M1: d=-1; break; case IDM_RI_M2: d=-2; break; case IDM_RI_M3: d=-3; break;
			case IDM_RI_M4: d=-4; break; case IDM_RI_M5: d=-5; break; case IDM_RI_M6: d=-6; break;
			case IDM_RI_M7: d=-7; break; case IDM_RI_M8: d=-8; break; case IDM_RI_M9: d=-9; break;
			case IDM_RI_M10: d=-10; break;
			case IDM_RI_SPIN:	CreateSpin( hDlg, wParam, O, L );
								bUpdate = FALSE; break;
			default: bUpdate = FALSE; break;
			}
			if(bUpdate)
			{
				bUpdate = TRUE;
				switch( L->spinsubitem )
				{
				case 1: a = L->H; break;
				case 2: a = L->K; break;
				case 3: a = L->h; break;
				case 4: a = L->k; break;
				default: bUpdate = FALSE; break;
				}
				if(bUpdate)
				{
					a += L->spinitem;
					*a = d;
					InitLREFList( hDlg, O, L );
				}
			}
			DestroyMenu( hmenu );
		}
		break;
	default:
		break;
	}
	return 0;
}
/****************************************************************************/
LRESULT CALLBACK LRefWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
	OBJ*		O = OBJ::GetOBJ( hDlg );
	LPLATTICE	L;
	HWND		hlvAB = GetDlgItem(hDlg, IDC_ML_LIST_AB);
	HWND		hlvab = GetDlgItem(hDlg, IDC_ML_LIST_AB2);
	if( O != NULL )
	{
		L = GetLATTICE( O );
	}

	switch (message)
	{
	case WM_INITDIALOG:
		// ADDED BY PETER ON 29 JUNE 2004
		// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
		// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
		O = (OBJ*)lParam;
		L = GetLATTICE( O );
		InitializeObjectDialog(hDlg, O);

		InitLREF(hDlg, O, L);
		wndChangeDlgSize( hDlg, hDlg, 0,
						  IsDlgButtonChecked(hDlg,IDC_ML_MORE) ? 0:IDC_ML_SMALL_AB_LATTICE, "LREF", hInst);
		DoLatticeRefine( hDlg, O, L, TRUE );	// Some one does not want us to do Refine at create time
		return TRUE;

	case FM_ParentLBUTTONUP:
		CheckDlgButton( hDlg, IDC_ML_BRAD, FALSE );
		break;

	case FM_Update:
		L->ft2 = GetFFT( OBJ::GetOBJ( O->ParentWnd ) )->ft2;
		if (DoLatticeRefine( hDlg, O, L, TRUE ))
			objInformChildren( hDlg, FM_Update, (WPARAM)O, lParam);
		break;

	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
    	objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
    	break;

	case FM_ParentInspect:
		{
			// 'AB' lattice
			int i = ListView_GetNextItem(hlvAB, -1, LVNI_SELECTED );
			if (i >= 0)
			{
				L->pDefRefl[i].x = LOINT(lParam);
				L->pDefRefl[i].y = HIINT(lParam);
				if (lrCheckRefl( L->ft2, &L->pDefRefl[i].x, &L->pDefRefl[i].y, -1 ))
				{
					L->bDefRefl[i] = TRUE;
					ListView_SetItemState( hlvAB, i, 0, LVIS_SELECTED);
					InitLREFList( hDlg, O, L );
					InvalidateRect( L->hFFTwnd, NULL, FALSE );
				}
				else
					Beep( 1000, 10 );
			}
			else if (IsDlgButtonChecked( hDlg, IDC_ML_BRAD ))
			{
				PaintRadiusOnFFT( hDlg, O, L );
				L->rad = sqrt((double) LOINT(lParam)*LOINT(lParam) + HIINT(lParam)*HIINT(lParam));
				L->rad = __min(L->rad, L->ft2.S/2.);
				PaintRadiusOnFFT( hDlg, O, L );
			}
			// 'ab' lattice
			i = ListView_GetNextItem(hlvab, -1, LVNI_SELECTED );
			if (i >= 0)
			{
				L->pDefRefl[i+2].x = LOINT(lParam);
				L->pDefRefl[i+2].y = HIINT(lParam);
				if (lrCheckRefl( L->ft2, &L->pDefRefl[i+2].x, &L->pDefRefl[i+2].y, -1 ))
				{
					L->bDefRefl[i+2] = TRUE;
					ListView_SetItemState( hlvab, i, 0, LVIS_SELECTED);
					InitLREFList( hDlg, O, L );
					InvalidateRect( L->hFFTwnd, NULL, FALSE );
				}
				else
					Beep( 1000, 10 );
			}
			else if (IsDlgButtonChecked( hDlg, IDC_ML_BRAD ))
			{
				PaintRadiusOnFFT( hDlg, O, L );
				L->rad = sqrt((double) LOINT(lParam)*LOINT(lParam) + HIINT(lParam)*HIINT(lParam));
				L->rad = __min(L->rad, L->ft2.S/2.);
				PaintRadiusOnFFT( hDlg, O, L );
			}
		}
		return TRUE;

	case FM_ParentPaint:
		PaintAxesOnFFT( O, L );
		PaintReflsOnFFT( O, L );
		PaintRadiusOnFFT( hDlg, O, L );
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		CheckSpinCancel( hDlg, IDC_ML_LIST_AB, O, L, lParam );
		CheckSpinCancel( hDlg, IDC_ML_LIST_AB2, O, L, lParam );
		break;

	case WM_NOTIFY:
		return MsgNotify( hDlg, message, wParam, lParam);

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
		if (IsWindow(L->hFFTwnd)) InvalidateRect( L->hFFTwnd, NULL, FALSE );
		if (L->rptr) free(L->rptr); L->rptr = NULL;
		objFreeObj( hDlg );
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDC_ML_MORE:
				wndChangeDlgSize( hDlg, hDlg, 0,
								  IsDlgButtonChecked(hDlg,IDC_ML_MORE) ? 0:IDC_ML_SMALL_AB_LATTICE, "LREF", hInst);
				break;
			case IDC_ML_SPIN:
				if (HIWORD(wParam)==SE_MOVE) UpdateSpin( hDlg, L, LOWORD(lParam) );
				break;

			case IDC_ML_CENTAB:  case IDC_ML_SWAPAB:
			case IDC_ML_NEGA:    case IDC_ML_NEGB:
			case IDC_ML_CN1:     case IDC_ML_CN2:    case IDC_ML_CN3:
			case IDC_ML_CENTABS: case IDC_ML_SWAPABS:
			case IDC_ML_NEGAS:   case IDC_ML_NEGBS:
			case IDC_ML_CN1S:    case IDC_ML_CN2S:   case IDC_ML_CN3S:
				DoLatticeRefine( hDlg, O, L, FALSE );
				objInformChildren( hDlg, FM_Update, (WPARAM)O, lParam);
				break;

			case IDC_ML_AUTO:
				EnableWindow( GetDlgItem(hDlg,IDC_ML_LIST_AB), !IsDlgButtonChecked(hDlg,IDC_ML_AUTO));
				EnableWindow( GetDlgItem(hDlg,IDC_ML_LIST_AB2), !IsDlgButtonChecked(hDlg,IDC_ML_AUTO));
				CheckSpinCancel( hDlg, IDC_ML_LIST_AB, O, L, 0 );
				CheckSpinCancel( hDlg, IDC_ML_LIST_AB2, O, L, 0 );
				break;

			case IDC_ML_REFINE:
				DoLatticeRefine( hDlg, O, L, TRUE );
				objInformChildren( hDlg, FM_Update, (WPARAM)O, lParam);
				return TRUE;

			case IDC_ORIGIN_REFINEMENT:
				RunMenuCommand(IDM_C_OREF);
				return TRUE;

			case IDOK:
			case IDCANCEL:
				DestroyWindow(hDlg);
			return TRUE;
		}
		break;
	}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("LRefWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
