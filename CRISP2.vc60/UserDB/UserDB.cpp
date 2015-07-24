// #include "allinc.h"
#include "StdAfx.h"
#include <time.h>

#include "customre.h"

extern "C" {
#include "..\xcbn\cbn_.h"
}

#include "..\xcbn\shcb.h"

/***************************************************************/
/*               GLOBAL VARIABLES                              */
/***************************************************************/

HINSTANCE  hInst   = 0;  /* Handle to instance.                   */
HWND	    MainhWnd= 0;  /* Handle to main window.                */
HWND		hClient = 0;  /* Handle to window in client area.      */

char win_name[] = "User Data Base & CB";
char Iname[128];
char Null[] = "";

char tmp[5000];
char sum=0;

static	DWORD IDc[] = {0, 0x022AB0, 0x2247B0, 0x79B4B0, 0x012287, 0x8BAF10, 0x7FA1B8, 0x384A5F, 0x375BAE };
static	DWORD IDr[] = {0,   0x420D,   0x00CA,   0xCD22,   0x1725,   0xBA80,   0x7A8B,   0x9A7B,   0xEA75 }; 

static	DWORD PW1 = 0x22880B00;
static	DWORD PW2 = 0xC65BAA81;			// stored @ 0 in RAM1
UINT	iPort;

int		nU=0;
LPUSER	lpU=NULL;

static LPSTR mnth[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CtrlDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

double epsilon()
{
	double ret = 1.0;
	while (ret + 1.0 != 1.0)
		ret = ret * 0.5;
	return 2.0 * ret;
}
/****************************************************************************/
/****************************************************************************/
#define CLIENTSTRIP WS_MINIMIZE|WS_MAXIMIZE|WS_CAPTION|WS_BORDER \
					|WS_DLGFRAME|WS_SYSMENU|WS_POPUP|WS_THICKFRAME|DS_MODALFRAME


typedef struct {   
	WORD   dlgVer; 
	WORD   signature; 
	DWORD  helpID; 
	DWORD  exStyle; 
	DWORD  style; 
	WORD   cDlgItems; 
	short  x; 
	short  y; 
	short  cx; 
	short  cy; 
//	sz_Or_Ord menu;         // name or ordinal of a menu resource
//	sz_Or_Ord windowClass;  // name or ordinal of a window class
//	WCHAR  title[titleLen]; // title string of the dialog box
//	short  pointsize;       // only if DS_SETFONT flag is set
//	short  weight;          // only if DS_SETFONT flag is set
//	short  bItalic;         // only if DS_SETFONT flag is set
//	WCHAR  font[fontLen];   // typeface name, if DS_SETFONT is set
} DLGTEMPLATEEX, *LP_DLGTEMPLATEEX;

typedef DLGTEMPLATE *LP_DLGTEMPLATE;
/****************************************************************************/
/****************************************************************************/
HWND WINAPI wndCreateClientControls(char *pTemplateName, HINSTANCE hInst, DLGPROC lpNew, HWND hWparent)
{
	RECT			rCli,rPar,rDlg;
	int				dxDlg,dyDlg,dyExtra,dtXold,dtYold;
	HRSRC			hRes;
	HGLOBAL			hMem;
	LP_DLGTEMPLATE	lpDlg;
	DWORD			styleold,style;
	HWND			hNew;

	if (!IsWindow(hWparent))
		return 0;
	if (IsZoomed(hWparent))
		ShowWindow(hWparent,SW_RESTORE);

	/* Get access to data structure of dialog box containing layout of controls */
	if (!(hRes=FindResource(hInst,(LPSTR)pTemplateName,RT_DIALOG)))
		return 0;
	if (!(hMem=LoadResource(hInst,hRes)))
		return 0;
	if (!(lpDlg=(LP_DLGTEMPLATE)LockResource(hMem)))
		return 0;

    /* Change dialog box data structure so it can be used as a window in client area */
	styleold     = lpDlg->style;
	style        = lpDlg->style&(CLIENTSTRIP);
	lpDlg->style = lpDlg->style^style;
	lpDlg->style = lpDlg->style | WS_CHILD | WS_CLIPSIBLINGS;
	dtXold       = lpDlg->x;
	dtYold       = lpDlg->y;
	lpDlg->x     = 0;
	lpDlg->y     = 0;

	if (!(hNew = CreateDialogIndirectParam(hInst, lpDlg, hWparent, lpNew, (LPARAM)hWparent)))
		return 0;

    /* Restore dialog box data structure. */
	lpDlg->style =styleold;
	lpDlg->x     = dtXold;
	lpDlg->y     = dtYold;

	UnlockResource(hMem);
	FreeResource(hMem);

    /* Move and size window in client area and main window */
	GetClientRect(hWparent,&rCli);
	GetWindowRect(hWparent,&rPar);
	MapWindowPoints( NULL, GetParent(hWparent), (LPPOINT)&rPar, 2 );
	GetWindowRect(hNew,&rDlg);
	dxDlg=(rDlg.right -rDlg.left)-(rCli.right -rCli.left);
	dyDlg=(rDlg.bottom-rDlg.top) -(rCli.bottom-rCli.top);

	MoveWindow(hWparent, rPar.left, rPar.top,
				 (rPar.right -rPar.left)+dxDlg,
				 (rPar.bottom-rPar.top) +dyDlg,
				 TRUE);

	MoveWindow(hNew,0,0,
                     (rDlg.right -rDlg.left),
                     (rDlg.bottom-rDlg.top),
                     TRUE);
    GetClientRect(hWparent,&rCli);

    /* Compensate size if menu bar is more than one line. */
	if ((rDlg.bottom-rDlg.top)>(rCli.bottom-rCli.top))
	{
		dyExtra=(rDlg.bottom-rDlg.top)-(rCli.bottom-rCli.top);

		MoveWindow(hWparent,rPar.left,rPar.top,
                   (rPar.right -rPar.left)+dxDlg,
                   (rPar.bottom-rPar.top) +dyDlg + dyExtra,
                   TRUE);

	}

	ShowWindow(hNew,SW_SHOW);
	return hNew;
}

/***************************************************************/
/*           PROCESSES KEYBOARD ACCELERATORS                   */
/*           AND MODELESS DIALOG BOX KEY INPUT                 */
/***************************************************************/
BOOL KeyTranslation(MSG *pMsg)
{
/* Translate keystrokes so client area works as a dialog box */
	if (hClient && IsDialogMessage(hClient,pMsg))
		return TRUE;
	return FALSE; /* No special key input                      */
}

/***************************************************************/
/*    FUNCTIONS FOR INITIALIZATION AND EXIT OF APPLICATION     */
/***************************************************************/
BOOL InitApplication(HINSTANCE hInst, HANDLE hPrev, int *pCmdShow, LPSTR lpCmd)
{
    WNDCLASS WndClass;
    int coor[4];           /* Coordinates of main window              */

	if (!hPrev)
	{
		WndClass.style         = 0;
		WndClass.lpfnWndProc   = MainWndProc;
		WndClass.cbClsExtra    = 0;
		WndClass.cbWndExtra    = 0;
		WndClass.hInstance     = hInst;
		WndClass.hIcon         = LoadIcon(NULL,IDI_APPLICATION);
		WndClass.hCursor       = LoadCursor(NULL,IDC_ARROW);
		WndClass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		WndClass.lpszMenuName  = "CUSTOM";
		WndClass.lpszClassName = "CUSTOM";

		if (!RegisterClass(&WndClass)) return FALSE;
	}

	coor[0]=40;
    coor[1]=40;
    coor[2]=560;
    coor[3]=384;

	MainhWnd = CreateWindow("CUSTOM", win_name,
			WS_OVERLAPPED | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX,                /* window style                            */
			coor[0], coor[1], coor[2], coor[3],
			0, 0, hInst, (LPSTR)NULL);
	if (!MainhWnd)
		return FALSE;


    return TRUE;
}
/***************************************************************/
/*               WinMain FUNCTION                              */
/***************************************************************/
bool SameSize(int a, int b)
{
	return a == b;
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG 	msg;

	hInst = hInstance;

	_timezone = 18000;

	if (!SameSize(sizeof(TRIX), APP_SIZE))
	{
		sprintf(tmp,"TRIX strcture size is invalid (%d)", sizeof(TRIX));
		MessageBox(NULL, tmp,"",MB_ICONSTOP);
		return FALSE;
	}
	if (!SameSize(sizeof(CRISP), APP_SIZE))
	{
		sprintf(tmp,"CRISP strcture size is invalid (%d)", sizeof(CRISP));
		MessageBox(NULL, tmp,"",MB_ICONSTOP);
		return FALSE;
	}
	if (!SameSize(sizeof(PENTACLE), APP_SIZE))
	{
		sprintf(tmp,"PENTACLE strcture size is invalid (%d)", sizeof(PENTACLE));
		MessageBox(NULL, tmp,"",MB_ICONSTOP);
		return FALSE;
	}

	if (!InitApplication(hInstance,hPrevInstance,&nCmdShow,lpCmdLine))
		return FALSE;

	ShowWindow(MainhWnd, nCmdShow);
	UpdateWindow(MainhWnd);
	while (GetMessage(&msg,0, 0, 0))
	{
		if (KeyTranslation(&msg))
			continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

/***************************************************************/
/*           WINDOW PROCEDURE FOR MAIN WINDOW                  */
/***************************************************************/
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
	{
	case WM_CREATE:
		MainhWnd=hWnd;
		hClient = wndCreateClientControls("MAIN", hInst, (DLGPROC)CtrlDlgProc, hWnd);
		break;

	case WM_SETFOCUS:
		if (IsWindow(hClient))
			SetFocus(hClient);
		break;

	case WM_DESTROY:
        PostQuitMessage(0);
        break;

	case WM_COMMAND:
		break;

	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
/****************************************************************************/
static LPSTR sprtime( LPSTR s, time_t ttt, BOOL bShort )
{
	struct tm *ltm;

	if (ttt==-1) {
		sprintf(s,"Unlimited");
	} else if (ttt==0) {
		sprintf(s,"");
	} else {
	    ltm = localtime( &ttt ); 
		if (bShort)
			sprintf(s, "%02d-%s-%d, %02d:%02d", ltm->tm_mday, mnth[ltm->tm_mon], 1900+ltm->tm_year, ltm->tm_hour, ltm->tm_min );
		else 
			sprintf(s, "%02d-%s-%d", ltm->tm_mday, mnth[ltm->tm_mon], 1900+ltm->tm_year );
	}
	return s;
}
/****************************************************************************/
time_t CvtDate( LPSTR date )
{
int day, month, year;
time_t t0,t1;
struct tm * tmm;
char	mn[10];

	if (stricmp(date,"unlimited")==0) return -1;
	if (sscanf(date,"%d-%3s-%d",&day,mn,&year) != 3) return 0;
	for(month=0;month<12;month++) if(stricmp(mn,mnth[month])==0) break;
	if (month==12) return 0;
	year -= 1900;
	if (year<95) return 0L;
	if (month<0 || month>11) return 0L;
	if (day<1 || day>31) return 0L;
	time(&t0);
	t1=t0;
	t1 = t1 | 0xFFFF;
	while (1){
		tmm = localtime(&t1);
		if(tmm->tm_year<year) { t1+=0x10000L; continue; }
		if(tmm->tm_year>year) break;
		if(tmm->tm_mon<month) { t1+=0x10000L; continue; }
		if(tmm->tm_mon>month) break;
		if(tmm->tm_mday<day)  { t1+=0x10000L; continue; }
		break;
	}
	if(tmm->tm_year!=year || tmm->tm_mon!=month || tmm->tm_mday!=day)
		return 0L;
	return t1;
}
/****************************************************************************/
void CalcCRC( LPUSER u )
{
DWORD	crc;
LPDWORD a;
int		i;

	crc = PW2 + u->wSRN;
	for(i=0,a=(LPDWORD)&u->c;i<APP_SIZE/4-1;i++,a++) crc+=*a;
	u->c.crc = crc;

	crc = PW2 + u->wSRN;
	for(i=0,a=(LPDWORD)&u->p;i<APP_SIZE/4-1;i++,a++) crc+=*a;
	u->p.crc = crc;

	crc = PW2 + u->wSRN;
	for(i=0,a=(LPDWORD)&u->t;i<APP_SIZE/4-1;i++,a++) crc+=*a;
	u->t.crc = crc;
}
/****************************************************************************/
BOOL CheckCRC( LPUSER u )
{
DWORD	crc;
LPDWORD a;
int		i;

	crc = PW2 + u->wSRN;
	for(i=0,a=(LPDWORD)&u->c;i<APP_SIZE/4-1;i++,a++) crc+=*a;
	if (u->c.crc != crc) return FALSE;

	crc = PW2 + u->wSRN;
	for(i=0,a=(LPDWORD)&u->p;i<APP_SIZE/4-1;i++,a++) crc+=*a;
	if (u->p.crc != crc) return FALSE;

	crc = PW2 + u->wSRN;
	for(i=0,a=(LPDWORD)&u->t;i<APP_SIZE/4-1;i++,a++) crc+=*a;
	if (u->t.crc != crc) return FALSE;
	return TRUE;
}
/****************************************************************************/
static void SetEnable( HWND hDlg )
{
BOOL	cen, pen, ten;

	cen = IsDlgButtonChecked( hDlg, IDC_ACQUSITION ) ||
		  IsDlgButtonChecked( hDlg, IDC_CRISP ) ||
		  IsDlgButtonChecked( hDlg, IDC_ELD ) ||
		  IsDlgButtonChecked( hDlg, IDC_ELD_SPINSTAR ) ||
		  IsDlgButtonChecked( hDlg, IDC_CALIBRATION ) ||
		  IsDlgButtonChecked( hDlg, IDC_EDRD ) ||
		  IsDlgButtonChecked( hDlg, IDC_PHIDO );
	pen = IsDlgButtonChecked( hDlg, IDC_PENTACLE );
	ten = IsDlgButtonChecked( hDlg, IDC_TRIX ) ||
		  IsDlgButtonChecked( hDlg, IDC_XDBASE ) ||
		  IsDlgButtonChecked( hDlg, IDC_XDDIRM );
	EnableWindow( GetDlgItem(hDlg,IDC_CEXP), cen );
	EnableWindow( GetDlgItem(hDlg,IDC_PEXP), pen );
	EnableWindow( GetDlgItem(hDlg,IDC_TEXP), ten );
}
/****************************************************************************/
static void SelectRec( HWND hDlg, int i )
{
	if (i!=LB_ERR)
	{
		SetDlgItemInt( hDlg, IDC_SERNO, lpU[i].wSRN, FALSE );
		SetDlgItemText( hDlg, IDC_USERNAME, lpU[i].UserName );
		SetDlgItemText( hDlg, IDC_CREATED, sprtime( tmp, lpU[i].dwCreated, TRUE ));
		SetDlgItemText( hDlg, IDC_MODIFIED, sprtime( tmp, lpU[i].dwModified, TRUE ));
		SetDlgItemText( hDlg, IDC_CEXP, sprtime( tmp, lpU[i].c.dwExp, FALSE ));
		SetDlgItemText( hDlg, IDC_PEXP, sprtime( tmp, lpU[i].p.dwExp, FALSE ));
		SetDlgItemText( hDlg, IDC_TEXP, sprtime( tmp, lpU[i].t.dwExp, FALSE ));
		SetDlgItemText( hDlg, IDC_UCOMMENT, lpU[i].comment );
		CheckDlgButton( hDlg, IDC_ACQUSITION,  (lpU[i].c.flags & CRISP_ACQUISITION)!=0 );
		CheckDlgButton( hDlg, IDC_CRISP,       (lpU[i].c.flags & CRISP_CRISP)!=0 );
		CheckDlgButton( hDlg, IDC_ELD,         (lpU[i].c.flags & CRISP_ELD)!=0 );
		CheckDlgButton( hDlg, IDC_ELD_SPINSTAR,(lpU[i].c.flags & CRISP_ELD_SPINSTAR)!=0 );
		CheckDlgButton( hDlg, IDC_CALIBRATION, (lpU[i].c.flags & CRISP_CALIBRATION)!=0 );
		CheckDlgButton( hDlg, IDC_EDRD,        (lpU[i].c.flags & CRISP_EDRD)!=0 );
		CheckDlgButton( hDlg, IDC_PHIDO,       (lpU[i].c.flags & CRISP_PHIDO)!=0 );
		CheckDlgButton( hDlg, IDC_PENTACLE,    (lpU[i].p.flags & PENTACLE_MAIN)!=0 );
		CheckDlgButton( hDlg, IDC_TRIX,        (lpU[i].t.flags & TRIX_MAIN)!=0 );
		CheckDlgButton( hDlg, IDC_XDBASE,      (lpU[i].t.flags & TRIX_XDBASE)!=0 );
		CheckDlgButton( hDlg, IDC_XDDIRM,      (lpU[i].t.flags & TRIX_XDDIRM)!=0 );
	}
	else
	{
		SetDlgItemText( hDlg, IDC_CREATED, "");
		SetDlgItemText( hDlg, IDC_MODIFIED, "");
		SetDlgItemText( hDlg, IDC_UCOMMENT, "");
		SetDlgItemText( hDlg, IDC_CEXP, "");
		SetDlgItemText( hDlg, IDC_PEXP, "");
		SetDlgItemText( hDlg, IDC_TEXP, "");
	}
	SetEnable( hDlg );
}
/****************************************************************************/
int _cdecl ucmp( const USER far *e1, const USER far *e2 )
{
	if (e1->wSRN < e2->wSRN) return -1;
	if (e1->wSRN > e2->wSRN) return 1;
	return 0;
}
/****************************************************************************/
static void FillList( HWND hDlg, int sel )
{
HWND	hwl = GetDlgItem( hDlg, IDC_DBLIST );
int		i;

	qsort( lpU, nU, sizeof(USER), (int (__cdecl *)(const void *,const void *))ucmp );
	ListBox_ResetContent( hwl );
	
	for(i=0;i<nU;i++) {
		sprintf(tmp, "%05d - %s", lpU[i].wSRN, lpU[i].UserName );
		ListBox_InsertString( hwl, -1, tmp );
	}
	ListBox_SetCurSel( hwl, sel );
	SelectRec( hDlg, (int)SendDlgItemMessage( hDlg, IDC_DBLIST, LB_GETCURSEL, 0,0L) );
}
/****************************************************************************/
static void ScanRec( HWND hDlg, BOOL bSrch )
{
	int		i,j,k;
	BOOL	ff;
	time_t	texp, cexp, pexp;
	DWORD	cf=0, pf=0, tf=0;

	if (IsDlgButtonChecked( hDlg, IDC_ACQUSITION ))		cf |= CRISP_ACQUISITION;
	if (IsDlgButtonChecked( hDlg, IDC_CRISP ))			cf |= CRISP_CRISP;
	if (IsDlgButtonChecked( hDlg, IDC_ELD ))			cf |= CRISP_ELD;
	if (IsDlgButtonChecked( hDlg, IDC_ELD_SPINSTAR ))	cf |= CRISP_ELD_SPINSTAR;
	if (IsDlgButtonChecked( hDlg, IDC_CALIBRATION ))	cf |= CRISP_CALIBRATION;
	if (IsDlgButtonChecked( hDlg, IDC_EDRD ))			cf |= CRISP_EDRD;
	if (IsDlgButtonChecked( hDlg, IDC_PHIDO ))			cf |= CRISP_PHIDO;
	if (IsDlgButtonChecked( hDlg, IDC_PENTACLE ))		pf |= PENTACLE_MAIN;
	if (IsDlgButtonChecked( hDlg, IDC_TRIX ))			tf |= TRIX_MAIN;
	if (IsDlgButtonChecked( hDlg, IDC_XDBASE ))			tf |= TRIX_XDBASE;
	if (IsDlgButtonChecked( hDlg, IDC_XDDIRM ))			tf |= TRIX_XDDIRM;

	GetDlgItemText( hDlg, IDC_CEXP, tmp, sizeof(tmp)); cexp = CvtDate( tmp );
	if (cexp==0)
	{
		MessageBox( MainhWnd, "Invalid CRISP expiry date", win_name, MB_ICONSTOP);
		return;
	}
	GetDlgItemText( hDlg, IDC_PEXP, tmp, sizeof(tmp)); pexp = CvtDate( tmp );
	if (pexp==0)
	{
		MessageBox( MainhWnd, "Invalid Pentacle expiry date", win_name, MB_ICONSTOP);
		return;
	}
	GetDlgItemText( hDlg, IDC_TEXP, tmp, sizeof(tmp)); texp = CvtDate( tmp );
	if (texp==0)
	{
		MessageBox( MainhWnd, "Invalid TriXXX expiry date", win_name, MB_ICONSTOP);
		return;
	}

	j = GetDlgItemInt( hDlg, IDC_SERNO, &ff, FALSE );
	if (bSrch)
	{
		i = (int)SendDlgItemMessage( hDlg, IDC_DBLIST, LB_GETCURSEL, 0,0L);
		if (j!=lpU[i].wSRN)
		{
			sprintf( tmp, "User %05d does not exist\nAdd to data base?", j );
			if (IDNO == MessageBox( MainhWnd, tmp, win_name, MB_ICONQUESTION | MB_YESNO)) return;
			goto newu;
		}
	}
	else
	{
		for(k=0;k<nU;k++) if (lpU[k].wSRN==j) break;
		if (k!=nU)
		{
			sprintf( tmp, "User %05d already exist\nChange its parameters?", lpU[k].wSRN );
			if(IDNO == MessageBox( MainhWnd, tmp, win_name, MB_ICONQUESTION | MB_YESNO))
				return;
			i = k;
		}
		else
		{
newu:		GlobalReAllocPtr( lpU, (nU+1)*sizeof(USER), GHND); nU++;
			i = nU-1;
			time((time_t*)&lpU[i].dwCreated);
		}
	}
	time((time_t*)&lpU[i].dwModified);
	lpU[i].c.dwExp = cexp; lpU[i].c.flags = cf;
	lpU[i].p.dwExp = pexp; lpU[i].p.flags = pf;
	lpU[i].t.dwExp = texp; lpU[i].t.flags = tf;
	if (pf & PENTACLE_MAIN) lpU[i].p.one = 1.0;
	else 				    lpU[i].p.one = 0.5;

	lpU[i].wSRN = GetDlgItemInt( hDlg, IDC_SERNO, &ff, FALSE );
	GetDlgItemText( hDlg, IDC_USERNAME, lpU[i].UserName, sizeof(lpU[i].UserName));
	GetDlgItemText( hDlg, IDC_UCOMMENT, lpU[i].comment, sizeof(lpU[i].comment));
	FillList( hDlg, i );
	CalcCRC( &lpU[i] );
}
/****************************************************************************/
static void DeleteRec( HWND hDlg )
{
	int	i;

	i = (int)SendDlgItemMessage( hDlg, IDC_DBLIST, LB_GETCURSEL, 0, 0L);
	if (i==LB_ERR) return;
	sprintf(tmp,"Deleting user No %05d,\nAre you sure?", lpU[i].wSRN);
	if (IDNO==MessageBox( MainhWnd, tmp, win_name, MB_ICONQUESTION | MB_YESNO )) return;
	lpU[i].wSRN = 0xFFFF;
	qsort( lpU, nU, sizeof(USER), (int (__cdecl *)(const void *,const void *))ucmp );
	nU--;
	GlobalReAllocPtr( lpU, nU*sizeof(USER), GHND);
	i--;
	if (i<0) i=0;
	FillList( hDlg, i );
}
/****************************************************************************/
static BOOL CheckCB( void )
{
	DWORD	ww;
	WORD	wBoxName;

	for(iPort=11;iPort<14;iPort++)
		if (CbN_BoxReady(iPort,(LPBYTE)&wBoxName)==0) break;
	if (iPort==14)
	{
		MessageBox( MainhWnd, "CryptoBox not found", win_name, MB_ICONSTOP);
		return FALSE;
	}

	if (CbN_ReadID1(iPort, (LPBYTE)&IDc[1], &ww) || ww!=IDr[1] ||
		CbN_ReadID2(iPort, (LPBYTE)&IDc[2], &ww) || ww!=IDr[2] ||
		CbN_ReadID3(iPort, (LPBYTE)&IDc[3], &ww) || ww!=IDr[3] ||
		CbN_ReadID4(iPort, (LPBYTE)&IDc[4], &ww) || ww!=IDr[4] ||
		CbN_ReadID5(iPort, (LPBYTE)&IDc[5], &ww) || ww!=IDr[5] ||
		CbN_ReadID6(iPort, (LPBYTE)&IDc[6], &ww) || ww!=IDr[6] ||
		CbN_ReadID7(iPort, (LPBYTE)&IDc[7], &ww) || ww!=IDr[7] ||
		CbN_ReadID8(iPort, (LPBYTE)&IDc[8], &ww) || ww!=IDr[8])
	{
			MessageBox( MainhWnd, "CryptoBox not preprogrammed (Max fault)", win_name, MB_ICONSTOP);
			return FALSE;
	}
	return TRUE;
}
/****************************************************************************/
static void WriteCB( HWND hDlg )
{
int		i;
USER	uu;
LPSTR	erm=NULL;
HCURSOR	hoc;

	i = (int)SendDlgItemMessage( hDlg, IDC_DBLIST, LB_GETCURSEL, 0, 0L);
	if (i==LB_ERR) return;

	if (!CheckCB()) return;

	hoc = SetCursor(LoadCursor(NULL, IDC_WAIT));

	uu = lpU[i];
	uu.dwPW2 = PW2;

	CalcCRC( &uu );
	CalcCRC( &lpU[i] );
	
	if (CbN_IDEA_Encrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)uu.UserName, sizeof(uu.UserName))) {
		erm = "Encryption error"; goto erex;
	}		
	if (CbN_WriteRAM1( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&PW1, 0, 50, (LPBYTE)&uu ) ) {
		erm = "Error writing RAM1"; goto erex;
	}
	if (CbN_IDEA_Encrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.c, sizeof(uu.c)) || 
		CbN_IDEA_Encrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.t, sizeof(uu.t)) ||
		CbN_IDEA_Encrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.p, sizeof(uu.p))) {
		erm = "Encryption error"; goto erex;
	}		
	if (CbN_WriteRAM2( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&PW2, 0, RAM2S, (LPBYTE)&uu.c )) {
		erm = "Error writing RAM2"; goto erex;
	}
	
	/*******************/
	if (CbN_ReadRAM1( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&PW1, 0, 50, (LPBYTE)&uu ) ) {
		erm = "Error reading RAM1"; goto erex;
	}
	if (CbN_IDEA_Decrypt(iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)uu.UserName, sizeof(uu.UserName))) {
		erm = "Decryption error"; goto erex;
	}
	if (CbN_ReadRAM2(  iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&PW2, 0, RAM2S, (LPBYTE)&uu.c )) {
		erm = "Error reading RAM2"; goto erex;
	}
	if (CbN_IDEA_Decrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.c, sizeof(uu.c)) || 
		CbN_IDEA_Decrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.t, sizeof(uu.t)) ||
		CbN_IDEA_Decrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.p, sizeof(uu.p))) {
		erm = "Encryption error"; goto erex;
	}		
	if (uu.wSRN != lpU[i].wSRN ||
		uu.dwPW2 != PW2 || 
		memcmp(uu.UserName,lpU[i].UserName, sizeof(uu.UserName)) ||
		uu.c.dwExp != lpU[i].c.dwExp ||
		uu.t.dwExp != lpU[i].t.dwExp ||
		uu.p.dwExp != lpU[i].p.dwExp ||
        memcmp(&uu.c, &lpU[i].c, sizeof(uu.c)) ||
        memcmp(&uu.t, &lpU[i].t, sizeof(uu.t)) ||
        memcmp(&uu.p, &lpU[i].p, sizeof(uu.p)) ) {
		erm = "Wrong decrypted data"; goto erex;
	}

erex:
	if (hoc) SetCursor( hoc );
	if (erm) MessageBox( MainhWnd, erm, win_name, MB_ICONSTOP);
}
/****************************************************************************/
void xxEncode( LPWORD data, int size )
{
int i;
	for(i=0; i<size/2; i++) data[i] ^= MAGIC_NUMBER;
}
/****************************************************************************/
static void WriteLic( HWND hDlg )
{
int		i;
USER	uu;
LPSTR	erm=NULL;
HCURSOR	hoc;
char fname[_MAX_PATH];

	i = (int)SendDlgItemMessage( hDlg, IDC_DBLIST, LB_GETCURSEL, 0, 0L);
	if (i==LB_ERR) return;

	hoc = SetCursor(LoadCursor(NULL, IDC_WAIT));

	uu = lpU[i];

	CalcCRC( &uu );
	CalcCRC( &lpU[i] );
	
	xxEncode( (LPWORD)&uu.wSRN, sizeof(uu.wSRN) );
	xxEncode( (LPWORD)&uu.c, sizeof(uu.c) );
	xxEncode( (LPWORD)&uu.t, sizeof(uu.t) );
	xxEncode( (LPWORD)&uu.p, sizeof(uu.p) );
	xxEncode( (LPWORD)&uu.UserName, sizeof(uu.UserName) );
	
	GetModuleFileName( hInst, fname, sizeof(fname) );
	*(strrchr( fname, '\\')+1) =0;
	strcat( fname, "crisp2.lic");
	
	WritePrivateProfileStruct( "CRISP2", "User",   &uu.UserName, sizeof(uu.UserName), fname );
	WritePrivateProfileStruct( "CRISP2", "SerNum", &uu.wSRN,     sizeof(uu.wSRN),     fname );
	WritePrivateProfileStruct( "CRISP2", "Lic_c",  &uu.c,        sizeof(uu.c),        fname );
	WritePrivateProfileStruct( "CRISP2", "Lic_t",  &uu.t,        sizeof(uu.t),        fname );
	WritePrivateProfileStruct( "CRISP2", "Lic_p",  &uu.p,        sizeof(uu.p),        fname );

	if (hoc) SetCursor( hoc );
	sprintf(tmp, "File '%s' successfully written", fname );
	MessageBox( MainhWnd, tmp, win_name, MB_ICONINFORMATION);
}
/****************************************************************************/
static void ReadCB( HWND hDlg )
{
int		i;
USER	uu;
LPSTR	erm=NULL;
HCURSOR	hoc;

	if (!CheckCB()) return;

	hoc = SetCursor(LoadCursor(NULL, IDC_WAIT));

	if (CbN_ReadRAM1( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&PW1, 0, 50, (LPBYTE)&uu ) ) {
		erm = "Error reading RAM1"; goto erex;
	}

	if (CbN_IDEA_Decrypt(iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)uu.UserName, sizeof(uu.UserName))) { 
		erm = "Decryption error"; goto erex;
	}
	if (CbN_ReadRAM2(  iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&PW2, 0, RAM2S, (LPBYTE)&uu.c )) {
		erm = "Error reading RAM2"; goto erex;
	}
	if (CbN_IDEA_Decrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.c, sizeof(uu.c)) || 
		CbN_IDEA_Decrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.t, sizeof(uu.t)) ||
		CbN_IDEA_Decrypt( iPort, 1, (LPBYTE)&IDc[1], (LPBYTE)&uu.p, sizeof(uu.p))) {
		erm = "Decryption error"; goto erex;
	}		

	for(i=0;i<nU;i++) if (lpU[i].wSRN==uu.wSRN) break;
	if (i==nU) {
		MessageBox( MainhWnd, "Unknown CryptoBox !!!", win_name, MB_ICONSTOP);
		i=LB_ERR;
	} else {
		if (uu.wSRN != lpU[i].wSRN ||
			uu.dwPW2 != PW2 ||
			memcmp(uu.UserName,lpU[i].UserName, sizeof(uu.UserName)) ||
			uu.c.dwExp != lpU[i].c.dwExp ||
			uu.t.dwExp != lpU[i].t.dwExp ||
			uu.p.dwExp != lpU[i].p.dwExp ||
	        memcmp(&uu.c, &lpU[i].c, sizeof(uu.c)) ||
	        memcmp(&uu.t, &lpU[i].t, sizeof(uu.t)) ||
	        memcmp(&uu.p, &lpU[i].p, sizeof(uu.p)) ) {
			MessageBox( MainhWnd, "This CryptoBox or database record has been modified !!!", win_name, MB_ICONSTOP);
		}
		if (!CheckCRC( &uu )) 
			MessageBox( MainhWnd, "This CryptoBox has been modified (CRC error) !!!", win_name, MB_ICONSTOP);
	}
	SendDlgItemMessage(hDlg, IDC_DBLIST, LB_SETCURSEL, i, 0L);
	SelectRec( hDlg, i );
	SetDlgItemInt( hDlg, IDC_SERNO, uu.wSRN, FALSE );
	SetDlgItemText( hDlg, IDC_USERNAME, uu.UserName );
erex:
	if (hoc) SetCursor( hoc );
	if (erm) MessageBox( MainhWnd, erm, win_name, MB_ICONSTOP);
}
/****************************************************************************/
/***************************************************************/
/*           WINDOW PROCEDURE FOR CONTROL WINDOW               */
/***************************************************************/
LRESULT CALLBACK CtrlDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
/*  Procedure for window in client area */
{
	FILE *fpt;

	switch(message)
	{
		case WM_INITDIALOG:
			if( NULL == (fpt = fopen("users.udb","rb")) )
			{
				MessageBox(hDlg, "Cannot open user DataBase 'users.udb'.", "ERROR!", MB_OK);
				return FALSE;
			}
			fseek(fpt, 0, SEEK_END );
			nU = ftell(fpt)/sizeof(USER);
			lpU = (LPUSER) GlobalAllocPtr( GHND, (nU+1)*sizeof(USER) );
			fseek(fpt, 0, SEEK_SET );
			fread(lpU, sizeof(USER), nU, fpt);
			fclose(fpt);
			FillList(hDlg,0);
			SendDlgItemMessage( hDlg, IDC_SERNO, EM_LIMITTEXT, 5, 0L);
			SendDlgItemMessage( hDlg, IDC_USERNAME, EM_LIMITTEXT, sizeof(lpU->UserName)-1, 0L);
			SendDlgItemMessage( hDlg, IDC_UCOMMENT, EM_LIMITTEXT, sizeof(lpU->comment)-1, 0L);
			return FALSE;

		case WM_NCDESTROY:
			hClient = 0;
			break;

		case WM_VKEYTOITEM:
			if ((HWND)lParam == GetDlgItem(hDlg,IDC_DBLIST)) {
				switch (LOWORD(wParam)) {
					case VK_DELETE:	DeleteRec(hDlg); return -2;
				}
				return -1;
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
			case IDC_ACQUSITION:
			case IDC_CRISP:
			case IDC_ELD:
			case IDC_ELD_SPINSTAR:
			case IDC_CALIBRATION:
			case IDC_EDRD:
			case IDC_PHIDO:
			case IDC_PENTACLE:
			case IDC_TRIX:
			case IDC_XDBASE:
			case IDC_XDDIRM:
				SetEnable( hDlg );
				break;

			case IDC_CBWRITE:
//				WriteCB( hDlg );
				WriteLic( hDlg );
				break;
			case IDC_CBREAD:
//				ReadCB( hDlg );
				break;
			case IDC_DBLIST:
				if (HIWORD(wParam)==LBN_SELCHANGE) 
					SelectRec(hDlg,(int)SendDlgItemMessage( hDlg, IDC_DBLIST, LB_GETCURSEL, 0,0));
				break;
			case IDC_ADD:
				ScanRec( hDlg, FALSE );
				break;
			case IDC_CHANGE:
				ScanRec( hDlg, TRUE );
				break;
			case IDC_DELETE:
				DeleteRec( hDlg );
				break;
			case IDOK:
			case IDCANCEL:
				fpt = fopen("users.udb","wb");
				fwrite(lpU, sizeof(USER), nU, fpt);
				fclose(fpt);
		        PostQuitMessage(0);
				return FALSE;
			default:
				return FALSE;
			}
			break;

		default:
			return FALSE;
	}
	return FALSE; /* Processed the message */
}
