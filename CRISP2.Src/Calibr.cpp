/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.EDCalibration * by ML *********************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "ft32util.h"

#include "HelperFn.h"

#pragma optimize ("",off)

/****************************************************************************/
typedef struct
{
	int		nStands;	// Number of standards in dspace.tbl
	int		nStand;		// Current standard number
	int		nRings;		// Number of rings for current standard
	int		nRing;		// Current ring number
#define	NRINGS	10		// The maximum number of rings
	double	dspace[NRINGS+1];	// d-values for current standard
	double	le[NRINGS+1];		// X coordinate of the center
	double	ri[NRINGS+1];		// X extents of the rings
	double	to[NRINGS+1];		// Y coordinate of the center
	double	bo[NRINGS+1];		// Y extents of the rings
	UINT	fl[NRINGS+1];		// Ring flags
#define	RF_def	1				// defined in standard
#define	RF_ex	2				// defined by user
} CALIBR, *LPCALIBR;

double	eldXcal = -1.;		// To be used in Information panel
double	eldYcal = -1.;		// Dito
double	eldXesd = -1.;
double	eldYesd = -1.;

extern	void	XY2Sc( float &x, float &y, OBJ* O, float xc, float yc);
extern	float	AbsC(float re, float im);
extern	float   AB_Angle(float ax, float ay, float bx, float by);
/****************************************************************************/
static void SaveProtocolString( OBJ* O )
{
	HWND	par = objFindParent( O, OT_IMG );
	OBJ*	pr  = OBJ::GetOBJ(par);
	FILE 	*out;
	char	sztim[20], fname[_MAX_PATH];
	struct	tm *timptr;
	time_t	timm;

	if (eldXcal<0.) { MessageBeep(-1); return; }
	GetModuleFileName( hInst, fname, sizeof(fname));
	strcpy(strrchr(fname,'\\'), "\\edcalibr.txt");
	out = fopen( fname, "rt" );
	if (out==NULL)
	{
 		out = fopen( fname, "at" );
		fputs("****************************************************************\n", out);
		fputs("                                      d*Rx           d*Ry       \n", out);
		fputs(" ImageName      Date       Time      Mean  StdDev   Mean  StdDev\n", out);
		fputs("****************************************************************\n", out);
	}
	else
	{
		fclose( out );
 		out = fopen( fname, "at" );
	}
	if (out==NULL) { MessageBeep(-1); return; }
	time( &timm );
	timptr = localtime( &timm );
	if (timptr)
	{
		strftime( sztim, sizeof(sztim), "%d-%b-%y  %H:%M:%S", timptr );
		fprintf( out, "%-13s %s  %7.2f %5.2f  %7.2f %5.2f\n",
			pr->title, sztim, eldXcal, eldXesd, eldYcal, eldYesd );
	}
	fclose(out);
}
/****************************************************************************/
static	void	DrawCAL( OBJ* O, BOOL active )
{
HWND		par = objFindParent( O, OT_IMG );
OBJ*		pr  = OBJ::GetOBJ(par);
HDC			hdc = GetDC   (par);
HPEN		hop = SelectPen( hdc, RedPen );
HBRUSH		hob = SelectBrush(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
int			orop = SetROP2(hdc, R2_COPYPEN);
LPCALIBR	S = (LPCALIBR) &O->dummy;
int			i;
float		x0,y0,x1,y1;

	for(i=1;i<=NRINGS;i++) {
		if ( (S->fl[i]&RF_ex) == 0 ) continue;			// Draw if exist
		if ((active) && (i!=S->nRing)) continue;		// Skip drawing if only active requested
		if (i==S->nRing) SetROP2(hdc, R2_NOT);			// Active drawn by XOR
		else			 SetROP2(hdc, R2_COPYPEN);		// Normal drawn by red pen
		x0 = S->le[i]; x1 = S->ri[i];
		y0 = S->to[i]; y1 = S->bo[i];
		XY2Sc( x0, y0, pr, 0, 0);
		XY2Sc( x1, y1, pr, 0, 0);
		Ellipse( hdc, round(x0), round(y0), round(x1)+1, round(y1)+1);
	}
	SelectObject(hdc, hop);
	SelectObject(hdc, hob);
	SetROP2( hdc, orop);
	ReleaseDC( par, hdc );
}
/****************************************************************************/
static void ChangeRing( HWND hDlg, OBJ* O, int nPos )
{
LPCALIBR	S = (LPCALIBR) &O->dummy;
int			i, good;
double		xc, yc;

	if (SendDlgItemMessage( hDlg, IDC_CAL_RING, LB_GETTEXT, nPos, (LPARAM)(LPSTR)TMP) == LB_ERR)
		return;
	if (sscanf(TMP, "%d", &nPos) != 1) return;
	S->nRing = nPos;

	if (S->fl[nPos]&RF_def) {
		if ((S->fl[nPos] & RF_ex) == 0 && eldXcal>0. && eldYcal>0.) {  // New ring
			for(i=1,xc=0,yc=0,good=0; i<=NRINGS; i++) if ((S->fl[i] & RF_def) && (S->fl[i]&RF_ex)) {
				xc += (S->le[i]+S->ri[i])/2;
				yc += (S->to[i]+S->bo[i])/2;
				good++;
			}
			if (good) {
				xc /= (double)good;
				yc /= (double)good;
				S->le[nPos] = xc-eldXcal/S->dspace[nPos];
				S->ri[nPos] = xc+eldXcal/S->dspace[nPos];
				S->to[nPos] = yc+eldYcal/S->dspace[nPos];
				S->bo[nPos] = yc-eldYcal/S->dspace[nPos];
			}
		}
		S->fl[nPos] |= RF_ex;
	}
	if (objFindParent( O, OT_IMG )) InvalidateRect( objFindParent( O, OT_IMG), NULL, FALSE );
}
/****************************************************************************/
static	void	FillRingList( HWND hDlg, OBJ* O )
{
LPCALIBR	S = (LPCALIBR) &O->dummy;
HWND		hw = GetDlgItem(hDlg, IDC_CAL_RING);
double		le, ri, to, bo;
double		xs, ys;
int			sel, i, good=0;
double		xc=0., yc=0.;

	sel = SendMessage( hw, LB_GETCURSEL, 0, 0);
//	if (sel==LB_ERR) sel=0;
//	if (sel>S->nRings) sel=0;
	SendMessage( hw, WM_SETREDRAW, 0, 0);
	SendMessage( hw, LB_RESETCONTENT, 0, 0);
	for(i=1; i<=NRINGS; i++) if (S->fl[i] & RF_def) {
		le = S->le[i]; ri = S->ri[i]; to = S->to[i]; bo = S->bo[i];
		xs = (ri-le)*.5; ys = (to-bo)*.5;
		if (S->fl[i]&RF_ex) {
			sprintf(TMP, "%2d %6.3f  %6.2f  %6.2f %7.2f %7.2f",
							   i, S->dspace[i], xs, ys, xs*S->dspace[i], ys*S->dspace[i]);
			good++;
			xc += xs*S->dspace[i];
			yc += ys*S->dspace[i];
		}
		else sprintf(TMP, "%2d %6.3f", i, S->dspace[i]);
		SendMessage( hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPSTR)TMP);
	}
	if (good) {
		eldXcal = xc/(double)good;
		eldYcal = yc/(double)good;
		xc=0.; yc=0.;
		for(i=1;i<=NRINGS;i++) if (S->fl[i] & RF_ex) {
			xs = fabs((S->ri[i]-S->le[i])*.5*S->dspace[i] - eldXcal);
			ys = fabs((S->to[i]-S->bo[i])*.5*S->dspace[i] - eldYcal);
			if (xs>xc) xc = xs;
			if (ys>yc) yc = ys;
		}
		eldXesd = xc; eldYesd = yc;
		TextToDlgItemW(hDlg, IDC_CAL_XAVE, "d*Rx=%6.2f±%4.2f(%c*pix)", eldXcal, xc, ANGSTR_CHAR);
		TextToDlgItemW(hDlg, IDC_CAL_YAVE, "d*Ry=%6.2f±%4.2f(%c*pix)", eldYcal, yc, ANGSTR_CHAR);
// 		sprintf(TMP, "d*Rx=%6.2f±%4.2f(Å*pix)", eldXcal, xc); 		SetDlgItemText(hDlg, IDC_CAL_XAVE, TMP);
// 		sprintf(TMP, "d*Ry=%6.2f±%4.2f(Å*pix)", eldYcal, yc);		SetDlgItemText(hDlg, IDC_CAL_YAVE, TMP);
	}
	else
	{
		eldXcal = -1.;
		eldYcal = -1.;
		SetDlgItemText(hDlg, IDC_CAL_XAVE, "");
		SetDlgItemText(hDlg, IDC_CAL_YAVE, "");
	}
	SendMessage( hw, LB_SETCURSEL, sel, 0);
	SendMessage( hw, WM_SETREDRAW, 1, 0);
	InvalidateRect( hw, NULL, TRUE);
}
/****************************************************************************/
static void ChangeStandard( HWND hDlg, OBJ* O, int nSt )
{
LPCALIBR	S = (LPCALIBR) &O->dummy;
FILE	* in;
char	c;
int		n;
double	d;
char	fname[_MAX_PATH];

	if ((nSt >= S->nStands) || (nSt<0) || (S->nStand == nSt)) return;

	S->nStand = nSt;
	GetModuleFileName( hInst, fname, sizeof(fname));
	strcpy(strrchr(fname,'\\'), "\\dspace.tbl");

	if ((in=fopen(fname,"r")) == NULL) return;
	n=0;
	while( !feof(in) ) {
		fgets(TMP, 80, in);
		c = TMP[0];
		if ((c==';') || (c==' ') || (c=='\t') || (c=='\n') || (c=='\r')) continue;
		if (n==nSt) break;
		n++;
	}
	S->nRings=0;
	for(n=0;n<=NRINGS;n++) S->fl[n] = 0;	// Clear all rings
	while( !feof(in) ) {
		fgets(TMP, 80, in);
		c = TMP[0];
		if ((c==';') || (c=='\n') || (c=='\r')) continue;
		if ((c!=' ') && (c!='\t')) break;
		if ( 2 != sscanf(TMP, "%d %lf", &n, &d)) continue;
		if (n>NRINGS) break;
		if (S->nRings >= NRINGS) break;
		if (S->fl[n]!=0) continue;		// Double defined
		S->nRings++;
		S->dspace[n] = d;
		S->fl[n] |= RF_def;
	}
	fclose(in);
	S->nRing = LB_ERR;
	if (S->nRings) for(n=1,d=1.;n<NRINGS;n++) if (S->fl[n] & RF_def){
		S->le[n] = O->x/2 - O->x/3*d/(double)S->nRings;
		S->ri[n] = O->x/2 + O->x/3*d/(double)S->nRings;
		S->to[n] = O->y/2 + O->x/3*d/(double)S->nRings;
		S->bo[n] = O->y/2 - O->x/3*d/(double)S->nRings;
		d+=1.;
	}
	FillRingList ( hDlg, O );
}
/****************************************************************************/
static	void	UpdateRing( HWND hDlg, OBJ* O, LPARAM lParam )
{
OBJ*	I = OBJ::GetOBJ( O->ParentWnd );	// Image Object
double	x = (LOWORD(lParam)) * I->scd / I->scu + I->xo;
double	y = (HIWORD(lParam)) * I->scd / I->scu + I->yo;
LPCALIBR	S = (LPCALIBR) &O->dummy;
int		n = S->nRing;
double	le, ri, to, bo;
double	xc, yc;

	if (n==LB_ERR) return;
	le = S->le[n]; ri = S->ri[n]; to = S->to[n]; bo = S->bo[n];
	xc = (le+ri)*.5; yc = (to+bo)*.5;
	DrawCAL( O, TRUE );
	if (fabs(xc-x) < fabs(yc-y)) {
		if (y<yc) bo = y;
		else      to = y;
	} else {
		if (x<xc) le = x;
		else 	  ri = x;
	}
	S->le[n]=le; S->ri[n]=ri; S->to[n]=to; S->bo[n]=bo;
	S->fl[n] |= RF_ex;
	DrawCAL( O, TRUE );
	FillRingList( hDlg, O );
}
/****************************************************************************/
static double CentGrav( double * dat, int siz, double thr)
{
int		i,l,r, k, j;
double	ma = 0., tw, td;

	for (i = 0; i <= siz; i++)
	{
		k=0;
		if(ma<dat[i+siz])  { ma = dat[ i+siz]; j=i;  k=1;}
		if(ma<dat[-i+siz]) { ma = dat[-i+siz]; j=-i; k=1;}
		if (k==0) break;
	}
	ma *= thr;
	for (l=j;l>=-siz;l--) if (dat[l+siz]<ma) break;
	for (r=j;r<= siz;r++) if (dat[r+siz]<ma) break;
/*
	if ((l>-siz) && (r<siz))
		td = r+(dat[r+siz-1]-ma)/(dat[r+siz-1]-dat[r+siz])
			+l-(dat[l+siz+1]-ma)/(dat[l+siz+1]-dat[l+siz]);
	else td = r+l;
	return td;
*/
	l++; r--; tw = 0.; td = 0.;
	for (i = l; i <= r; i++)
	{
		tw += dat[i+siz]-ma;
		td += (dat[i+siz]-ma)*i;
	}
	if (tw>1.)	return td/tw;
	else 		return 0.;
}
/****************************************************************************/
static void RefineRing( HWND hDlg, OBJ* O )
{
LPCALIBR	S = (LPCALIBR) &O->dummy;
int			n = S->nRing, ir, xbo, xbi, ybo, ybi, xeo, xei, yeo, yei;
register int i,j;
double		le = S->le[n], ri = S->ri[n], to = S->to[n], bo = S->bo[n];
double		x0 = (ri+le)*.5, y0 = (to+bo)*.5;
double		a = (ri-le)*.5, b = (to-bo)*.5;
double		r, ma, ky, rout, rin, xx, yy; //dd;
HWND		par = objFindParent( O, OT_IMG );
OBJ*		pr  = OBJ::GetOBJ(par);
LPBYTE		d = pr->dp;
int			bwid = (pr->x+3)&0xFFFC;
HCURSOR		hoc;

#define		bPix(x,y) (*(d + bwid*y+x))
#define		PN	6
#define		Z	2

static double xp[PN*2*Z+1], xpw[PN*2*Z+1],
			 xm[PN*2*Z+1], xmw[PN*2*Z+1],
			 yp[PN*2*Z+1], ypw[PN*2*Z+1],
			 ym[PN*2*Z+1], ymw[PN*2*Z+1];

	if (n==LB_ERR) return;
	hoc = SetCursor( hcWait );
	DrawCAL( O, TRUE );

	memset( xp, 0, sizeof(xp)); memset( xpw, 0, sizeof(xp));
	memset( xm, 0, sizeof(xp)); memset( xmw, 0, sizeof(xp));
	memset( yp, 0, sizeof(xp)); memset( ypw, 0, sizeof(xp));
	memset( ym, 0, sizeof(xp)); memset( ymw, 0, sizeof(xp));

	ky = a/b;
	rin = a-PN;
	rout = a+PN;
	xbo = round(x0-a-PN); xeo = round(x0+a+PN);
	ybo = round(y0-b-PN); yeo = round(y0+b+PN);
	r = __min(a,b)*FastSqrt(.5);
	// removed by Peter
//	dd = a*.125;
	xbi = round(x0-r+PN); xei = round(x0+r-PN);
	ybi = round(y0-r+PN); yei = round(y0+r-PN);

	if (xbo<0) xbo=0; if (xeo>=pr->x) xeo = pr->x-1;
	if (ybo<0) ybo=0; if (yeo>=pr->y) yeo = pr->y-1;
	for(j = ybo; j <= yeo; j++)
	{
		for(i = xbo; i <= xeo; i++)
		{
//			if ((i>xbi) && (i<xei) && (j>ybi) && (j<yei)) continue;
   			xx = i-x0;
			yy = (j-y0)*ky;
//			if (fabs(xx)>dd && fabs(yy)>dd) continue;
			r = sqrt(SQR(xx) + SQR(yy));
			if ((r <= rin) || (r >= rout)) continue;
			ir = round((r-a)*Z);
			if (abs(ir) > PN*Z) continue;
			ma = bPix(i,j);
			ir += PN*Z;
			if (xx>0.) { xp[ir] += ma*xx; xpw[ir] += xx; }
			if (xx<0.) { xm[ir] += ma*xx; xmw[ir] += xx; }
			if (yy>0.) { yp[ir] += ma*yy; ypw[ir] += yy; }
			if (yy<0.) { ym[ir] += ma*yy; ymw[ir] += yy; }
//			if ((xx>0.)&&fabs(yy)<dd) { xp[ir] += ma*xx; xpw[ir] += xx; }
//			if ((xx<0.)&&fabs(yy)<dd) { xm[ir] += ma*xx; xmw[ir] += xx; }
//			if ((yy>0.)&&fabs(xx)<dd) { yp[ir] += ma*yy; ypw[ir] += yy; }
//			if ((yy<0.)&&fabs(xx)<dd) { ym[ir] += ma*yy; ymw[ir] += yy; }
		}
    }
	for(j = -PN*Z; j <= PN*Z; j++)
	{
		if (xpw[j+PN*Z]>0.5)	xp[j+PN*Z] /= xpw[j+PN*Z];
		if (xmw[j+PN*Z]<-0.5)	xm[j+PN*Z] /= xmw[j+PN*Z];
		if (ypw[j+PN*Z]>0.5)	yp[j+PN*Z] /= ypw[j+PN*Z];
		if (ymw[j+PN*Z]<-0.5)	ym[j+PN*Z] /= ymw[j+PN*Z];
	}

	ri += CentGrav(xp, PN*Z, 0.95)/(double)Z;
	to += CentGrav(yp, PN*Z, 0.95)/(double)Z;
	le -= CentGrav(xm, PN*Z, 0.95)/(double)Z;
	bo -= CentGrav(ym, PN*Z, 0.95)/(double)Z;

	S->le[n]=le;
	S->ri[n]=ri;
	S->to[n]=to;
	S->bo[n]=bo;
	S->fl[n] |= RF_ex;
	DrawCAL( O, TRUE );
	FillRingList( hDlg, O );
#undef	PN
	SetCursor( hoc );
}
/****************************************************************************/
static BOOL	InitCalibrDialog( HWND hDlg , OBJ* O, LPCALIBR S )
{
	FILE	* in;
	char	c;
	char	fname[_MAX_PATH];

	GetModuleFileName( hInst, fname, sizeof(fname));
	strcpy(strrchr(fname,'\\'), "\\dspace.tbl");

	S->nStands = 0;
	if ((in=fopen(fname,"r")) == NULL) return myError(IDS_ERRORDSPACETBL);

	while( !feof(in) )
	{
		fgets(TMP, 80, in);
		c = TMP[0];
		if ((c==';') || (c==' ') || (c=='\t') || (c=='\n') || (c=='\r'))
			continue;
		TMP[strlen(TMP)-1] = 0;
		SendDlgItemMessage( hDlg, IDC_CAL_STANDARD, CB_INSERTSTRING, -1, (LPARAM)(LPSTR)TMP);
		S->nStands++;
	}
	fclose(in);

	SendDlgItemMessage( hDlg, IDC_CAL_STANDARD, CB_SETCURSEL, 0, 0);

#ifdef USE_FONT_MANAGER
	HFONT hFont = FontManager::GetFont(FM_NORMAL_FONT);
#else
	HFONT hFont = hfNormal;
#endif
	SendDlgItemMessage( hDlg, IDC_CAL_XAVE, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage( hDlg, IDC_CAL_YAVE, WM_SETFONT, (WPARAM)hFont, 0);

	S->nStand = -1;
	ChangeStandard( hDlg, O, 0 );
	return TRUE;
}
/****************************************************************************/
LRESULT CALLBACK CalWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		OBJ*		O = OBJ::GetOBJ(hDlg);
		LPCALIBR	S = (LPCALIBR) &O->dummy;

		switch (message)
		{
		case WM_INITDIALOG:
			// ADDED BY PETER ON 29 JUNE 2004
			// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
			// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
			O = (OBJ*)lParam;
			S = (LPCALIBR) &O->dummy;
			InitializeObjectDialog(hDlg, O);

			if (!InitCalibrDialog( hDlg, O, S))
			{
				DestroyWindow( hDlg );
				break;
			}
			InvalidateRect( objFindParent( O, OT_IMG), NULL, FALSE );
			break;

		case WM_NCDESTROY:
			InvalidateRect( objFindParent( O, OT_IMG), NULL, FALSE );
			objFreeObj( hDlg );
			break;

		case FM_ParentInspect:
			UpdateRing( hDlg, O, lParam );
			return TRUE;

		case FM_UpdateNI:
			O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
	    	objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
			return TRUE;

		case FM_ParentPaint:
			DrawCAL( O, FALSE );
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_CAL_RING:
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					ChangeRing( hDlg, O, SendDlgItemMessage(hDlg, LOWORD(wParam), LB_GETCURSEL, 0, 0));
					FillRingList( hDlg, O );
				}
				return TRUE;

			case IDC_CAL_STANDARD:
				if (HIWORD(wParam) == CBN_SELCHANGE)
					ChangeStandard( hDlg, O, SendDlgItemMessage(hDlg, LOWORD(wParam), CB_GETCURSEL, 0, 0));
				return TRUE;

			case IDC_CAL_REFINE:
				RefineRing( hDlg, O );
				return TRUE;

			case IDC_CAL_CLEAR:
				if (S->nRing == LB_ERR)
					return TRUE;
				S->fl[S->nRing] = RF_def;
				{
					int i, j, n = LB_ERR;
					for(i = 1, j = 0; i < NRINGS; i++)
					{
						if (S->fl[i] & RF_def)
						{
							if (S->fl[i] & RF_ex)
							{
								n=j;
								break;
							}
							j++;
						}
					}
					SendDlgItemMessage( hDlg, IDC_CAL_RING, LB_SETCURSEL, n, 0);
					if (n == LB_ERR)
						S->nRing = n;
					else
						S->nRing = i;
					InvalidateRect( objFindParent( O, OT_IMG), NULL, FALSE );
				}
				FillRingList( hDlg, O );
				return TRUE;

			case IDC_CAL_PROTOCOL:
				SaveProtocolString( O );
				return TRUE;

			case IDCANCEL:
				DestroyWindow( hDlg );
				return TRUE;
		}
		break;

		case WM_MOUSEACTIVATE:
		case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
		case WM_QUERYOPEN:
			ActivateObj(hDlg);
			break;
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("CalWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
