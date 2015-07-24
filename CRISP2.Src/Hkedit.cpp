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
#include "symm.h"

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

void	ChangeSymmetry( OBJ* O, LPLATTICE	L, int sel, HWND hDlg );
void	UpdateORef( OBJ* O, LPLATTICE L );


static	int HKe_sel = 0;								// List box selected position
static	int HKe_selref = 0;								// Selected reflection No
static	char szHdrMod[]  ="  h   k   l   m   Amp  AmpS  Pha PhaS";
static	char szFmtMod[]  ="%3d %3d %3d %3d %5.0f %5.0f %4.0f %4.0f";
static	char szHdrFull[]  ="  h    k    Amp   AmpS   Pha  PhaS  Err";
static	char szFmtFull[]  ="%3d  %3d  %5.0f  %5.0f  %4.0f  %4.0f %c%c%c%c";
static	char szHdrShort[] ="  h    k   AmpS  PhaS  Err";
static	char szFmtShort[] ="%3d  %3d  %5.0f  %4.0f  %c%c%c";

static	char szHdr3[] ="Format: h k l a p";
static	char szFmt3[] ="%3d  %3d  %3d  %5.0f  %4.0f";
static	char szCellp[] ="Cell: %G %G 10 90 90 %G";

static	int		def_uu [2] = {1,0}, def_vv[2] = {0,1};
static	int		def_hh [2] = {1,0}, def_kk[2] = {0,1}, def_ll[2] = {0,0};

#define	XRFL	0x10000
#define	SVENHK	0
/****************************************************************************/
/* Edit HK list dialog ******************************************************/
/****************************************************************************/
static void	Print( int AUnit, LPLATTICE L, LPREFL R )
{
LPSGINFO	SI = L->SI+L->Spg;
int			h0, k0, p, p0, bh, bk;

	h0 = R->h0; k0 = R->k0;	p  = R->p;
	if(LM0(R)) p0 = p;
	else
	{
		bh = SI->Bh+SI->Ch;	bk = SI->Bk+SI->Ck;
		if(SI->flg & OR_MAN) { bh += SI->Sh; bk += SI->Sk; }
		p0 = R->p0 + h0*bh + k0*bk;
		if (R->f & RIP1) {
			if ( SVENHK ) { h0 = -h0; k0 = -k0; }
			else 		  { p = -p; if (p==0x7FFFFFFF) p++; }
		}
		if (R->f & RIP2) p += P_PI;
	}

	if((L->cl>3.)||(L->dl>3.))
		 sprintf(TMP, szFmtMod,
				R->h0, R->k0, R->l0, R->m0,
				R->a0, R->a,
				-P2D(-p0),	-P2D(-p) );
	else if(AUnit)
		 sprintf(TMP, szFmtShort,
				R->h0, R->k0, R->a, -P2D(-p),
				(R->f&(BSN|BAMP))?'A':' ',
				(abs(p-p0) > 0x20000000)?'P':' ',
				(R->f&BPHU)?'U':' ' );
	else sprintf(TMP, szFmtFull,
				h0, k0, R->a0, R->a,
				-P2D(-p0),	-P2D(-p),
				' ',
				(R->f&(BSN|BAMP))?'A':' ',
				(abs(p-p0) > 0x20000000)?'P':' ',
				(R->f&BPHU)?'U':' ' );
}
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
void	DrawListItem( HWND hDlg, LPDRAWITEMSTRUCT lpD, LPLATTICE L )
{
	RECT	rc;
	HDC		hdc;
	int		f;
	WORD	is;
	COLORREF	col;
	HFONT	hof;
	LPREFL	R = L->rptr;

	if(lpD->CtlType != ODT_LISTBOX) return;
	is = lpD->itemState; hdc = lpD->hDC;
	R += LOWORD(lpD->itemData); f = HIWORD(lpD->itemData);
	CopyRect((LPRECT)&rc, (LPRECT)&lpD->rcItem);

	FillRect(hdc, (LPRECT)&rc, (HBRUSH)GetStockObject(f ? WHITE_BRUSH : LTGRAY_BRUSH));
	if (is & ODS_SELECTED) FrameRect(hdc, (LPRECT)&rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
	InflateRect( &rc, -1, -1);
	if (is & ODS_FOCUS)	   FrameRect(hdc, (LPRECT)&rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

// 	hof = SelectFont(hdc, hfMedium);
	hof = SelectFont(hdc, L->m_hFont);
	SetBkMode(hdc, TRANSPARENT);

	if     ( R->f & UADD) 			 col = RGB(  0,  0,255);
	else if( R->f & (BPHU|BSN|BAMP)) col = RGB(255,  0,  0);
	else							 col = RGB(  0,  0,  0);
	SetTextColor(hdc, col);
	Print( IsDlgButtonChecked( hDlg, IDC_HKE_UNIC ), L, R);
	DrawText(hdc, TMP, -1, (LPRECT)&rc,
			 (DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_TABSTOP) | (6<<8));

	SelectObject(hdc, hof);
}
/****************************************************************************/
static void DisplayReflectionInfo( HWND hDlg, LPLATTICE L, int num )
{
LPREFL	R = L->rptr+num;
int		p  = R->p;
/*
	if (R->f & RIP1) {
		if ( SVENHK ) {}
		else	{ p = -p; if (p==0x7FFFFFFF) p++; }
	}

	if (R->f & RIP2) p += P_PI;
*/
	sprintf(TMP, "%d",   R->h);        SetDlgItemText( hDlg, IDC_HKE_CHH,   TMP );
	sprintf(TMP, "%d",   R->k);        SetDlgItemText( hDlg, IDC_HKE_CHK,   TMP );
	sprintf(TMP, "%1.0f", -P2D(-p));   SetDlgItemText( hDlg, IDC_HKE_CHPHA, TMP );
	sprintf(TMP, "%1.0f", R->a);       SetDlgItemText( hDlg, IDC_HKE_CHAMP, TMP );
}
/****************************************************************************/
static void SetGroupFlag( HWND hDlg, LPLATTICE L )
{
int		i, hh=32000, kk=32000, f=0, j, num;
LPREFL	R = L->rptr;
HWND	hwl = GetDlgItem( hDlg, IDC_HKE_LIST );

	j = ListBox_GetCount(hwl);
	for(i=0; i<j; i++) {
		num = LOINT(ListBox_GetItemData(hwl, i));
		if (hh!=(R+num)->h || kk!=(R+num)->k) {
			hh = (R+num)->h; kk = (R+num)->k; f^=1;
		}
		ListBox_SetItemData(hwl, i, MAKELONG(num,f));
	}
}

#define REFMASK ((R->f & (SABS|UDEL))==0)
#define THRMASK ((R->a >= (L->Thr /**L->MaxAmp/100.*/))||(R->f & UADD))
/****************************************************************************/
static void FillHKList( HWND hDlg, LPLATTICE L )
{
	int		i, j, hk=32000;
	LPREFL	R = L->rptr;
	int		onum = HKe_selref;
	HWND	hwl = GetDlgItem( hDlg, IDC_HKE_LIST );

	SetWindowRedraw( hwl, FALSE );
	ListBox_ResetContent( hwl );
	if (IsDlgButtonChecked( hDlg, IDC_HKE_UNIC ))
	{
		for(i = 0, j = 0; i < L->nrefl; i++, R++)
		{
			if( REFMASK && THRMASK )
			{
				if (HK0(R) == HK(R))
				{
					hk = HK(R);
					ListBox_AddString(hwl, MAKELONG(i,0));
					if ( onum<= i)
					{
						HKe_selref = i; HKe_sel = j; onum=32000;
					}
					j++;
				}
			}
		}
		SetDlgItemText(hDlg, IDC_HKE_TEXT, szHdrShort);
	}
	else
	{
		for(i=0, j=0; i < L->nrefl; i++, R++)
		{
			if( REFMASK && THRMASK )
			{
				ListBox_AddString(hwl, MAKELONG(i,0));
				if ( onum <= i)
				{
					HKe_selref = i;	HKe_sel = j; onum=32000;
				}
				j++;
			}
		}
		if((L->cl>3.)||(L->dl>3.))
			SetDlgItemText(hDlg, IDC_HKE_TEXT, szHdrMod);
		else
			SetDlgItemText(hDlg, IDC_HKE_TEXT, szHdrFull);
	}

	SetGroupFlag( hDlg, L );
	ListBox_SetCurSel(hwl, HKe_sel);
	DisplayReflectionInfo( hDlg, L, HKe_selref);

	SetWindowRedraw(hwl, TRUE);
	InvalidateRect( GetDlgItem(hDlg, IDC_HKE_LIST), NULL, TRUE);
}
/****************************************************************************/
static	BOOL DeleteReflection( HWND hDlg, LPLATTICE L )
{
SGC		h, k;
BOOL	flg;
LPREFL	R = L->rptr;
int		i, j, num;
HWND	hwl = GetDlgItem( hDlg, IDC_HKE_LIST);

	SetWindowRedraw( hwl, FALSE );
	h = (SGC) GetDlgItemInt( hDlg, IDC_HKE_CHH, &flg, TRUE );
	if ( ! flg ) return dlgError( IDS_ERRINVALIDHINDX, hDlg);
	k = (SGC) GetDlgItemInt( hDlg, IDC_HKE_CHK, &flg, TRUE );
	if ( ! flg ) return dlgError( IDS_ERRINVALIDKINDX, hDlg);

	for( i=j=0; i<L->nrefl; i++,R++)
		if ((R->h == h) && (R->k == k) && REFMASK && THRMASK)
			{ R->f |= UDEL; R->f &= ~UADD; j++ ;}
	if (j==0) return dlgError(IDS_ERRNOSUCHREFL, hDlg);

	j = ListBox_GetCount(hwl);
	for(i=j-1; i>=0; i--) {
		num = LOINT(ListBox_GetItemData(hwl, i));
		if ((L->rptr+num)->f & UDEL)
			ListBox_DeleteString(hwl, i);
	}
	SetGroupFlag(hDlg, L);
	SetWindowRedraw( hwl, TRUE );
	InvalidateRect( hwl, NULL, TRUE );
	return TRUE;
}
/****************************************************************************/
static	BOOL AddReflection( HWND hDlg, LPLATTICE L )
{
SGC		h, k;
double	a=0,p=0;
BOOL	flg;
LPREFL	R = L->rptr;
int		i,j;
HWND	hwl = GetDlgItem( hDlg, IDC_HKE_LIST);

	SetWindowRedraw( hwl, FALSE );
	h = (SGC) GetDlgItemInt( hDlg, IDC_HKE_CHH, &flg, TRUE );
	if ( ! flg ) return dlgError( IDS_ERRINVALIDHINDX, hDlg);
	k = (SGC) GetDlgItemInt( hDlg, IDC_HKE_CHK, &flg, TRUE );
	if ( ! flg ) return dlgError( IDS_ERRINVALIDKINDX, hDlg);

	for( i=0, j=0; i<L->nrefl; i++,R++)
		if ((R->h0 == h) && (R->k0 == k)) break;
	if (i==L->nrefl/* && i>=MaxRfl-1*/) return dlgError(IDS_ERRNOPLACEFORREFL, hDlg);

	SendDlgItemMessage(hDlg, IDC_HKE_CHAMP, EM_SETMODIFY,TRUE,0);
	SendDlgItemMessage(hDlg, IDC_HKE_CHPHA, EM_SETMODIFY,TRUE,0);

	if (!GetDlgItemDouble( hDlg, IDC_HKE_CHAMP, &a, 0 ) || a<0. || a>10000. )
		return dlgError(IDS_ERRINVALIDAMP, hDlg);
	if (!GetDlgItemDouble( hDlg, IDC_HKE_CHPHA, &p, 0 ))
		return dlgError(IDS_ERRINVALIDPHA, hDlg);

	if (i!=L->nrefl) { // Already present
		if (R->f & SABS) return dlgError(IDS_ERRFORBIDDENREFL, hDlg);
		if (((R->f & UDEL)==0) && THRMASK) return dlgError(IDS_ERRREFLALREADYEXIST, hDlg);
		h = R->h; k = R->k;	// Assign h and k from unic asymetric unit
		while((i>=0) && (R->h==h) && (R->k==k)) {R--; i--;}
		R++; i++; j=-1;
		while((R->h==h) && (R->k==k)) {
			R->f &= ~UDEL;	R->f |= UADD;
			R->a = a;
			R->p = D2P(p);
//			if (R->f & RIP1) R->p = -R->p;
//			if (R->f & RIP2) R->p += P_PI;
			if (j<0 || !IsDlgButtonChecked( hDlg, IDC_HKE_UNIC))
				j = ListBox_AddString( hwl, MAKELONG( i, 0));
			R++; i++;
		}
	} else {
		memset(R, 0, sizeof(REFL));
		R->h = R->h0 = h;
		R->k = R->k0 = k;
		R->f |= UADD;
		R->a = R->a0 = a;
		R->p = R->p0 = D2P(p);
		if (!IsDlgButtonChecked( hDlg, IDC_HKE_UNIC)) j = ListBox_AddString( hwl, MAKELONG( i, 0));
		L->nrefl++;
	}
	SetGroupFlag(hDlg, L);
	ListBox_SetCurSel( hwl, j );
	SetWindowRedraw( hwl, TRUE );
	InvalidateRect( hwl, NULL, TRUE );

	return TRUE;
}
/****************************************************************************/
static	BOOL InverseReflection( HWND hDlg, LPLATTICE L)
{
SGC		h, k;
BOOL	flg;
LPREFL	R = L->rptr;
int		i;

	h = (SGC) GetDlgItemInt( hDlg, IDC_HKE_CHH, &flg, TRUE );
	if ( ! flg ) return dlgError( IDS_ERRINVALIDHINDX, hDlg);
	k = (SGC) GetDlgItemInt( hDlg, IDC_HKE_CHK, &flg, TRUE );
	if ( ! flg ) return dlgError( IDS_ERRINVALIDKINDX, hDlg);

	for( i=0; i<L->nrefl; i++,R++)	if ((R->h0 == h) && (R->k0 == k)) break;
	if (i==L->nrefl) return dlgError(IDS_ERRNOSUCHREFL, hDlg);
	if (R->f & SABS) return dlgError(IDS_ERRFORBIDDENREFL, hDlg);

	h = R->h; k = R->k;	// Assign h and k from unic asymetric unit
	while((i>=0) && (R->h==h) && (R->k==k)) {R--; i--;}
	R++;

	while((R->h==h) && (R->k==k) && (i < L->nrefl) ) {
		R->f &= ~UDEL;	R->f |= UADD;	R->p += P_PI;
		R++; i++;
	}
	InvalidateRect( GetDlgItem( hDlg, IDC_HKE_LIST), NULL, FALSE );
	return TRUE;
}
/****************************************************************************/
static	BOOL ChangeReflection( HWND hDlg, LPLATTICE L)
{
SGC		h, k;
BOOL	flg;
LPREFL	R = L->rptr;
int		i;
double	a=R->a,p=P2D(R->p);

	h = (SGC) GetDlgItemInt( hDlg, IDC_HKE_CHH, &flg, TRUE );
	if ( ! flg ) return dlgError( IDS_ERRINVALIDHINDX, hDlg);
	k = (SGC) GetDlgItemInt( hDlg, IDC_HKE_CHK, &flg, TRUE );
	if ( ! flg ) return dlgError( IDS_ERRINVALIDKINDX, hDlg);
	SendDlgItemMessage(hDlg, IDC_HKE_CHAMP, EM_SETMODIFY,TRUE,0);
	SendDlgItemMessage(hDlg, IDC_HKE_CHPHA, EM_SETMODIFY,TRUE,0);
	if (!GetDlgItemDouble( hDlg, IDC_HKE_CHAMP, &a, 0 ) || a<0. || a>10000. )
		return dlgError(IDS_ERRINVALIDAMP, hDlg);
	if (!GetDlgItemDouble( hDlg, IDC_HKE_CHPHA, &p, 0 ))
		return dlgError(IDS_ERRINVALIDPHA, hDlg);
	for( i=0; i<L->nrefl; i++,R++)	if ((R->h0 == h) && (R->k0 == k)) break;
	if (i==L->nrefl) return dlgError(IDS_ERRNOSUCHREFL, hDlg);
	if (R->f & SABS) return dlgError(IDS_ERRFORBIDDENREFL, hDlg);

	h = R->h; k = R->k;	// Assign h and k from unic asymetric unit

	while((i>=0) && (R->h==h) && (R->k==k)) {R--; i--;}
	R++;
	while((R->h==h) && (R->k==k)) {
		R->f &= ~UDEL;	R->f |= UADD;
		R->a = a;	R->p = D2P(p);
		R++;
	}
	InvalidateRect( GetDlgItem( hDlg, IDC_HKE_LIST), NULL, FALSE );
	return TRUE;
}
/****************************************************************************/
static	BOOL	SaveHKA( HWND hDlg, OBJ* O, LPLATTICE L, LPSTR fname, int AUnit )
{
FILE	*out;
int		i;
LPREFL	R = L->rptr;
LPSGINFO SI = L->SI+L->Spg;
SGC		hh=(SGC)-250, kk=(SGC)-250;
double	xx = O->NI.Scale * 1e10;
HCURSOR	hoc;

	if ((out = fopen( fname, "w")) == NULL) return dlgError(IDS_ERRCREATEFILE, hDlg);

	hoc = SetCursor(hcWait);
	fprintf(out,";	Project\t%s\n", O->fname);
	fprintf(out,"SpaceGroup\t%d\n", L->Spg);
	fprintf(out,"Cell_A\t%G\n",		SI->ral*xx);
	fprintf(out,"Cell_B\t%G\n",		SI->rbl*xx);
	fprintf(out,"Gamma\t%G\n",		R2D(fabs(SI->rgamma)));

	if((L->cl>3.)||(L->dl>3.))
	{
		fprintf(out,"Cell_a\t%G\n",		SI->rcl*xx);
		fprintf(out,"Cell_b\t%G\n",		SI->rdl*xx);
		fprintf(out,"ab_Angle\t%G\n",	R2D(SI->rgamma2));
	}

	if(AUnit) fprintf(out, "Asym_Unit\t1\n");

	if((L->cl>3.)||(L->dl>3.))	fprintf(out, szHdrMod );
	else if(AUnit)	fprintf(out, szHdrShort );
		 else		fprintf(out, szHdrFull  );
	fprintf(out, "\n");

	for(i=0;i<strlen(szHdrFull);i++) fputc( '-', out); fprintf(out, "\n");
	for(i=0; i<L->nrefl; i++, R++) if (REFMASK && THRMASK) {

		if( AUnit && (*(int*)(&R->h0) != *(int*)(&R->h)) ) continue;
		if (hh!=R->h || kk!=R->k) {	hh = R->h; kk = R->k; fprintf(out,"\n");}
		Print(AUnit, L, R);	fputs(TMP, out);
		fprintf(out, "\n");
	}
	SetCursor(hoc);
	if (ferror(out)) { fclose(out); return dlgError(IDS_ERRWRITEFILE, hDlg); }
	fclose(out);
	return TRUE;
}
/****************************************************************************/
#define	REVHK	0x01
#define SGNXCH	0x02
#define ROT180	0x04
#define	AUNIT	0x08
#define	ELDATA	0x8000
static BOOL	bNewHKL;
static int		nn[10];

static BOOL GetNewHeader( LPSTR buf )
{
LPSTR	p;
int		fnum;

	buf[strlen(buf)-1]=0;	// To cut \n
	p=strchr(buf, ':')+1;
	fnum=1;
	memset(nn,0,sizeof(nn));
	while (*p) {
		switch (*p) {
			case 'x': nn[fnum]=0;  fnum++; break; // Unused field
			case 'h': nn[fnum]=1;  fnum++; break;
			case 'k': nn[fnum]=2;  fnum++; break;
			case 'l': nn[fnum]=3;  fnum++; break;
			case 'a': nn[fnum]=4;  fnum++; break;
			case 'p': nn[fnum]=5;  fnum++; break;
			case 'q': nn[fnum]=6;  fnum++; break;
			case 'f': nn[fnum]=7;  fnum++; break;
			case 'g': nn[fnum]=8;  fnum++; break;
			case 'd': nn[fnum]=9;  fnum++; break;
			case 's': nn[fnum]=10; fnum++; break;
			case ' ':
			case '\t': break;
			default:
				sprintf(TMP, "Unknow character '%c' found in format string.\nContinue reading this file?", *p );
				if (MessageBox( MainhWnd, TMP, szProg, MB_ICONINFORMATION | MB_YESNO ) == IDNO ) {
					return FALSE;
				}
		}
		p++;
	}
	return TRUE;
}
/***************************************************************************/
int	Load_Film_Header(LPLATTICE L, FILE * inf)
{
float	a,k;
int		ff=0;
char	cmd[20], *p;
float	d[6];

	L->gamma = M_PI/2.;
	L->al = L->bl = 16;
	bNewHKL = FALSE;

	while (feof(inf)==0)
	{
		if(fgets (TMP,80,inf)  == NULL) break;
		if(TMP[0] == ';') continue;
		if(strstr(TMP,"------")) break;
		strlwr( TMP );
		if (strstr(TMP,"format:"))
		{
			bNewHKL = GetNewHeader( TMP );
			continue;
		}
		if (strstr(TMP,"cell:"))
		{
			p = strchr(TMP, ':')+1;
			if (sscanf(p,"%f %f %f %f %f %f", &d[0], &d[1], &d[2], &d[3], &d[4], &d[5]) != 6) continue;
			L->al = d[0];
			L->bl = d[1];
			L->cl = d[2];
			L->gamma = D2R(d[5]);
			continue;
		}
		if(sscanf(TMP, "%10s%f", cmd, &a) < 2) continue;
		strlwr(cmd);

			 if(strstr(cmd,"revhk" ))	{if(a>0.1) ff |= REVHK;}
		else if(strstr(cmd,"sgnxch"))	{if(a>0.1) ff |= SGNXCH;}
		else if(strstr(cmd,"rot180"))	{if(a>0.1) ff |= ROT180;}
		else if(strstr(cmd,"asym_unit"))	{if(a>0.1) ff |= AUNIT;}
		else if(strstr(cmd,"cell_a"))	L->al = a;
		else if(strstr(cmd,"cell_b"))	L->bl = a;
		else if(strstr(cmd,"cell_c"))	L->cl = a;
		else if(strstr(cmd,"gamma"))	L->gamma = D2R(a);
		else if(strstr(cmd,"spacegroup")) L->Spg = (int)a;
//		else if(strstr(cmd,"eld"))    ff |= ELDATA;
	}
//	if (L->dxs < 64) L->dxs = 256;
	L->gamma = M_PI - L->gamma;
	k = fabs(256. / sin(L->gamma));
	L->al = L->ax0 = L->ax = k / L->al;
	L->ay = L->ay0 = 0.;
	// changed by Peter on 10 Oct 2001 - solves origin after loading
//	L->bl = L->by0 = L->by = k / L->bl;
//	L->ay = L->ay0 = L->bx = L->bx0 = 0.;
	L->bl = k / L->bl;
	L->bx = L->bx0 = L->bl * cos(L->gamma);
	L->by = L->by0 = L->bl * sin(L->gamma);

	L->cl = L->cx0 = L->cx = L->cy0 = L->cy = 0.;
	L->dl = L->dx0 = L->dx = L->dy0 = L->dy = 0.;

	return ff;
}
/***************************************************************************/
// Added by Peter on 1 Sep 2001
typedef struct
{
	SGC h, k, l, m;
	DWORD f;
	double a, a0;
	PHASE p, p0;//, x0, y0;

} READ_REFL;

#include <vector>
using namespace std;
/***************************************************************************/
void	Load_Film_Data(LPLATTICE L, FILE * inf, int ff)
{
	// Added by Peter on 1 Sep 2001
	vector<READ_REFL>  reflst;
	vector<READ_REFL>::iterator rfi;
	READ_REFL rref;

	LPREFL	R;// = L->rptr;
	int		i;
	int		h, k, p, p0;//, n;
	float	p3, p4, p5, p6, p7;
	float	a0, z = 0., amax = 1.;
//float	a = tan(L->tt) / (float)L->xs;
//float	sa = L->al * a * sin(L->ta);
//float	sb = L->bl * a * sin(L->ta + M_PI - L->gamma);
	float	a;
	char	aa[10][20];
	LPSTR	buf;
	int		fn, itmp;
	float	ftmp;
	LPSTR	pp, qq;
//	int		MaxRfl = L->nrefl;
	int nSG = L->Spg;
	LPSYMMETRY lpSymm = &Symmetry[nSG];
	int iMX;

	while ( (feof(inf) == 0) && (strstr(fgets (TMP, 256, inf),"-------") == NULL) )
		;
/*
	memset( R, 0, sizeof(REFL) * L->nrefl);
//	n = (int)sqrt(MaxRfl/2.);
	L->nrefl = 0;

	R = L->rptr;
	for(i = 0; i < L->nrefl; i++, R++)
		R->f = BAMP;
	R = L->rptr;
*/
	while (feof(inf) == 0)
	{
		if(fgets (TMP, 256, inf) == NULL)
			break;
		// TODO: trim left
		if(TMP[0] == ';')
			continue;
		if (bNewHKL)
		{
			buf = TMP;
			buf[strlen(buf)-1]=0;
			memset(aa, 0, sizeof(aa));
			pp = buf;
			fn = 1;
			while(*pp)
			{
				while(*pp && (*pp==' ' || *pp=='\t')) pp++;
				if (*pp==0) break;
				qq = aa[fn];
				while(*pp && *pp!=' ' && *pp!='\t') *(qq++) = *(pp++);
				fn++;
			}
			for(i = 1; i <= fn; i++)
				if(nn[i]==5) break;	// phase field present
			if (i > fn) goto skiprefl;
			if (fn > 0)
			{
				for(i = 1; i <= fn; i++)
				{
					itmp = 0;
					ftmp = 0;
					switch(nn[i])
					{
						case 1: if (1!=sscanf(aa[i],"%d",&itmp)) goto skiprefl; h = itmp; break;	// h
						case 2: if (1!=sscanf(aa[i],"%d",&itmp)) goto skiprefl; k = itmp; break;	// k
						case 4: if (1!=sscanf(aa[i],"%f",&ftmp)) goto skiprefl; a0=a = ftmp; break;	// amp
						case 5: if (1!=sscanf(aa[i],"%f",&ftmp)) goto skiprefl; p0=p = D2P(ftmp); break; // pha
						case 8:	// flags
							pp = aa[i];
							while (*pp)
							{
								switch (*pp)
								{
									case '-':
									case 'x':
										 goto skiprefl;
								}
								pp++;
							}
							break;
					}
				}
			}
		}
		else
		{
			i = sscanf(TMP, "%d%d%f%f%f%f%f",&h,&k,&p3,&p4,&p5,&p6,&p7);
			if(i < 4)
				continue;
			// we have the Asymm_unit flag in the reading file
			if(ff & AUNIT)
			{
				p0 = p = D2P(p4);
				a0 = a = p3;
			}
			else
			{
				if(i <= 5)
				{
					p0 = p = D2P(p3);
					a0 = a = p4;
				}
				else
				{
					p0 = D2P(p5);
					p = D2P(p6);
					a0 = p3;
					a = p4;
				}
			}

//			z = (float)h * sa + (float)k * sb;
			if(ff & REVHK)  { i= h; h= k; k= i; z=-z;}
			if(ff & SGNXCH) { k=-k; z=-z;}
			if(ff & ROT180) { h=-h; k=-k;}
		}
		if((h < 0) || ((h == 0) && (k < 0)))
		{
			h=-h; k=-k; z=-z; p=-p; p0 = -p0;
		}
		if((k == 0) && (h == 0)) continue;

		rref.f = BAMP;
		if(a > 0) rref.f = 0; else a = 0.;
//		if(a > 0) R->f = 0; else a = 0.;
		if(a > amax) amax = a;
		if(a0 > amax) amax = a0;
		// added by Peter on 1 Sep 2001
		rref.h = h;
		rref.k = k;
		rref.a = a;
		rref.a0 = a0;
		rref.p = p;
		rref.p0 = p0;
//		rref.x0 = L->ax*h + L->bx*k;
//		rref.y0 = L->ay*h + L->by*k;
		reflst.push_back(rref);
		// added by Peter on 5 Sep 2001
		// we have the Asymm_unit flag in the reading file
		if(ff & AUNIT)
		{
			int *pMX2;
			// skip identity matrix
			for(iMX = 1, pMX2 = &lpSymm->mx[1][0]; iMX < lpSymm->N >> 1; iMX++, pMX2+=sizeof(MX2))
			{
//				pMX = lpSymm->mx;
				rref.h = pMX2[0]*h + pMX2[3]*k;
				rref.k = pMX2[1]*h + pMX2[4]*k;

				reflst.push_back(rref);
			}
		}
		// removed by Peter on 1 Sep 2001
/*
		R->h0 = R->h = h;
		R->k0 = R->k = k;
		R->l0 = R->l = 0;
		R->m0 = R->m = 0;
		R->x0 = L->ax*h + L->bx*k;
		R->y0 = L->ay*h + L->by*k;

		R->a0 = a0; R->a = a;
		R->p0 = p0; R->p = p;
		R++; L->nrefl++;
		if(L->nrefl >= MaxRfl-1) break;
*/
skiprefl:;
	}
	// added by Peter on 1 Sep 2001
	L->nrefl = reflst.size();
	R = L->rptr = (LPREFL)realloc(L->rptr, sizeof(REFL)*L->nrefl);
	memset( R, 0, sizeof(REFL) * L->nrefl);
//	FILE *pTest = fopen("C:\\temp\\test.txt", "wt");
	for(rfi = reflst.begin(); rfi != reflst.end(); ++rfi, R++)
	{
		R->a = rfi->a; R->a0 = rfi->a0;
		R->p = rfi->p; R->p0 = rfi->p0;
		R->h0 = R->h = rfi->h;
		R->k0 = R->k = rfi->k;
		R->l0 = R->l = 0;
		R->m0 = R->m = 0;
		R->x0 = L->ax*rfi->h + L->bx*rfi->k;
		R->y0 = L->ay*rfi->h + L->by*rfi->k;
//		fprintf(pTest, "%d\t%d\t%f\t%f\t%d\t%d\n", R->h, R->k, R->a, R->a0, R->p, R->p0);
	}
//	fclose(pTest);
	reflst.clear();
	// removed by Peter on 1 Sep 2001
//	R = L->rptr;
	L->MaxAmp = amax;
//	amax = 1e7/amax;
//	for(i=L->nrefl; i>0; i--) {R->a *= amax; R->a0 *= amax; R++;}
}
/***************************************************************************/
BOOL	SubstituteAmplitudes(LPLATTICE L, FILE *inf, int ff)
{
	LPREFL	R = L->rptr;
	int		i;
	int		max_h, max_k, h, k;
	float	p3, p4, p5, p6, p7, p8;
	float	a0, amax = 1.;
	BOOL	first = FALSE;
	int		MaxRfl = L->nrefl;

	// ADDED by Peter on 23 Aug 2001
	if( inf == NULL )
		return FALSE;

	while ( (feof(inf) == 0) && (strstr(fgets (TMP, 256, inf),"-------") == NULL) )
		;

	while (feof(inf) == 0)
	{
		if(fgets (TMP, 256, inf) == NULL)
			break;
		// TODO: trim left
		if(TMP[0] == ';')
			continue;
		i = sscanf(TMP, "%d%d%f%f%f%f%f",&h,&k,&p3,&p4,&p5,&p6,&p7,&p8);
		if(i < 4)
			continue;
		if ((i == 4) || (i == 7))
			a0 = p4;	// "h k Aobs Aest", or "h k Aobs Aest dx dy sigma"
		else if((i == 5) || (i == 8))
			a0 = p5;	// "h k d-val Aobs Aest", or  "h k d-val Aobs Aest dx dy sigma"
		else
			return FALSE;		// Unknown format

		if (!first)
		{
			max_h = max_k = 0;
			for(i = 0, R = L->rptr; i < L->nrefl; i++, R++)
			{
				R->f |= UDEL;
				R->a = R->a0 = 0.;
				if(abs(R->h0) > max_h)
					max_h = abs(R->h0);
				if(abs(R->k0) > max_k)
					max_k = abs(R->k0);
			}
//			n = (int)sqrt(MaxRfl/2.);
			first = TRUE;
		}
//		if((abs(h) >= n) || abs(k) >= n)
//			continue;
		if((abs(h) >= max_h) || abs(k) >= max_k)
			continue;

		if((h < 0) || ((h == 0) && (k < 0)))
		{
			h = -h;
			k = -k;
		}
		if((k == 0) && (h == 0))
			continue;
		// find reflection
		for(i = 0, R = L->rptr; i < L->nrefl; i++, R++)
		{
			if((R->h0 == h) && (R->k0 == k))
				break;
		}
		if (i == L->nrefl)
			continue;
		if (R->f & SABS)
			continue;

		if ((R->f & UDEL) == 0)
			R->a0 = (a0 + R->a0) * .5;	// Fridel pair
//			R->a = R->a0 = (a0 + R->a) * .5;	// Fridel pair
		else
			R->a0 = a0;				// It self
//			R->a = R->a0 = a0;				// It self
		R->f &= ~UDEL;
	}
	for(i = 0, R = L->rptr; i < L->nrefl; i++, R++)
	{
		if(((R->f & UDEL)==0) && (R->a > amax))
		{
//			amax = R->a;
			amax = R->a0;
		}
	}
	L->MaxAmp = amax;
	for(i = 0, R = L->rptr; i < L->nrefl; i++, R++)
	{
		if ((R->f & UDEL)==0)
		{
			R->f &= ~(BSN | BAMP | BPHU );
		}
	}
	return TRUE;
}
/****************************************************************************/
static	UINT	LoadHKA( HWND hDlg, LPLATTICE L, LPSTR fname )
{
FILE	*inf;
int		ff;
	if ((inf = fopen( fname, "rt")) == NULL) return dlgError(IDS_ERROPENFILE, hDlg);
	ff = Load_Film_Header(L, inf);
	fseek(inf, 0, SEEK_SET);

	Load_Film_Data(L, inf, ff);

//	fclose(inf); return dlgError("Error reading file", hDlg);
	fclose(inf);
	if(ff & AUNIT)
		ff = XRFL;
	ff |= TRUE;
	return ff;
}
/****************************************************************************/
static	UINT	SubstituteHKA( HWND hDlg, LPLATTICE L, LPSTR fname )
{
	FILE	*inf;
	int		ff = 0;

	if ((inf = fopen( fname, "rt")) == NULL)
		return dlgError(IDS_ERROPENFILE, hDlg);

	while (feof(inf) == 0)
	{
		if(fgets (TMP, 256, inf) == NULL)
			break;
		if(TMP[0] == ';')
			continue;
		if(strstr(TMP,"------"))
			break;
	}
	fseek(inf, 0, SEEK_SET);

	if (!SubstituteAmplitudes(L, inf, ff))
	{
		fclose(inf);
		return dlgError(IDS_ERRREADFILE, hDlg);
	}
	fclose(inf);
	if(ff & AUNIT)
		ff = XRFL;
	ff |= TRUE;
	return ff;
}
/****************************************************************************/
static	void	FixButton(HWND hDlg, int f)
{
	if(f != 1)	CheckDlgButton( hDlg, IDC_HKE_INVFIX, 0);
	if(f != 2)	CheckDlgButton( hDlg, IDC_HKE_DELFIX, 0);
	SendDlgItemMessage(hDlg, IDC_HKE_INVERSE, BM_SETSTATE,IsDlgButtonChecked( hDlg, IDC_HKE_INVFIX ), 0);
	SendDlgItemMessage(hDlg, IDC_HKE_DELETE,  BM_SETSTATE,IsDlgButtonChecked( hDlg, IDC_HKE_DELFIX ), 0);
}
/****************************************************************************/
BOOL FAR PASCAL Save3dDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL FAR PASCAL HKeditDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND	ParentWnd = (HWND)GetWindowLong( hDlg, DWL_USER );
	OBJ*	O = NULL;
	LPLATTICE L = NULL;
	UINT f;

	if (ParentWnd)
	{
		O = OBJ::GetOBJ(ParentWnd);
		L = (LPLATTICE)&O->dummy;
	}

	switch(message)
	{
		case WM_INITDIALOG:
			{
				SetWindowLong( hDlg, DWL_USER, lParam);
				L = (LPLATTICE)&OBJ::GetOBJ((HWND)lParam)->dummy;
#ifdef USE_FONT_MANAGER
				int nFntSizeW;
				L->m_hFont = FontManager::GetFont(FM_MEDIUM_FONT_CW, nFntSizeW, L->m_nFontH);
				SendDlgItemMessage(hDlg, IDC_HKE_TEXT, WM_SETFONT, (WPARAM)L->m_hFont, 0);
#else
				L->m_nFontH = fntm_h;
				L->m_hFont = hfMedium;
				SendDlgItemMessage(hDlg, IDC_HKE_TEXT, WM_SETFONT, (WPARAM)hfMedium, 0);
#endif
				SetDlgItemDouble( hDlg, IDC_HKE_THRESHOLD, L->Thr, 0);
				FillHKList( hDlg, L );
				wndCenter(hDlg, MDIhWnd, 0);

			}
			return TRUE;

		case WM_DRAWITEM:
			DrawListItem(hDlg, (LPDRAWITEMSTRUCT)lParam, L);
			break;

		case WM_COMPAREITEM:
		{
			LPCOMPAREITEMSTRUCT c = (LPCOMPAREITEMSTRUCT)lParam;
			if (LOWORD(c->itemData1) < LOWORD(c->itemData2)) return -1;
			if (LOWORD(c->itemData1) > LOWORD(c->itemData2)) return 1;
		}
			return 0;

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lp = (LPMEASUREITEMSTRUCT)lParam;
			int nFntSizeH = 10; // default value
			if( NULL == L )
			{
				int nFntSizeW;
				FontManager::GetFont(FM_MEDIUM_FONT_CW, nFntSizeW, nFntSizeH);
			}
			else
			{
				nFntSizeH = L->m_nFontH;
			}
		    ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = nFntSizeH + 4;
// 			((LPMEASUREITEMSTRUCT)lParam)->itemHeight = fntm_h+4;
		}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_HKE_UNIC:
					FillHKList( hDlg, L );
					break;
				case IDC_HKE_DELFIX:
					FixButton(hDlg, 2);
					break;
				case IDC_HKE_INVFIX:
					FixButton(hDlg, 1);
					break;

				case IDC_HKE_ADD:
					AddReflection( hDlg, L );
					goto xxx;
				case IDC_HKE_DELETE:
					DeleteReflection( hDlg, L );
					goto xxx;
				case IDC_HKE_INVERSE:
					InverseReflection( hDlg, L);
					DisplayReflectionInfo( hDlg, L, HKe_selref );
					goto xxx;
				case IDC_HKE_CHANGE:
					ChangeReflection( hDlg, L);
xxx:				FixButton(hDlg, 0);
					SetWindowRedraw( GetDlgItem( hDlg, IDC_HKE_LIST), TRUE );
//					PaintOREFDlg( O, TRUE );
					DoOriginRefine( ParentWnd, O, L, 0 );
					break;

				case IDC_HKE_LIST:
					switch ( HIWORD(wParam) )
					{
						case LBN_SELCHANGE:
							HKe_sel = ListBox_GetCurSel((HWND)lParam);
							HKe_selref = LOINT(ListBox_GetItemData((HWND)lParam, HKe_sel));
							DisplayReflectionInfo( hDlg, L, HKe_selref );
							if(IsDlgButtonChecked( hDlg, IDC_HKE_INVFIX ))
							{
								InverseReflection( hDlg, L);
								SetWindowRedraw	( GetDlgItem( hDlg, IDC_HKE_LIST), TRUE );
							}
							if(IsDlgButtonChecked( hDlg, IDC_HKE_DELFIX ))
							{
								DeleteReflection( hDlg, L );
								SetWindowRedraw	( GetDlgItem( hDlg, IDC_HKE_LIST), TRUE );
							}
							DisplayReflectionInfo( hDlg, L, HKe_selref );
							break;
					}
					break;
				case IDC_HKE_APPLY:
					{
						double thr;
						if (GetDlgItemDouble(hDlg,IDC_HKE_THRESHOLD, &thr, 0))
						{
							if (thr>9999.) thr = 9999;
							if (thr<0.)   thr = 0.;
							L->Thr = thr;
							FillHKList( hDlg, L );
							DoOriginRefine( ParentWnd, O, L, 0 );
						}
						SetDlgItemDouble( hDlg, IDC_HKE_THRESHOLD, L->Thr, 0);
					}
                    break;
				case IDC_HKE_SAVE:
					_splitpath( O->fname, NULL, NULL, TMP, NULL );	// take file name only
					strcat(TMP, ".hka");
					if (!fioGetFileNameDialog( MainhWnd, "Save HK list", &ofNames[OFN_HKLIST], TMP, TRUE))
						return FALSE;
					SaveHKA( hDlg, O, L, fioFileName(), IsDlgButtonChecked( hDlg, IDC_HKE_UNIC ));
					FixButton(hDlg, 0);
					break;
				case IDC_HKE_SAVE3D:
					DialogBoxParam(hInst, "HKL3DSAVE", hDlg, Save3dDlgProc, (LPARAM)ParentWnd);
					return TRUE;
					break;

				case IDC_HKE_LOAD:
					if (!fioGetFileNameDialog( MainhWnd, "Load HK list", &ofNames[OFN_HKLIST], "", FALSE))
						return FALSE;
					f = LoadHKA( hDlg, L, fioFileName() );
					if (IsDlgButtonChecked( hDlg, IDC_HKE_UNIC )) f |= XRFL;
					DoOriginRefine( ParentWnd, O, L, DOR_FULL );
					ChangeSymmetry	( O, L, TRUE, O->hDlg );
					FillHKList( hDlg, L );
					FixButton(hDlg, 0);
					break;

				case IDC_HKE_SUBFILE:
					// changed by Peter for HKE list (instead HKA list)
//					if (!fioGetFileNameDialog( MainhWnd, "Substitute Amplitudes", &ofNames[OFN_HKLIST], "", FALSE))
					if (!fioGetFileNameDialog( MainhWnd, "Substitute Amplitudes", &ofNames[OFN_HKELIST], "", FALSE))
						return FALSE;
					f = SubstituteHKA( hDlg, L, fioFileName() );
					//					if ( !f ) { MessageBeep(MB_ICONQUESTION); break; }
					if (IsDlgButtonChecked( hDlg, IDC_HKE_UNIC )) f |= XRFL;
					DoOriginRefine( ParentWnd, O, L, DOR_FULL );
					ChangeSymmetry	( O, L, TRUE, O->hDlg );
					FillHKList( hDlg, L );
					FixButton(hDlg, 0);
					break;

				case IDC_HKE_SUBRESTORE:
				{ int sp = L->Spg;
					UpdateORef( O, L );
					DoOriginRefine( ParentWnd, O, L, DOR_EXTRACT | DOR_FULL );
					ChangeSymmetry	( O, L, TRUE, O->hDlg );
					FillHKList( hDlg, L );
					FixButton(hDlg, 0);
				}
					break;
				case IDC_HKE_SUBCBRD:
					break;

				case IDC_HKE_HELP:
					WinHelp(hDlg, "crispd.hlp", HELP_KEY, (DWORD)(LPSTR)"HK Edit");
					break;

				case IDCANCEL:
					UpdateORef( O, L );
					DoOriginRefine( ParentWnd, O, L, DOR_EXTRACT | DOR_FULL );
				case IDOK:
					EndDialog(hDlg, TRUE);
					return (TRUE);
			default:
				return FALSE;
			}
			break;

		default:
			return FALSE;
	}
	return TRUE; /* Processed the message */
}
/****************************************************************************/
static BOOL	Calc2to3d( OBJ* O, HWND hDlg )
{
	int		dt;
	int		u1,v1, u2,v2;
	int		uh, uk, ul;
	int		vh, vk, vl;
	int		h1, k1, l1;
	int		h2, k2, l2;
	int		a,b,c;
	LPLATTICE S = (LPLATTICE) &O->dummy;

	u1 = S->uu[0]; v1 = S->vv[0];
	u2 = S->uu[1]; v2 = S->vv[1];
	h1 = S->hh[0]; k1 = S->kk[0]; l1 = S->ll[0];
	h2 = S->hh[1]; k2 = S->kk[1]; l2 = S->ll[1];

//	S->f2to3 = FALSE;

	dt = u1*v2-u2*v1;

	SetDlgItemText( hDlg, IDC_SD_ZONEAX, "[Invalid]" );

	if (dt==0) return FALSE;

	uh = v2*h1-v1*h2; if (uh%dt) return FALSE; uh /= dt;
	uk = v2*k1-v1*k2; if (uk%dt) return FALSE; uk /= dt;
	ul = v2*l1-v1*l2; if (ul%dt) return FALSE; ul /= dt;

	vh = u1*h2-u2*h1; if (vh%dt) return FALSE; vh /= dt;
	vk = u1*k2-u2*k1; if (vk%dt) return FALSE; vk /= dt;
	vl = u1*l2-u2*l1; if (vl%dt) return FALSE; vl /= dt;

	if (abs(uk*vl-vk*ul)+abs(vh*ul-uh*vl)+abs(uh*vk-vh*uk) == 0) return FALSE;

	S->uh = uh; S->uk = uk; S->ul = ul;
	S->vh = vh; S->vk = vk; S->vl = vl;

//	S->f2to3 = TRUE;
	a = uk*vl-vk*ul;
	b = vh*ul-uh*vl;
	c = uh*vk-vh*uk;

	if      (a < 0)            { a = -a; b = -b; c = -c; }
	else if (a == 0 && b < 0)  {         b = -b; c = -c; }
	else if (a == 0 && b == 0 && c < 0 ) {       c = -c; }

	sprintf(TMP, "[%d,%d,%d]", a, b, c);
	SetDlgItemText( hDlg, IDC_SD_ZONEAX, TMP );

	return TRUE;
}

static WORD idn_U[] = { IDC_SD_UU1, IDC_SD_UU2 };
static WORD idn_V[] = { IDC_SD_VV1, IDC_SD_VV2 };
static WORD idn_H[] = { IDC_SD_HH1, IDC_SD_HH2 };
static WORD idn_K[] = { IDC_SD_KK1, IDC_SD_KK2 };
static WORD idn_L[] = { IDC_SD_LL1, IDC_SD_LL2 };
/*-------------------------------------------------------------------------*/
static VOID UpdateUVHKL( HWND hDlg, WPARAM wParam, LPARAM lParam, LPLATTICE S )
{
int		i, d=0, f;
static BOOL rec = FALSE;
BOOL	bUpdate = FALSE;

	if (rec) return;
	rec = TRUE;

	if (HIWORD(wParam) == EN_KILLFOCUS ||
		HIWORD(wParam) == EN_CHANGE ) {
		d = GetDlgItemInt( hDlg, LOWORD(wParam), &f, TRUE);
		for(i=0;i<2;i++) {
			if (idn_U[i]==LOWORD(wParam)) S->uu[i] =d;
			if (idn_V[i]==LOWORD(wParam)) S->vv[i] =d;
			if (idn_H[i]==LOWORD(wParam)) S->hh[i] =d;
			if (idn_K[i]==LOWORD(wParam)) S->kk[i] =d;
			if (idn_L[i]==LOWORD(wParam)) S->ll[i] =d;
		}
		if (HIWORD(wParam) == EN_KILLFOCUS) bUpdate = TRUE;
	}

	if (HIWORD(wParam) == SE_MOVE) {
		if (LOWORD(lParam)==JC_UP)   d=1;
		if (LOWORD(lParam)==JC_DOWN) d=-1;
		for(i=0;i<2;i++) {
			if (idn_U[i]==LOWORD(wParam)) S->uu[i]+=d;
			if (idn_V[i]==LOWORD(wParam)) S->vv[i]+=d;
			if (idn_H[i]==LOWORD(wParam)) S->hh[i]+=d;
			if (idn_K[i]==LOWORD(wParam)) S->kk[i]+=d;
			if (idn_L[i]==LOWORD(wParam)) S->ll[i]+=d;
		}
		bUpdate = TRUE;
	}
	if (bUpdate) {
		SetDlgItemInt(hDlg,IDC_SD_UU1,S->uu[0],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_UU2,S->uu[1],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_VV1,S->vv[0],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_VV2,S->vv[1],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_HH1,S->hh[0],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_HH2,S->hh[1],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_KK1,S->kk[0],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_KK2,S->kk[1],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_LL1,S->ll[0],TRUE);
		SetDlgItemInt(hDlg,IDC_SD_LL2,S->ll[1],TRUE);
	}
	rec = FALSE;
}
/*-------------------------------------------------------------------------*/
static	BOOL	Save3dHKL( HWND hDlg, OBJ* O, LPLATTICE L, LPSTR fname )
{
	FILE	*out;
	int		i;
	LPREFL	R = L->rptr;
	LPSGINFO SI = L->SI+L->Spg;
	SGC		hh=(SGC)-250, kk=(SGC)-250;
	double	xx = O->NI.Scale * 1e10;
	int		h, k, l, p;
	HCURSOR hoc;

	if ((out = fopen( fname, "w")) == NULL) return dlgError(IDS_ERRCREATEFILE, hDlg);

	hoc = SetCursor(hcWait);
	fprintf(out,";	Project\t%s\n", O->fname);
	fprintf(out,"; SpaceGroup\t%d\n", L->Spg);
	fprintf(out,"; Cell_A\t%G\n",		SI->ral*xx);
	fprintf(out,"; Cell_B\t%G\n",		SI->rbl*xx);
	fprintf(out,"; Gamma\t%G\n",		R2D(fabs(SI->rgamma)));

	fprintf(out, szHdr3 ); fprintf(out, "\n");
	fprintf(out, szCellp, SI->ral*xx, SI->rbl*xx, R2D(fabs(SI->rgamma)) );
	fprintf(out, "\n");

	for(i=0;i<strlen(szHdrFull);i++) fputc( '-', out); fprintf(out, "\n");

	for(i=0; i<L->nrefl; i++, R++) if (REFMASK && THRMASK) {
		if (hh!=R->h || kk!=R->k) {	hh = R->h; kk = R->k; fprintf(out,"\n");}
		h = R->h0*L->uh + R->k0*L->vh;
		k = R->h0*L->uk + R->k0*L->vk;
		l = R->h0*L->ul + R->k0*L->vl;
		p  = R->p;
		if (R->f & RIP1) { p = -p; if (p==0x7FFFFFFF) p++; }
		if (R->f & RIP2) p += P_PI;
		sprintf(TMP, szFmt3, h, k, l, R->a, -P2D(-p) );
		fputs(TMP, out);
		fprintf(out, "\n");
	}
	SetCursor(hoc);
	if (ferror(out)) { fclose(out); return dlgError(IDS_ERRWRITEFILE, hDlg); }
	fclose(out);
	return TRUE;
}
/****************************************************************************/
BOOL CALLBACK Save3dDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND	ParentWnd = (HWND)GetWindowLong( hDlg, DWL_USER );
	OBJ*	O;
	LPLATTICE L;
	int i;

	if (ParentWnd) {
		O = OBJ::GetOBJ(ParentWnd);
		L = (LPLATTICE)&O->dummy;
	}

	switch(message)
	{
		case WM_INITDIALOG:
			SetWindowLong( hDlg, DWL_USER, lParam);
			L = (LPLATTICE)&OBJ::GetOBJ((HWND)lParam)->dummy;
			wndCenter(hDlg, MDIhWnd, 0);
			for(i=0; i<2; i++) {
				L->uu[i] = def_uu[i]; L->vv[i] = def_vv[i];
				L->hh[i] = def_hh[i]; L->kk[i] = def_kk[i]; L->ll[i] = def_ll[i];
				SetDlgItemInt( hDlg, idn_U[i], L->uu[i], TRUE );
				SetDlgItemInt( hDlg, idn_V[i], L->vv[i], TRUE );
				SetDlgItemInt( hDlg, idn_H[i], L->hh[i], TRUE );
				SetDlgItemInt( hDlg, idn_K[i], L->kk[i], TRUE );
				SetDlgItemInt( hDlg, idn_L[i], L->ll[i], TRUE );
			}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_SD_UU1:	case IDC_SD_VV1:
				case IDC_SD_HH1:	case IDC_SD_KK1:    case IDC_SD_LL1:
				case IDC_SD_UU2:	case IDC_SD_VV2:
				case IDC_SD_HH2:	case IDC_SD_KK2:    case IDC_SD_LL2:
					UpdateUVHKL( hDlg, wParam, lParam, L );
					Calc2to3d( O, hDlg );
					return TRUE;

				case IDOK:
					_splitpath( O->fname, NULL, NULL, TMP, NULL );	// take file name only
					strcat(TMP, ".hke");
					if (!fioGetFileNameDialog( MainhWnd, "Save 3D HKL list", &ofNames[OFN_HKLLIST], TMP, TRUE))
						return FALSE;
					Save3dHKL( hDlg, O, L, fioFileName() );
				case IDCANCEL:
					EndDialog(hDlg, TRUE);
					return (TRUE);
			}
			return FALSE;
	}
	return FALSE;
}

