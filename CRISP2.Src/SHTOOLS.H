/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Tool Bar window definitions * by ML **************************************/
/****************************************************************************/
#pragma once

#include <vector>

//----------------------------------------------------------------------------
//- Tool and Status bar API --------------------------------------------------
//----------------------------------------------------------------------------
// shttlbar.c
BOOL WINAPI tobInit( void );
BOOL WINAPI tobClose( void );

HWND WINAPI tobCreateToolBar( HWND hWndPar );

BOOL WINAPI tobCreateTButton( HWND hWndPar, int wID, int wIndx, BYTE wtbStyle );

#define	IDB_OPEN	0
#define	IDB_SAVE	1
#define	IDB_EXIT	2
#define	IDB_CBCUT	3
#define	IDB_CBCOPY	4
#define	IDB_CBPASTE	5
#define	IDB_PRINT	6
#define	IDB_A128	7
#define	IDB_A256	8
#define	IDB_A512	9
#define	IDB_A1K		10
#define	IDB_FFT		11
#define	IDB_DIGI	12
#define	IDB_LREF	13
#define	IDB_OREF	14
#define	IDB_DMAP	15
#define	IDB_AMTR	16
#define	IDB_ONED	17
#define	IDB_IFFT	18
#define	IDB_FILT	19
#define	IDB_ELD		20
#define	IDB_CAL		21
#define	IDB_EDRD	22
#define	IDB_AXTL	23
#define	IDB_GRAB	24
#define	IDB_ITOOLS	25
#define	IDB_EDRING	26
#define	IDB_PHIDO	27
#define	IDB_HISTO	28
#define	IDB_FREEA	29
#define	IDB_A2K		30
#define	IDB_A4K		31
#define IDB_COLOR	32
#define IDB_S4TOOLS	33
#define IDB_INFO	34

/*
#define	IDB_EMPTY	6
#define	IDB_CHANNEL	7
#define	IDB_COLOR	8
#define	IDB_FULL	9
#define	IDB_LIVE	10
#define	IDB_NEW		11
#define	IDB_SNAP	12
#define	IDB_TUNE	13
*/

#define	TOOLBAR_ID		100
#define	NAVIG_ID		110

int  WINAPI tobGetHeight( void );
VOID WINAPI tobShowToolBar( BOOL sw );
int  WINAPI tobGetState( int id );
VOID WINAPI tobSetState( int id, int state );
VOID WINAPI tobEnableButton( int id, BOOL f );
VOID WINAPI tobAdjustToolBar( HWND hWndPar );
VOID WINAPI tobUpdateToolBarWnd( void );

HWND WINAPI tobCreateInfoBar( HWND hWndPar );
BOOL WINAPI tobCreateInfoText( int id, int size );
VOID WINAPI tobSetInfoText( int id, LPSTR str );
VOID WINAPI tobSetInfoTextAndBar( int id, LPSTR str, int perc );
VOID WINAPI tobAdjustInfoBar( HWND hWndPar );
int  WINAPI tobGetInfoHeight( void );
VOID WINAPI tobShowInfoBar( BOOL sw );

BOOL WINAPI tobCreateLED( int id, int offs );
VOID WINAPI tobSetLED( int id, BOOL state);

//----------------------------------------------------------------------------
//- Window managment ---------------------------------------------------------
//----------------------------------------------------------------------------
// shwind.c
/*--------------------------------------------------------------------------*/
typedef struct {
	int		p;				// Current scroll bar position
	int		def;			// Default value
	int		mi;				// Minimum scroll bar value
	int		ma;				// Maximum scroll bar value
	int		st;				// Step up/down value
	int		pg;				// Page up/down value
	int		idc;			// IDC of the respective numeric field
} SCROLLBAR;
typedef SCROLLBAR * LPSCROLLBAR;

VOID WINAPI wndPlaceToolWindow(HWND hWnd, HWND hDlg );
VOID WINAPI wndSetWindowSize(HWND hWnd, int x, int y, BOOL bMenu );
VOID WINAPI wndSetWindowPos(HWND hWnd, int x, int y );
VOID WINAPI wndGetWindowPos(HWND hWnd, LPINT x, LPINT y );
VOID WINAPI wndCenter(HWND hWnd, HWND Mhwnd, int top);
VOID WINAPI wndMoveChiWindow(HWND hWnd, int x, int y, BOOL f);
VOID WINAPI wndUpdateScrollBars( HWND hWnd, WPARAM wParam, LPARAM lParam, int sb);
VOID WINAPI wndInitScrollBars( HWND hWnd );
VOID WINAPI wndTrackWindow( HWND hWnd, OBJ* O, LPRECT r );

VOID WINAPI SetDlgItemDouble( HWND hDlg, int idc, double x, int w );
BOOL WINAPI GetDlgItemDouble(HWND hDlg, int IDC, double *result, int w);
VOID WINAPI UpdateScrollCtl( HWND hWnd, WPARAM wParam, LPARAM lParam, LPSCROLLBAR s);
VOID WINAPI InitScrollCtl( HWND hWnd, int id, LPSCROLLBAR s);
VOID WINAPI UpdateSliderCtl( HWND hWnd, WPARAM wParam, LPARAM lParam, LPSCROLLBAR s);
VOID WINAPI InitSliderCtl( HWND hWnd, int id, LPSCROLLBAR s);

VOID WINAPI wndChangeDlgSize(HWND hWnd, HWND hDlg, int idw, int idh, LPSTR lpName, HINSTANCE hInst);
HWND WINAPI wndCreateClientControls(char *pTemplateName, HINSTANCE hInst, FARPROC lpNew, HWND hWparent);
//----------------------------------------------------------------------------
//- File I/O API -------------------------------------------------------------
//----------------------------------------------------------------------------
// shtfile.c
class SAVEIMAGE
{
public:
	SAVEIMAGE()
	{
		lpData = NULL;
		lpAdd = NULL;
		x = y = 0;					// Whole Image dimensions
#ifdef USE_XB_LENGTH
		xb = 0;
#else
		stride = 0;
#endif
		x0 = y0 = 0;				// Start position
		cx = cy = 0;				// Size of area to be saved
		strip = 0;					// Min # of lines to R/W
		iAddSize = 0;				// Size of add on info
		ps = 0;						// Pixel size in bytes
		bSigned = FALSE;			// Data is a signed type
		_pix = PIX_BYTE;			// the pixel format
	}
	~SAVEIMAGE() {}
	LPBYTE	lpData;				// Data pointer
	int		x, y;				// Whole Image dimensions
#ifdef USE_XB_LENGTH
	int		xb;
#else
	int		stride;
#endif
	int		x0, y0;				// Start position
	int		cx, cy;				// Size of area to be saved
	int		strip;				// Min # of lines to R/W
	LPVOID	lpAdd;				// Pointer to Add on data
	int		iAddSize;			// Size of add on info
	int		ps;					// Pixel size in bytes
	BOOL	bSigned;			// Data is a signed type
	PIXEL_FORMAT _pix;			// the pixel format
} ;

typedef SAVEIMAGE * LPSAVEIMAGE;

_declspec (dllexport) BOOL WINAPI	fioOpenImageFile	( LPSTR, OBJ*, LPSAVEIMAGE, std::vector<OBJ*> & );
_declspec (dllexport) BOOL WINAPI	fioFreeImageFile	( LPSAVEIMAGE );
_declspec (dllexport) BOOL WINAPI	fioSaveImageFile	( LPSTR, LPSAVEIMAGE, WORD/* wJPEGQuality*/ );

//----------------------------------------------------------------------------
//- Private classes -----------------------------------------------------------
//----------------------------------------------------------------------------
// shtools.c
#define	JM_MOVE		0x1000

#define	SE_MOVE		0xA000

#define	JC_LEFT		1
#define	JC_RIGHT	2
#define	JC_UP		4
#define	JC_DOWN		8

BOOL	InitSHTtools( HINSTANCE hInst );
VOID	CloseSHTtools( HINSTANCE hInst );
