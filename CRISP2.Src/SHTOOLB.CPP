/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.shtoolb * by ML ***************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "globals.h"

#include "objects.h"
#include "shtools.h"
#include "commondef.h"
#include "resource.h"

/****************************************************************************/

static const char szCNLED[] = "myLED";

/****************************************************************************/

static HMODULE	_hModule;

static int		wInfoYc=24;		// Info window height

static HBITMAP	hbLEDON  = NULL;
static HBITMAP	hbLEDOFF = NULL;
#define	wLEDW 14
#define	wLEDH 14

/****************************************************************************/
#define	BUTTONWIDTH		0
#define	BUTTONHEIGHT	0
// changed by Peter on 18 Sep 2001
#define	IMAGEWIDTH		24//16
#define	IMAGEHEIGHT		24//15

/****************************************************************************/
static HWND	hWndToolBar = NULL;
static HWND	hWndInfoBar = NULL;
static HWND hwndPB = NULL;
static HWND hWndToolBarA = NULL;

/****************************************************************************/
static void PaintLED( HWND hWnd, BOOL state )
{
	HDC		hDC = GetDC( hWnd );
	HDC		hMemDC = CreateCompatibleDC( hDC );
	RECT	r;

	GetUpdateRect( hWnd, &r, FALSE );
	SelectObject( hMemDC, state?hbLEDON:hbLEDOFF );
	BitBlt( hDC, 0, 0, wLEDW, wLEDH, hMemDC, 0, 0, SRCCOPY);
	DeleteDC( hMemDC );
	ReleaseDC( hWnd, hDC);
}
/****************************************************************************/
LRESULT CALLBACK LEDWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch(message)
	{
		case WM_USER:
			SetWindowLong( hWnd, 0, wParam);

		case WM_PAINT:
			PaintLED( hWnd, GetWindowLong( hWnd, 0));
			ValidateRect(hWnd, NULL);
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
/****************************************************************************/
/* API Level ****************************************************************/
/****************************************************************************/
VOID WINAPI tobSetInfoText( int id, LPSTR str )
{
	if ( hWndInfoBar == NULL ) return;
	SendNotifyMessage( hWndInfoBar, SB_SETTEXT, id, (LPARAM)str );
	SendNotifyMessage(hwndPB, PBM_SETPOS, 0, 0 );
	UpdateWindow( hWndInfoBar );
}
/****************************************************************************/
VOID WINAPI tobSetInfoTextAndBar( int id, LPSTR str, int perc )
{
	if ( hWndInfoBar == NULL ) return;
	SendNotifyMessage( hWndInfoBar, SB_SETTEXT, id, (LPARAM)str );
	SendNotifyMessage(hwndPB, PBM_SETPOS, perc, 0 );
	UpdateWindow( hWndInfoBar );
//	SendDlgItemMessage(hWndInfo, id, MYWM_BAR, (WORD)perc, (LONG)str );
}
/****************************************************************************/
VOID WINAPI tobAdjustInfoBar( HWND hWndPar )
{
RECT	r, rc, rc1;
int		np;

	if ( hWndInfoBar == NULL ) return;
	if ( ! IsWindow(hWndInfoBar) ) return;
	GetClientRect( hWndPar, &r );
	SetWindowPos( hWndInfoBar,0,0, r.bottom-wInfoYc, r.right, wInfoYc,
				   SWP_NOZORDER | SWP_NOACTIVATE );

	GetWindowRect( hWndInfoBar, &rc );
	np=SendMessage( hWndInfoBar, SB_GETPARTS, 0, 0 );
	SendMessage( hWndInfoBar, SB_GETRECT, np-1, (LPARAM)&rc1 );

	SetWindowPos( hwndPB, 0, rc1.right+4, rc1.top, rc.right-rc.left-rc1.right-20, rc1.bottom-rc1.top,
				   SWP_NOZORDER | SWP_NOACTIVATE );
}
/****************************************************************************/
HWND WINAPI tobCreateInfoBar( HWND hWndPar )
{
RECT	rc;

	hWndInfoBar = CreateStatusWindow(WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP | WS_BORDER,
		"", hWndPar, 12345 );
	SendMessage( hWndInfoBar, WM_SETFONT, (WPARAM)hfInfo, MAKELPARAM(TRUE,0));
	GetWindowRect( hWndInfoBar, &rc );
	wInfoYc = rc.bottom - rc.top;

	hwndPB = CreateWindowEx(WS_EX_TRANSPARENT, PROGRESS_CLASS, (LPSTR) NULL, 
		    WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
			1, 1, 2, 2, hWndInfoBar, (HMENU) 1020, _hModule, NULL);
	tobAdjustInfoBar(hWndPar);
	return hWndInfoBar;
}
/****************************************************************************/
BOOL WINAPI tobCreateInfoText( int id, int size )
{
int	pp[100], brd[4], np;

	if ( hWndInfoBar == NULL ) return FALSE;
	SendMessage( hWndInfoBar, SB_GETBORDERS, 0, (LPARAM)brd );
	np=SendMessage( hWndInfoBar, SB_GETPARTS, 100, (LPARAM)pp );
	pp[0] = wLEDW+6;
	SendMessage( hWndInfoBar, SB_SETTEXT, SBT_NOBORDERS, (LPARAM)"" );
	pp[np] = pp[np-1] + size+4;
	np++;
	SendMessage( hWndInfoBar, SB_SETPARTS, np, (LPARAM)pp );
	return TRUE;
}
/****************************************************************************/
int  WINAPI tobGetInfoHeight( void )
{
	return wInfoYc;            
}
/****************************************************************************/
VOID WINAPI tobShowInfoBar( BOOL sw )
{
	if (hWndInfoBar == NULL) return;
	ShowWindow( hWndInfoBar, sw ? SW_SHOWNA : SW_HIDE);
}
/****************************************************************************/
BOOL WINAPI tobCreateLED( int id, int offs )
{
	return (BOOL)CreateWindow(szCNLED, "",
							WS_VISIBLE | WS_CLIPSIBLINGS | WS_CHILD | WS_CLIPCHILDREN,
							offs, (wInfoYc-wLEDH)/2, wLEDW, wLEDH,
							hWndInfoBar, (HMENU)id, _hModule, NULL);
}
/****************************************************************************/
VOID WINAPI tobSetLED( int id, BOOL state)
{
	SendNotifyMessage( GetDlgItem(hWndInfoBar, id), WM_USER, state, 0);
}
/****************************************************************************/
int  WINAPI tobGetHeight( void )
{
RECT	r;
	GetWindowRect( hWndToolBar, &r );
	return r.bottom-r.top;            
}
/****************************************************************************/
int  WINAPI tobGetState( int id )
{
	if (hWndToolBar == NULL) return 0;
	return SendMessage(hWndToolBar, TB_ISBUTTONCHECKED, id, 0);
}
/****************************************************************************/
void WINAPI tobSetState( int id, int f )
{
	if (hWndToolBar == NULL) return;
	SendMessage(hWndToolBar, TB_CHECKBUTTON, id, MAKELONG(f,0));
}
/****************************************************************************/
void WINAPI tobEnableButton( int id, BOOL f )
{
	WORD	state;

	if (hWndToolBar == NULL || !::IsWindow(hWndToolBar))
		return;
	state = HIWORD(SendMessage(hWndToolBar, TB_GETSTATE, id, 0));
	if (f) { state &= ~TBSTATE_INDETERMINATE; state |=  TBSTATE_ENABLED; }
	else   { state |=  TBSTATE_INDETERMINATE; state &= ~TBSTATE_ENABLED; }
	SendMessage(hWndToolBar, TB_SETSTATE, id, MAKELONG(state,0));
}
/****************************************************************************/
void WINAPI tobUpdateToolBarWnd( void )
{
	if (hWndToolBar == NULL) return;
	UpdateWindow( hWndToolBar );
}
/****************************************************************************/
HWND WINAPI tobCreateToolBar( HWND hWndPar )
{
/*
#if 0
TBBUTTON but =  {	0,	0,	TBSTATE_ENABLED, TBSTYLE_SEP,	{0,0},	0,	0 };

	hWndToolBar = CreateToolbarEx(hWndPar,
								WS_CHILD | WS_VISIBLE | WS_BORDER |
								TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | TBSTYLE_ALTDRAG | TBSTYLE_WRAPABLE, // | TBSTYLE_FLAT,
								TOOLBAR_ID,	//TOOLBAR_ID,
								10,			// NUMIMAGES,
								_hModule,
								IDR_TOOLBAR1,
								&but,
								0,
								BUTTONWIDTH,
								BUTTONHEIGHT,
								IMAGEWIDTH,
								IMAGEHEIGHT,
								sizeof(TBBUTTON));
#elif 0
HIMAGELIST himl;
HWND	hwndRB;
REBARINFO     rbi;
REBARBANDINFO rbBand;

	hwndRB = CreateWindowEx(WS_EX_TOOLWINDOW,
						   REBARCLASSNAME,
						   NULL,
						   WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_OVERLAPPED|
						   WS_CLIPCHILDREN|RBS_VARHEIGHT|RBS_AUTOSIZE,
						   0,0,0,0,
						   hWndPar,
						   NULL,
						   _hModule,
						   NULL);
	if(!hwndRB)
		return NULL;

	hWndToolBar=CreateWindowEx(WS_EX_TOOLWINDOW,
								TOOLBARCLASSNAME,
								NULL,
								WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER |
								TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | TBSTYLE_ALTDRAG | TBSTYLE_WRAPABLE | (TBSTYLE_FLAT*0) ,
								0,0,0,0,  // Make it zero, Let owner resize it.
								hwndRB, //hWndPar,
								NULL, _hModule, NULL);

	SendMessage(hWndToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	// Set the maximum number of text rows and bitmap size.
	SendMessage(hWndToolBar, TB_SETMAXTEXTROWS, 1, 0);
	SendMessage(hWndToolBar, TB_SETBITMAPSIZE, 0, (LPARAM)MAKELONG(IMAGEWIDTH, IMAGEHEIGHT));
	himl = ImageList_LoadImage( hInst, MAKEINTRESOURCE(IDR_TOOLBAR1), IMAGEWIDTH, 100, CLR_DEFAULT, IMAGE_BITMAP, LR_DEFAULTCOLOR );
	SendMessage(hWndToolBar, TB_SETIMAGELIST, 0, (LPARAM)himl);
	SendMessage(hWndToolBar, TB_SETHOTIMAGELIST, 0, (LPARAM)himl);
	SendMessage(hWndToolBar, TB_SETPARENT, (WPARAM)hWndPar, 0);
	SendMessage(hWndToolBar, TB_AUTOSIZE, 0, 0);

	
	// Initialize and send the REBARINFO structure.
	memset(&rbi,0,sizeof(rbi));
	rbi.cbSize = sizeof(REBARINFO);  // Required when using this struct.
	rbi.fMask  = 0;
	rbi.himl   = (HIMAGELIST)NULL;
	if(!SendMessage(hwndRB, RB_SETBARINFO, 0, (LPARAM)&rbi))
		return NULL;

	// Initialize structure members that both bands will share.
	memset(&rbBand,0,sizeof(rbBand));
	rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
	rbBand.fMask  = RBBIM_COLORS | RBBIM_TEXT | (RBBIM_BACKGROUND*0) | 
				   RBBIM_STYLE | RBBIM_CHILD  | RBBIM_CHILDSIZE | 
				   RBBIM_SIZE;
	rbBand.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDBMP | RBBS_GRIPPERALWAYS;
	rbBand.hbmBack= NULL;
	rbBand.clrBack= RGB(0,0,255);
	rbBand.clrFore= RGB(255,0,0);

	// Set values unique to the band with the toolbar.
	rbBand.lpText     = "Tool Bar";
	rbBand.hwndChild  = hWndToolBar;
	rbBand.cxMinChild = 100;
	rbBand.cyMinChild = tobGetHeight()+40;
	rbBand.cx         = 250;
	// Add the band that has the toolbar.
	SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

	rbBand.hwndChild  = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR)"ABCD", 
        WS_CHILD | WS_VISIBLE, 10,10,30,10, 
        hwndRB, (HMENU) 0, _hModule, NULL);
	rbBand.lpText     = "ToolBar2";
	rbBand.cxMinChild = 100;
	rbBand.cyMinChild = tobGetHeight()+40;
	rbBand.cx         = 250;
	// Add the band that has the toolbar.
	SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
#else
*/
HIMAGELIST himl;

	hWndToolBar=CreateWindowEx(WS_EX_TOOLWINDOW,
								TOOLBARCLASSNAME,
								NULL,
								WS_CHILD | WS_VISIBLE | /*WS_CLIPCHILDREN | WS_CLIPSIBLINGS |  */WS_BORDER |
								TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | TBSTYLE_ALTDRAG | TBSTYLE_WRAPABLE /*| TBSTYLE_FLAT*/,
								0,0,0,0,	// Make it zero, Let owner resize it.
								hWndPar,	//hWndPar,
								NULL, _hModule, NULL);

	SendMessage(hWndToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	// Set the maximum number of text rows and bitmap size.
	SendMessage(hWndToolBar, TB_SETMAXTEXTROWS, 1, 0);
	SendMessage(hWndToolBar, TB_SETBITMAPSIZE, 0, (LPARAM)MAKELONG(IMAGEWIDTH, IMAGEHEIGHT));
	himl = ImageList_LoadImage( hInst, MAKEINTRESOURCE(IDR_TOOLBAR1), IMAGEWIDTH, 100, CLR_DEFAULT, IMAGE_BITMAP, LR_DEFAULTCOLOR );
	SendMessage(hWndToolBar, TB_SETIMAGELIST, 0, (LPARAM)himl);
	SendMessage(hWndToolBar, TB_SETHOTIMAGELIST, 0, (LPARAM)himl);
	SendMessage(hWndToolBar, TB_SETPARENT, (WPARAM)hWndPar, 0);

//#endif

	return hWndToolBar;
}
/****************************************************************************/
BOOL WINAPI tobCreateTButton( HWND hWndPar, int wID, int wIndx, BYTE wStyle )
{
TBBUTTON but={wIndx, wID, TBSTATE_ENABLED, wStyle, {0,0},  0, 0};

	if (hWndToolBar == NULL) return FALSE;
	return SendMessage(hWndToolBar, TB_ADDBUTTONS, 1, (LPARAM)&but);
}
/****************************************************************************/
VOID WINAPI tobShowToolBar( BOOL sw )
{
	if (hWndToolBar == NULL) return;
	ShowWindow( hWndToolBar, sw ? SW_SHOWNA : SW_HIDE);
}
/****************************************************************************/
VOID WINAPI tobAdjustToolBar( HWND hWndPar )
{
	if (hWndToolBar == NULL) return;
	SendMessage(hWndToolBar, TB_AUTOSIZE, 0, 0);
}
/****************************************************************************/
HWND WINAPI tobCreateToolBarA( HWND hWndPar )
{                           
HIMAGELIST himl;

	hWndToolBarA=CreateWindowEx(WS_EX_TOOLWINDOW,
								TOOLBARCLASSNAME,
								NULL,
								WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER |
								TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | TBSTYLE_ALTDRAG | TBSTYLE_WRAPABLE | (TBSTYLE_FLAT*0) ,
								0,0,0,0,	// Make it zero, Let owner resize it.
								hWndPar,	//hWndPar,
								NULL, _hModule, NULL);

	SendMessage(hWndToolBarA, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	// Set the maximum number of text rows and bitmap size.
	SendMessage(hWndToolBarA, TB_SETMAXTEXTROWS, 1, 0);
	SendMessage(hWndToolBarA, TB_SETBITMAPSIZE, 0, (LPARAM)MAKELONG(IMAGEWIDTH, IMAGEHEIGHT));
	himl = ImageList_LoadImage( hInst, MAKEINTRESOURCE(IDR_TOOLBAR1), IMAGEWIDTH, 100, CLR_DEFAULT, IMAGE_BITMAP, LR_DEFAULTCOLOR );
	SendMessage(hWndToolBarA, TB_SETIMAGELIST, 0, (LPARAM)himl);
	SendMessage(hWndToolBarA, TB_SETHOTIMAGELIST, 0, (LPARAM)himl);
	SendMessage(hWndToolBarA, TB_SETPARENT, (WPARAM)hWndPar, 0);
	return hWndToolBarA;
}
/****************************************************************************/
BOOL WINAPI tobCreateTButtonA( HWND hWnd, int wID, int wIndx, BYTE wStyle )
{
TBBUTTON but={wIndx, wID, TBSTATE_ENABLED, wStyle, {0,0},  0, 0};

	if (hWnd == NULL) return FALSE;
	return SendMessage(hWnd, TB_ADDBUTTONS, 1, (LPARAM)&but);
}
/****************************************************************************/
VOID WINAPI tobAdjustToolBarA( HWND hWnd )
{
	if (hWnd == NULL) return;
	SendMessage(hWnd, TB_AUTOSIZE, 0, 0);
}
/****************************************************************************/
BOOL WINAPI tobInit( void )
{
WNDCLASS  wc;
extern	HINSTANCE hInst;
INITCOMMONCONTROLSEX	icc={sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_BAR_CLASSES };

	_hModule = hInst;

	InitCommonControlsEx(&icc);

// LED
	memset(&wc,0,sizeof(wc));
	wc.style = 0;
	wc.cbWndExtra = 4;
	wc.hInstance = _hModule;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpfnWndProc = LEDWndProc;
	wc.lpszClassName = szCNLED;
	RegisterClass(&wc);
// Info text

	
	hbLEDON  = LoadBitmap( _hModule, "LEDON" );
	hbLEDOFF = LoadBitmap( _hModule, "LEDOFF" );

	if (hbLEDON==NULL || hbLEDOFF==NULL ) {
		tobClose();
		return FALSE;
	}

	return TRUE;
}
/****************************************************************************/
BOOL WINAPI tobClose( void )
{
	if (hbLEDON)  { DeleteObject( hbLEDON );  hbLEDON  = NULL; }
	if (hbLEDOFF) { DeleteObject( hbLEDOFF ); hbLEDOFF = NULL; }
	UnregisterClass( szCNLED, _hModule );
	return TRUE;            
}
