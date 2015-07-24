/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Atomic meter window * by ML **********************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "rgconfig.h"
#include "fft.h"
#include "lattice.h"

#include "HelperFn.h"

#if defined _DEBUG

#include "Complex/Complex_tmpl.h"
#include "Complex/BM_templ.h"

static void test()
{
	BMF bmf;
	BOOL bRes;
	int iRes;
	SPOT p;

	// 8-bit binary matrix test
	{
		BM8 bm;
		BYTE b1 = bm[0];
		BYTE b2 = bm[ICOMPLEX(0, 0)];
		BYTE b3 = bm(0, 0);
		float f1 = bm[0.0f];
		float f2 = bm(0.0f, 0.0f);
		float f3 = bm[FCOMPLEX(0.0f, 0.0f)];
		bm.PutPixel(FCOMPLEX(0.0f, 0.0f), 0.0f);
		bm.AddPixel(FCOMPLEX(0.0f, 0.0f), 0.0f);
		bRes = bm.Load(bmf, 1.0f);
		bm.Alloc(256, 256);
		bRes = bm.MulBlob(bmf);
		iRes = bm.GetRDis(NULL, 0, FCOMPLEX(0, 0));
		float f4 = bm.GetRDis1(NULL, 0, FCOMPLEX(0, 0));
		iRes = bm.CheckSpot(p, 0, FCOMPLEX(0, 0));
		iRes = bm.CheckSpot1(p, 0, FCOMPLEX(0, 0));
		f4 = bm.GetSpotSN(p, 0);
		f4 = bm.GetSpotSN1(p, 0);
		iRes = bm.RefineSpotCenter(p, 0, 0, 0.0f);
	}
	// 16-bit binary matrix test
	{
		BM16 bm;
		WORD b1 = bm[0];
		WORD b2 = bm[ICOMPLEX(0, 0)];
		WORD b3 = bm(0, 0);
		float f1 = bm[0.0f];
		float f2 = bm(0.0f, 0.0f);
		float f3 = bm[FCOMPLEX(0.0f, 0.0f)];
		bm.PutPixel(FCOMPLEX(0.0f, 0.0f), 0.0f);
		bm.AddPixel(FCOMPLEX(0.0f, 0.0f), 0.0f);
		bm.Load(bmf, 1.0f);
		bRes = bm.Load(bmf, 1.0f);
		bm.Alloc(256, 256);
		bRes = bm.MulBlob(bmf);
		iRes = bm.GetRDis(NULL, 0, FCOMPLEX(0, 0));
		float f4 = bm.GetRDis1(NULL, 0, FCOMPLEX(0, 0));
		iRes = bm.CheckSpot(p, 0, FCOMPLEX(0, 0));
		iRes = bm.CheckSpot1(p, 0, FCOMPLEX(0, 0));
		f4 = bm.GetSpotSN(p, 0);
		f4 = bm.GetSpotSN1(p, 0);
		iRes = bm.RefineSpotCenter(p, 0, 0, 0.0f);
	}
	// float binary matrix test
	{
		BMF bm;
		float b1 = bm[0];
		float b2 = bm[ICOMPLEX(0, 0)];
		float b3 = bm(0, 0);
		float f1 = bm[0.0f];
		float f2 = bm(0.0f, 0.0f);
		float f3 = bm[FCOMPLEX(0.0f, 0.0f)];
		bm.PutPixel(FCOMPLEX(0.0f, 0.0f), 0.0f);
		bm.AddPixel(FCOMPLEX(0.0f, 0.0f), 0.0f);
		bRes = bm.Load(bmf, 1.0f);
		bm.Alloc(256, 256);
		bRes = bm.MulBlob(bmf);
		iRes = bm.GetRDis(NULL, 0, FCOMPLEX(0, 0));
		float f4 = bm.GetRDis1(NULL, 0, FCOMPLEX(0, 0));
		iRes = bm.CheckSpot(p, 0, FCOMPLEX(0, 0));
		iRes = bm.CheckSpot1(p, 0, FCOMPLEX(0, 0));
		f4 = bm.GetSpotSN(p, 0);
		f4 = bm.GetSpotSN1(p, 0);
		iRes = bm.RefineSpotCenter(p, 0, 0, 0.0f);
	}
	// SPOT binary matrix test
	{
		BMSP bm;
		SPOT b1 = bm[0];
		SPOT b2 = bm[ICOMPLEX(0, 0)];
		SPOT b3 = bm(0, 0);
		float f1 = bm[0.0f];
		float f2 = bm(0.0f, 0.0f);
		float f3 = bm[FCOMPLEX(0.0f, 0.0f)];
		bm.PutPixel(FCOMPLEX(0.0f, 0.0f), 0.0f);
		bm.AddPixel(FCOMPLEX(0.0f, 0.0f), 0.0f);
		bRes = bm.Load(bmf, 1.0f);
		bm.Alloc(256, 256);
		bRes = bm.MulBlob(bmf);
		iRes = bm.GetRDis(NULL, 0, FCOMPLEX(0, 0));
		float f4 = bm.GetRDis1(NULL, 0, FCOMPLEX(0, 0));
		iRes = bm.CheckSpot(p, 0, FCOMPLEX(0, 0));
		iRes = bm.CheckSpot1(p, 0, FCOMPLEX(0, 0));
		f4 = bm.GetSpotSN(p, 0);
		f4 = bm.GetSpotSN1(p, 0);
		iRes = bm.RefineSpotCenter(p, 0, 0, 0.0f);
	}
}

#endif _DEBUG

/****************************************************************************/
#define	MAXA	99			// Maximum number of atoms
typedef struct
{
	double	x0, y0;			// Origin coordinates
	double	far *af;		// a coordinates of atoms
	double	far *bf;		// b coordinates of atoms
	double	far *d;			// Density of atoms
	BOOL	far *flg;		// Atom flags
	int		nat;			// number of placed atoms
	int		nact;			// active one
	double	dx1, dy1;		// First  atom in distance meter
	double	dx2, dy2;		// Second atom in distance meter
	double	ral, rbl, rgamma;// Cell parameters
	HFONT	m_hFont;		// reference only
	int		m_nFontW;
	int		m_nFontH;

} AMETER, *LPAMETER;

//      	          "##  +1.345  +1.345  123"
static	char	szTitlFmt[] = " #   x/a     y/b    Den  Ref";
static	char	szListFmt[] = "%2d  %+6.3f  %+6.3f  %3.0f  %c";

/****************************************************************************/
static	void	DrawOneAtomCross( LPAMETER S, OBJ* O, int n, BOOL asactive )
{
	OBJ*		pr = OBJ::GetOBJ( O->ParentWnd );
	HDC			phdc = GetDC( O->ParentWnd );
	HDC			hdc = CreateCompatibleDC( phdc );
	LPSGINFO	SI = (LPSGINFO)(pr->dummy);
	int			orop, obkm;
	RECT		r;
	LONG		x, y;
	int			c = S->m_nFontH / 2 + 1;
	int			sx = S->m_nFontW*3+c+2, sy = S->m_nFontH+c+2;
	int			cx = c+1, cy = S->m_nFontH+1;
// 	int			c = fntm_h/2+1;
// 	int			sx = fntm_w*3+c+2, sy = fntm_h+c+2;
// 	int			cx = c+1, cy = fntm_h+1;
	HFONT		oldf;
	HBITMAP		hobmp, hbmp = CreateCompatibleBitmap( hdc, sx, sy );

	GetClientRect( O->ParentWnd, &r );
	hobmp = SelectBitmap( hdc, hbmp );
	PatBlt( hdc, 0, 0, sx, sy, BLACKNESS );

	Fract2Client(SI, r.right, r.bottom, S->af[n], S->bf[n], &x, &y);
//	y = r.bottom - y - 1;
	orop = SetROP2( hdc, R2_WHITE );
	SetTextColor( hdc, RGB(255,255,255));
	if ( asactive )
	{
		_MoveTo( hdc, cx-1,  cy-c ); LineTo( hdc, cx-1,  cy-1 ); LineTo( hdc, cx-c-1,  cy-1 );
		_MoveTo( hdc, cx+1,  cy-c ); LineTo( hdc, cx+1,  cy-1 ); LineTo( hdc, cx+c+1,  cy-1 );
		_MoveTo( hdc, cx-c,  cy+1 ); LineTo( hdc, cx-1,  cy+1 ); LineTo( hdc, cx-1,    cy+c+1 );
		_MoveTo( hdc, cx+1,  cy+c ); LineTo( hdc, cx+1,  cy+1 ); LineTo( hdc, cx+c+1,  cy+1 );
	}
	else
	{
		_MoveTo( hdc, cx,   cy-c ); LineTo( hdc, cx,     cy+c+1 );
		_MoveTo( hdc, cx-c, cy );   LineTo( hdc, cx+c+1, cy );
	}

	obkm = SetBkMode( hdc, TRANSPARENT );
	oldf = SelectFont( hdc, S->m_hFont );
// 	oldf = SelectFont( hdc, hfMedium );
	sprintf( TMP, "%d", n+1);
	TextOut(hdc, cx+2, 1, TMP, strlen(TMP));
	SelectObject( hdc, oldf );
	SetBkMode( hdc, obkm );
	SetROP2( hdc, orop );

	BitBlt( phdc, x-cx, y-cy, sx, sy, hdc, 0, 0, SRCINVERT );

	DeleteObject( SelectObject( hdc, hobmp));
	ReleaseDC( O->ParentWnd, phdc );
	DeleteDC( hdc );
}
/****************************************************************************/
static	void	DrawAtomsCross( LPAMETER S, OBJ* O )
{
	int		i;
	for(i=0; i<S->nat; i++)	DrawOneAtomCross( S, O, i, (i==S->nact) );
}
/****************************************************************************/
static	void	DrawDistanceLine( LPAMETER S, OBJ* O )
{
OBJ*		pr = OBJ::GetOBJ( O->ParentWnd );
HDC			hdc = GetDC( O->ParentWnd );
LPSGINFO	SI = (LPSGINFO)(pr->dummy);
int			orop;
RECT		r;
LONG		x1, y1, x2, y2;

	GetClientRect( O->ParentWnd, &r );
	Fract2Client(SI, r.right, r.bottom, S->dx1, S->dy1, &x1, &y1);
	Fract2Client(SI, r.right, r.bottom, S->dx2, S->dy2, &x2, &y2);
//	y1 = r.bottom - y1 - 1;
//	y2 = r.bottom - y2 - 1;
	orop = SetROP2( hdc, R2_NOT );
	_MoveTo( hdc, x1,  y1 ); LineTo( hdc, x2,  y2 );
	SetROP2( hdc, orop);
	ReleaseDC( O->ParentWnd, hdc );
}
/****************************************************************************/
static	void	UpdateList( HWND hDlg , OBJ* O, LPAMETER S)
{
int		i;
LPSGINFO SI = (LPSGINFO)(OBJ::GetOBJ(O->ParentWnd)->dummy);

	SendDlgItemMessage( hDlg, IDC_AM_LIST, WM_SETREDRAW, 0, 0);
   	SendDlgItemMessage( hDlg, IDC_AM_LIST, LB_RESETCONTENT, 0, 0);

	SetDlgItemText( hDlg, IDC_AM_TEXT, szTitlFmt);
	for (i = 0; i<S->nat; i++ ) {
		sprintf(TMP, szListFmt, i+1, S->af[i], S->bf[i], S->d[i], S->flg[i]?'R':' ' );
    	SendDlgItemMessage(hDlg, IDC_AM_LIST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPSTR)TMP);
    }

	SendDlgItemMessage( hDlg, IDC_AM_LIST, LB_SETCURSEL, S->nact, 0 );
	SendDlgItemMessage( hDlg, IDC_AM_LIST, WM_SETREDRAW, 1, 0);
	InvalidateRect( GetDlgItem(hDlg, IDC_AM_LIST), NULL, TRUE);
}
/****************************************************************************/
static	void	ChangeListItem( HWND hDlg , OBJ* O, LPAMETER S, int n)
{
LPSGINFO SI = (LPSGINFO)(OBJ::GetOBJ(O->ParentWnd)->dummy);

	SendDlgItemMessage( hDlg, IDC_AM_LIST, WM_SETREDRAW, 0, 0);
   	SendDlgItemMessage( hDlg, IDC_AM_LIST, LB_DELETESTRING, n, 0);

	sprintf(TMP, szListFmt, n+1, S->af[n], S->bf[n], S->d[n], S->flg[n]?'R':' ' );
   	SendDlgItemMessage( hDlg, IDC_AM_LIST, LB_INSERTSTRING, n, (LPARAM)(LPSTR)TMP);
	SendDlgItemMessage( hDlg, IDC_AM_LIST, WM_SETREDRAW, 1, 0 );

	SendDlgItemMessage( hDlg, IDC_AM_LIST, LB_SETCURSEL, n, 0 );
}
/****************************************************************************/
static	void	UpdateCell( HWND hDlg , OBJ* O, LPAMETER S)
{
	double xx = O->NI.Scale * 1e10;

	TextToDlgItemW(hDlg, IDC_AM_AR, "%5.2f%c", S->ral*xx, ANGSTR_CHAR);
	TextToDlgItemW(hDlg, IDC_AM_BR, "%5.2f%c", S->rbl*xx, ANGSTR_CHAR);
	TextToDlgItemW(hDlg, IDC_AM_G,  "%5.1f%c", R2D(fabs(S->rgamma)), DEGREE_CHAR );
//	sprintf(TMP, "%5.2f", S->ral*xx ); SetDlgItemText( hDlg, IDC_AM_AR, TMP);
//	sprintf(TMP, "%5.2f", S->rbl*xx ); SetDlgItemText( hDlg, IDC_AM_BR, TMP);
//	sprintf(TMP, "gamma=%5.1f", R2D(fabs(S->rgamma)) ); SetDlgItemText( hDlg, IDC_AM_G, TMP);
}
/****************************************************************************/
static	void	UpdateDistance( HWND hDlg , OBJ* O, LPAMETER S)
{
double xx = O->NI.Scale * 1e10;

	double a = (S->dx1 - S->dx2)*S->ral*xx;
	double b = (S->dy1 - S->dy2)*S->rbl*xx;
	double gam = S->rgamma;
	double d;
	a = a+b*(double)cos(S->rgamma);
	b =   b*(double)sin(S->rgamma);
	d = (double)sqrt(a*a+b*b);

	sprintf(TMP, "1 %+6.3f %+6.3f", S->dx1, S->dy1);
	SetDlgItemText( hDlg, IDC_AM_DISA1, TMP);
	sprintf(TMP, "2 %+6.3f %+6.3f", S->dx2, S->dy2);
	SetDlgItemText( hDlg, IDC_AM_DISA2, TMP);

	TextToDlgItemW(hDlg, IDC_AM_DISTANCE, "Dist = %6.2f%c", d, ANGSTR_CHAR);
//	sprintf(TMP, "Dist = %6.2f≈", d);
//	SetDlgItemText( hDlg, IDC_AM_DISTANCE, TMP);
}
/****************************************************************************/
static	BOOL	SaveAmeter( HWND hDlg, OBJ* O, LPAMETER S,  LPSTR fname )
{
FILE	*out;
int		i;

	if ((out = fopen( fname, "w")) == NULL) return dlgError(IDS_ERRCREATEFILE, hDlg);
	fprintf(out, ";%s\n", szTitlFmt );
	fprintf(out, "-----------------------------\n");
	for (i = 0; i < S->nat; i++ ) {
		fprintf(out, szListFmt, i+1, S->af[i], S->bf[i], S->d[i], S->flg[i]?'R':' ' );
		fprintf(out, "\n");
    }
	if (ferror(out)){ fclose(out); return dlgError(IDS_ERRWRITEFILE, hDlg); }
	fclose(out);
	return TRUE;
}
/****************************************************************************/
static	BOOL	LoadAmeter( HWND hDlg, OBJ* O, LPAMETER S, LPSTR fname )
{
	FILE	*in;
	if ((in = fopen( fname, "r")) == NULL) return dlgError(IDS_ERROPENFILE, hDlg);
	if (feof(in))	{ fclose(in); return dlgError(IDS_ERRREADFILE, hDlg); }
	
	const int bufSize = 1024;
	char buf[bufSize];
	S->nat = 0;
	while (fgets(buf, bufSize, in) != NULL)
	{
		int len = strlen(buf);
		if (len == 0)
			continue;
		if (buf[0] == '\n')
			continue;
		if (buf[0] ==  ';')
			continue;

		if (len >= 26)
		{
			if (buf[0] ==  '-' || buf[1] ==  '-')
				continue;
			// "%2d  %+6.3f  %+6.3f  %3.0f  %c";
			
			memset(TMP, 0, TMPSIZE); strncpy(TMP, buf, 2);
			int index = atoi(TMP);
			assert(index == S->nat);

			memset(TMP, 0, TMPSIZE); strncpy(TMP, buf+4, 7);
			S->af[S->nat] = atof(TMP);

			memset(TMP, 0, TMPSIZE); strncpy(TMP, buf+12, 7);
			S->bf[S->nat] = atof(TMP);

			memset(TMP, 0, TMPSIZE); strncpy(TMP, buf+20, 3);
			S->d[S->nat] = atof(TMP);

			S->flg[S->nat] = buf[25] == 'R';

			S->nat++;
		}
	}

	UpdateList(hDlg, O, S);

	fclose(in);
	return TRUE;
}
/****************************************************************************/
static	BOOL InitAMTRDialog( HWND hDlg , OBJ* O )
{
	OBJ*	pr  = OBJ::GetOBJ(O->ParentWnd);
	LPAMETER S  = (LPAMETER) &O->dummy;
	LPSGINFO SI = (LPSGINFO)(OBJ::GetOBJ(O->ParentWnd)->dummy);

// DMAP data memory
	memset(S,0,sizeof(AMETER));
	O->bp = (LPBYTE)calloc( 1, MAXA * (sizeof(double) * 3 + sizeof(int) * 1) );
	if ( ! O->bp ) return FALSE;
	S->af = (LPDOUBLE)O->bp;
	S->bf = S->af+MAXA;
	S->d  = S->bf+MAXA;
	S->flg = (BOOL far *)(S->d+MAXA);

	S->nat = 1;						// Initially one dummy atom
	S->ral = SI->ral;
	S->rbl = SI->rbl;
	S->rgamma = SI->rgamma;

#ifdef USE_FONT_MANAGER
	S->m_hFont = FontManager::GetFont(FM_MEDIUM_FONT, S->m_nFontW, S->m_nFontH);
	HFONT hFontNormal = FontManager::GetFont(FM_NORMAL_FONT);
	HFONT hFontGreek = FontManager::GetFont(FM_GREEK_FONT);
#else
	HFONT hFontNormal = hfNormal;
	HFONT hFontGreek = hfGreek;
	S->m_hFont = hfMedium;
	S->m_nFontW = fntm_w;
	S->m_nFontH = fntm_h;
#endif

	SendDlgItemMessage(hDlg, IDC_AM_LIST,     WM_SETFONT, (WPARAM)S->m_hFont, 0);
	SendDlgItemMessage(hDlg, IDC_AM_TEXT,     WM_SETFONT, (WPARAM)S->m_hFont, 0);
	SendDlgItemMessage(hDlg, IDC_AM_DISTITLE, WM_SETFONT, (WPARAM)S->m_hFont, 0);
	SendDlgItemMessage(hDlg, IDC_AM_DISA1,    WM_SETFONT, (WPARAM)S->m_hFont, 0);
	SendDlgItemMessage(hDlg, IDC_AM_DISA2,    WM_SETFONT, (WPARAM)S->m_hFont, 0);

	SendDlgItemMessage(hDlg, IDC_AM_DISTANCE, WM_SETFONT, (WPARAM)hFontNormal, 0);

	SendDlgItemMessage(hDlg, IDC_GAMMA_TXT,  WM_SETFONT, (WPARAM)hFontGreek, 0);
	TextToDlgItemW(hDlg, IDC_GAMMA_TXT, "%c*=", GAMMA_CHAR);

	SendDlgItemMessage(hDlg, IDC_AM_AR, WM_SETFONT, (WPARAM)hFontNormal, 0);
	SendDlgItemMessage(hDlg, IDC_AM_BR, WM_SETFONT, (WPARAM)hFontNormal, 0);
	SendDlgItemMessage(hDlg, IDC_AM_G,  WM_SETFONT, (WPARAM)hFontGreek, 0);

// 	SendDlgItemMessage(hDlg, IDC_AMTR_A_ANGSTR, WM_SETFONT, (WPARAM)hfNormal, 0);
// 	SendDlgItemMessage(hDlg, IDC_AMTR_B_ANGSTR, WM_SETFONT, (WPARAM)hfNormal, 0);
	UpdateCell    ( hDlg, O, S );
	UpdateList    ( hDlg, O, S );
	UpdateDistance( hDlg, O, S );

	return TRUE;
}
/****************************************************************************/
LRESULT CALLBACK AmtrWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	 O = OBJ::GetOBJ(hDlg);
	LPAMETER S = (LPAMETER) &O->dummy;

	switch (message)
	{
		case WM_INITDIALOG:
			// ADDED BY PETER ON 29 JUNE 2004
			// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
			// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
			O = (OBJ*)lParam;
			S = (LPAMETER) &O->dummy;
			InitializeObjectDialog(hDlg, O);

			InitAMTRDialog(hDlg, O);
			DrawAtomsCross( S, O );
//			TextToDlgItemW( hDlg, IDC_AMTR_A_ANGSTR, L"%c", ANGSTR_CHAR);
//			TextToDlgItemW( hDlg, IDC_AMTR_B_ANGSTR, L"%c", ANGSTR_CHAR);
//			SetDlgItemText( hDlg, IDC_AMTR_A_ANGSTR, "\302");
//			SetDlgItemText( hDlg, IDC_AMTR_B_ANGSTR, "\302");
			return (TRUE);

		case WM_NCDESTROY:
			if (O->bp)   free( O->bp ); O->bp = NULL;
			if (O->ParentWnd)  InvalidateRect(O->ParentWnd, NULL, FALSE );
			objFreeObj(hDlg);
			break;

		case FM_UpdateNI:
			O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
			UpdateCell    ( hDlg, O, S );
			UpdateDistance( hDlg, O, S );
    		objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
    		break;

		case FM_ParentPaint:
			DrawAtomsCross( S, O );
			DrawDistanceLine( S, O );
			break;

		case FM_ParentInspect:
			{
				double	dxy, dc;
				double	xc, yc, af, bf;
				int		xs, ys;
				RECT	r;
				BOOL	result=FALSE, changes;
				LPSGINFO SI = (LPSGINFO)(OBJ::GetOBJ(O->ParentWnd)->dummy);

				GetClientRect( O->ParentWnd, &r );
				xs = r.right; ys = r.bottom;
				xc = LOWORD(lParam);
				yc = HIWORD(lParam);
				if (Get_DCenter( SI, xs, ys, &xc, &yc, &af, &bf, &dxy, &dc, IsDlgButtonChecked(hDlg, IDC_AM_CGRAV)))
					result = Get_DCenter( SI, xs, ys, &xc, &yc, &af, &bf, &dxy, &dc, IsDlgButtonChecked(hDlg, IDC_AM_CGRAV));
				if (wParam & MK_MBUTTON)
				{
					if (IsDlgButtonChecked(hDlg, IDC_AM_DISTANCEON))
					{
						changes = FALSE;
						if (((S->dx2 != af) || (S->dy2 != bf)))
						{
							DrawDistanceLine( S, O );
							changes = TRUE;
							S->dx2 = af; S->dy2 = bf;
						}
						if (changes)
						{
							UpdateDistance( hDlg, O, S );
							DrawDistanceLine( S, O );
						}
					}
				}
				if (wParam & MK_LBUTTON)
				{
					if (IsDlgButtonChecked(hDlg, IDC_AM_DISTANCEON))
					{
						changes = FALSE;
						if (((wParam & MK_SHIFT) == 0) && ((S->dx1 != af) || (S->dy1 != bf)))
						{
							DrawDistanceLine( S, O );
							changes = TRUE;
							S->dx1 = af; S->dy1 = bf;
						}
						if ((wParam & MK_SHIFT) && ((S->dx2 != af) || (S->dy2 != bf)))
						{
							DrawDistanceLine( S, O );
							changes = TRUE;
							S->dx2 = af; S->dy2 = bf;
						}
						if (changes)
						{
							UpdateDistance( hDlg, O, S );
							DrawDistanceLine( S, O );
						}
					}
					else
					{
						if ( (S->af[S->nact] != af) || (S->bf[S->nact] != bf))
						{
                    		DrawOneAtomCross( S, O, S->nact, TRUE );
							S->af[S->nact] = af;
							S->bf[S->nact] = bf;
							S->d[S->nact] = dc;
							S->flg[S->nact] = result;
							ChangeListItem( hDlg, O, S, S->nact );
							DrawOneAtomCross( S, O, S->nact, TRUE );
						}
					}
				}
			}
			break;

	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
	case WM_QUERYOPEN:
		ActivateObj(hDlg);
		break;

	case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					DestroyWindow( hDlg );
					break;

				case IDC_AM_AR:
					{
						double dd;
						double xx = O->NI.Scale * 1e10;
						if (GetDlgItemDouble( hDlg, IDC_AM_AR, &dd, 0 ))
							if (xx>1e-5) S->ral = dd/xx;
					}
					break;
				case IDC_AM_BR:
					{
						double dd;
						double xx = O->NI.Scale * 1e10;
						if (GetDlgItemDouble( hDlg, IDC_AM_BR, &dd, 0 ))
							if (xx>1e-5) S->rbl = dd/xx;
					}
					break;

				case IDC_AM_ADD:
					if (S->nat < MAXA-1) {
						DrawOneAtomCross( S, O, S->nact, TRUE ); // Erase as Active
						DrawOneAtomCross( S, O, S->nact, FALSE );// Draw as Inactive
						S->nat++;
						S->nact = S->nat-1;
						ChangeListItem( hDlg, O, S, S->nact );
						DrawOneAtomCross( S, O, S->nact, TRUE ); // Draw as Active
					}
					break;

				case IDC_AM_CLEAR:
					DrawAtomsCross( S, O );
					memset( S->af, 0, MAXA*(sizeof(double)*3 + sizeof(int)));
					S->nat = 1; S->nact = 0;
					UpdateList( hDlg, O, S );
					DrawAtomsCross( S, O );
					break;

				case IDC_AM_DISTANCEON:
				{
					BOOL f = IsDlgButtonChecked(hDlg, IDC_AM_DISTANCEON);

					EnableWindow(GetDlgItem(hDlg, IDC_AM_LIST),  !f);
					EnableWindow(GetDlgItem(hDlg, IDC_AM_ADD),   !f);
					EnableWindow(GetDlgItem(hDlg, IDC_AM_CLEAR), !f);
				}
					break;

				case IDC_AM_LIST:
					switch ( HIWORD(lParam) ) {
						case LBN_SELCHANGE:
						{
							int i = (int)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
							if ( i == S->nact ) break;
							DrawOneAtomCross( S, O, S->nact, TRUE ); // Erase as Active
							DrawOneAtomCross( S, O, S->nact, FALSE );// Draw as Inactive
							S->nact = i;
							DrawOneAtomCross( S, O, S->nact, FALSE );// Erase as Inactive
							DrawOneAtomCross( S, O, S->nact, TRUE ); // Draw as Active
						}
							break;
					}
					break;
				case IDC_AM_SAVE:
					_splitpath( O->fname, NULL, NULL, TMP, NULL );	// take file name only
					strcat(TMP, ".lst");
					if (!fioGetFileNameDialog( MainhWnd, "Save Atomic Coordinates", &ofNames[OFN_LIST], TMP, TRUE))
						return FALSE;
					SaveAmeter( hDlg, O, S, fioFileName() );
					break;

				case IDC_AM_LOAD:
					if (!fioGetFileNameDialog( MainhWnd, "Load Atomic Coordinates", &ofNames[OFN_LIST], "", FALSE))
						return FALSE;
					LoadAmeter( hDlg, O, S, fioFileName() );
					break;
			}
			break;
	}
	return FALSE;
}
