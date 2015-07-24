/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.OrigRef * by ML ***************************************************/
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

#include "HelperFn.h"

#define	SN	((L->OR_Mode & OR_SYNAMES)!=0)

#pragma optimize ("",off)

extern	void xLine( HDC hdc, OBJ* O, double x0, double y0, double x1, double y1);

/****************************************************************************/
BOOL	GetSelSymm( LPLATTICE L, HWND lb )
{
	if ( L->Spg == (int)SendMessage( lb, LB_GETCURSEL, 0, 0)+1) return FALSE;
	else L->Spg =  (int)SendMessage( lb, LB_GETCURSEL, 0, 0)+1;
	Create_DMap( L, 0 );
	return TRUE;
}
/****************************************************************************/
void	ChangeSymmetry( OBJ* O, LPLATTICE	L, int sel, HWND hDlg )
{
	LPSGINFO SI = L->SI+L->Spg;

	// removed by Peter on 9 Sep 2001
//	CheckDlgButton( hDlg, IDC_OR_REOR, SI->flg&DM_REOR);
	CheckDlgButton( hDlg, IDC_ORT_SHE, SI->flg&OR_MAN);
	SetDlgItemDouble( hDlg, IDC_ORT_SH, P2D(SI->Sh), -1);
	SetDlgItemDouble( hDlg, IDC_ORT_SK, P2D(SI->Sk), -1);

//	SetDlgItemText( hDlg, IDC_???, rflSymmName(L->Spg, SN));
	if(sel) SendDlgItemMessage( hDlg, IDC_OR_LIST, LB_SETCURSEL, L->Spg-1, 0 );
//	EnableDMAP( O, hDlg );
}
/****************************************************************************/
static	void	FillORefList( HWND hDlg, OBJ* O, LPLATTICE L )
{
	int 		i;
	LPSGINFO	SI;
	wchar_t		ra[32], rs[32], rr[32];
	wchar_t		not[] = L"  - ";
	wchar_t		sname[32];
//	char		ra[20], rs[20], rr[20];
//	char		not[] = " -  ";

//	int nStart = 32;
   	SendDlgItemMessage(hDlg, IDC_OR_LIST, LB_RESETCONTENT, 0, 0);
	SI = L->SI+1;
	for (i = 1; i < NUMSYMM+1; i++ )
	{
//		if (SI->flg & OR_DONE)
//		{
//			if (SI->NFa) sprintf(ra, "%4.1f", SI->RFa); else strcpy(ra, not);
//			if (SI->NFp) sprintf(rs, "%4.1f", SI->RFp); else strcpy(rs, not);
//			if (SI->Nr ) sprintf(rr, "%4.1f", SI->Rr ); else strcpy(rr, not);
//			sprintf(TMP, "%s %s %s %s", rflSymmName(i, SN), ra, rs, rr );
//		}
//		else sprintf(TMP, "%s", rflSymmName(i, SN));
//    	SendDlgItemMessage(hDlg, IDC_OR_LIST, LB_INSERTSTRING, UM1, (WPARAM)TMP);
		LPSTR pszSymm = rflSymmName(i, SN);
		size_t len = strlen(pszSymm);
		memset(sname, 0, sizeof(sname));
		mbstowcs(sname, pszSymm, len);
//		PrintWCharText((LPTSTR)sname, "%s", );
//		for(size_t k = 0; k < len; k++)
//		{
//			sname[k] = nStart+k+(i-1)*7;
//			if( 131 == sname[k] )
//			{
//				sname[k] = 0x2534;//PERPEND_CHAR;
//			}
//		}
//		LPTSTR pszSymm = rflSymmName(i, SN);
//		memset(sname, 0, strlen(pszSymm));
//		mbstowcs(sname, pszSymm, strlen(pszSymm));
		if (SI->flg & OR_DONE)
		{
			if (SI->NFa) swprintf(ra, L"%4.1f", SI->RFa); else wcscpy(ra, not);
			if (SI->NFp) swprintf(rs, L"%4.1f", SI->RFp); else wcscpy(rs, not);
			if (SI->Nr ) swprintf(rr, L"%4.1f", SI->Rr ); else wcscpy(rr, not);
			swprintf((wchar_t*)TMP, L"%s   %s   %s   %s", sname/*rflSymmName(i, SN)*/, ra, rs, rr );
		}
		else swprintf((wchar_t*)TMP, L"%s", sname/*rflSymmName(i, SN)*/);
    	SendDlgItemMessageW(hDlg, IDC_OR_LIST, LB_INSERTSTRING, UM1, (WPARAM)TMP);
	   	SI++;
    }
	ChangeSymmetry( O, L, TRUE, hDlg );
}
/****************************************************************************/
static	void	ScanAll( HWND hDlg, OBJ* O, LPLATTICE L)
{
	LPSGINFO SI = L->SI+L->Spg;
	double	fd;

	GetDlgItemDouble( hDlg, IDC_ORT_THR, &L->Thr, 0);
	fd=P2D(SI->Sh); GetDlgItemDouble( hDlg, IDC_ORT_SH, &fd, -1 ); SI->Sh = D2P(fd);
	fd=P2D(SI->Sk); GetDlgItemDouble( hDlg, IDC_ORT_SK, &fd, -1 ); SI->Sk = D2P(fd);
	SetDlgItemDouble( hDlg, IDC_ORT_SH, P2D(SI->Sh), -1);
	SetDlgItemDouble( hDlg, IDC_ORT_SK, P2D(SI->Sk), -1);
}
/****************************************************************************/
static BOOL InitOREF( HWND hDlg, OBJ* O, LPLATTICE L )
{
	int			i;
	LPSGINFO	SI;

	L->rptr = NULL;		// Not use parents REFLs

//	L->Ex_Mode = 1 | EM_AVERAGE;	// Asym amp is average sum, extraction mode = 1
	L->Ex_Mode = 4 | EM_AVERAGE;	// Pha is from pixel, noise from box
	L->OR_Mode = OR_WEIGHT | OR_RESTR | OR_RELAT;

	L->Spg = SYMM_P1;
	L->Thr = 50.0;		// modified by Peter on 23 Aug 2001

	L->SI = NULL;
	O->dp = NULL;
	O->imin = 0;
	O->imax = 255;
	// SGINFO structures for each symmetry (NUMSYMM+1)
	if ((L->SI = (LPSGINFO)calloc(NUMSYMM+1, sizeof(SGINFO)))==NULL) return FALSE;

	// residuals maps and density maps (NUMSYMM+1)
	if ((O->dp = (LPBYTE)calloc(NUMSYMM+1, (RXS*RXS+IXS*IXS)))==NULL) return FALSE;

	// assigning pointers
	for(i = 0, SI = L->SI; i <= NUMSYMM; i++, SI++)
	{
		SI->rmap = O->dp+i*(SQR(RXS)+SQR(IXS));				SI->rxs = RXS;
		SI->dmap = O->dp+i*(SQR(RXS)+SQR(IXS))+SQR(RXS);	SI->dxs = IXS;
		SI->scale = 1.5;
	}
	SI = L->SI + L->Spg;

//	SendDlgItemMessage(hDlg, IDC_ORT_SYNAME0, WM_SETFONT, (WPARAM)hfNormal, 0);
//	SendDlgItemMessage(hDlg, IDC_ORT_SYNAME1, WM_SETFONT, (WPARAM)hfNormal, 0);
//	SendDlgItemMessage(hDlg, IDC_OR_LIST,    WM_SETFONT, (WPARAM)hfNormal, 0);
//	SendDlgItemMessage(hDlg, IDC_OR_LTITLE,  WM_SETFONT, (WPARAM)hfMedium, 0);
//	SendDlgItemMessage(hDlg, IDC_OR_ABGS,  WM_SETFONT, (WPARAM)hfMedium, 0);
#ifdef USE_FONT_MANAGER
	SendDlgItemMessage(hDlg, IDC_ORT_SYNAME0, WM_SETFONT, (WPARAM)FontManager::GetFont(FM_MEDIUM_FONT), 0);
	SendDlgItemMessage(hDlg, IDC_ORT_SYNAME1_TEXT, WM_SETFONT, (WPARAM)FontManager::GetFont(FM_MEDIUM_FONT), 0);
	SendDlgItemMessage(hDlg, IDC_OR_ABGS,  WM_SETFONT, (WPARAM)FontManager::GetFont(FM_NORMAL_FONT), 0);
	SendDlgItemMessage(hDlg, IDC_OR_GS,  WM_SETFONT, (WPARAM)FontManager::GetFont(FM_GREEK_FONT), 0);
	
	SendDlgItemMessage(hDlg, IDC_OR_LIST,    WM_SETFONT, (WPARAM)FontManager::GetFont(FM_MEDIUM_FONT_CW), 0);
	SendDlgItemMessage(hDlg, IDC_OR_LTITLE,  WM_SETFONT, (WPARAM)FontManager::GetFont(FM_GREEK_FONT_CW), 0);
#else
	SendDlgItemMessage(hDlg, IDC_ORT_SYNAME0, WM_SETFONT, (WPARAM)hfMedium, 0);
	SendDlgItemMessage(hDlg, IDC_ORT_SYNAME1_TEXT, WM_SETFONT, (WPARAM)hfMedium, 0);
	SendDlgItemMessage(hDlg, IDC_OR_ABGS,  WM_SETFONT, (WPARAM)hfNormal, 0);
	SendDlgItemMessage(hDlg, IDC_OR_GS,  WM_SETFONT, (WPARAM)hfGreek, 0);
	
	SendDlgItemMessage(hDlg, IDC_OR_LIST,    WM_SETFONT, (WPARAM)hfMedium, 0);
	SendDlgItemMessage(hDlg, IDC_OR_LTITLE,  WM_SETFONT, (WPARAM)hfGreek, 0);
#endif

//	SetDlgItemText(hDlg, IDC_OR_LTITLE, "Symm    RA%  \200Res Ao/Ae");
	TextToDlgItemW(hDlg, IDC_OR_LTITLE, "Symm       RA%%   %cRes   Ao/Ae", PHI_CHAR);

	CheckDlgButton( hDlg, IDC_OR_BRMAP,  TRUE);

// 	HWND hListBox = CreateWindowW(L"LISTBOX", NULL, WS_CHILD|WS_VISIBLE|LBS_STANDARD, 10, 10, 100, 200, hDlg, NULL, NULL, NULL);
//  	SendMessageW(hListBox, WM_SETFONT, (WPARAM)hfMedium, 0);
//   	SendMessageW(hListBox, LB_INSERTSTRING, UM1, (WPARAM)L"Like p22¹2");

	FillORefList( hDlg, O, L );

	CheckDlgButton( hDlg, IDC_ORT_SYNAME1,    L->OR_Mode & OR_SYNAMES );
	CheckDlgButton( hDlg, IDC_ORT_SYNAME0,  !(L->OR_Mode & OR_SYNAMES));
	TextToDlgItemW( hDlg, IDC_ORT_SYNAME1_TEXT, "Like p2212");
//	TextToDlgItemW( hDlg, IDC_ORT_SYNAME1_TEXT, "Like p22¹2");
/*
	CheckDlgButton( hDlg, IDC_ORT_EMODE0,   ((L->Ex_Mode & EM_MODE) == 0) );	// Pha in the pixel
	CheckDlgButton( hDlg, IDC_ORT_EMODE1,   ((L->Ex_Mode & EM_MODE) == 1) );	// Pha interpol
	CheckDlgButton( hDlg, IDC_ORT_EMODE2,   ((L->Ex_Mode & EM_MODE) == 2) );	// Noise 9x9
	CheckDlgButton( hDlg, IDC_ORT_EMODE3,   ((L->Ex_Mode & EM_MODE) == 4) );	// Noise - Halfway corners
	CheckDlgButton( hDlg, IDC_ORT_EMODE4,   ((L->Ex_Mode & EM_MODE) == 6) );	// Noise - Box
*/
	CheckDlgButton( hDlg, IDC_ORT_EMODE0,   ((L->Ex_Mode & EM_PHA_MODE) == 0) );	// Pha in the pixel
	CheckDlgButton( hDlg, IDC_ORT_EMODE1,   ((L->Ex_Mode & EM_PHA_MODE) == 1) );	// Pha interpol
	CheckDlgButton( hDlg, IDC_ORT_EMODE2,   ((L->Ex_Mode & EM_NOISE_MODE) == 0) );	// Noise 9x9
	CheckDlgButton( hDlg, IDC_ORT_EMODE3,   ((L->Ex_Mode & EM_NOISE_MODE) == 2) );	// Noise - Halfway corners
	CheckDlgButton( hDlg, IDC_ORT_EMODE4,   ((L->Ex_Mode & EM_NOISE_MODE) == 4) );	// Noise - Box

	CheckDlgButton( hDlg, IDC_ORT_AMPVSUM,   (L->Ex_Mode & EM_AMPVSUM));
	CheckDlgButton( hDlg, IDC_ORT_MAXAMP,    (L->Ex_Mode & EM_MAX));
	CheckDlgButton( hDlg, IDC_ORT_AVERAGE,   (L->Ex_Mode & EM_AVERAGE));
	CheckDlgButton( hDlg, IDC_ORT_WEIGHT,    (L->OR_Mode & OR_WEIGHT));
	CheckDlgButton( hDlg, IDC_ORT_SHE,		 (SI->flg & OR_MAN));

	SetDlgItemDouble( hDlg, IDC_ORT_THR, L->Thr, -1 );

	SetDlgItemDouble( hDlg, IDC_ORT_SH, P2D(SI->Sh), -1);
	SetDlgItemDouble( hDlg, IDC_ORT_SK, P2D(SI->Sk), -1);

	CheckDlgButton( hDlg, IDC_ORT_G, (L->OR_Mode & OR_RELAT));
	CheckDlgButton( hDlg, IDC_ORT_R, (L->OR_Mode & OR_RESTR));
	rflGetRefls(L);

	return TRUE;
}
/****************************************************************************/
void UpdateORef( OBJ* O, LPLATTICE L )
{
	LPLATTICE P = GetLATTICE( OBJ::GetOBJ(O->ParentWnd) );
	int		i;

	L->hFFTwnd = P->hFFTwnd;
	L->F = P->F;
	L->ft2 = P->ft2;
	L->rad = P->rad;
	L->ax = P->ax;	L->ay = P->ay;	L->bx = P->bx;	L->by = P->by;
	L->ax0 = P->ax0;L->ay0 = P->ay0; L->bx0 = P->bx0; L->by0 = P->by0;
	L->cx = P->cx;	L->cy = P->cy;	L->dx = P->dx;	L->dy = P->dy;
	L->cx0 = P->cx0;L->cy0 = P->cy0; L->dx0 = P->dx0; L->dy0 = P->dy0;
	L->al = P->al;	L->bl = P->bl;	L->cl = P->cl;	L->dl = P->dl;
	L->gamma = P->gamma; L->cdgamma = P->cdgamma;
	for (i=0;i<NUMSYMM+1;i++) BitClr( L->SI[i].flg, OR_DONE );
}
/****************************************************************************/
void	PaintOREFDlg( HWND hDlg, OBJ* O, LPLATTICE L )
{
	HDC		hdc=0;
	HWND	hw1, hw2;
	RECT	re1, re2;
	LPSGINFO	SI;
	int		x, y, i, j;
	int		x1s, y1s, x2s, y2s;
	HPEN	open;
	double	xx = O->NI.Scale * 1e10, ra, rb;
	LPBYTE	bimem;

	try
	{
		bimem = (LPBYTE)calloc(1, 128*1024 );

		SI = L->SI+L->Spg;

		hw1 = GetDlgItem( hDlg, IDC_OR_W1 );	hw2 = GetDlgItem( hDlg, IDC_OR_W2 );
		GetClientRect( hw1, &re1 );				GetClientRect( hw2, &re2 );
		InvalidateRect( hw1, NULL, FALSE );		InvalidateRect( hw2, NULL, FALSE );
		x1s = re1.right  - re1.left;			x2s = re2.right  - re2.left;
		y1s = re1.bottom - re1.top;				y2s = re2.bottom - re2.top;

		if ((SI->flg & OR_DONE)==0)
		{
			hdc = GetDC( hw2 );
			PatBlt( hdc, re2.left, re2.top, x2s, y2s, BLACKNESS );
			ReleaseDC( hw2, hdc );
			hdc = GetDC( hw1 );
			PatBlt( hdc, re1.left, re1.top, x1s, y1s, BLACKNESS );
			ReleaseDC( hw1, hdc );
		}
		else
		{
			if ( IsDlgButtonChecked( hDlg, IDC_OR_BRMAP ) )
			{
				if (SI->rmap)
				{
					RMap_To_Img( L, SI,  bimem, x2s, y2s);
					pntBitmapToScreen( hw2, bimem, x2s, y2s, 0, 0, 1, 1, NULL, 0 );
					Get_CSym(L, x2s, y2s, &x, &y);
					hdc = GetDC( hw2 );
					open = SelectPen( hdc, RedPen );
					_MoveTo( hdc, 0, y ); LineTo( hdc, re2.right, y);
					_MoveTo( hdc, x, 0 ); LineTo( hdc, x, re2.bottom);
					SelectPen( hdc, open );
					ReleaseDC( hw2, hdc );
					i = SI->Bh + SI->Ch; if (SI->flg & OR_MAN) i += SI->Sh;
					j = SI->Bk + SI->Ck; if (SI->flg & OR_MAN) j += SI->Sk;
					sprintf( TMP, "Shift h=%-6.1f  Shift k=%-6.1f", P2D(i), P2D(j) );
					SetDlgItemText( hDlg, IDC_OR_SHK , TMP );
				}
				else
				{
					hdc = GetDC( hw2 );
					PatBlt( hdc, re2.left, re2.top, x2s, y2s, BLACKNESS );
					ReleaseDC( hw2, hdc );
				}
			}
			else
			{
				DMap_To_Img(L->SI+1, bimem, x2s, y2s);
				pntBitmapToScreen( hw2, bimem, x2s, y2s, 0, 0, 1, 1, O, 0 );
			}
			DMap_To_Img( SI,	 bimem, x1s, y1s);
			pntBitmapToScreen( hw1, bimem, x1s, y1s, 0, 0, 1, 1, O, 0 );
			ra = SI->ral*xx;
			rb = SI->rbl*xx;
			i=2; if(ra>=10.) i--; if(ra>=100.) i--;
			j=2; if(rb>=10.) j--; if(rb>=100.) j--;
			// changed by Peter on 10 Oct 2001 and 22 Apr 2003 and 12 Nov 2006
			TextToDlgItemW(hDlg, IDC_OR_ABGS, "a=%-4.*f%c  b=%-4.*f%c",
				i, FastAbs(ra), ANGSTR_CHAR, j, FastAbs(rb), ANGSTR_CHAR);
			TextToDlgItemW(hDlg, IDC_OR_GS, "%c=%5.1f%c", GAMMA_CHAR, R2D(FastAbs(SI->rgamma)), DEGREE_CHAR);
//			sprintf( TMP, "a=%-4.*fÅ  b=%-4.*fÅ  g=%-5.1f", i, ra, j, rb, -R2D(fabs(SI->rgamma)));
//			sprintf( TMP, "a=%-4.*fÂ  b=%-4.*fÂ  g=%-5.1f", i, ra, j, rb, R2D(SI->rgamma));
//			SetDlgItemText( hDlg, IDC_OR_ABGS , TMP );
		}
		MapWindowPoints( hw1, hDlg, (LPPOINT) &re1, 2);
		ValidateRect( hDlg, &re1 );
		MapWindowPoints( hw2, hDlg, (LPPOINT) &re2, 2);
		ValidateRect( hDlg, &re2 );
		free( bimem );
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("PaintOREFDlg() -> %s"), e.what()));
	}
}

/****************************************************************************/
// #define	DOR_EXTRACT	1
// #define	DOR_TRYALL	2
// #define	DOR_FULL	4
// #define	DOR_SHORT	8
BOOL DoOriginRefine( HWND hDlg, OBJ* O, LPLATTICE L, UINT what )
{
	LPSGINFO	SI = L->SI+L->Spg;

	if ((what & DOR_FULL) != 0)
		BitClr(SI->flg, RA_DONE | RS_DONE | RR_DONE | RF_DONE | OR_DONE);

	if ((what & DOR_EXTRACT) != 0 || L->rptr == NULL)
		rflGetRefls(L);

	if ((what & (DOR_FULL | DOR_SHORT)) != 0 && Asym(L, what & DOR_TRYALL))
	{
		if ((what & DOR_FULL) != 0)
		{
			if (!IsDlgButtonChecked(hDlg, IDC_ORT_ANYSHIFT))
				OriginRefine( L );
			else
				BitSet( SI->flg, OR_DONE );
		}
		ApplyPhaseShift( L );
		ApplySymmetry( L );
		CalcRFactor( L );
		Create_DMap( L, 0 );
		Create_DMap( L, 1 );
	}
	else
	{
		CalcRFactor( L );
		Create_DMap( L, 0 );
		Create_DMap( L, 1 );
	}
	PaintOREFDlg( hDlg, O, L );
	FillORefList( hDlg, O, L );
	return TRUE;
}
/****************************************************************************/
// SubGroups: (7 total)
// 0-a) pm, pm, pg, pg, cm and cm
// 1-b) pmm, pmg, pmg, pgg and cmm
// 2-c) p4
// 3-d) p4m and p4g
// 4-e) p3
// 5-f) p3m1, p31m and p6
// 6-g) p6m
#define	S_GR	0
#define	A_GR	1
#define	B_GR	2
#define	C_GR	3
#define	D_GR	4
#define	E_GR	5
#define	F_GR	6
#define	G_GR	7
static int auiSubGroups[] =	// '-1' shows the end of each subgroup, '0' - end of the table
{
	SYMM_P1, SYMM_P2, -1,
	SYMM_PMa, SYMM_PMb, SYMM_PGa, SYMM_PGb, /*SYMM_CMa, SYMM_CMb, */-1,
	SYMM_PMM, SYMM_PMGa, SYMM_PMGb, SYMM_PGG, /*SYMM_CMM, */-1,
	SYMM_P4, -1,
	SYMM_P4M, SYMM_P4G, -1,
	SYMM_P3, -1,
	SYMM_P3M1, SYMM_P31M, SYMM_P6, -1,
	SYMM_P6M,
	0
};
static void GuessSymmetry( LPLATTICE L )
{
	int		auiBest[8];		// winners in each group (with min residual)
	int		auiBestA[8];	// winners in each group (with min Aodd/Aeven)
	double	adPhRes[8];		// array of min phase residual for each group
	double	adPhResA[8];	// array of min phase residual for each group (Aodd/Aeven)
	int		i, opt = -1, nSubGroup, nIndex, nCurSG;
	double	pr = 1000.;//, p;
	BOOL	bWork = TRUE;
	LPSGINFO	SI;

	// 1. Find the plane group with lowest phase residual
	for(i = 0; i < 8; i++)
	{
		adPhRes[i] = 1000.;		// > than 1000 - means that it is not set
		adPhResA[i] = 1000.;	// > than 1000 - means that it is not set
		auiBest[i] = 0;			// none of SG is best
		auiBestA[i] = 0;		// none of SG is best
	}
	nSubGroup = 0;
	nIndex = 0;
	while(bWork)
	{
		nCurSG = auiSubGroups[nIndex];
		if(nCurSG == -1)
			nSubGroup++;			// next group
		else if(nCurSG == 0)
			bWork = FALSE;
		else
		{
			SI = &L->SI[nCurSG];
			if(SI->Rr <= 0.5 && SI->Nr)
			{
				if((adPhRes[nSubGroup] > SI->RFp) && (SI->flg & OR_DONE) && (SI->NFp))
				{
					adPhRes[nSubGroup] = SI->RFp;
					auiBest[nSubGroup] = nCurSG;
				}
			}
			else
			{
				if((adPhResA[nSubGroup] > SI->RFp) && (SI->flg & OR_DONE) && (SI->NFp))
				{
					adPhResA[nSubGroup] = SI->RFp;
					auiBestA[nSubGroup] = nCurSG;
				}
			}
		}
		nIndex++;	// next in the group
	}
	// compare results
	for(i = 0; i < 8; i++)
	{
		if( (auiBest[i] == 0) && (auiBestA[i] != 0) )
		{
			auiBest[i] = auiBestA[i];
			adPhRes[i] = adPhResA[i];
		}
	}
	// gamma != 90 && gamma != 60. - only P1 & P2
	if( (fabs(R2D(L->gamma) - 90.) > 10.) && (fabs(R2D(L->gamma) - 60.) > 10.) )
	{
		opt = auiBest[S_GR];
	}
	else if( (fabs(R2D(L->gamma) - 90.) <= 10.) )	// gamma ~ 90
	{
		double ratio = L->al / L->bl;
		int opt_AB = -1, opt_CD = -1;
		// look through A & B - we need this in both cases
		if( auiBest[A_GR] != 0 || auiBest[B_GR] != 0)
		{
			if( adPhRes[B_GR] > 30. && adPhRes[A_GR] > 30. )
			{
				opt_AB = (adPhRes[A_GR] < adPhRes[B_GR]) ? auiBest[A_GR] : auiBest[B_GR];
			}
			else
			{
				opt_AB = ( adPhRes[B_GR] <= 30. && adPhRes[B_GR] < 1.5*adPhRes[A_GR] ) ? auiBest[B_GR] : auiBest[A_GR];
			}
		}
		// look through C & D
		if( auiBest[C_GR] != 0 || auiBest[D_GR] != 0)
		{
			if( adPhRes[D_GR] > 30. && adPhRes[C_GR] > 30. )
			{
				opt_CD = (adPhRes[C_GR] < adPhRes[D_GR]) ? auiBest[C_GR] : auiBest[D_GR];
			}
			else
			{
				opt_CD = ( adPhRes[D_GR] <= 30. && adPhRes[D_GR] < 1.5*adPhRes[C_GR] ) ? auiBest[D_GR] : auiBest[C_GR];
			}
		}
		if( ratio > 1.2 || ratio < 1./1.2 )		// a != b	=> only A_GR & B_GR
		{
			opt = opt_AB;
		}
		else									// a == b	=> A_GR & B_GR & C_GR & D_GR
		{
			if( opt_AB >= 0 && opt_CD >= 0 )
			{
				opt = ( (adPhRes[opt_CD] <= 30.) && (adPhRes[opt_CD] <= 1.5*adPhRes[opt_AB]) ) ? opt_CD : opt_AB;
			}
			else if ( opt_AB >= 0 )
			{
				opt = opt_AB;
			}
			else if ( opt_CD >= 0 )
			{
				opt = opt_CD;
			}
		}
	}
	else if ( (fabs(R2D(L->gamma) - 60.) <= 10.) )
	{
		if( adPhRes[F_GR] < 1.5*adPhRes[E_GR] )
		{
			opt = auiBest[F_GR];
			if( adPhRes[G_GR] < 1.2*adPhRes[F_GR] )
			{
				opt = auiBest[G_GR];
			}
		}
		else
		{
			opt = auiBest[E_GR];
		}
	}
	L->Spg = opt;
}
/*
static void GuessSymmetry( LPLATTICE L )
{
	int		i, opt=1;
	double	pr = 1000., p;

	for(i = SYMM_P2; i <= SYMM_P6M; i++)
	{
		if ((L->SI[i].flg & OR_DONE) && (L->SI[i].NFp))
		{
			p = L->SI[i].RFp;
			switch (i)
			{
			case	SYMM_P2:
			case	SYMM_PMa:
			case	SYMM_PMb:
			case	SYMM_PGa:
			case	SYMM_PGb:
			case	SYMM_PMM:
			case	SYMM_PMGa:
			case	SYMM_PMGb:
			case	SYMM_PGG:
			case	SYMM_P4:
			case	SYMM_P4M:
			case	SYMM_P4G:
			case	SYMM_P3:
			case	SYMM_P3M1:
			case	SYMM_P31M:
			case	SYMM_P6:
			case	SYMM_P6M:
				if ((p > 1.5*pr) && (p > pr + 5.))
					continue;
				break;

			case	SYMM_CMa:
			case	SYMM_CMb:
			case	SYMM_CMM:
				if (L->SI[i].Rr > 0.5)
					continue;
				break;
			}
			pr = L->SI[i].RFp;
			opt = i;
		}
	}
	L->Spg = opt;
}
*/
/****************************************************************************/
static LRESULT ChangeItemColors( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC		hdc = (HDC)wParam;
	HWND	hwnd = (HWND)lParam;
	int		idc = GetDlgCtrlID(hwnd);
	LRESULT	ret;
	COLORREF	col  = RGB(0,0,0);
	DWORD	bcol = GetSysColor(COLOR_WINDOW);
	static	BOOL brec = FALSE, bPaint = TRUE;

	if (brec)
		return FALSE;
	brec = TRUE;
	ret = DefDlgProc(hDlg, msg, wParam, lParam);
	switch ( idc )
	{
	case IDC_OR_SH:
	case IDC_OR_SK:
		col = RGB(128,0,0);
		bcol = RGB(192,192,192);
		break;
	case IDC_OR_AS:
	case IDC_OR_BS:
	case IDC_OR_GS:
		col = RGB(128,0,0);
		bcol = RGB(192,192,192);
		break;
	default:
		bPaint = FALSE;
		break;
	}
	if( bPaint )
	{
		SetTextColor(hdc, col);
		SetBkColor(hdc, bcol);
	}

	brec = FALSE;
	return	ret;
}
/****************************************************************************/
/*
void Est_TP_BG(FT2R &ft2, int xs, int rad, float &t)
{
	int h2max, h, k, r2 = rad*rad;
	FCOMPLEX fontWidth;
	double a2, sx, sy;

	sx = sy = 0.;

	for(k = 0; k <= rad; k++)
	{
		h2max = r2 - k*k;
		for(h = 0; h*h < h2max; h++)
		{
			fontWidth = ft2.GetVal(h, k);
			a2 = fontWidth.i*fontWidth.i + fontWidth.j*fontWidth.j;
			sx += a2 * (h*h - k*k);
			sy += 2 * a2 * h*k;
		}
	}
	for(k = -1; k >= -rad+1; k--)
	{
		h2max = r2 - k*k;
		for(h = 1; h*h < h2max; h++)
		{
			fontWidth = ft2.GetVal(h, k);
			a2 = fontWidth.i*fontWidth.i + fontWidth.j*fontWidth.j;
			sx += a2 * (h*h - k*k);
			sy += 2 * a2 * h*k;
		}
	}
	t = atan2(sy, sx) * .5;
}
*/
static void Tilt_Estimate( HWND hDlg, OBJ* O )
{
	LPLATTICE 	S = (LPLATTICE) &O->dummy;
	LPFFT		fft;
	HDC		hdc = NULL;
	HWND	hWndFFT;
	OBJ*	pr;
	HPEN	open;
	int		orop;
	float	t, x, y, r, xa, xs, ta;

	hWndFFT = objFindParent( O, OT_FFT );	// Searching back for FFT object
	pr = OBJ::GetOBJ( hWndFFT );
	hdc = GetDC( hWndFFT );
	orop = SetROP2(hdc, R2_COPYPEN );
	open = (HPEN)SelectObject(hdc, GrayPen );

	if( (fft = GetFFT( S->F )) == NULL )
		return;
	xs = fft->fs;
	r = (float)xs * .47;
	FCOMPLEX fontWidth = fft->ft2.GetVal(1, -1);
	EstTP_BG(fft->ft2.BX.P, xs, S->rad, &t);
//	Est_TP_BG(fft->ft2, xs, S->rad, t);

	xa = atan2(S->ay, S->ax);
	ta = t - xa;
	while (ta > M_PI)
		ta -= (float)M_PI;
	while (ta < -M_PI)
		ta += (float)M_PI;
	sprintf( TMP, "Tilt axis^a*= %4.0f", R2D(ta) );
	SetDlgItemText( hDlg, IDC_ORT_TP , TMP );
	sprintf( TMP, "a*^x = %4.1f", R2D(xa) );
	SetDlgItemText( hDlg, IDC_ORT_TV , TMP );

	x = r*cos(t);
	y = r*sin(t);
	xLine( hdc, pr, -x, -y, x, y);
	SetROP2(hdc, orop);
	SelectObject(hdc, open);
	ReleaseDC(hWndFFT, hdc);
}
//////////////////////////////////////////////////////////////////////////
void DrawListBox(HWND hDlg, UINT uiID, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL bSymNames)
{
	//CRect members to store the position of the items
	RECT rItem;
	HDC dc = lpDrawItemStruct->hDC;

	if( (int)lpDrawItemStruct->itemID < 0 )
	{
		// If there are no elements in the CListBox
		// based on whether the list box has Focus  or not
		// draw the Focus Rect or Erase it,
		if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && (lpDrawItemStruct->itemState & ODS_FOCUS))
		{
			::DrawFocusRect(dc, &lpDrawItemStruct->rcItem);
		}
		else if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && !(lpDrawItemStruct->itemState & ODS_FOCUS))
		{
			::DrawFocusRect(dc, &lpDrawItemStruct->rcItem);
		}
		return;
	}
	// String to store the text
	// Get the item text.
//	wchar_t lpszBuffer[256];
	char lpszBuffer[256];
	memset(lpszBuffer, 0, sizeof(lpszBuffer));
//	::SendDlgItemMessageW(hDlg, uiID, LB_GETTEXT, lpDrawItemStruct->itemID, (LPARAM)lpszBuffer);
	::SendDlgItemMessage(hDlg, uiID, LB_GETTEXT, lpDrawItemStruct->itemID, (LPARAM)lpszBuffer);
/*
	for(size_t i = 0; i < wcslen(lpszBuffer); i++)
	{
		if( 63 == lpszBuffer[i] )
		{
			if( bSymNames )
			{
				lpszBuffer[i] = 185;
			}
			else
			{
				lpszBuffer[i] = '_';
			}
		}
//		lpszBuffer[i] = lpDrawItemStruct->itemID * 16 + i;
	}
*/
	// Initialize the CListBox Item's row size
	rItem = lpDrawItemStruct->rcItem;

	COLORREF crText;
	
	crText = RGB(0, 0, 0);

	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	// If CListBox item selected, draw the highlight rectangle.
	// Or if CListBox item deselected, draw the rectangle using the window color.
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) &&
		(lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		HBRUSH br = CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
		FillRect(dc, &rItem, br);
		DeleteObject(br);
		crText = RGB(255, 255, 255);
	}
	else if (!(lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & ODA_SELECT))
	{
		HBRUSH br = CreateSolidBrush(::GetSysColor(COLOR_WINDOW));
		FillRect(dc, &rItem, br);
		DeleteObject(br);
// 		crText = RGB(255, 255, 255);
	}

	// If the CListBox item has focus, draw the focus rect.
	// If the item does not have focus, erase the focus rect.
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && (lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		DrawFocusRect(dc, &rItem);
		crText = RGB(255, 255, 255);
	}
	else if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && !(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		DrawFocusRect(dc, &rItem);
		crText = RGB(255, 255, 255);
	}

	// To draw the Text in the CListBox set the background mode to Transparent.
	int iBkMode = SetBkMode(dc, TRANSPARENT);

	SetTextColor(dc, crText);
#ifdef USE_FONT_MANAGER
	SelectObject(dc, FontManager::GetFont(FM_MEDIUM_FONT_CW));
#else
	SelectObject(dc, hfMedium);
#endif

	//Draw the Text
//	TextOut(dc, rItem.left, rItem.top, lpszBuffer, strlen(lpszBuffer));
//	DrawTextW(dc, lpszBuffer, wcslen(lpszBuffer), &rItem, DT_LEFT | DT_TOP);
	DrawText(dc, lpszBuffer, strlen(lpszBuffer), &rItem, DT_LEFT | DT_TOP);

	// if perpendicular sign - draw it
	for(size_t i = 0; i < strlen(lpszBuffer); i++)
	{
		if( '_' == lpszBuffer[i] )
		{
			lpszBuffer[i] = 0;
			int x, y, width, height;
			DrawText(dc, lpszBuffer, strlen(lpszBuffer), &rItem, DT_LEFT | DT_TOP | DT_CALCRECT);
			x = rItem.right;
			y = rItem.top;
			DrawText(dc, "_", strlen("_"), &rItem, DT_LEFT | DT_TOP | DT_CALCRECT);
			width = rItem.right - rItem.left;
			height = abs(rItem.bottom - rItem.top);
			_MoveTo(dc, x + (width >> 1), y + 2);
			LineTo(dc, x + (width >> 1), y + height - 1);
			break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void MeasureListBoxItem(HWND hDlg, UINT uiID, LPMEASUREITEMSTRUCT lpMeasureItem)
{
//	wchar_t *str = L"XXXXXXXXXXXXXXXXXXXXXXXXX";
	char *str = "XXXXXXXXXXXXXXXXXXXXXXXXX";
	RECT rcRect;
	HDC hWinDC = GetDC(GetDlgItem(hDlg, uiID));
	lpMeasureItem->CtlType = ODT_LISTBOX;
	lpMeasureItem->CtlID = uiID;
	HDC hDC = CreateCompatibleDC(hWinDC);
#ifdef USE_FONT_MANAGER
	SelectObject(hDC, FontManager::GetFont(FM_MEDIUM_FONT_CW));
#else
	SelectObject(hDC, hfMedium);
#endif
//	DrawTextW(hDC, str, wcslen(str), &rcRect, DT_LEFT | DT_TOP | DT_CALCRECT);
	DrawText(hDC, str, strlen(str), &rcRect, DT_LEFT | DT_TOP | DT_CALCRECT);
	lpMeasureItem->itemWidth = rcRect.right - rcRect.left;
	lpMeasureItem->itemHeight = abs(rcRect.bottom - rcRect.top);
	DeleteDC(hDC);
	ReleaseDC(hDlg, hWinDC);
}
/****************************************************************************/
LRESULT CALLBACK ORefWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
	OBJ*	O = OBJ::GetOBJ( hDlg );
	LPLATTICE	L = NULL;
	LPSGINFO	SI = NULL;
	if( O != NULL )
	{
		L = GetLATTICE( O );
		SI = L->SI+L->Spg;
	}

	switch (message)
	{
	case WM_INITDIALOG:
		// ADDED BY PETER ON 29 JUNE 2004
		// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
		// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
		O = (OBJ*)lParam;
		L = GetLATTICE( O );
		SI = L->SI+L->Spg;
		InitializeObjectDialog(hDlg, O);

		if (!InitOREF(hDlg, O, L))
		{
			DestroyWindow( hDlg );
			break;
		}
		wndChangeDlgSize( hDlg, hDlg, 0,
						  IsDlgButtonChecked(hDlg,IDC_OR_MORE) ? 0:IDC_ORT_METHOD, "OREF", hInst);
		DoOriginRefine( hDlg, O, L, DOR_FULL );
		SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_OR_TRYALL, 0), 0);
		return TRUE;

	case FM_Update:
		UpdateORef( O, L );
		DoOriginRefine( hDlg, O, L, DOR_EXTRACT | DOR_FULL );
		break;

	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
    	objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
    	break;

//	case WM_NOTIFY:
//		return MsgNotify( hDlg, message, wParam, lParam);

	case WM_DRAWITEM:
		{
			UINT uiID = (UINT)wParam;
			if( IDC_OR_LIST == uiID )
			{
				DrawListBox(hDlg, uiID, (LPDRAWITEMSTRUCT)lParam, L->OR_Mode & OR_SYNAMES);
			}
		}
		break;

	case WM_MEASUREITEM:
		{
			UINT uiID = (UINT)wParam;
			if( IDC_OR_LIST == uiID )
			{
				MeasureListBoxItem(hDlg, uiID, (LPMEASUREITEMSTRUCT)lParam);
			}
		}
		return TRUE;
		break;

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
		if (L->SI) free( L->SI ); L->SI = NULL;
		if (O->dp) free( O->dp ); O->dp = NULL;
		if (L->rptr) free( L->rptr ); L->rptr = NULL;
		objFreeObj( hDlg );
		break;

	case WM_PAINT:
		PaintOREFDlg( hDlg, O, L );
		break;

	case WM_PARENTNOTIFY:
		sprintf(TMP,"");
		break;

	case WM_COMMAND:
		if (wndDefSysMenuProc(hDlg, wParam, lParam)) break;

		switch(LOWORD(wParam))
		{
		case IDC_ORT_SYNAME0:
		case IDC_ORT_SYNAME1:
			BitFromID( hDlg, IDC_ORT_SYNAME1, L->OR_Mode, OR_SYNAMES);
			FillORefList( hDlg, O, L );
			break;

		case IDC_OR_W2: {
			RECT	r;
			HWND	hw;
			POINT	p;

			GetCursorPos(&p);
			hw = GetDlgItem( hDlg, IDC_OR_W2 );
			MapWindowPoints( NULL, hw, &p, 1);
			GetClientRect( hw, &r );
			if ( ! PtInRect( &r, p) ) break;
			if ( Set_CSym( L, r.right-r.left, r.bottom-r.top, p.x, p.y, IsDlgButtonChecked(hDlg, IDC_ORT_ANYSHIFT)) ) {
				DoOriginRefine( hDlg,  O, L, DOR_FULL );
			}
		}
			break;
		case IDC_OR_W1:
			wndCreateObjMenu(hDlg, FALSE, FALSE, FALSE, 0);
			break;

		case IDC_ORT_ANYSHIFT:
			if (!IsDlgButtonChecked(hDlg, IDC_ORT_ANYSHIFT))
				DoOriginRefine( hDlg,  O, L, DOR_FULL );
			break;

		case IDC_OR_MORE:
			wndChangeDlgSize( hDlg, hDlg, 0,
							  IsDlgButtonChecked(hDlg,IDC_OR_MORE) ? 0:IDC_ORT_METHOD, "OREF", hInst);
			break;

		case IDC_OR_LIST:
			switch ( HIWORD(wParam) )
			{
				case LBN_SELCHANGE:
					if ( ! GetSelSymm( L, (HWND)lParam ) ) break;
					ChangeSymmetry( O, L, FALSE, hDlg );
					DoOriginRefine( hDlg, O, L, DOR_SHORT );
					break;

				case LBN_DBLCLK:
					GetSelSymm( L, (HWND)lParam );
					ScanAll( hDlg, O, L );
					DoOriginRefine( hDlg, O, L, DOR_FULL /*| DOR_EXTRACT*/);
					break;
			}
			break;

		case IDC_OR_REFINE:
			ScanAll( hDlg, O, L );
			DoOriginRefine( hDlg, O, L, DOR_FULL );
			break;

		case IDC_OR_TRYALL:
			{
			int		i;
			ScanAll( hDlg, O, L );
			for( i = SYMM_P1; i <= SYMM_P6M; i++)
			{
				L->Spg = i;
				DoOriginRefine( hDlg, O, L, DOR_TRYALL | DOR_FULL );
				// added by Peter on 13 Sep 2001
//				Sleep(500);
			}
			GuessSymmetry( L );
			ChangeSymmetry( O, L, TRUE, hDlg );
			DoOriginRefine( hDlg, O, L, DOR_FULL );
		}
			break;

		case IDC_OR_BRMAP:
		case IDC_OR_BP1:
			PaintOREFDlg( hDlg, O, L );
			break;

		case IDC_OR_CREATEIMG:
			PostMessage( MainhWnd, WM_COMMAND, IDM_C_DMAP, 0 );
			break;

		case IDC_OR_HKEDIT:
BOOL CALLBACK HKeditDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
			DialogBoxParam(hInst, "HKEDIT", hDlg, HKeditDlgProc, (LPARAM)hDlg );
			break;

		case IDC_ORT_EST:
			Tilt_Estimate( hDlg, O );
			break;

		case IDC_ORT_MAXAMP:
		case IDC_ORT_AMPVSUM:
		case IDC_ORT_AVERAGE:
			BitClr( L->Ex_Mode, EM_MAX | EM_AMPVSUM | EM_AVERAGE);
			BitFromID( hDlg, IDC_ORT_MAXAMP,  L->Ex_Mode, EM_MAX);
			BitFromID( hDlg, IDC_ORT_AMPVSUM, L->Ex_Mode, EM_AMPVSUM);
			BitFromID( hDlg, IDC_ORT_AVERAGE, L->Ex_Mode, EM_AVERAGE);
			DoOriginRefine( hDlg, O, L, DOR_FULL );
			break;

//		case IDC_OR_REOR:		BitFromID( hDlg, IDC_OR_REOR,     SI->flg, DM_REOR); goto sss1;
		case IDC_ORT_SHE:		BitFromID( hDlg, IDC_ORT_SHE,     SI->flg, OR_MAN); goto sss1;

		case IDC_ORT_THR:
		case IDC_ORT_SH:
		case IDC_ORT_SK:
			if (HIWORD(wParam) == EN_KILLFOCUS) {
				ScanAll( hDlg, O, L );
				goto sss;
			}

		case IDC_ORT_HKSHIFT:
			switch(LOWORD(lParam)) {
				case JC_LEFT:  SI->Sh = D2P(P2D(SI->Sh)-0.1); goto sss;
				case JC_RIGHT: SI->Sh = D2P(P2D(SI->Sh)+0.1); goto sss;
				case JC_UP:    SI->Sk = D2P(P2D(SI->Sk)-0.1); goto sss;
				case JC_DOWN:  SI->Sk = D2P(P2D(SI->Sk)+0.1); goto sss;
			}
			break;
sss:
			SetDlgItemDouble( hDlg, IDC_ORT_SH, P2D(SI->Sh), -1);
			SetDlgItemDouble( hDlg, IDC_ORT_SK, P2D(SI->Sk), -1);

			if(SI->flg & OR_MAN) {
sss1:			DoOriginRefine( hDlg, O, L, DOR_FULL );
			}
			break;


		case IDC_ORT_G:
			SI->Ch = SI->Ck = 0;
			BitFromID( hDlg, IDC_ORT_G, L->OR_Mode, OR_RELAT );
			if ( Bit(L->OR_Mode, OR_RESTR | OR_RELAT) == 0)	{
				BitSet(L->OR_Mode, OR_RESTR);
				CheckDlgButton( hDlg, IDC_ORT_R, TRUE);
			}
			goto refine;

		case IDC_ORT_R:
			SI->Ch = SI->Ck = 0;
			BitFromID( hDlg, IDC_ORT_R, L->OR_Mode, OR_RESTR );
			if ( Bit(L->OR_Mode, OR_RESTR | OR_RELAT) == 0)	{
				BitSet(L->OR_Mode, OR_RELAT);
				CheckDlgButton( hDlg, IDC_ORT_G, TRUE);
			}
			goto refine;

		case IDC_ORT_WEIGHT:
			BitFromID( hDlg, IDC_ORT_WEIGHT, L->OR_Mode, OR_WEIGHT);
			goto refine;
refine:		DoOriginRefine( hDlg, O, L, DOR_FULL );
			break;

		case IDC_ORT_EMODE0:	L->Ex_Mode &= ~EM_PHA_MODE;							goto update;
		case IDC_ORT_EMODE1:	L->Ex_Mode &= ~EM_PHA_MODE;		L->Ex_Mode |=  1;	goto update;
		case IDC_ORT_EMODE2:	L->Ex_Mode &= ~EM_NOISE_MODE;						goto update;
		case IDC_ORT_EMODE3:	L->Ex_Mode &= ~EM_NOISE_MODE;	L->Ex_Mode |=  2;	goto update;
		case IDC_ORT_EMODE4:									L->Ex_Mode |=  4;
update:		DoOriginRefine( hDlg, O, L, DOR_EXTRACT | DOR_FULL );
			break;

		case IDOK:
		case IDCANCEL:
			DestroyWindow(hDlg);
		return TRUE;
	}
	}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("ORefWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
