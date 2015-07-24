/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2 * by ML ***********************************************************/
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
#include "setinp.h"
#include "myprint.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#ifdef __FEI__
#	include "FEI.h"
#endif

// Enable the following line only for SU version (it will enable FOOS IP address range check)
// #define		STOCKHOLM_UNIVERSITY_ONLY

#ifdef		STOCKHOLM_UNIVERSITY_ONLY
#	pragma comment(lib, "Wsock32.lib")
#endif

// Added by Peter on 24 February 2006
#include "Crash.h"

#include "..\shark49x\pcidev.h"
#include "..\shark49x\shark4.h"

#include "eld.h"
#include "xCBN\shcb.h"

// Added by Peter on 22 Aug 2002
// SoftHard Technology MV4.0 support
#include "..\MV40\mv40api.h"
#include "..\MV40\PixFilt.h"
#include "MV40Live.h"
// end of addition

#include "testutils.h"

bool gRunTests = false;

#pragma optimize ("",off)
/***************************************************************/
/*               GLOBAL VARIABLES                              */
/***************************************************************/

HINSTANCE	hInst;			// Handle to instance.
HWND    	MainhWnd;		// Handle to main window.
HACCEL		hAccel;			// Accelerator resource

// Added by Peter on [11/8/2005]
HMODULE		hSpinStar = NULL;	// Spinning Star library
//typedef BOOL (*pfnSPINSTAR)(LPVOID, LPVOID);
typedef BOOL (__stdcall *pfnSPINSTAR)(LPVOID);
pfnSPINSTAR		g_pfnSPINSTAR = NULL;
// End of addition [11/8/2005]

// Added by Peter on [27/02/2006]
HMODULE		hPeakFit = NULL;	// PeakFit dialog
typedef BOOL (*pfnAcquireImage)(LPVOID);
typedef BOOL (*pfnGetImage)(LPVOID);
typedef BOOL (*pfnGetEmVector)(LPVOID);
pfnAcquireImage	g_pfnTVIPS_acq_img = NULL;
pfnGetImage		g_pfnTVIPS_get_img = NULL;
pfnGetEmVector	g_pfnTVIPS_get_vec = NULL;
// End of addition [27/02/2006]

// Added by Peter on [04/04/2006]
HMODULE		hTVIPS = NULL;	// TVIPS functions
//typedef LPVOID (*pfnPEAKFIT)(LPVOID, LPVOID, LONG &, double);
typedef BOOL (__stdcall *pfnPEAKFIT)(LPVOID, double);
typedef BOOL (__stdcall *pfnPEAKFIT_Free)(LPVOID);
pfnPEAKFIT		g_pfnPEAKFIT = NULL;
pfnPEAKFIT_Free	g_pfnPEAKFIT_Free = NULL;
// End of addition [04/04/2006]

// added by Peter on 28 June 2004
HMODULE		hSharkLib = NULL;	// shark library

pfnS4INIT g_pfnS4INIT = NULL;
pfnS4CLOSE g_pfnS4CLOSE = NULL;
pfnS4GETREALRESOLUTION g_pfnS4GETREALRESOLUTION = NULL;
pfnS4START g_pfnS4START = NULL;
pfnS4STOP g_pfnS4STOP = NULL;
pfnS4GETSTATUS g_pfnS4GETSTATUS = NULL;
pfnS4LIVEBUFFERREADY g_pfnS4LIVEBUFFERREADY = NULL;
pfnS4SELECTLIVEBUFFER g_pfnS4SELECTLIVEBUFFER = NULL;
pfnS4HOOKLIVEBUFFER g_pfnS4HOOKLIVEBUFFER = NULL;
pfnS4GETPARAMETER g_pfnS4GETPARAMETER = NULL;
pfnS4SETCLIPPING g_pfnS4SETCLIPPING = NULL;
pfnS4SETRESOLUTION g_pfnS4SETRESOLUTION = NULL;
pfnS4SETPARAMETER g_pfnS4SETPARAMETER = NULL;
pfnS4SETSOURCETYPE g_pfnS4SETSOURCETYPE = NULL;
pfnS4SETINPUT g_pfnS4SETINPUT = NULL;
pfnS4GETINPUT g_pfnS4GETINPUT = NULL;

// end of 28 June 2004 addition

HMODULE		hBWCorr = NULL;		// Black & White correction dll-module handle
FARPROC		pfnStartCorrProc = NULL;

char		szProgName[]="CRISP2";		// for resources only
char		szProg[]="CRISP 2.2";		// title
int			iVersion= 2;
int			iRelease= 2;
char		cSubRel = ' ';
char	 	szUserName[128];
char		szValidUntil[128];
char		szSerNo[128];

HWND		splashWnd = NULL;	// Logo window

extern HANDLE	hLiveThreadShark4;
GRABBER	nUseGrabber = GB_USE_NONE;

static char gszCmdLine[_MAX_PATH] = {0};
/*
extern BYTE szDemoUser[19];
extern BYTE szUntil[23];
extern BYTE szUnlimited[9];
extern BYTE szExpired[7];
extern BYTE szCrippled[8];
*/
#pragma comment( exestr, "Build on " __TIMESTAMP__ )

#define TIMESTAMP		"Build on " ##__TIMESTAMP__

static LPCTSTR s_pszTimeStamp = TIMESTAMP;

HWND		MDIhWnd=NULL;	// Handle to MDI frame
HWND		hWndToolBar;	// Toolbar window

HWND		hColors = NULL;	// Color tune box
HWND		hNavig = NULL;	// Navigator tree window
HWND		hNavigDlg = NULL;// Navigator frame box
HWND		hInputs = NULL;	// S4 inputs tune box
HWND		hInfo = NULL;	// Information panel box
HWND		hICalc = NULL;	// Image calculator box
HWND		hEditTop = NULL;	// Edit toolbar
HWND		hSelectET = NULL;// Image edit tool window

HWND		hActive = NULL;	// Active object window

int			iBPP, iPS;		// Number of bits per pixel, and pixel size in bytes

BOOL		bDoRealizePal=FALSE;

BOOL		fShowTool = TRUE;
BOOL		fShowStatus = TRUE;

WORD		wShowCmd = 2;	// Maximized
WORD		wShowCommands[3] = {SW_SHOWMINIMIZED, SW_SHOWNORMAL, SW_SHOWMAXIMIZED};

HFONT		hfInfo = NULL;
int			wFntW, wFntH;

#include	"copyrt.c"

char		TMP[TMPSIZE];

OFNAMES ofNames[]  = {
#ifdef __FEI__
/*OFN_IMAGER*/ { "Image files\0*.ndr;*.ser;*.emi;*.tif;*.jpg;*.pcx;*.bmp;*.mrc;*.img;*.vif;*.spe;*.xif;*.ip;*.dm2;*.dm3;*.ipf\0\
Demo image files load\0*.xif\0",	"emi", "", ""},
/*OFN_IMAGEW*/ { "Image files\0*.ser;*.emi;*.tif;*.jpg;*.pcx;*.bmp;*.mrc;*.img\0", 	"emi", "", ""},
#else // no __FEI__ support
/*OFN_IMAGER*/ { "Image files\0*.ndr;*.tif;*.jpg;*.pcx;*.bmp;*.mrc;*.img;*.vif;*.spe;*.xif;*.ip;*.dm2;*.dm3;*.ipf\0\
Demo image files\0*.xif\0",	"tif", "", ""},
/*OFN_IMAGEW*/ { "Image files\0*.tif;*.jpg;*.pcx;*.bmp;*.mrc;*.img\0",					"tif", "", ""},
#endif // __FEI__
/*OFN_HKLIST*/ { "CRISP HK list\0*.hka\0", 													"hka", "", ""},
/*OFN_FILTER*/ { "Filter files\0*.flt\0", 													"flt", "", ""},
/*OFN_PALETTE*/{ "Palette files\0*.pal\0", 													"pal", "", ""},
/*OFN_CALIBR*/ { "Calibration curve\0*.crv\0", 												"crv", "", ""},
/*OFN_LIST*/   { "List files\0*.lst\0", 													"lst", "", ""},
/*OFN_CERIUS*/ { "CERIUS Grapher Files\0*.grf\0", 											"grf", "", ""},
/*OFN_HKLLIST*/{ "CRISP 3D HKL list\0*.hkl\0", 												"hkl", "", ""},
/*OFN_HKELIST*/{ "ELD HK list\0*.hke\0", 													"hke", "", ""},
/*OFN_ELDDATA*/{ "ELD data file\0*.eld\0", 													"eld", "", ""},
/*OFN_BINARY*/ { "Binary files\0*.bin\0", 													"bin", "", ""},
/*end token*/{ NULL,																		"   ", "", ""}
};

HWND CreateObjectDialog(HINSTANCE hInst, LPCTSTR pszDlgName, HWND hMainWnd, OBJ* pObj);

LRESULT CALLBACK MainWndProc   (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChildWndProc  (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BigPlotWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL    CALLBACK AboutDlgProc  (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL    CALLBACK ICalcDlgProc  (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL	CALLBACK EditTopDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL    CALLBACK SelectETDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Grand parent object
OBJ	GPO;

LRESULT (CALLBACK * ObjectsWndProc[MAXOBJECTS])(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) =
{
	// OT_IMG		OT_AREA			OT_FFT			OT_HIST
	ImageWndProc,	AreaWndProc,	FFTWndProc,		HistWndProc,
	// OT_CORR		OT_DIGI			OT_IFFT			OT_FILT
	CorrWndProc,	DigiWndProc,	IFFTWndProc,	FiltWndProc,
	// OT_LREF		OT_OREF			OT_DMAP			OT_AMTR
	LRefWndProc,	ORefWndProc,	DMapWndProc,	AmtrWndProc,
	// OT_ONED		OT_ELD			OT_CAL			OT_PHIDO
	OneDWndProc,	ELDWndProc,		CalWndProc,		PhidoWndProc,
	// OT_EDRD		OT_AXTL			OT_UBEND
	EdRDWndProc,	AXtalWndProc,	UBendWndProc
};

UINT	uNewX0, uNewY0;
//UINT	uDlgNewX0, uDlgNewY0;

HPEN		RedPen;			// Red pen
HPEN		BluePen;		// Blue pen
HPEN		GreenPen;		// Green pen
HPEN		GrayPen;		// Grey pen
HPEN		YellowPen;		// Yellow pen
HPEN		MagentaPen;		// Magenta pen
HPEN		BluePen2;		// Blue pen 2pixel width
HPEN		RedPen2;		// Red pen 2pixel width
HCURSOR		hcWait;			// Hourglass cursor
HCURSOR		hcFFT;			// FFT window cursor
HCURSOR		hcHeight;		// Histogram window cursor
HCURSOR		hcAppStart;		// Arrow+Hourglass
HBRUSH		hPlotBrush;		// Plot window bkg
HPEN		hPlotPen;		// Pen used to draw plot bars
COLORREF	rgbPlotText;	// fg text color

// Added by Peter on 22 Aug 2002, defined as external in globals.h
BOOL	bMV40 = FALSE;
HANDLE	hMV = INVALID_HANDLE_VALUE;		// Handle to the camera device
DWORD	dwSerial;						// Camera serial number
// end of addition
BOOL	bShark4 = FALSE;
BOOL	bGrabberActive = FALSE;
BOOL	bAbrupt = FALSE;
HWND	hwndLive = NULL;
BOOL	bInformChilds = FALSE;

int		iImageSerial = 1;	// Grabbed image serial number

BOOL	bDemo = TRUE;

//---------------------- Default Project Info ------------------------------
// the function calls to read default microscope settings into NIDefault variable
// defined in Prjinfo.cpp
// Added by Peter on 03 Feb 2003
extern void CRISP_ReadNIDefault();
extern void CRISP_SaveNIDefault(HWND hDlg);

// ElMicroscope is -1 i.e. unknown
// Thus Acceleration voltage, Spherical abberation, Defocus0, Defocus1 values,
// Astigm. Azimuth, Spread of defocus, Convergence -- semi-angle at specimen all equ 0.

NEWINFO		NIDefault = { {'N','I'}, 0, 400e3f, 0.9e-3f, -470e-10f, -470e-10f, 0.f, 0.f, 0.f,
								1e-10f, 0.f, 0.f, 1e-10f, 0, 500000.f, 3.04145e-3f, 20e3f, 1.f, 0 };

//////////////////////////////////////////////////////////////////////////
//		CLIPBOARD FORMATS

UINT	g_Clipboard_8s;		// Clipboard format for 8 bit signed image
char	g_szClipboard_8s[] = "CRISP 8bit signed image";

UINT	g_Clipboard_16u;		// Clipboard format for 16 bit unsigned image
char	g_szClipboard_16u[] = "CRISP 16bit unsigned image";

UINT	g_Clipboard_16s;		// Clipboard format for 16 bit signed image
char	g_szClipboard_16s[] = "CRISP 16bit signed image";

UINT	g_Clipboard_32u;		// Clipboard format for 32 bit unsigned image
char	g_szClipboard_32u[] = "CRISP 32bit unsigned image";

UINT	g_Clipboard_32s;		// Clipboard format for 32 bit signed image
char	g_szClipboard_32s[] = "CRISP 32bit signed image";

UINT	g_Clipboard_32f;		// Clipboard format for FLOAT image
char	g_szClipboard_32f[] = "CRISP FLOAT image";

UINT	g_Clipboard_64f;		// Clipboard format for DOUBLE image
char	g_szClipboard_64f[] = "CRISP DOUBLE image";

CRISP	scbCrisp;

/****************************************************************************/
static	BOOL	InitFonts( HINSTANCE hInst )
{
	FontManager::Initialize();
	return TRUE;
}
/****************************************************************************/
static void InitCommonRes( HINSTANCE hInst )
{
	RedPen		= CreatePen(PS_SOLID, 1, RGB(255,  0,  0));
	GreenPen	= CreatePen(PS_SOLID, 1, RGB(  0,255,  0));
	BluePen		= CreatePen(PS_SOLID, 1, RGB(  0,  0,255));
	GrayPen		= CreatePen(PS_SOLID, 1, RGB(128,128,128));
	YellowPen	= CreatePen(PS_SOLID, 1, RGB(255,255,  0));
	MagentaPen	= CreatePen(PS_SOLID, 1, RGB(255,0,255));
	BluePen2	= CreatePen(PS_SOLID, 2, RGB(  0,  0,255));
	RedPen2		= CreatePen(PS_SOLID, 2, RGB(255,  0,  0));
	hcWait		= LoadCursor( NULL, IDC_WAIT );
	hcAppStart	= LoadCursor( NULL, IDC_APPSTARTING );
	hcFFT		= LoadCursor( hInst, "FFTCURSOR" );
	hcHeight	= LoadCursor( hInst, "MYHEIGHT" );
	hPlotBrush	= CreateSolidBrush(RGB(255,255,255));
	hPlotPen    = CreatePen(PS_SOLID, 1, RGB(0,0,0));
	rgbPlotText = RGB(0,0,0);

	g_Clipboard_8s		= RegisterClipboardFormat( g_szClipboard_8s  );
	g_Clipboard_16u		= RegisterClipboardFormat( g_szClipboard_16u );
	g_Clipboard_16s		= RegisterClipboardFormat( g_szClipboard_16s );
	g_Clipboard_32u		= RegisterClipboardFormat( g_szClipboard_32u );
	g_Clipboard_32s		= RegisterClipboardFormat( g_szClipboard_32s );
	g_Clipboard_32f		= RegisterClipboardFormat( g_szClipboard_32f );
	g_Clipboard_64f		= RegisterClipboardFormat( g_szClipboard_64f );
}
/****************************************************************************/
static void	FreeAllResources( void )
{
	pntClose();
	tobClose();
	InitSHTtools(hInst);
	DeletePen(RedPen);
	DeletePen(BluePen);
	DeletePen(GreenPen);
	DeletePen(GrayPen);
	DeletePen(YellowPen);
	DeletePen(MagentaPen);
	DeletePen(BluePen2);
	DeletePen(RedPen2);
	DeleteBrush(hPlotBrush);
	DeletePen(hPlotPen);
	DeleteFont(hfInfo);
	FontManager::Uninitialize();
}
/****************************************************************************/
void xxEncode( LPWORD data, int size )
{
	for(int i=0; i<size/2; i++) data[i] ^= MAGIC_NUMBER;
}
/****************************************************************************/
static void InitSecurity()
{
	USER	usr;
	CRISP	l;
	DWORD	t0;
	int		i;
	struct tm *ltm = NULL;
	static LPSTR mnth[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

	GetModuleFileName( hInst, TMP, sizeof(TMP) );
	*(strrchr( TMP, '\\')+1) =0;
	strcat( TMP, "crisp2.lic");

	if (GetPrivateProfileStruct( "CRISP2", "Lic_c",  &usr.c,         sizeof(usr.c),        TMP ) &&
		GetPrivateProfileStruct( "CRISP2", "SerNum", &usr.wSRN,      sizeof(usr.wSRN),     TMP ) &&
		GetPrivateProfileStruct( "CRISP2", "User",   &usr.UserName,  sizeof(usr.UserName), TMP ) )
	{
		xxEncode( (LPWORD)&usr.wSRN,     sizeof(usr.wSRN) );
		xxEncode( (LPWORD)&usr.c,        sizeof(usr.c) );
		xxEncode( (LPWORD)&usr.UserName, sizeof(usr.UserName) );

		l = usr.c;

		sprintf(szSerNo, "%05d", usr.wSRN); sscanf("0","%d",&bDemo);

		t0 = (DWORD)time(NULL);

		if (t0>l.dwExp)
		{
			for(i = 0; i < sizeof(szExpired); i++)
				szValidUntil[i] = szExpired[i]   ^ (5+(i%7)+(i%23));
		}
		else if(l.dwExp==-1)
		{
			for(i = 0; i < sizeof(szUnlimited); i++)
				szValidUntil[i] = szUnlimited[i] ^ (5+(i%7)+(i%23));
		}
		else
		{
	    	ltm = localtime( (time_t*)&l.dwExp );
			for(i=0;i<sizeof(szUntil); i++) TMP[i] = szUntil[i] ^ (5+(i%7)+(i%23));
			sprintf(szValidUntil, TMP, ltm->tm_mday, mnth[ltm->tm_mon], 1900+ltm->tm_year );
		}
		scbCrisp = l;

		strcpy( szUserName, usr.UserName );
	}
	else
	{
		for(i=0;i<sizeof(szDemoUser); i++) szUserName[i]   = szDemoUser[i] ^ (5+(i%7)+(i%23));
		for(i=0;i<sizeof(szCrippled); i++) szValidUntil[i] = szCrippled[i] ^ (5+(i%7)+(i%23));
		bDemo = TRUE;
	}

	if(szValidUntil[0]=='E' && szValidUntil[1] == 'x' && szValidUntil[2] == 'p')
	{
		szSerNo[0]=0;
		bDemo = TRUE;
	}
}

// Added by Peter on 28 June 2004
BOOL SharkLibInit()
{
	OSVERSIONINFO versionInfo;
	ZeroMemory(&versionInfo, sizeof(OSVERSIONINFO));
	if (GetVersionEx(&versionInfo))
	{
		if (versionInfo.dwMajorVersion >= 6)
			return FALSE; // shark driver not supported on win2k8, vista a
	}

	hSharkLib = ::LoadLibrary(_T("shark432.dll"));
	if( NULL == hSharkLib )
	{
		//::MessageBox(NULL, GetUserErrorAndTrace(_FMT(_T("Failed to load SHARK432.dll: %s"), GetLastErrorString().c_str())).c_str(), _T("ERROR"), MB_OK);
		return FALSE;
	}
	g_pfnS4INIT = (pfnS4INIT)::GetProcAddress(hSharkLib, _T("s4Init"));
	g_pfnS4CLOSE = (pfnS4CLOSE)::GetProcAddress(hSharkLib, _T("s4Close"));
	g_pfnS4GETREALRESOLUTION = (pfnS4GETREALRESOLUTION)
		::GetProcAddress(hSharkLib, _T("s4GetRealResolution"));
	g_pfnS4START = (pfnS4START)::GetProcAddress(hSharkLib, _T("s4Start"));
	g_pfnS4STOP = (pfnS4STOP)::GetProcAddress(hSharkLib, _T("s4Stop"));
	g_pfnS4GETSTATUS = (pfnS4GETSTATUS)::GetProcAddress(hSharkLib, _T("s4GetStatus"));
	g_pfnS4LIVEBUFFERREADY = (pfnS4LIVEBUFFERREADY)::GetProcAddress(hSharkLib, _T("s4LiveBufferReady"));
	g_pfnS4SELECTLIVEBUFFER = (pfnS4SELECTLIVEBUFFER)::GetProcAddress(hSharkLib, _T("s4SelectLiveBuffer"));
	g_pfnS4HOOKLIVEBUFFER = (pfnS4HOOKLIVEBUFFER)::GetProcAddress(hSharkLib, _T("s4HookLiveBuffer"));
	g_pfnS4GETPARAMETER = (pfnS4GETPARAMETER)::GetProcAddress(hSharkLib, _T("s4GetParameter"));
	g_pfnS4SETCLIPPING = (pfnS4SETCLIPPING)::GetProcAddress(hSharkLib, _T("s4SetClipping"));
	g_pfnS4SETRESOLUTION = (pfnS4SETRESOLUTION)::GetProcAddress(hSharkLib, _T("s4SetResolution"));
	g_pfnS4SETPARAMETER = (pfnS4SETPARAMETER)::GetProcAddress(hSharkLib, _T("s4SetParameter"));
	g_pfnS4SETSOURCETYPE = (pfnS4SETSOURCETYPE)::GetProcAddress(hSharkLib, _T("s4SetSourceType"));
	g_pfnS4SETINPUT = (pfnS4SETINPUT)::GetProcAddress(hSharkLib, _T("s4SetInput"));
	g_pfnS4GETINPUT = (pfnS4GETINPUT)::GetProcAddress(hSharkLib, _T("s4GetInput"));

	if( g_pfnS4INIT )
	{
		if( g_pfnS4INIT() )
		{
			bShark4 = TRUE;
		}
	}

	return TRUE;
}
// end of the addition on 28 June 2004

#ifdef		STOCKHOLM_UNIVERSITY_ONLY
BOOL StockholmUniversityIPCheck()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	char name[255];
	std::string ip;
	PHOSTENT hostinfo;
	BOOL bRet = FALSE;
	wVersionRequested = MAKEWORD( 2, 0 );
	
	if( 0 == WSAStartup( wVersionRequested, &wsaData ) )
	{
		
		if( 0 == gethostname ( name, sizeof(name)) )
		{
			if( NULL != (hostinfo = gethostbyname(name)) )
			{
				ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
				int n1, n2, n3, n4;
				sscanf(ip.c_str(), "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
				// check the SU IP range
				//if( (169 == n1) && (254 == n2) && (210 == n3) && (95 == n4) ) // 169.254.210.95
				if( (130 == n1) && (237 == n2) && (185 == n3) )
				{
					bRet = TRUE;
				}
			}
		}
		
		WSACleanup( );
	}
	return bRet;
}
#endif
/****************************************************************************/
//#if !defined USE_MFC

BOOL InitApplication(HINSTANCE hInst, HANDLE hPrev, int *pCmdShow, LPTSTR lpCmdLine)
{
	WNDCLASS wc;
	static int a;

	if( (NULL != lpCmdLine) && (0 != lpCmdLine[0]) && strcmp(lpCmdLine, "test") != 0)
	{
		strcpy(gszCmdLine, lpCmdLine);
	}

#ifdef		STOCKHOLM_UNIVERSITY_ONLY
	if( FALSE == StockholmUniversityIPCheck() )
	{
		MessageBox(NULL, "THIS VERSION OF CRISP EXCLUSEVLY BELONGS TO:\r\n"
			"PHYSICAL, INORGANIC AND STRUCTURAL CHEMISTRY DEPARTMENT\r\n"
			"STOCKHOLM UNIVERSITY, STOCKHOLM, SWEDEN", "ERROR", MB_OK);
		return FALSE;
	}
#endif

	memset(&wc,0,sizeof(wc));
	if (!hPrev)
	{
		wc.style         = 0;
		wc.lpfnWndProc   = MainWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInst;
		wc.hIcon         = LoadIcon(hInst,_T("CRISPICON"));
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);
		wc.lpszMenuName  = szProgName;
		wc.lpszClassName = szProgName;
		if (!RegisterClass(&wc)) return FALSE;

		wc.style         = CS_DBLCLKS;
		wc.lpfnWndProc   = ChildWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 4;
		wc.hIcon         = LoadIcon(hInst,_T("CALIDRIS"));	// "CRISPICON"
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_BTNSHADOW+1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = "CHILD";
		if (!RegisterClass(&wc)) return FALSE;

		wc.style         = CS_DBLCLKS;
		wc.lpfnWndProc   = BigPlotWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 4;
		wc.hIcon         = LoadIcon(hInst,_T("CALIDRIS"));	// "CRISPICON"
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_BTNSHADOW+1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = "BIGPLOT";
		if (!RegisterClass(&wc)) return FALSE;

		wc.style         = CS_DBLCLKS | CS_SAVEBITS | CS_BYTEALIGNWINDOW;
		wc.lpfnWndProc   = DefDlgProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = DLGWINDOWEXTRA;
		wc.hInstance     = hInst;
		wc.hIcon         = LoadIcon(hInst, _T("CALIDRIS"));	// "CRISPICON"
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = "CrispDlgClass";
		if (!RegisterClass(&wc)) return FALSE;
	}

	InitSecurity();

	if (! pntInit() )
	{
		MessageBox( NULL, _T("Can not initilalize Paint resource"), szProg, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (! InitFonts(hInst) )
	{
		MessageBox( NULL, _T("Can not initilalize Font resource"), szProg, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!gRunTests)
	{
		splashWnd = CreateDialogParam(hInst, "ABOUT", NULL, AboutDlgProc, TRUE);
		UpdateWindow( splashWnd );
	}

	if (! tobInit() ) {
		MessageBox( NULL, "Can not initilalize ToolBar resource", szProg, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (! InitSHTtools(hInst) ) {
		MessageBox( NULL, "Can not initilalize shtTools resource", szProg, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

// Added by Peter on 28 June 2004
	//SharkLibInit();

// end of the addition on 28 June 2004
/*
	if (s4Init())
	{
		bShark4 = TRUE;
	}
*/

// Added by Peter on 22 Aug 2002
	U32 devices;
	mv40GetNumberDevices( szMV40_Name, &devices );
	if ( devices )
	{
		bMV40 = TRUE;

/*		mv40GetDevice( 0, &dwSerial );
		if (dwSerial == 0x53535353)		// bootloader here
			return FALSE;

		mv40GetDevice( 0, &dwSerial );

		mv40Initialize( szMV40_Name, dwSerial, &hMV );
		if (hMV == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, "MV40 device found, but initialization failed", "CRISP", MB_OK | MB_ICONSTOP );
		}
*/	}
// end of addition
	{
		HDC hDC = GetDC(NULL);
		HFONT	hof;
		TEXTMETRIC	tm;

		if (hDC)
		{
			iBPP = GetDeviceCaps( hDC, BITSPIXEL );
			if (iBPP<8) iBPP = 8;
			iPS = (iBPP+7)/8;

			hfInfo = CreateFont( MulDiv((-8), GetDeviceCaps(hDC, LOGPIXELSY), 72),
								  0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
								  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
								  FIXED_PITCH | FF_MODERN, "MS Sans Serif");
			hof = SelectFont( hDC, hfInfo);
			GetTextMetrics( hDC, &tm );
			wFntH = tm.tmHeight;
			wFntW = tm.tmAveCharWidth;
			if (hof) SelectFont( hDC, hof );
			ReleaseDC(MainhWnd, hDC);
		}
		else
		{
			return FALSE;
		}
	}

	// added by Peter on 03 Feb 2003
	CRISP_ReadNIDefault();

	MainhWnd = CreateWindow(szProgName, szProg,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			0, 0, hInst, (LPSTR)NULL);
	if (!MainhWnd) return FALSE;

	// Added by Peter on 28 May 2008
	::DragAcceptFiles(MainhWnd, TRUE);
	// End of Peters addition on 28 May 2008

	InitCommonRes( hInst );

	if (!(hAccel = LoadAccelerators(hInst, "CRISPACC")))
		return FALSE;

	*pCmdShow = wShowCommands[wShowCmd];

#ifdef __FEI__
	FEI_Init();
#endif
/*
#ifdef _DEBUG
	hTVIPS = ::LoadLibrary(_T("tvipsD.dll"));
#else
	hTVIPS = ::LoadLibrary(_T("tvips.dll"));
#endif
	if( NULL != hTVIPS )
	{
		g_pfnTVIPS_acq_img = (pfnAcquireImage)::GetProcAddress(hTVIPS, _T("TVIPS_AcquireImage"));
		g_pfnTVIPS_get_img = (pfnGetImage)::GetProcAddress(hTVIPS, _T("TVIPS_GetImage"));
		g_pfnTVIPS_get_vec = (pfnGetEmVector)::GetProcAddress(hTVIPS, _T("TVIPS_GetEmVector"));
	}
*/
#ifdef _DEBUG
	hPeakFit = ::LoadLibrary(_T("PeakFitD.dll"));
#else
	hPeakFit = ::LoadLibrary(_T("PeakFit.dll"));
#endif // _DEBUG
	if( NULL != hPeakFit )
	{
		g_pfnPEAKFIT = (pfnPEAKFIT)::GetProcAddress(hPeakFit, _T("Run_PeakFit"));
		g_pfnPEAKFIT_Free = (pfnPEAKFIT_Free)::GetProcAddress(hPeakFit, _T("Free_PeakFit"));
	}
	else
	{
		Trace(_FMT(_T("Failed to load PeakFit library: %s"), GetLastErrorString().c_str()));
	}

#ifdef _DEBUG
	hSpinStar = ::LoadLibrary(_T("spinstarD.dll"));
#else
	hSpinStar = ::LoadLibrary(_T("spinstar.dll"));
#endif // _DEBUG
	if( NULL != hSpinStar )
	{
		g_pfnSPINSTAR = (pfnSPINSTAR)::GetProcAddress(hSpinStar, _T("Run_SpinStar"));
	}
	else
	{
		Trace(_FMT(_T("Failed to load spinstar library: %s"), GetLastErrorString().c_str()));
	}

	// added by Peter on 9 Oct 2001
	if( (hBWCorr = ::LoadLibrary("BWCorr.dll")) != NULL )
	{
		if( NULL == (pfnStartCorrProc = ::GetProcAddress(hBWCorr, "StartCorrProc")) )
		{
			DWORD dwErr = ::GetLastError();
			LPVOID lpMsgBuf;

			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL
				);
			::OutputDebugString((LPCTSTR)lpMsgBuf);
			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
	}
	else
	{
		Trace(_FMT(_T("Failed to load BWCorr library: %s"), GetLastErrorString().c_str()));
	}
	extern BOOL TVIPS_Initialize();
	TVIPS_Initialize();

	return TRUE;
}
//#endif // USE_MFC

// Added by Peter on [28 May 2008]
/*****************************************************************************
*
* MWnd_OnDropFiles
*
* Handle WM_DROPFILES message to main application window.
*
* HWND hWnd                 - Window handle
* HDROP hDrop               - Handle to dropped file information
*
* Get the 0th filename and free the drop handle.
* Extract the file title from the full pathname.
* Open the file.
* If we opened successfully, start playback by forwarding a WM_COMMAND
*   of IDM_PLAY to the main window.
*
*****************************************************************************/

static void MWnd_OnDropFiles(HDROP hDrop)
{
	std::vector<char> szOpenName;

	if( NULL == hDrop )
	{
		return;
	}
	// For multiple selections
	UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if( 0 == nFiles )
	{
		return;
	}
	// open files one by one
	for(UINT i = 0; i < nFiles; i++)
	{
		// get the length of the string buffer
		UINT nLength = DragQueryFile(hDrop, i, NULL, 0);
		if( nLength + 1 > szOpenName.size() )
		{
			szOpenName.resize(nLength + 1);
		}
		DragQueryFile(hDrop, i, &*szOpenName.begin(), szOpenName.size());
		_OpenShellFile(&*szOpenName.begin());
	}
	DragFinish(hDrop);
}
// End of addition by Peter on [28 May 2008]

// Added by Peter on [5/8/2005]
static void FinalizeCRISP()
{
	if( NULL != hSharkLib )
	{
		::FreeLibrary(hSharkLib);
		hSharkLib = NULL;
	}
	if( NULL != hSpinStar )
	{
		::FreeLibrary(hSpinStar);
		hSpinStar = NULL;
	}
	if( NULL != hPeakFit )
	{
		::FreeLibrary(hPeakFit);
		hPeakFit = NULL;
	}
	if( NULL != hTVIPS )
	{
		::FreeLibrary(hTVIPS);
		hTVIPS = NULL;
	}
	extern BOOL TVIPS_Uninitialize();
	TVIPS_Uninitialize();
}
// End of addition [5/8/2005]

/****************************************************************************/
static void	myProcessMessage( LPMSG msg )
{
	OBJ* pObj;
	if (hAccel && TranslateAccelerator(MainhWnd, hAccel, msg)) return;

	if (hColors		&& IsWindow(hColors))	{ if (IsDialogMessage(hColors,msg))		return; } else { hColors = NULL; }
	if (hInputs		&& IsWindow(hInputs))	{ if (IsDialogMessage(hInputs,msg))		return; } else { hInputs = NULL; }
	if (hNavigDlg	&& IsWindow(hNavigDlg)) { if (IsDialogMessage(hNavigDlg,msg))	return; } else { hNavigDlg = NULL; }
	if (hInfo		&& IsWindow(hInfo))		{ if (IsDialogMessage(hInfo,msg))		return; } else { hInfo = NULL; }
	if (hICalc		&& IsWindow(hICalc))	{ if (IsDialogMessage(hICalc,msg))		return; } else { hICalc = NULL; }
	if (hEditTop	&& IsWindow(hEditTop))	{ if (IsDialogMessage(hEditTop,msg))	return;	} else { hEditTop = NULL; }
	if (hSelectET	&& IsWindow(hSelectET))	{ if (IsDialogMessage(hSelectET,msg))	return; } else { hSelectET = NULL; }
	// Added by Peter on 27 Feb 2006
	try
	{
		pObj = OBJ::GetOBJ(hActive);
		if( NULL != pObj )
		{
			if (hActive && IsWindow(hActive) && pObj->bDialog)
			{
				if (IsDialogMessage(hActive,msg)) 
					return;
			}
			if (hActive && IsWindow(hActive) && pObj->hDlg)
			{
				if (IsDialogMessage(pObj->hDlg,msg)) 
					return;
			}
		}
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("myProcessMessage() -> %s"), e.what()));
	}
	TranslateMessage(msg); /* Translates character keys                */
	DispatchMessage (msg); /* Dispatches message to window             */
}
/****************************************************************************/
void    EmptyQueue( LPWORD lpbrk )
{
	MSG		msg;
    while (PeekMessage( &msg,0,0,0,PM_NOREMOVE))
	{
        if (msg.message == WM_QUIT && lpbrk) { *lpbrk = FALSE; break; }
        PeekMessage( &msg,0,0,0,PM_REMOVE);
		myProcessMessage( &msg );
    }
}
/****************************************************************************/
void InitializeMenu( HWND hWnd )
{
	HMENU	hm = GetMenu( hWnd );
	DWORD	dd = 0;
	OBJ*	O;
	BOOL	bSave = FALSE;

#define xEnable( id, ff ) tobEnableButton(id,ff);\
						  EnableMenuItem( hm, id, MF_BYCOMMAND | (ff?MF_ENABLED : (MF_DISABLED|MF_GRAYED)))
#define xCheck( id, ff ) tobSetState(id,ff);\
						 CheckMenuItem( hm, id, MF_BYCOMMAND | (ff?MF_CHECKED : MF_UNCHECKED))
#define	b(a) (1<<a)

	if (hActive==NULL || (O=OBJ::GetOBJ(hActive))==NULL ) goto cont;
	switch(O->ID)
	{
	case OT_NONE:
		break;
	case OT_IFFT:
	case OT_IMG:
		if (bGrabberActive) dd = b(OT_AREA) | b(OT_DIGI) | b(OT_HIST) | b(OT_ONED) | b(OT_AXTL);
		else				dd = b(OT_AREA) | b(OT_DIGI) | b(OT_HIST) | b(OT_ONED) | b(OT_AXTL) | b(OT_ELD) | b(OT_EDRD) | b(OT_CAL);
		bSave=TRUE;
		break;
	case OT_AREA:	dd = b(OT_CORR) | b(OT_HIST); if (O->ui!=0) dd |= b(OT_FFT); bSave=TRUE; break;
	case OT_FFT:	dd = b(OT_DIGI) | b(OT_LREF) | b(OT_FILT) | b(OT_IFFT); bSave=TRUE; break;
	// Added by Peter on 6 Aug 2003
	case OT_DIGI:	 bSave=TRUE; break;
	// Added by Peter on 21 Aug 2003
	case OT_ONED:	 bSave=TRUE; break;
	// END OF ADD
	case OT_LREF:
		{
			LPLATTICE	L = GetLATTICE( O );
			if(Bit(L->LR_Mode, LR_DONE)) dd = b(OT_OREF);
			break;
		}

	case OT_AXTL:	dd = b(OT_DMAP); break;
	case OT_OREF:	dd = b(OT_DMAP) | b(OT_UBEND); break;
	case OT_DMAP:	dd = b(OT_AMTR); bSave = TRUE; break;
	case OT_ELD:	dd = b(OT_PHIDO); break;
	case OT_FILT:	dd = b(OT_IFFT) | b(OT_FILT) | b(OT_LREF); break;
	default: break;
	}
cont:

// Change by Peter on 22 Aug 2002
	int nGrabByShark4 = bShark4 && (scbCrisp.flags & CRISP_ACQUISITION);
	int nGrabByMV40   = bMV40;
	xEnable( IDM_F_GRAB, (nGrabByShark4 != 0) || (nGrabByMV40 != 0) );
	xCheck ( IDM_F_GRAB, bGrabberActive );

	if (!bDemo)
	{
		if ( (scbCrisp.flags & CRISP_CRISP) == 0 )
			dd &= ~(b(OT_FFT) | b(OT_LREF) | b(OT_OREF) | b(OT_DMAP) | b(OT_AMTR) | b(OT_UBEND) | b(OT_FILT) | b(OT_IFFT) | b(OT_AXTL));

		if ( (scbCrisp.flags & CRISP_ELD) == 0)
			dd &= ~(b(OT_ELD));

		if ( (scbCrisp.flags & CRISP_CALIBRATION) == 0)
			dd &= ~(b(OT_CAL));

		if ( (scbCrisp.flags & CRISP_PHIDO) == 0)
			dd &= ~(b(OT_PHIDO));

		if ( (scbCrisp.flags & CRISP_EDRD) == 0 )
			dd &= ~(b(OT_EDRD));
	}

	xEnable( IDM_A_AR128,  dd&b(OT_AREA));
	xEnable( IDM_A_AR256,  dd&b(OT_AREA));
	xEnable( IDM_A_AR512,  dd&b(OT_AREA));
	xEnable( IDM_A_AR1K ,  dd&b(OT_AREA));
	xEnable( IDM_A_AR2K ,  dd&b(OT_AREA));
	xEnable( IDM_A_AR4K ,  dd&b(OT_AREA));
	xEnable( IDM_A_ARFREE, dd&b(OT_AREA));
	xEnable( IDM_A_DIGI,   dd&b(OT_DIGI));
	xEnable( IDM_C_HIST,   dd&b(OT_HIST));
	xEnable( IDM_C_CORR,   dd&b(OT_CORR));

	xEnable( IDM_C_FFT,    dd&b(OT_FFT));
	xEnable( IDM_C_LREF,   dd&b(OT_LREF));
	xEnable( IDM_C_OREF,   dd&b(OT_OREF));
	xEnable( IDM_C_DMAP,   dd&b(OT_DMAP));
	xEnable( IDM_C_AMTR,   dd&b(OT_AMTR));
	xEnable( IDM_C_ONED,   dd&b(OT_ONED));
// removed by Peter on 20 Sep 2001
//	xEnable( IDM_C_UBEND,  dd&b(OT_UBEND));
	xEnable( IDM_C_AXTAL,  dd&b(OT_ONED));

	xEnable( IDM_A_FILT,   dd&b(OT_FILT));
	xEnable( IDM_A_IFFT,   dd&b(OT_IFFT));

	xEnable( IDM_E_ELD,    dd&b(OT_ELD));

	xEnable( IDM_E_CAL,    dd&b(OT_CAL));

	xEnable( IDM_E_EDRD,   dd&b(OT_EDRD));
	xEnable( IDM_E_PHIDO,  dd&b(OT_PHIDO));

	xEnable( IDM_F_OPEN,   TRUE);
	xEnable( IDM_F_OPEN_ELD, b(OT_ELD) );
	xEnable( IDM_F_SAVE,   bSave);
	xEnable( IDM_F_SSAVE,  bSave);
	xEnable( IDM_F_PRINT,  bSave);
	xEnable( IDM_F_EXIT,   TRUE);

	xEnable( IDM_W_CLOSE,  hActive!=NULL);

/*
	EnableMenuItem( hm, IDM_F_PREF, (!bLive ? MF_ENABLED : (MF_DISABLED|MF_GRAYED)));
	st = f?MF_ENABLED : (MF_DISABLED|MF_GRAYED);
	EnableMenuItem( hm, IDM_V_FIT, st);
	EnableMenuItem( hm, IDM_W_VTILE, st);
	EnableMenuItem( hm, IDM_W_HTILE, st);
	EnableMenuItem( hm, IDM_W_IARRANGE, st);
	EnableMenuItem( hm, IDM_W_CASCADE, st);
	EnableMenuItem( hm, IDM_W_CLOSEALL, st);
*/

	CheckMenuItem( hm, IDM_V_TOOL,    MF_BYCOMMAND | (fShowTool   ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hm, IDM_V_STATUS,  MF_BYCOMMAND | (fShowStatus ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hm, IDM_V_COLORS,  MF_BYCOMMAND | (hColors     ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hm, IDM_V_SETINPUT,MF_BYCOMMAND | (hInputs     ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hm, IDM_V_NAVIG,   MF_BYCOMMAND | (hNavigDlg   ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hm, IDM_V_SELECTET,MF_BYCOMMAND | (hSelectET   ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hm, IDM_V_ICALC,   MF_BYCOMMAND | (hICalc      ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hm, IDM_L_EDITTOP, MF_BYCOMMAND | (hEditTop    ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem( hm, IDM_V_INFO,    MF_BYCOMMAND | (hInfo       ? MF_CHECKED : MF_UNCHECKED));

	// Added by Peter on 12 April 2007
	if( !bDemo )
	{
		BOOL bEnable =
			IsClipboardFormatAvailable(CF_DIB) ||
			IsClipboardFormatAvailable(g_Clipboard_8s) ||
			IsClipboardFormatAvailable(g_Clipboard_16u) ||
			IsClipboardFormatAvailable(g_Clipboard_16s) ||
			IsClipboardFormatAvailable(g_Clipboard_32u) ||
			IsClipboardFormatAvailable(g_Clipboard_32s) ||
			IsClipboardFormatAvailable(g_Clipboard_32f) ||
			IsClipboardFormatAvailable(g_Clipboard_64f) ;
		xEnable( IDM_E_PASTE, bEnable );
	}
	else
	{
		xEnable( IDM_E_PASTE, FALSE );
	}
	// End of addition by Peter on 12 April 2007
	xEnable( IDM_E_COPY,  bSave );
	xEnable( IDM_E_XCOPY, hActive != NULL );
	tobUpdateToolBarWnd();
	regUpdateFilesMenu(hWnd);

#undef xEnable
#undef xCheck
#undef b

}
/****************************************************************************/
void LoadConfig( void )
{
	int		i;

	// Added by Peter on [8/8/2005]
	regCheckKeyAccess();
	// end of addition by Peter [8/8/2005]
	for(i = 0; ofNames[i].szFilter != NULL; i++)
	{
		regGetData( "File locations", ofNames[i].szFilter, ofNames[i].szDefDir,
			sizeof(ofNames[i].szDefDir));
	}
	regReadRecentFiles();
	prnLoadConfig();
}
/****************************************************************************/
static void SaveConfig( void )
{
	int		i;
	for(i = 0; ofNames[i].szFilter != NULL; i++)
	{
		regSetData( "File locations", ofNames[i].szFilter, ofNames[i].szDefDir, 0);
	}
	regWriteRecentFiles();
	prnSaveConfig();
}
/****************************************************************************/

#if !defined USE_MFC

/****************************************************************************/
/* WinMain FUNCTION *********************************************************/
/****************************************************************************/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,  LPSTR lpCmdLine, int nCmdShow)
{
	if (strcmp(lpCmdLine, "test") == 0)
		gRunTests = true;

	Trace(_T("\r\n\r\nStarting ---------------------------------------------"));
	SetUnhandledExceptionFilter(CrashHandler);
	_set_se_translator(CrashHandlerTranslator);
	if (!IsDebuggerPresent())
	{
		try
		{
			Trace(_T("Begin stack trace test"));
			int* asd = 0; 
			*asd = 0;
		}
		catch (UnhandledException& e)
		{
			Trace(e.what());
			Trace(_T("End stack trace test"));
		}
	}

	MSG 	msg;
	static	WORD ccww = 0x137F;
	BOOL bWorking = TRUE;

	hInst = hInstance;
	try
	{
		_asm {
			fldcw	[ccww];
		}
		LoadConfig();
		if (!InitApplication(hInstance,hPrevInstance,&nCmdShow,lpCmdLine))
			goto exx;

		if (bShark4)
		{
			siInitInputs();
			siSetSharkInput();
		}
		ShowWindow(MainhWnd, nCmdShow);
		UpdateWindow(MainhWnd);
		bWorking = TRUE;
		while( bWorking )
		{
			try
			{
				bWorking = GetMessage(&msg,0, 0, 0);
				myProcessMessage( &msg );
			}
			catch (std::exception& e)
			{
				::MessageBox(NULL, GetUserErrorAndTrace(_FMT(_T("Error processing windows message %d lparam %d wparam %d: %s"),
					msg.message, msg.lParam, msg.wParam, e.what())).c_str(), "ERROR", MB_OK);
			}
		}
		FinalizeCRISP();
		::OutputDebugString("Finalizing CRISP\n");
	}
	catch(std::exception& e)
	{
		::MessageBox(NULL, GetUserErrorAndTrace(_FMT(_T("Internal error - CRISP2 will be closed now: %s"), e.what())).c_str(), "ERROR", MB_OK);
	}
exx:
	if (splashWnd)
		DestroyWindow(splashWnd);
	FreeAllResources();
	SaveConfig();
	if (bShark4)
	{
		siSaveInputs();
		if( g_pfnS4CLOSE )
		{
			g_pfnS4CLOSE();
		}
	}
	if ( bMV40 )
	{
		//		mv40Uninitialize(hMV);
		hMV = INVALID_HANDLE_VALUE;
	}
	if(hBWCorr != NULL)
		::FreeLibrary(hBWCorr);

	return 0;
}
#endif // USE_MFC

/****************************************************************************/
/* Main Window **************************************************************/
/****************************************************************************/
static void AdjustMDIsize( void )
{
	RECT	r;
	int		tbh;

	tobAdjustToolBar( MainhWnd );
	tbh = tobGetHeight();
	GetClientRect( MainhWnd, &r );
	SetWindowPos( MDIhWnd, NULL,
				 0, fShowTool?tbh:0,
				 r.right,
				 r.bottom-(fShowTool?tbh:0) - (fShowStatus?tobGetInfoHeight():0),
				 SWP_NOZORDER);
}
/****************************************************************************/
void SendMessageToAllChildren (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hChild;

	if (hChild = GetWindow(hWnd, GW_CHILD))     // Get 1st child.
		do  SendMessage(hChild, message, wParam, lParam);
		while (hChild = GetWindow(hChild, GW_HWNDNEXT));
}
/****************************************************************************/
static VOID InitializeToolbar( HWND hWnd )
{
	tobCreateInfoBar( hWnd );
	tobCreateLED( ID_LIVELED, 4 );
	tobCreateInfoText( ID_INFO, 300 );
	tobCreateInfoText( ID_DDB, 200 );
	tobSetInfoText( ID_INFO, "*CRISP* 2" );
	tobSetInfoText( ID_DDB, "Ready" );

	hWndToolBar = tobCreateToolBar( hWnd );
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_F_GRAB, IDB_GRAB,		TBSTYLE_CHECK);
	tobCreateTButton( hWnd, IDM_F_OPEN, IDB_OPEN,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_F_SAVE, IDB_SAVE,		TBSTYLE_BUTTON);
//	tobCreateTButton( hWnd, IDM_F_PRINT,IDB_PRINT,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_E_COPY, IDB_CBCOPY,		TBSTYLE_BUTTON);
//	tobCreateTButton( hWnd, IDM_E_PASTE,IDB_CBPASTE,	TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_A_AR128,IDB_A128,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_A_AR256,IDB_A256,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_A_AR512,IDB_A512,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_A_AR1K, IDB_A1K,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_A_AR2K, IDB_A2K,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_A_AR4K, IDB_A4K,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_A_ARFREE,IDB_FREEA,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_C_FFT,  IDB_FFT,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_A_FILT, IDB_FILT,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_A_IFFT, IDB_IFFT,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_C_LREF, IDB_LREF,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_C_OREF, IDB_OREF,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_C_DMAP, IDB_DMAP,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_C_AMTR, IDB_AMTR,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_E_CAL,  IDB_EDRD,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_E_ELD,  IDB_ELD,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_E_EDRD, IDB_EDRING,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_E_PHIDO,IDB_PHIDO,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_A_DIGI, IDB_DIGI,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_C_HIST,	IDB_HISTO,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_C_ONED, IDB_ONED,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);
	tobCreateTButton( hWnd, IDM_C_CORR,	IDB_CAL,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_V_ICALC,IDB_ITOOLS,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, IDM_C_AXTAL,IDB_AXTL,		TBSTYLE_BUTTON);
	tobCreateTButton( hWnd, 0,          0,				TBSTYLE_SEP);

//	tobCreateTButton( hWnd, IDM_V_SELECTET,IDB_ITOOLS,  TBSTYLE_BUTTON);

//	tobCreateTButton( hWnd, IDM_F_EXIT, IDB_EXIT,    TBSTYLE_BUTTON);
}
/****************************************************************************/
int	HaveChildren( HWND hw )
{
	OBJ*	pObj = OBJ::GetOBJ(hw);

	if( NULL != pObj )
	{
		return (false == pObj->m_vChildren.empty()) ? 1 : 0;
	}
	// Commented by Peter on 06 July 2006
//	int		i;
//	for(int i = 0; i < MAXCHI; i++)
//	{
//		if( pObj->Chi[i] )
//			return 1;
//	}
	// End of comments by Peter on 06 July 2006
/*	if( NULL != ::GetTopWindow(hw) )
	{
		return 1;
	}
	for(HWND hWndChild = ::GetTopWindow(hw); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT) )
	{
	}
*/
	return 0;
}
/****************************************************************************/
BOOL objInsertChildren( HWND hw, HTREEITEM item, BOOL bForceCreate )
{
	OBJ* pTmpObj;
	OBJ* pObj = OBJ::GetOBJ(hw);
	static OBJ* OC;
	TV_ITEM	tvi;
	TV_INSERTSTRUCT tvins;
	HwndVecCit cit;

	if( NULL == pObj )
	{
		return FALSE;
	}
	for(cit = pObj->m_vChildren.begin(); cit != pObj->m_vChildren.end(); ++cit)
	{
		HWND hChild = *cit;
		if( (NULL != hChild) && (TRUE == ::IsWindow(hChild)) )
		{
			OC = OBJ::GetOBJ(hChild);
			tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			GetWindowText(hChild, TMP, 100);
			tvi.pszText = TMP;
			tvi.cchTextMax = strlen(TMP);
			tvi.lParam = (LPARAM)hChild;
			tvi.cChildren = HaveChildren(hChild);
			pTmpObj = OBJ::GetOBJ(hChild);
			if( NULL != pTmpObj )
			{
				tvi.iImage = pTmpObj->ID;
			}
			tvi.iSelectedImage = tvi.iImage;
			if ( OC->hTree==0 || bForceCreate )
			{
				tvins.item=tvi;
				tvins.hParent = item;
				tvins.hInsertAfter = TVI_LAST;
				OC->hTree = TreeView_InsertItem( hNavig, &tvins );
			}
			else
			{
				tvi.hItem = OC->hTree;
				TreeView_SetItem( hNavig, &tvi );
			}
			objInsertChildren( hChild, OC->hTree, bForceCreate );
		}
	}
	return TRUE;
}

/****************************************************************************/
BOOL CALLBACK NavigDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
		RECT r;
		HIMAGELIST himl;
		hNavig = CreateWindowEx(WS_EX_TOOLWINDOW, WC_TREEVIEW, "*CRISP* Navigator",
				WS_VISIBLE | WS_CHILD | /*WS_BORDER |*/
				TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
				0, 0, 0, 0, hDlg, (HMENU)NAVIG_ID, hInst, NULL);
		regReadWindowPos( hDlg, "Navigator");
		GetClientRect( hDlg, &r );
		SetWindowPos( hNavig, NULL, 0,0, r.right, r.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		himl = ImageList_LoadImage( hInst, "SMALLICONS", 16, 100, RGB(0,255,0), IMAGE_BITMAP, LR_DEFAULTCOLOR );
		TreeView_SetImageList(hNavig, himl, TVSIL_NORMAL);
		DeleteObject(himl);
		}
		return TRUE;

	case WM_NCDESTROY:
		regWriteWindowPos( hDlg, "Navigator", TRUE);
		break;

	case WM_CLOSE:
		DestroyWindow( hDlg );
		hNavigDlg = NULL;
		hNavig = NULL;
		if (IsWindow(MainhWnd)) InitializeMenu(MainhWnd);
		break;

	case WM_SIZE:
		SetWindowPos( hNavig, NULL, 0,0, LOWORD(lParam), HIWORD(lParam), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		break;

	case WM_NOTIFY:
		{
		LPNM_TREEVIEW lpntv=(LPNM_TREEVIEW)lParam;

			switch (((LPNMHDR) lParam)->code) {
			case TVN_SELCHANGED:
				if (lpntv->itemNew.lParam) ActivateObj( (HWND)lpntv->itemNew.lParam ); break;
			default: return FALSE;
			}
			return TRUE;
		}
	}
	return FALSE;
}
/****************************************************************************/
HWND	objModifyNavig( void )
{
	BOOL	bForce = FALSE;

	if (hNavig==NULL || !IsWindow(hNavig))
	{
		hNavigDlg = CreateDialog( hInst, "NAVIG", MainhWnd, (DLGPROC)NavigDlgProc );
		bForce = TRUE;
	}
	BringWindowToTop( hNavigDlg );
	objInsertChildren( MDIhWnd, TVI_ROOT, bForce );
    return hNavig;
}
/****************************************************************************/
HWND	objFindParent( OBJ* O, int type )
{
	HWND	ParentWnd = NULL;

	while( 1 )
	{
		ParentWnd=O->ParentWnd;
		if (ParentWnd == NULL || !IsWindow(ParentWnd) || (O=OBJ::GetOBJ(ParentWnd))==NULL) { ParentWnd = NULL; break;}
		if (O->ID == type) break;
		if ((O->ID == OT_NONE) || (O->ParentWnd == NULL)) { ParentWnd = NULL; break; }
	}
	return ParentWnd;
}
/****************************************************************************/
void objInformChildren( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		OBJ*	pObj;
		HwndVecCit cit;

		bInformChilds = TRUE;
		if( NULL != (pObj = OBJ::GetOBJ(hWnd)) )
		{
			for(cit = pObj->m_vChildren.begin(); cit != pObj->m_vChildren.end(); ++cit)
			{
				if( (NULL != *cit) && (TRUE == ::IsWindow(*cit)) )
				{
					SendNotifyMessage( *cit, message, wParam, lParam );
				}
			}
		}
		bInformChilds = FALSE;
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("objInformChildren() -> %s"), e.what()));
	}
}

/****************************************************************************/
HMENU	objModifySysMenu( HWND hWnd )
{
	HMENU hm;
	OBJ*	O = OBJ::GetOBJ( hWnd );

	hm = GetSystemMenu( hWnd, 0 );	// Create a copy of sysytem menu
	DeleteMenu( hm, SC_MAXIMIZE, MF_BYCOMMAND);
	ModifyMenu( hm, SC_CLOSE,    MF_BYCOMMAND, SC_CLOSE,   "&Close");
	DeleteMenu( hm, SC_TASKLIST, MF_BYCOMMAND);
	switch( O->ID ) {
	case OT_IMG:
		break;
	case OT_AREA:
		InsertMenu( hm, UM1, MF_BYPOSITION | MF_SEPARATOR,  0, NULL);
		InsertMenu( hm, UM1, MF_BYPOSITION,  SC_myCIRCLE, "C&ircle");
		break;
	case OT_DIGI:
		InsertMenu( hm, UM1, MF_BYPOSITION | MF_SEPARATOR,  0, NULL);
		InsertMenu( hm, UM1, MF_BYPOSITION , SC_FSMALL, "Small font");
		InsertMenu( hm, UM1, MF_BYPOSITION , SC_FMEDIUM, "Medium font");
		InsertMenu( hm, UM1, MF_BYPOSITION , SC_FLARGE, "Large font");
		break;
	}
	O->sysmenu = hm;
	return hm;

}
/****************************************************************************/
// TODO: is bDestroyDMap really needed?
static void EliminateNullObjects(OBJ* pObj)
{
	HwndVecIt it;
	while( 1 )
	{
		it = find(pObj->m_vChildren.begin(), pObj->m_vChildren.end(), (HWND)NULL);
		if( it == pObj->m_vChildren.end() )
		{
			break;
		}
		pObj->m_vChildren.erase(it);
	}
}

void objFreeObj( HWND ohw, BOOL bDestroyDMap/*= FALSE*/ )
{
	OBJ*	pObj;
	OBJ* pParent;
	OBJ* pObj2;
	HwndVecIt it;

	try
	{
		ActivateObj( NULL );
		GetSystemMenu( ohw, TRUE ); // To destroy a copy of it
		pObj = OBJ::GetOBJ(ohw);
		if (pObj)
		{
			if (pObj->ParentWnd)
				pParent = OBJ::GetOBJ( pObj->ParentWnd );
			else
				return;
			if( pParent == NULL )
			{
				pObj->ParentWnd = MDIhWnd;
				if( (pParent = OBJ::GetOBJ( pObj->ParentWnd )) == NULL )
				{
					::OutputDebugString("MDIhWnd == NULL in objFreeObj().\n");
					return;
				}
			}
			// look through all the child windows
			for(it = pObj->m_vChildren.begin(); it != pObj->m_vChildren.end(); ++it)
			{
				if( (NULL == *it) || (FALSE == ::IsWindow(*it)) )
				{
					continue;
				}
				if( NULL == (pObj2 = OBJ::GetOBJ(*it)) )
				{
					continue;
				}
				// check if the object is Density map
				if( OT_DMAP == pObj2->ID )
				{
					// set Main Window as the parent window
					pObj2->ParentWnd = MDIhWnd;
					// move it to the root of GPO and break without destroying it
					::GPO.m_vChildren.push_back(pObj2->hWnd);
					// delete item from the Navigator window
					if( hNavig )
					{
						TreeView_DeleteItem( hNavig, pObj2->hTree );
						// Get the handle in the navigator tree
						pObj2->hTree = NULL;
						// modify NavigatorTree
//						objModifyNavig();
						// TODO: insert item???
					}
					// mark it for delete
					*it = NULL;
				}
				else
					DestroyWindow( *it );
			}
			// delete all NULL-marked elements
			EliminateNullObjects(pObj);
			if( (it = pParent->FindByWindow(ohw)) != pParent->m_vChildren.end() )
			{
				// show that we just marked this element for deleting
				*it = NULL;
			}
			if( pParent == &::GPO )
			{
				EliminateNullObjects(&::GPO);
			}
			if (pObj->hTree && hNavig && IsWindow(hNavig))
			{
				TreeView_DeleteItem( hNavig, pObj->hTree );
				objModifyNavig();
			}
			if (pObj->ParentWnd != MDIhWnd)
			{
				ActivateObj(pObj->ParentWnd);
			}
			else
			{
				for(it = pObj->m_vChildren.begin(); it != pObj->m_vChildren.end(); ++it)
				{
					if( (NULL != *it) && (TRUE == ::IsWindow(*it)) )
					{
						ActivateObj( *it );
						break;
					}
				}
			}
			// now free all the data
			delete pObj;
		}
		OBJ::SetOBJ(ohw, NULL);
	}
	catch(std::exception& e)
	{
		::MessageBox(NULL, GetUserErrorAndTrace(_FMT(_T("objFreeObj() -> %s"), e.what())).c_str(), "ERROR", MB_OK);
	}
}
/****************************************************************************/
static BOOL	objSaveObject( void )
{
	SAVEIMAGE Img;
	OBJ*	pObj;
	LPSTR	lpWinName;
	_tstd::string defaultFilename;
	int		iFileType;

	if( NULL == hActive )
		return FALSE;
	if( NULL == (pObj = OBJ::GetOBJ( hActive )) )
		return FALSE;

	memset( &Img, 0, sizeof(SAVEIMAGE) );

	switch( pObj->ID )
	{
	case OT_IMG:
	case OT_FFT:
	case OT_DMAP:
	case OT_IFFT:
		defaultFilename = pObj->title;
		_splitpath( pObj->fname, NULL, NULL, TMP, NULL );	// take file name only
		defaultFilename += _T(" of ");
		defaultFilename += TMP;
		defaultFilename += _T(".jpg");
		lpWinName = "Save Image File";
		iFileType = OFN_IMAGEW;
		Img.x = pObj->x;
		Img.y = pObj->y;
#ifdef USE_XB_LENGTH
		Img.xb = pObj->xb;
#else
		Img.stride = pObj->stride;
#endif
		Img.ps = IMAGE::PixFmt2Bpp(pObj->npix);
		Img._pix = pObj->npix;
		Img.lpData = pObj->dp;
		break;
		// ADDED by Peter on 6 Aug 2003
	case OT_DIGI:
		SendMessage(hActive, IDM_F_SAVE, 0, 0);
		return FALSE;
		break;
		// END ADD
		// ADDED by Peter on 21 Aug 2003
	case OT_ONED:
		SendMessage(hActive, IDM_F_SAVE, 0, 0);
		return FALSE;
		break;
		// END ADD
	case OT_AREA:
		defaultFilename = _T("Area of ");
		defaultFilename += pObj->title;
		defaultFilename += _T(".jpg");
		lpWinName = "Save Image File";
		iFileType = OFN_IMAGEW;
		Img.x = __min(pObj->xas, pObj->x-pObj->xa);
		Img.y = __min(pObj->yas, pObj->y-pObj->ya);
		Img.ps = IMAGE::PixFmt2Bpp(pObj->npix);
		Img._pix = pObj->npix;
#ifdef USE_XB_LENGTH
		Img.xb = pObj->xb;
		Img.lpData = pObj->dp+(pObj->xa+pObj->xb*pObj->ya)*IMAGE::PixFmt2Bpp(pObj->npix);
#else
		Img.stride = pObj->stride;
		Img.lpData = pObj->dp + (pObj->xa + pObj->stride*pObj->ya);
#endif
		break;
	default: return FALSE;
	}

	if (!fioGetFileNameDialog( MainhWnd, lpWinName, &ofNames[iFileType], defaultFilename.c_str(), TRUE))
		return FALSE;
	Img.lpAdd = &pObj->NI;
	Img.iAddSize = sizeof(NEWINFO);
	fioSaveImageFile(fioFileName(), &Img, 100);
	strcpy( pObj->title, fioFileTitle() );
	strcpy( pObj->fname, fioFileName() );
	wndSetZoom( hActive, pObj->scu, pObj->scd );
	return TRUE;
}
/****************************************************************************/
BOOL CALLBACK TextSaveParamsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPSAVEIMAGE	S = (LPSAVEIMAGE)GetWindowLong( hDlg, DWL_USER );
	static int x0=0, x1=10, y0=0, y1=10;

	switch (message)
	{
		case WM_INITDIALOG:
			S = (LPSAVEIMAGE)lParam;
			SetWindowLong( hDlg, DWL_USER, lParam);
			SetDlgItemInt( hDlg, IDC_TSP_X0, x0, FALSE);
			SetDlgItemInt( hDlg, IDC_TSP_X1, x1, FALSE);
			SetDlgItemInt( hDlg, IDC_TSP_Y0, y0, FALSE);
			SetDlgItemInt( hDlg, IDC_TSP_Y1, y1, FALSE);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					{ int a, f;
					a = GetDlgItemInt( hDlg, IDC_TSP_X0, &f, FALSE ); if (f) x0 = S->x0 = a;
					a = GetDlgItemInt( hDlg, IDC_TSP_X1, &f, FALSE ); if (f) x1 = S->cx = a;
					a = GetDlgItemInt( hDlg, IDC_TSP_Y0, &f, FALSE ); if (f) y0 = S->y0 = a;
					a = GetDlgItemInt( hDlg, IDC_TSP_Y1, &f, FALSE ); if (f) y1 = S->cy = a;
					S->bSigned = IsDlgButtonChecked( hDlg, IDC_TSP_BINARY );
					EndDialog( hDlg, TRUE );
					}
					return TRUE;

				case IDCANCEL:
					EndDialog( hDlg, FALSE );
					return TRUE;
			}
			break;
	}
	return FALSE;
}

/****************************************************************************/
static BOOL	objSaveTextObject( void )
{
	SAVEIMAGE S;
	OBJ*	O;
	LPSTR	lpWinName, defaultFilename;
	int		iFileType, x, y;
	FILE * f;
	LPBYTE lpb;
#ifdef USE_XB_LENGTH
	LPWORD lpw;
	LPDWORD lpdw;
	float *lpf;
	double *lpd;
#endif
	char	fname[_MAX_PATH];

	if (hActive==NULL) return FALSE;
	if ((O = OBJ::GetOBJ( hActive ))==NULL) return FALSE;

	memset( &S, 0, sizeof(SAVEIMAGE) );

	switch( O->ID )
	{
	case OT_IMG:
		defaultFilename = O->title;
		lpWinName = "Save Image File Text/binary";
		S.x = O->x;
		S.y = O->y;
#ifdef USE_XB_LENGTH
		S.xb = O->xb;
#else
		S.stride = O->stride;
#endif
		S.ps = IMAGE::PixFmt2Bpp(O->npix);
		S._pix = O->npix;
		S.lpData = O->dp;
		break;
		// ADDED by Peter on 6 Aug 2003
	case OT_DIGI:
		SendMessage(hActive, IDM_F_SSAVE, 0, 0);
		return FALSE;
		break;
		// END ADD
	default: return FALSE;
	}
	if (! DialogBoxParam(hInst, "TextSaveParams", NULL, TextSaveParamsDlgProc, (LPARAM)&S))
		return FALSE;
	if (S.cx <= S.x0 || S.cy <= S.y0 )
	{
		MessageBox( MainhWnd, "Invalid parameters", szProg, MB_ICONSTOP );
		return FALSE;
	}

	strcpy( fname, O->title );
	if (strrchr(fname,'.')) *strrchr(fname,'.') = 0;

	strcat( fname, S.bSigned ? ".bin" : ".lst" );
	iFileType = S.bSigned ? OFN_BINARY : OFN_LIST;

	if (!fioGetFileNameDialog( MainhWnd, lpWinName, &ofNames[iFileType], fname, TRUE))
		return FALSE;

	lpb = (LPBYTE)S.lpData;
#ifdef USE_XB_LENGTH
	lpw = (LPWORD)S.lpData;
	lpdw = (LPDWORD)S.lpData;
	lpf = (float*)S.lpData;
	lpd = (double*)S.lpData;
#endif

	if (!S.bSigned)
	{
		// Text Mode
		if( NULL == (f = fopen(fioFileName(), "w")) ) return FALSE;
		for(y=S.y0; y < S.cy; y++)
		{
			for(x=S.x0; x < S.cx; x++)
			{
				switch( O->npix )
				{
#ifdef USE_XB_LENGTH
				case PIX_BYTE: fprintf( f, "%3d ", lpb[y*O->xb + x]); break;
				case PIX_CHAR: fprintf( f, "%3d ", lpb[y*O->xb + x]); break;
				case PIX_WORD: fprintf( f, "%5d ", lpw[y*O->xb + x]); break;
				case PIX_SHORT: fprintf( f, "%5d ", lpw[y*O->xb + x]); break;
				case PIX_DWORD: fprintf( f, "%10d ", lpdw[y*O->xb + x]); break;
				case PIX_LONG: fprintf( f, "%10d ", lpdw[y*O->xb + x]); break;
				case PIX_FLOAT: fprintf( f, "%g ", lpf[y*O->xb + x]); break;
				case PIX_DOUBLE: fprintf( f, "%g ", lpd[y*O->xb + x]); break;
#else
				case PIX_BYTE: fprintf( f, "%3d ", *(LPBYTE)OBJ::GetDataPtr(lpb, O->npix, x, O->stride, y)); break;
				case PIX_CHAR: fprintf( f, "%3d ", *(LPSTR)OBJ::GetDataPtr(lpb, O->npix, x, O->stride, y)); break;
				case PIX_WORD: fprintf( f, "%5d ", *(LPWORD)OBJ::GetDataPtr(lpb, O->npix, x, O->stride, y)); break;
				case PIX_SHORT: fprintf( f, "%5d ", *(SHORT*)OBJ::GetDataPtr(lpb, O->npix, x, O->stride, y)); break;
				case PIX_DWORD: fprintf( f, "%10d ", *(LPDWORD)OBJ::GetDataPtr(lpb, O->npix, x, O->stride, y)); break;
				case PIX_LONG: fprintf( f, "%10d ", *(LPLONG)OBJ::GetDataPtr(lpb, O->npix, x, O->stride, y)); break;
				case PIX_FLOAT: fprintf( f, "%g ", *(LPFLOAT)OBJ::GetDataPtr(lpb, O->npix, x, O->stride, y)); break;
				case PIX_DOUBLE: fprintf( f, "%g ", *(LPDOUBLE)OBJ::GetDataPtr(lpb, O->npix, x, O->stride, y)); break;
#endif
				}
			}
			fprintf( f, "\n");
		}
	}
	else
	{
		// Binary Mode
		if( NULL == (f = fopen(fioFileName(), "wb")) ) return FALSE;
		for(y=S.y0; y < S.cy; y++)
		{
			switch( O->npix )
			{
#ifdef USE_XB_LENGTH
			case PIX_BYTE: fwrite( &lpb[y*O->xb], 1, S.cx - S.x0, f); break;
			case PIX_CHAR: fwrite( &lpb[y*O->xb], 1, S.cx - S.x0, f); break;
			case PIX_WORD: fwrite( &lpw[y*O->xb], 2, S.cx - S.x0, f); break;
			case PIX_SHORT: fwrite( &lpw[y*O->xb], 2, S.cx - S.x0, f); break;
			case PIX_DWORD: fwrite( &lpdw[y*O->xb], 4, S.cx - S.x0, f); break;
			case PIX_LONG: fwrite( &lpdw[y*O->xb], 4, S.cx - S.x0, f); break;
			case PIX_FLOAT: fwrite( &lpf[y*O->xb], 4, S.cx - S.x0, f); break;
			case PIX_DOUBLE: fwrite( &lpd[y*O->xb], 8, S.cx - S.x0, f); break;
#else
			case PIX_BYTE: fwrite( OBJ::GetDataPtr(lpb, O->npix, 0, O->stride, y), 1, S.cx - S.x0, f); break;
			case PIX_CHAR: fwrite( OBJ::GetDataPtr(lpb, O->npix, 0, O->stride, y), 1, S.cx - S.x0, f); break;
			case PIX_WORD: fwrite( OBJ::GetDataPtr(lpb, O->npix, 0, O->stride, y), 2, S.cx - S.x0, f); break;
			case PIX_SHORT: fwrite( OBJ::GetDataPtr(lpb, O->npix, 0, O->stride, y), 2, S.cx - S.x0, f); break;
			case PIX_DWORD: fwrite( OBJ::GetDataPtr(lpb, O->npix, 0, O->stride, y), 4, S.cx - S.x0, f); break;
			case PIX_LONG: fwrite( OBJ::GetDataPtr(lpb, O->npix, 0, O->stride, y), 4, S.cx - S.x0, f); break;
			case PIX_FLOAT: fwrite( OBJ::GetDataPtr(lpb, O->npix, 0, O->stride, y), 4, S.cx - S.x0, f); break;
			case PIX_DOUBLE: fwrite( OBJ::GetDataPtr(lpb, O->npix, 0, O->stride, y), 8, S.cx - S.x0, f); break;
#endif
			}
		}
	}

	fclose(f);

	return TRUE;
}

/****************************************************************************/
static BOOL objPrintObject( void )
{
	OBJ*	O;
	int		xs, ys, ps, srcw;
	LPBYTE	lpSrc;
	BOOL	bInv;

	if (hActive==NULL) return FALSE;
	if ((O = OBJ::GetOBJ( hActive ))==NULL) return FALSE;

	xs = O->x;
	ys = O->y;
#ifdef USE_XB_LENGTH
	srcw = O->xb;
#else
	// TODO: modify!
	srcw = R4(O->x);
#endif
	ps = IMAGE::PixFmt2Bpp(O->npix);
	lpSrc = O->dp;
	switch( O->ID )
	{
	case OT_IMG:  /*xs = O->x;  ys = O->y;  srcw = O->xb; ps = O->pix; lpSrc = O->dp;*/ bInv = FALSE; break;
	case OT_FFT:  /*xs = O->x;  ys = O->y;  srcw = O->xb; ps = O->pix; lpSrc = O->dp;*/ bInv = TRUE;  break;
	case OT_DMAP: /*xs = O->x;  ys = O->y;  srcw = O->xb; ps = O->pix; lpSrc = O->dp;*/ bInv = FALSE; break;
	default: return FALSE;
	}

	if (prnPageSetup( MainhWnd, bInv ))
		prnPrintImage( lpSrc, xs, ys, srcw, O->fname, IMAGE::PixFmt2Bpp(O->npix) );
	return TRUE;
}

/****************************************************************************/
static BOOL objCopyToCB( void )
{
	OBJ*	pObj;
	HANDLE	h;
	LPBITMAPINFOHEADER lpBM;
	LPRGBQUAD lpRGBQ;
	int		i, xs, ys, ps, srcw, dstw;
	int		xcs, ycs;
	LPBYTE	lpDst, lpSrc;

	if( (NULL == hActive) || (NULL == (pObj = OBJ::GetOBJ(hActive))) ) return FALSE;

	switch( pObj->ID )
	{
	case OT_IMG:
	case OT_FFT:
	case OT_DMAP:
	case OT_IFFT:
		xs = pObj->x;
		ys = pObj->y;
		xcs = pObj->x;
		ycs = pObj->y;
#ifdef USE_XB_LENGTH
		srcw = pObj->xb;
#else
		srcw = pObj->stride;
#endif
		ps = IMAGE::PixFmt2Bpp(pObj->npix);
		lpSrc = pObj->dp;
		break;
	case OT_AREA:
		xs = pObj->xas;
		ys = pObj->yas;
		xcs = __min(pObj->xas, pObj->x-pObj->xa);
		ycs = __min(pObj->yas, pObj->y-pObj->ya);
		ps = IMAGE::PixFmt2Bpp(pObj->npix);
#ifdef USE_XB_LENGTH
		srcw = pObj->xb;
		lpSrc = pObj->dp+(pObj->xa+pObj->xb*pObj->ya)*IMAGE::PixFmt2Bpp(pObj->npix);
#else
		srcw = pObj->stride;
		lpSrc = (LPBYTE)pObj->GetDataPtr(pObj->xa, pObj->ya);//pObj->dp+(pObj->xa+pObj->xb*pObj->ya)*IMAGE::PixFmt2Bpp(pObj->npix);
#endif
		break;
	default: return FALSE;
	}

/*
 	dstw = R4(xs);
	h = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, dstw * ys * ps + sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));
	if( NULL == h )
		return FALSE;
	lpBM = (LPBITMAPINFOHEADER)GlobalLock(h);
	if( NULL == lpBM )
	{
		GlobalFree( h );
		return FALSE;
	}
	memset( lpBM, 1, dstw * ys * ps + sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD) );
	lpBM->biSize = sizeof(BITMAPINFOHEADER);
	lpBM->biWidth = xs;
	lpBM->biHeight = ys;
	lpBM->biSizeImage = 0; //dstw*ys*ps;
	lpBM->biPlanes = 1;
	lpBM->biBitCount = 8*ps;
	lpBM->biCompression = BI_RGB;
	lpBM->biClrUsed = lpBM->biClrImportant = 256;
	lpBM->biXPelsPerMeter = lpBM->biYPelsPerMeter = 0;
	lpRGBQ = (LPRGBQUAD) ((LPSTR)lpBM + sizeof(BITMAPINFOHEADER));
	for(i = 0; i < 256; i++)
	{
		lpRGBQ[i].rgbRed = lpRGBQ[i].rgbGreen = lpRGBQ[i].rgbBlue = i;
	}

	lpDst = (LPBYTE)(lpRGBQ + 256) + ((ys-1)*dstw*ps);
	for(i = 0; i < ycs; i++)
	{
		memcpy( lpDst, lpSrc, xcs*ps );
		lpDst -= dstw*ps;
#ifdef USE_XB_LENGTH
		lpSrc += srcw*ps;
#else
		lpSrc += srcw;
#endif
	}
/*/
	dstw = R4(xs * ps);
	h = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, dstw * ys + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));
	if( NULL == h )
		return FALSE;
	lpBM = (LPBITMAPINFOHEADER)GlobalLock(h);
	if( NULL == lpBM )
	{
		GlobalFree( h );
		return FALSE;
	}
	memset( lpBM, 1, dstw * ys + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD) );
	lpBM->biSize = sizeof(BITMAPINFOHEADER);
	lpBM->biWidth = xs;
	lpBM->biHeight = ys;
	lpBM->biSizeImage = 0;
	lpBM->biPlanes = 1;
	//lpBM->biBitCount = 8*ps; // <-- see below
	lpBM->biCompression = BI_RGB;
	lpBM->biClrUsed = lpBM->biClrImportant = 256;
	lpBM->biXPelsPerMeter = lpBM->biYPelsPerMeter = 0;
	lpRGBQ = (LPRGBQUAD) ((LPSTR)lpBM + sizeof(BITMAPINFOHEADER));
	for(i = 0; i < 256; i++)
	{
		lpRGBQ[i].rgbRed = lpRGBQ[i].rgbGreen = lpRGBQ[i].rgbBlue = i;
	}

	lpDst = (LPBYTE)(lpRGBQ + 256) + ((ys-1)*dstw);
	for(i = 0; i < ycs; i++)
	{
		memcpy( lpDst, lpSrc, xcs*ps );
		lpDst -= dstw;
#ifdef USE_XB_LENGTH
		lpSrc += srcw*ps;
#else
		lpSrc += srcw;
#endif
	}
	GlobalUnlock( h );
	if (OpenClipboard( MainhWnd ))
	{
		EmptyClipboard();
		UINT format = CF_DIB;
		// 0x8000 mask for BITCOUNT means signed pixel format, the bit count can be obtained by & 0xFF
		switch( pObj->npix )
		{
		case PIX_CHAR: 		lpBM->biBitCount = (WORD) 0x8108; format = g_Clipboard_8s;  break;
		case PIX_BYTE: 		lpBM->biBitCount = (WORD) 0x0108; format = CF_DIB;          break;
		case PIX_SHORT: 	lpBM->biBitCount = (WORD) 0x8110; format = g_Clipboard_16s; break;
		case PIX_WORD: 		lpBM->biBitCount = (WORD) 0x0110; format = g_Clipboard_16u; break;
		case PIX_LONG: 		lpBM->biBitCount = (WORD) 0x8120; format = g_Clipboard_32s; break;
		case PIX_DWORD: 	lpBM->biBitCount = (WORD) 0x0120; format = g_Clipboard_32u; break;
		case PIX_FLOAT: 	lpBM->biBitCount = (WORD) 0x0220; format = g_Clipboard_32f; break;
		case PIX_DOUBLE: 	lpBM->biBitCount = (WORD) 0x0240; format = g_Clipboard_64f; break;
		}
		SetClipboardData( format, h );
		CloseClipboard();
	}
	else
	{
		GlobalFree( h );
	}
	return TRUE;
}

/****************************************************************************/
static BOOL objCopyChildToCB( void  )
{
	HDC		shdc, hdc;
	HBITMAP	hbmp;
	RECT	r;
	int		xs, ys;

	if (hActive == NULL) return FALSE;
	shdc = GetWindowDC( hActive  );
	hdc = CreateCompatibleDC( shdc );
	GetWindowRect(hActive, &r);
	xs = r.right - r.left; ys = r.bottom - r.top;
	hbmp = CreateCompatibleBitmap( shdc, xs, ys);
	if ( hbmp ) {
		SelectObject( hdc, hbmp );
		BitBlt( hdc, 0, 0, xs, ys, shdc, 0, 0, SRCCOPY);
	}
	DeleteDC( hdc );
	ReleaseDC( hActive, shdc );

	if (OpenClipboard( MainhWnd )) {
		EmptyClipboard();
		if ( hbmp ) SetClipboardData( CF_BITMAP, hbmp );
		CloseClipboard();
	} else {
		if (hbmp) DeleteObject( hbmp );
	}
	return TRUE;
}

/****************************************************************************/
static BOOL objPasteFromCB( OBJ* O )
{
	HANDLE		hDIB = NULL;
	LPBITMAPINFOHEADER lpBM=NULL;
	LPRGBQUAD	lpRGBQ=NULL;
	LPBYTE		lpBIT=NULL, s, d;
	LPWORD		sw;
	LPDWORD		clrmasks;
	BYTE		yylat[256];
	int			ncol, j;
	BOOL		bdib;	// conversion required
	BOOL		b15bits = FALSE;

	O->NI = NIDefault;
	O->ID = OT_IMG;
	O->scu = 1;
	O->scd = 1;
	O->maxcontr = TRUE;

	UINT aFormats[] = { CF_DIB, g_Clipboard_8s, g_Clipboard_16u, g_Clipboard_16s, g_Clipboard_32u, g_Clipboard_32s, g_Clipboard_32f, g_Clipboard_64f, };

	if( OpenClipboard(GetFocus()) )
	{
		bdib = FALSE;
		for(int i = 0; i < sizeof(aFormats)/sizeof(aFormats[0]); i++)
		{
			UINT nFormat = aFormats[i];
			if( IsClipboardFormatAvailable(nFormat) )
			{
				hDIB = GetClipboardData(nFormat);
				bdib = (CF_DIB == nFormat) ? TRUE : FALSE;
				break;
			}
		}
		CloseClipboard();
		if( NULL != hDIB )
		{
			// DIB processing
			lpBM = (LPBITMAPINFOHEADER) GlobalLock(hDIB);
			lpRGBQ = (LPRGBQUAD) ((LPSTR)lpBM + sizeof(BITMAPINFOHEADER));
			ncol = lpBM->biClrUsed;
			if (ncol==0 && bdib && lpBM->biBitCount <= 8)
			{
					ncol = 1<<lpBM->biBitCount;
			}
			if( BI_BITFIELDS == lpBM->biCompression )
			{
				lpBIT = (LPBYTE)(lpRGBQ) + 4*3;
				clrmasks = (LPDWORD)(lpRGBQ);
				if (clrmasks[2] == 0x000001F &&
					clrmasks[1] == 0x00003E0 &&
					clrmasks[0] == 0x0007C00)
						b15bits = TRUE;
			}
			else
			{
				lpBIT = (LPBYTE)(lpRGBQ + ncol);
			}

			for(int i=0;i<ncol;i++)
				yylat[i] = (BYTE) (lpRGBQ[i].rgbRed*0.287 + lpRGBQ[i].rgbGreen*0.599 + lpRGBQ[i].rgbBlue*0.114+0.5);
			O->x = lpBM->biWidth;
			O->y = lpBM->biHeight;

			int nBPP = 1;
			O->npix = PIX_BYTE;
			if( FALSE == bdib )
			{
				nBPP = __max(1, (lpBM->biBitCount & 0xFF) >> 3);
				switch( lpBM->biBitCount )
				{
				case 0x8108: O->npix = PIX_CHAR;	break;
				case 0x0108: O->npix = PIX_BYTE;	break;
				case 0x8110: O->npix = PIX_SHORT;	break;
				case 0x0110: O->npix = PIX_WORD;	break;
				case 0x8120: O->npix = PIX_LONG;	break;
				case 0x0120: O->npix = PIX_DWORD;	break;
				case 0x0220: O->npix = PIX_FLOAT;	break;
				case 0x0240: O->npix = PIX_DOUBLE;	break;
				}
			}

			O->xas = O->x;
			O->yas = O->y;
			// the stride
			O->stride = R4(O->x * nBPP);
			// allocate correct amount of data
			O->dp = (LPBYTE)malloc(O->stride * O->y);

			if( NULL == O->dp ) return FALSE;

			s = lpBIT;
			d = (LPBYTE)O->GetDataPtr(0, O->y - 1);

			switch( lpBM->biBitCount )
			{
			case 32:
				// 32 bit RGBA
				if( bdib )
				{
					for( int i=0; i<O->y; i++)
					{
						for( j=0; j<O->x; j++)
						{
							d[j] = (BYTE)((s[j*4]*0.114) + (s[j*4+1]*0.599) + (s[j*4+2]*0.287) );
						}
#ifdef USE_XB_LENGTH
						d -= O->xb*nBPP;
#else
						d -= O->stride;
#endif
						s += O->x*4;
					}
				}
				break;
			case 0x8108:
			case 0x8110:
			case 0x0110:
			case 0x8120:
			case 0x0120:
			case 0x0220:
			case 0x0240:
				{
					for(int i = 0; i < O->y; i++)
					{
						memcpy( d, s, O->x*nBPP);
						d -= O->stride;
						s += R4(O->x*nBPP);
					}
				}
				break;
			case 24:
				// 24 bit RGB
				if (bdib)
				{
					for( int i=0; i<O->y; i++) {
						for( j=0; j<O->x; j++) {
							d[j] = (BYTE)((s[j*3]*0.114) + (s[j*3+1]*0.599) + (s[j*3+2]*0.287) );
						}
#ifdef USE_XB_LENGTH
						d -= O->xb*nBPP;
#else
						d -= O->stride;
#endif
						s += R4(O->x*3);
					}
				}
				break;
			case 15:
				// 15 bit RGB
				if (bdib)
				{
					for( int i=0; i<O->y; i++)
					{
						sw = (LPWORD)s;
						for( j=0; j<O->x; j++)
						{
							d[j] = (BYTE)((((sw[j] & 0x1F)*0.114) + (((sw[j]>>5) & 0x1F)*0.599) + (((sw[j]>>10) & 0x1F)*0.287) ) * 8 );
						}
#ifdef USE_XB_LENGTH
						d -= O->xb*nBPP;
#else
						d -= O->stride;
#endif
						s += R4(O->x*2);
					}
				}
				break;

			case 16:
				if (bdib)
				{
					for(int i=0; i<O->y; i++) {
						sw = (LPWORD)s;
						for( j=0; j<O->x; j++) {
							if (b15bits)
								d[j] = (BYTE)((((sw[j] & 0x1F)*0.114) + (((sw[j]>>5) & 0x1F)*0.599) + (((sw[j]>>10) & 0x1F)*0.287) ) * 8 );
							else
								d[j] = (BYTE)((((sw[j] & 0x1F)*0.114) + (((sw[j]>>6) & 0x1F)*0.599) + (((sw[j]>>11) & 0x1F)*0.287) ) * 8 );
						}
#ifdef USE_XB_LENGTH
						d -= O->xb*nBPP;
#else
						d -= O->stride;
#endif
						s += R4(O->x*2);
					}
				}
				break;
			case 8:
			case 0x0108:
				{
					for(int i=0; i<O->y; i++)
					{
						im8Xlat( d, s, 0, yylat, O->x, 1);
#ifdef USE_XB_LENGTH
						d -= O->xb;
#else
						d -= O->stride;
#endif
						s += R4(O->x);
					}
				}
				break;
			case 4:
				{
					for(int i=0; i<O->y; i++)
					{
	//				im4Xlat( d, s, yylat, O->x);	// not implemented yet
#ifdef USE_XB_LENGTH
						d -= O->xb;
#else
						d -= O->stride;
#endif
						s += R4((O->x+1)/4);
					}
				}
			}
			if (hDIB) GlobalUnlock(hDIB);
		}
		else if( IsClipboardFormatAvailable(CF_BITMAP) )
		{
			// BITMAP processing
			HDC hdc;
			HPALETTE hop, hPAL;
			BITMAP	bmp;
			HBITMAP hBMP;

			hBMP = (HBITMAP)GetClipboardData( CF_BITMAP );
			if (IsClipboardFormatAvailable (CF_PALETTE))
				hPAL = (HPALETTE) GetClipboardData (CF_PALETTE);
			else
				hPAL = (HPALETTE) GetStockObject (DEFAULT_PALETTE);
			CloseClipboard ();
			GetObject( hBMP, sizeof(BITMAP), &bmp);
			O->x = bmp.bmWidth;
			O->y = bmp.bmHeight;
			O->npix = PIX_BYTE;
			O->xas = O->x;
			O->yas = O->y;
#ifdef USE_XB_LENGTH
			O->xb = R4(O->x);
			O->dp = (LPBYTE)malloc(O->xb * O->y * IMAGE::PixFmt2Bpp(O->npix));
#else
			O->stride = R4(O->x * IMAGE::PixFmt2Bpp(O->npix));
			O->dp = (LPBYTE)malloc(O->stride * O->y);
#endif
			if( NULL == O->dp )
				return FALSE;

			hdc = GetDC( NULL );

			// Memory for BMP header, palette, bits
#ifdef USE_XB_LENGTH
			lpBM = (LPBITMAPINFOHEADER)malloc(O->xb*O->y + sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));
#else
			lpBM = (LPBITMAPINFOHEADER)malloc(O->stride*O->y + sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));
#endif
			// Init it
			lpBM->biSize = sizeof(BITMAPINFOHEADER);
			lpBM->biWidth = O->x;
			lpBM->biHeight = O->y;
			lpBM->biSizeImage = O->x*O->y;
			lpBM->biPlanes = 1;
			lpBM->biBitCount = 8;
			lpBM->biCompression = BI_RGB;
			lpBM->biClrUsed = lpBM->biClrImportant = 256;
			lpBM->biXPelsPerMeter = lpBM->biYPelsPerMeter = 0L;
			lpRGBQ = (LPRGBQUAD) ((LPSTR)lpBM + sizeof(BITMAPINFOHEADER));
			lpBIT = (LPBYTE)(lpRGBQ + 256);

			// Select given palette
			hop = SelectPalette (hdc, hPAL, FALSE);
			RealizePalette (hdc);
			// Finally get bits
			GetDIBits( hdc, hBMP, 0, O->y, lpBIT, (LPBITMAPINFO)lpBM, DIB_RGB_COLORS);
			if (hop) SelectPalette (hdc, hop, FALSE);
			// XLAT table for these bits
			for(int i=0;i<256;i++)
				yylat[i] = (BYTE) (lpRGBQ[i].rgbRed*0.287 + lpRGBQ[i].rgbGreen*0.599 + lpRGBQ[i].rgbBlue*0.114+0.5);
			// xlat it
			s = lpBIT;
#ifdef USE_XB_LENGTH
			d = O->dp + O->xb*(O->y-1);
#else
			d = (LPBYTE)O->GetDataPtr(0, O->y-1);
#endif
			{
				for(int i=0; i<O->y; i++)
				{
					im8Xlat( d, s, 0, yylat, O->x, 1);
#ifdef USE_XB_LENGTH
					d -= R4(O->x);
#else
					d -= O->stride;
#endif
					s += R4(O->x);
				}
			}
			// Free everything
			free( lpBM );
			ReleaseDC( NULL, hdc );
		}
		else
		{
			return FALSE;
		}
		// Neither DIB nor BITMAP is available
	}

	O->imageserial = iImageSerial++;
	sprintf( O->title, "Clipboard paste #%d", O->imageserial );
	strcpy( O->fname, "" );
	imgCalcMinMax( O );
	return TRUE;
}
/****************************************************************************/
// Added by Peter on 10 Nov 2005
static BOOL CRISP_OpenFile(OBJ* pObj, int nOpenType = OFN_IMAGER)
{
	// NOTE: don't use this function directly, instead use _OpenShellFile
	if( !fioGetFileNameDialog(MainhWnd, "Open Image File",
		&ofNames[nOpenType], "", FALSE, bDemo?2:1) )
	{
		return FALSE;
	}
	strcpy( pObj->title, fioFileTitle() );
	strcpy( pObj->fname, fioFileName() );
	strcpy( pObj->extra, pObj->fname );
	return TRUE;
}
/****************************************************************************/
// Added by Peter on 10 Nov 2005

OBJ* CRISP_CreateObject(OBJ* pParent, OBJ* OE, BOOL &bExternalObject, HWND hParent)
{
	OBJ* pObj = NULL;

	if( NULL == (pObj = new OBJ/*(OBJ*)calloc(1, sizeof( OBJ ))*/) )
		return NULL;
	if( OE == NULL )
	{
		*pObj = *pParent;
		pObj->bright = 128;
		pObj->contrast = 128;
		pObj->palno = -1;
		bExternalObject = FALSE;
	}
	else
	{
		*pObj = *OE;
		bExternalObject = TRUE;
	}
	// no children - they were copied actually
	pObj->m_vChildren.clear();

	pObj->hDlg = NULL;
	pObj->hTree = NULL;
	pObj->hWnd = NULL;
	pObj->sysmenu = NULL;
	pObj->ParentWnd = hParent;
	pObj->pfnPreActivate = NULL;

	return pObj;
}
/****************************************************************************/
// Added by Peter on 10 Nov 2005
BOOL CRISP_InitObjFromImage(OBJ* pObj, SAVEIMAGE &S)
{
	if( NULL == pObj )
	{
		return FALSE;
	}
//	if( (NULL != S.lpAdd) && (sizeof(NEWINFO) == S.iAddSize) )
//	{
//		memcpy(&pObj->NI, S.lpAdd, S.iAddSize);
//	}
//	else
//	{
//		pObj->NI = NIDefault;
//	}
	pObj->ID = OT_IMG;
	pObj->x = S.x;
	pObj->y = S.y;
	pObj->xas = S.x;
	pObj->yas = S.y;
#ifdef USE_XB_LENGTH
	pObj->xb = S.xb;
#else
	pObj->stride = S.stride;
#endif
	pObj->npix = S._pix;
	pObj->dp = S.lpData;
	pObj->scu = 1;
	pObj->scd = 1;
	pObj->maxcontr = TRUE;
	imgCalcMinMax( pObj );
	return TRUE;
}
/****************************************************************************/
// Added by Peter on 10 Nov 2005
BOOL CRISP_ReadImage(OBJ* pObj, vObjectsVec &vObjects)
{
	SAVEIMAGE S;
	HCURSOR	hoc;

	if( NULL == pObj )
	{
		return FALSE;
	}

	memset( &S, 0, sizeof(SAVEIMAGE) );
	S.lpAdd = &pObj->NI;
	S.iAddSize = sizeof(NEWINFO);
	hoc = SetCursor( hcWait );
	pObj->NI = NIDefault;
	if( fioOpenImageFile(pObj->fname, pObj, &S, vObjects) )
	{
/*
		pObj->ID = OT_IMG;
		pObj->x = S.x;
		pObj->y = S.y;
		pObj->xas = S.x;
		pObj->yas = S.y;
		pObj->xb = S.xb;
		pObj->pix = S.ps;
		pObj->dp = S.lpData;
		pObj->scu = 1;
		pObj->scd = 1;
		pObj->maxcontr = TRUE;
		imgCalcMinMax( pObj );
/*
		x0 = rp.left + xyoffs;
		y0 = rp.top + xyoffs;
*/
		SetCursor( hoc );
	}
	else
	{
		SetCursor( hoc );
		return FALSE;
		/*goto errexit*/;
	}
	return TRUE;
}
/****************************************************************************/
HWND objCreateObject( UINT uType, OBJ* OE, pfnCREATEOBJ_CALLBACK pfnPreCreate/* = NULL*/,
					 LPVOID pExtraData/* = NULL*/ )
{
	HWND	chi;
	OBJ*	O;
	OBJ* P;
	OBJ* PP;
	int		i, pfs, xyoffs, x0, y0;
	HWND	hParent, hPosPar, hImgPar;
	RECT	rp;
	LPSTR	lpDlgName = NULL, lpImgFileName="???";
//	HCURSOR	hoc;
	char	wtitle[256];	// Window title
	BOOL	bExternalObject = FALSE;
	// list of loaded images, if any
	vObjectsVec vObjects;

	// Added by Peter on 12 April 2007
	if( bDemo && (IDM_E_PASTE == uType) )
	{
		return NULL;
	}
	// End of addition by Peter on 12 April 2007

	if (uType==IDM_F_SAVE)
	{
		objSaveObject();
		return NULL;
	}
	if (uType==IDM_F_SSAVE)
	{
		objSaveTextObject();
		return NULL;
	}
	if (uType==IDM_F_GRAB && bGrabberActive)
	{
		bGrabberActive = FALSE;
		// Added by Peter on 25 Sep 2001
//		::hwndLive = NULL;
		InitializeMenu( MainhWnd );
		return NULL;
	}

	if ((uType==IDM_F_OPEN) || (uType==IDM_F_OPEN_ELD) || (uType==IDM_F_GRAB)  ||
		(uType>=IDM_F_FILE0 &&  uType<=IDM_F_FILE9) ||
		(uType==IDM_E_PASTE) || (CRISP_EMPTY_IMG == uType) )
		hParent = MDIhWnd;
	else
		hParent = hActive;

	if( hParent )
	{
		if( NULL == (P = OBJ::GetOBJ( hParent )) )
			return NULL;
	}

	if (P->ID == OT_AREA) hPosPar = P->ParentWnd;
	else                  hPosPar = hParent;
	PP = OBJ::GetOBJ( hPosPar );
	pfs = 1;
	if( (NULL != PP) && (false == PP->m_vChildren.empty()) )
	{
		OBJ* pChild;
		HwndVecCit cit;
		// Modified by Peter on 06 July 2006
//		for(i=0,pfs=0;i<MAXCHI;i++)
		for(cit = PP->m_vChildren.begin(); cit != PP->m_vChildren.end(); ++cit)
		{
			if( NULL != (pChild = OBJ::GetOBJ(*cit)) )
			{
				if( OT_AREA != pChild->ID ) pfs++;
				else if( false == pChild->m_vChildren.empty() ) pfs++;
			}
//			if (PP->Chi[i]==NULL) break;
//			else if (OBJ::GetOBJ(PP->Chi[i])->ID != OT_AREA) pfs++;
//			else if (OBJ::GetOBJ(PP->Chi[i])->Chi[0] != NULL) pfs++;
		}
		// End of modifications by Peter on 06 July 2006
	}

	O = CRISP_CreateObject(P, OE, bExternalObject, hParent);

	GetWindowRect( hPosPar, &rp );
	MapWindowPoints( NULL, MDIhWnd, (LPPOINT)&rp, 2);
	xyoffs = pfs * (GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYFRAME));

	// Pointer to the original image file name
	if ((hImgPar=objFindParent( O, OT_IMG ))) lpImgFileName = &(OBJ::GetOBJ(hImgPar)->title[0]);

	x0 = rp.left + xyoffs;
	y0 = rp.top + xyoffs;

	if (!bExternalObject)
	switch( uType ) // if not external object
	{
	case IDM_F_FILE0:
	case IDM_F_FILE1:
	case IDM_F_FILE2:
	case IDM_F_FILE3:
	case IDM_F_FILE4:
	case IDM_F_FILE5:
	case IDM_F_FILE6:
	case IDM_F_FILE7:
	case IDM_F_FILE8:
	case IDM_F_FILE9:
		{
			char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
			strcpy( O->fname, szRecentFiles[uType-IDM_F_FILE0] );
			_splitpath( O->fname, drive, dir, fname, ext );
			_makepath( O->title, NULL, NULL, fname, ext );
			if( FALSE == CRISP_ReadImage(O, vObjects) )
			{
				goto errexit;
			}
			x0 = rp.left + xyoffs;
			y0 = rp.top + xyoffs;
			break;
//			goto readimg;
		}

	// Added by Peter on 5 July 2006
	case IDM_F_OPEN_ELD:
		CRISP_OpenFile(O, OFN_ELDDATA);
		if( FALSE == ELD_ReadEldData(O/*, vObjects*/) )
		{
			goto errexit;
		}
		x0 = rp.left + xyoffs;
		y0 = rp.top + xyoffs;
		break;
	// End of Peters addition on 5 July 2006

	case IDM_F_OPEN:
		//_OpenShellFile
		if (FALSE == CRISP_OpenFile(O))
		{
			goto errexit;
		}
		if( FALSE == CRISP_ReadImage(O, vObjects) )
		{
			goto errexit;
		}
		x0 = rp.left + xyoffs;
		y0 = rp.top + xyoffs;
		break;

	case IDM_E_PASTE:
		// DEMO mode check added by Peter on 10 Nov 2005
		if ( !bDemo && (0 != strlen(szSerNo)) )
		{
			if( !objPasteFromCB( O ) )
			{
				free( O );
				return FALSE;
			}
		}
		break;

	case IDM_F_GRAB:
		{
			int x, y;
			DWORD dwPixSize;
			// if we have only 1 type of grabber
			if( (TRUE == bShark4) && (FALSE == bMV40) )
			{
				nUseGrabber = GB_USE_SHARK4;
			}
			else if( (FALSE == bShark4) && (TRUE == bMV40) )
			{
				U32	devices;
				mv40GetNumberDevices( szMV40_Name, &devices );
				if (1 == devices)
				{
					nUseGrabber = GB_USE_MV40;
					nMV40Device = 0;
				}
				else
				{
					nUseGrabber = ChooseGrabber();
				}
			}
			else if( (TRUE == bShark4) && (TRUE == bMV40) )
			{
				nUseGrabber = ChooseGrabber();
			}
			if( GB_USE_SHARK4 == nUseGrabber )
			{
				if( NULL != g_pfnS4GETREALRESOLUTION )
				{
					g_pfnS4GETREALRESOLUTION( &x, &y );
				}
//				s4GetRealResolution( &x, &y );
				dwPixSize = siGetBitRes();
			}
			else if( GB_USE_MV40 == nUseGrabber )
			{
				mv40GetDevice( nMV40Device, &dwSerial );
				if (dwSerial == 0x53535353)		// bootloader here
					return FALSE;

				mv40Initialize( szMV40_Name, dwSerial, &hMV );
				if( INVALID_HANDLE_VALUE == hMV )
				{
					sprintf(TMP, "For selected grabber: %s, device:%d, s/n: 0x%08X\r\n"
						"initialization failed.", szMV40_Name, nMV40Device, dwSerial);
					MessageBox(NULL, TMP, "CRISP", MB_OK | MB_ICONSTOP );
					goto errexit;
				}
				mv40SetModeEx( hMV, 1/*uiMode*/, 0, 0, 4096, 4096 );
				SIZE resolution;
				DWORD dwField1, dwBuffWidth;
				mv40GetResolution( hMV, &resolution, &dwBuffWidth, &dwField1, &dwPixSize );
				x = resolution.cx;
				y = resolution.cy;
				dwPixSize = (dwPixSize + 7) & (~7);
			}
			if( GB_USE_NONE != nUseGrabber )
			{
				O->NI = NIDefault;
				O->ID = OT_IMG;
				O->x = x;
				O->y = y;
				O->xas = x;
				O->yas = y;
				O->npix = (1 == dwPixSize >> 3) ? PIX_BYTE : PIX_WORD;
#ifdef USE_XB_LENGTH
				O->xb = R4(x);
				O->dp000 = (LPBYTE)calloc(1, O->xb*O->y*IMAGE::PixFmt2Bpp(O->npix)*2+16);	// *2 for HookLiveBuffer, and +16 for rounding
#else
				O->stride = OBJ::CalculateStride(O->npix, x);
				O->dp000 = (LPBYTE)calloc(1, O->stride*O->y*2 + 16);	// *2 for HookLiveBuffer, and +16 for rounding
#endif
				if( NULL == O->dp000 )
					return FALSE;
				O->dp = (LPBYTE)((((DWORD)O->dp000)+15)&~15);
				O->scu = 1;
				O->scd = 1;
				O->imageserial = iImageSerial++;
				sprintf( O->title, "Live image #%d", O->imageserial );
				strcpy( O->fname, "" );
				x0 = rp.left + xyoffs;
				y0 = rp.top + xyoffs;
				bGrabberActive = TRUE;
				O->flags |= OF_grabberON;
				O->maxcontr = TRUE;
				// added by Peter on 2 Oct 2001
				if(hwndLive != NULL)
				{
					if( GB_USE_SHARK4 == nUseGrabber )
					{
						TerminateThread(hLiveThreadShark4, -1);
						hLiveThreadShark4 = NULL;
					}
					else if( GB_USE_MV40 == nUseGrabber )
					{
						TerminateMV40Capturing();
					}
					hwndLive = NULL;
				}
			}
			else
			{
				goto errexit;	// none of grabbers has been chosen - nothing to do
			}
		}
		break;

	case IDM_A_AR128: i = 128;  goto arcommon;
	case IDM_A_AR256: i = 256;  goto arcommon;
	case IDM_A_AR512: i = 512;  goto arcommon;
	case IDM_A_AR1K:  i = 1024; goto arcommon;
	case IDM_A_AR2K:  i = 2048; goto arcommon;
	case IDM_A_AR4K:  i = 4096; goto arcommon;
	case IDM_A_ARFREE: i=0; goto arcommon;
arcommon:
		O->ID = OT_AREA;
		if (i) { O->xas = i;  O->yas = i;  O->ui = i; }
		else   { O->xas = 32; O->yas = 32; O->ui = 0; }
		sprintf(wtitle, "Area %dx%d on %s", O->xas, O->yas, P->title );
		x0 = y0 = xyoffs;
		break;

	case IDM_C_HIST:
		O->ID = OT_HIST;
		sprintf(wtitle, "Histogram of %s", P->title );
		x0 = rp.right;	y0 = rp.top+xyoffs;
		break;

	case IDM_C_CORR:
		O->ID = OT_CORR;
		sprintf(wtitle, "CCD lookup correction of %s", P->title );
		lpDlgName = "CCD";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_C_FFT:
		O->ID = OT_FFT;
		O->maxcontr = FALSE;
		x0 = rp.right;	y0 = rp.top+xyoffs;
		break;

	case IDM_A_DIGI:
		O->ID = OT_DIGI;
		sprintf(wtitle, "Digital Map of %s", P->title );
		x0 = rp.right;	y0 = rp.top+xyoffs;
		break;

	case IDM_C_LREF:
		O->ID = OT_LREF;
		sprintf(wtitle,"Lattice refinement of %s", lpImgFileName );
		lpDlgName = "LREF";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_C_OREF:
		O->ID = OT_OREF;
		sprintf(wtitle,"Origin refinement of %s", lpImgFileName );
		lpDlgName = "OREF";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_C_DMAP:
		O->ID = OT_DMAP;
		sprintf(wtitle,"density map from %s", lpImgFileName );
		x0 = rp.right; y0 = rp.top+xyoffs;
		break;

	case IDM_C_AMTR:
		O->ID = OT_AMTR;
		// changed by Peter on 23 Sep 2001
		// since AMTR can be opened for the DMAP which is alone
		// without any parent image (this image had been closed).
//		sprintf(wtitle,"Atomic meter on %s", lpImgFileName );
		char	_fname[_MAX_FNAME], _ext[_MAX_EXT];
		_splitpath(O->fname, NULL, NULL, _fname, _ext);
		sprintf(wtitle, "Atomic meter on %s%s", _fname, _ext );
		lpDlgName = "AMTR";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_C_ONED:
		O->ID = OT_ONED;
		sprintf(wtitle,"One-D tools on %s", lpImgFileName );
		lpDlgName = "ONED";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_C_UBEND:
		O->ID = OT_UBEND;
		sprintf(wtitle, "Unbending of (%s)", lpImgFileName );
		lpDlgName = "UBEND";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_C_AXTAL:
		O->ID = OT_AXTL;
		sprintf(wtitle, "Artificial crystal on (%s)", lpImgFileName );
		lpDlgName = "AXTAL";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_E_ELD:
		O->ID = OT_ELD;
		sprintf(wtitle,"ELD on %s", lpImgFileName );
		lpDlgName = "ELD";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_E_CAL:
		O->ID = OT_CAL;
		sprintf(wtitle,"ED calibration on %s", lpImgFileName );
		lpDlgName = "CAL"; x0 = rp.right; y0 = rp.top+xyoffs;
		break;

	case IDM_E_EDRD:
		O->ID = OT_EDRD;
		sprintf(wtitle,"ELD for rings on %s", lpImgFileName );
		lpDlgName = "EDRD"; x0 = rp.right; y0 = rp.top+xyoffs;
		break;

	case IDM_E_PHIDO:
		O->ID = OT_PHIDO;
		sprintf(wtitle,"Phase Identification from d-spacing (%s)", lpImgFileName );
		lpDlgName = "PHIDO";
		x0 = rp.right; y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_A_FILT:
		O->ID = OT_FILT;
		sprintf(wtitle,"Filter on %s", lpImgFileName );
		lpDlgName = "FILTER";
		x0 = rp.right;
		y0 = rp.top+xyoffs;
//		x0 = rp2.right; y0 = rp2.top+xyoffs;
		break;

	case IDM_A_IFFT:
		O->ID = OT_IFFT;
		sprintf(O->title,"Inverse FFT on %s", lpImgFileName );
		strcpy( O->fname, "" );
		x0 = rp.right;
		y0 = rp.top+xyoffs;
		break;

		// Added by Peter on [5/4/2006]
	case CRISP_EMPTY_IMG:
		sprintf(wtitle, "");
		if( NULL != pfnPreCreate )
		{
			pfnPreCreate(O, pExtraData);
		}
		break;
		// end of addition by Peter [5/4/2006]

	default:
errexit:
		free( O );
		InitializeMenu( MainhWnd );
		return NULL;
	}

	if (x0+20 > GetSystemMetrics( SM_CXSCREEN )) x0 = 50;
	if (y0+20 > GetSystemMetrics( SM_CYSCREEN )) y0 = 50;
	uNewX0 = x0;
	uNewY0 = y0;
	HWND hWndChi = NULL;
	if( true == vObjects.empty() )
	{
		vObjects.push_back(O);
	}
	if( false == vObjects.empty() )
	{
		vObjectsVecIt oit;
		for(oit = vObjects.begin(); oit != vObjects.end(); ++oit)
		{
			if( NULL == lpDlgName )
			{
				if( OT_AREA != O->ID )
				{
					hWndChi = ::CreateWindowEx(WS_EX_WINDOWEDGE, "CHILD", wtitle,
						WS_VISIBLE | WS_CHILD | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,
						x0, y0,
						GetSystemMetrics( SM_CXMAXTRACK ), GetSystemMetrics( SM_CYMAXTRACK ),
						MDIhWnd, NULL, hInst, (*oit));
				}
				else
				{
					hWndChi = ::CreateWindowEx( WS_EX_TRANSPARENT, "CHILD", wtitle,
						WS_VISIBLE | WS_CHILD | WS_THICKFRAME | WS_BORDER | WS_SYSMENU,
						x0, y0, (*oit)->xas*P->scu/P->scd, (*oit)->yas*P->scu/P->scd,
						hParent, NULL, hInst, (*oit));
				}
				x0 += 20;
				y0 += 20;
			}
			else
			{
		//		chi = CreateDialogParam( hInst, lpDlgName, MDIhWnd, (DLGPROC)ChildWndProc, (LPARAM)O );
				hWndChi = CreateObjectDialog( hInst, lpDlgName, MDIhWnd, (*oit) );
				SetWindowText( hWndChi, wtitle );
			}
			if( (NULL != hWndChi) && (NULL != P) )
			{
				P->m_vChildren.push_back(hWndChi);
				objModifySysMenu( hWndChi );
				if( oit == vObjects.begin() )
				{
					chi = hWndChi;
				}
			}
			UpdateWindow( hWndChi );
			if( hNavig )
			{
				objModifyNavig();
				TreeView_SelectItem( hNavig, (*oit)->hTree );
			}
		}
	}
	// end of Peters addition on 5 July 2006
	// return
	return chi;
/*
	// single object
	if( NULL == lpDlgName )
	{
		if (O->ID != OT_AREA)
		{
			chi = CreateWindowEx(WS_EX_WINDOWEDGE, "CHILD", wtitle,
								WS_VISIBLE | WS_CHILD | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,
								x0, y0, GetSystemMetrics( SM_CXMAXTRACK ),GetSystemMetrics( SM_CYMAXTRACK ),
								MDIhWnd, NULL, hInst, O);
		}
		else
		{
			chi = CreateWindowEx( WS_EX_TRANSPARENT / *| WS_EX_CLIENTEDGE* /, "CHILD", wtitle,
								WS_VISIBLE | WS_CHILD | WS_THICKFRAME | WS_BORDER | WS_SYSMENU,
								x0, y0, O->xas*P->scu/P->scd, O->yas*P->scu/P->scd,
								hParent, NULL, hInst, O);
		}
	}
	else
	{
//		chi = CreateDialogParam( hInst, lpDlgName, MDIhWnd, (DLGPROC)ChildWndProc, (LPARAM)O );
		chi = CreateObjectDialog( hInst, lpDlgName, MDIhWnd, O );

		SetWindowText( chi, wtitle );
	}
	if (chi)
	{
		P->Chi[nchi] = chi;
		objModifySysMenu( chi );
	}
	UpdateWindow( chi );

	if (hNavig)
	{
		objModifyNavig( );
		TreeView_SelectItem( hNavig, O->hTree );
	}
	return chi;
*/
}
/****************************************************************************/
VOID ActivateObj( HWND hWnd )
{
	OBJ*	pObj = NULL;
	if( (NULL != hWnd) && (TRUE == ::IsWindow(hWnd)) )
	{
		pObj = OBJ::GetOBJ(hWnd);

		if( hActive != hWnd )
		{
			if( hActive && IsWindow(hActive) )
				SendMessage( hActive, WM_NCACTIVATE, FALSE, 0 );
			hActive = hWnd;
			if( hActive && IsWindow(hActive) )
			{
				SendMessage( hActive, WM_NCACTIVATE, TRUE, 0 );
				BringWindowToTop( hActive );
			}
		}
	}
	if (hColors)   SendMessage(hColors,  WM_INITCOLORS, 0, (LPARAM)hWnd );
	if (hInfo)     SendMessage(hInfo,    WM_INITINFO,   0, (LPARAM)hWnd );
	if (hSelectET) SendMessage(hSelectET,WM_INITINFO,   0, (LPARAM)hWnd );

	InitializeMenu( MainhWnd );
	if( hWnd && pObj && hNavig && IsWindow(hNavig) && pObj->hTree)
		TreeView_SelectItem( hNavig, pObj->hTree );
}
/****************************************************************************/
BOOL DispatchTBarNotify( LPARAM lParam, LPBOOL ret )
{
	switch (((LPNMHDR) lParam)->code)
	{
		case TTN_NEEDTEXT:
		{
			LPTOOLTIPTEXT lpttt;

			lpttt = (LPTOOLTIPTEXT) lParam;
			lpttt->hinst = hInst;
			lpttt->lpszText = MAKEINTRESOURCE(lpttt->hdr.idFrom);
			break;
		}
		case TBN_QUERYDELETE:	// Toolbar customization -- can we delete this button?
			*ret = TRUE;
			break;

		case TBN_GETBUTTONINFO:	// The toolbar needs information about a button.
			*ret = FALSE;
			break;

		case TBN_QUERYINSERT:	// Can this button be inserted? Just say yo.
			*ret = TRUE;
			break;

		case TBN_CUSTHELP: // Need to display custom help.
			MessageBox(MainhWnd, "This help is custom.",NULL, MB_OK);
			break;

		case TBN_TOOLBARCHANGE: 	// Done dragging a bitmap to the toolbar.
			SendMessage(hWndToolBar, TB_AUTOSIZE, 0, 0);
			break;

		default: return FALSE;
	}
	return TRUE;
}
/****************************************************************************/
// Added by Peter on 25 Sep 2001
static std::vector<HWND> vChildWindows;

static BOOL CALLBACK EnumChildProc(HWND hwnd,			// handle to child window
								   LPARAM lParam)		// application-defined value
{
	if(hwnd != hwndLive)
		vChildWindows.push_back(hwnd);
	else if(bGrabberActive == FALSE)
	{
		if( GB_USE_SHARK4 == nUseGrabber )
		{
			TerminateThread(hLiveThreadShark4, -1);
			hLiveThreadShark4 = NULL;
		}
		else if( GB_USE_MV40 == nUseGrabber )
		{
			TerminateMV40Capturing();
		}
		hwndLive = NULL;
		DestroyWindow( hwnd );
	}
	return TRUE;
}
// hWnd - all child widows of it will be closed
static void CloseAllWindows(HWND hWnd)
{
	ShowWindow(hWnd, SW_HIDE);

	::EnumChildWindows(hWnd, EnumChildProc, 0);

	std::vector<HWND>::iterator iWnd;
	for(iWnd = vChildWindows.begin(); iWnd != vChildWindows.end(); ++iWnd)
		DestroyWindow( *iWnd );
	ShowWindow(hWnd,SW_SHOW);
}
/****************************************************************************/

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		static	BOOL	bInExit = FALSE;

    switch (message)
	{

	case WM_CREATE:
	{
		InitializeToolbar( hWnd );
//		ccs.hWindowMenu = GetSubMenu(GetMenu(hWnd), 4 );
//		ccs.idFirstChild = IDM_W_CHILDS;
		memset(&GPO,0,sizeof(OBJ));
		GPO.ID = OT_NONE;

		MDIhWnd = CreateWindow("CHILD", NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
								0, 0, 0, 0,
								hWnd, (HMENU)IDM_W_CHILDS, hInst, &GPO);
		ShowWindow(MDIhWnd, SW_SHOW);
		OBJ::SetOBJ( MDIhWnd, &GPO);
		InitializeMenu( hWnd );
		PostMessage(hWnd, WM_USER+1, 0, 0);
		break;
	}
	case WM_USER+1:
		if( 0 != gszCmdLine[0])
		{
			_OpenShellFile(gszCmdLine); // TODO: handle multiple files
		}

		if (gRunTests)
		{	
			test::RunUnitTests();
		}
		break;

	case WM_DESTROY:
/*
		// added by Peter on 2 Oct 2001
		if(hwndLive != NULL)
		{
			TerminateThread(hLiveThreadShark4, -1);
			hLiveThreadShark4 = NULL;
			hwndLive = NULL;
		}
*/
		if (bGrabberActive)
		{
			bAbrupt = TRUE;	// Abrupt grabbing
			bGrabberActive = FALSE;
			// added by Peter on 22 Aug 2002
			if( GB_USE_MV40 == nUseGrabber )
			{
				bStopGrabbingMV40 = TRUE;
			}
			while(hwndLive != NULL)
				Sleep(5);
			MessageBeep(-1);
		}
		PostQuitMessage(0);
        break;

		// added by Peter On 3 Feb 2003
	case WM_QUIT:
		Sleep(5);
		break;

	case WM_SIZE:
		// Update new window size in case of "normal" sizes
		if ((wParam == SIZENORMAL)||(wParam == SIZEFULLSCREEN)) {
			tobAdjustInfoBar(hWnd);
			AdjustMDIsize();
			break;
		} else goto CallDFP;

		// added by Peter on 28 May 2008
	case WM_DROPFILES:
		MWnd_OnDropFiles((HDROP)wParam);
		break;
		// end of addition by Peter on 28 May 2008

/*
	case WM_NOTIFY:
	{
		BOOL retnot=FALSE;
		if  (((LPNMHDR) lParam)->code == TTN_NEEDTEXT ) {
			LPTOOLTIPTEXT lpttt;
			lpttt = (LPTOOLTIPTEXT) lParam;
			lpttt->hinst = hInst;
			lpttt->lpszText = MAKEINTRESOURCE(lpttt->hdr.idFrom);
			break;
		}
		switch (((LPNMHDR) lParam)->idFrom) {
		case TOOLBAR_ID: if (!DispatchTBarNotify( lParam, &retnot ))  goto CallDFP; break;
		default:	goto CallDFP;
		}
		return retnot;
	}
*/
	case WM_COMMAND:
		if (splashWnd) DestroyWindow(splashWnd);
		switch (LOWORD(wParam))
		{
			case NAVIG_ID: 
				break;

			case IDM_V_TOOL:
				fShowTool = !fShowTool;
				tobShowToolBar( fShowTool );
				AdjustMDIsize();
				goto IniMnu;

			case IDM_V_STATUS:
				fShowStatus = !fShowStatus;
				tobShowInfoBar( fShowStatus );
				AdjustMDIsize();
				goto IniMnu;

			case IDM_V_NAVIG: objModifyNavig(); break;

			case IDM_V_FIT:
				if (hActive && IsWindow(hActive)) {
					OBJ* O = OBJ::GetOBJ(hActive);
					if (O->ID==OT_IMG || O->ID==OT_FFT || O->ID==OT_IFFT)
						SendMessage(hActive,WM_SYSCOMMAND,SC_MAXIMIZE,0);
				}
				break;

			case IDM_V_COLORS:
				if (hColors) BringWindowToTop( hColors );
				else hColors = CreateDialogParam( hInst, "COLORS", MainhWnd, ColorsDlgProc, (LPARAM)hActive );
				break;

			case IDM_V_SETINPUT:
				if( TRUE == bShark4 )
				{
					if (hInputs) BringWindowToTop( hInputs );
					else hInputs = CreateDialogParam( hInst, "SETINPUT", MainhWnd, SetInputDlgProc, (LPARAM)0 );
				}
				break;

			case IDM_V_INFO:
				if (hInfo) BringWindowToTop( hInfo );
				else hInfo = CreateDialogParam( hInst, "INFORMATION", MainhWnd, InfoDlgProc, (LPARAM)hActive );
				break;

			case IDM_V_ICALC:
				if (hICalc) BringWindowToTop( hICalc );
				else hICalc = CreateDialogParam( hInst, "ICALC", hWnd, ICalcDlgProc, (LPARAM)hActive );
				break;

			// added by Peter on 18 Sep 2001
			case IDM_L_EDITTOP:
				if(hEditTop) BringWindowToTop( hEditTop );
//				else hEditTop = CreateDialogParam( hInst, "EditTop", hWnd, EditTopDlgProc, (LPARAM)hActive );
				break;

			case IDM_V_SELECTET:
				if (hSelectET) BringWindowToTop( hSelectET );
				else hSelectET = CreateDialogParam( hInst, "IMEDIT", MainhWnd, SelectETDlgProc, (LPARAM)hActive );
				break;

/*			case IDM_W_CASCADE:	SendMessage( MDIhWnd, WM_MDICASCADE, 0, 0); break;
			case IDM_W_HTILE:	SendMessage( MDIhWnd, WM_MDITILE, MDITILE_HORIZONTAL, 0); break;
			case IDM_W_VTILE:	SendMessage( MDIhWnd, WM_MDITILE, MDITILE_VERTICAL,   0); break;
			case IDM_W_IARRANGE:SendMessage( MDIhWnd, WM_MDIICONARRANGE,0, 0); break; */

				// added by Peter on 9 Oct 2001
			case IDM_F_BWCORR:
				{
					if(pfnStartCorrProc != NULL)
					{
						pfnStartCorrProc();
					}
					break;
				}

			case IDM_F_TVIPS_ACQUIRE_IMG:
				{
//					if( NULL != g_pfnTVIPS_acq_img )
//					{
//						if( FALSE == g_pfnTVIPS_acq_img(NULL) )
//						{
//						}
//					}
					extern BOOL TVIPS_AcquireImage(LPVOID pData);
					TVIPS_AcquireImage(NULL);
					break;
				}

			case IDM_F_TVIPS_GET_IMG:
				{
//					if( NULL != g_pfnTVIPS_get_img )
//					{
//						OBJ* pCurObj = NULL;
//						BOOL bExternalObj = FALSE;
//						// create a new object
//						if( NULL == (pCurObj = CRISP_CreateObject(&GPO, NULL, bExternalObj, hWnd)) )
//							break;
//						if( TRUE == g_pfnTVIPS_get_img(pCurObj) )
//						{
//						}
//					}
//					OBJ* pCurObj = NULL;
//					BOOL bExternalObj = FALSE;
					// create a new object
//					HWND hImg = objCreateObject(IMG)
//					if( NULL == (pCurObj = CRISP_CreateObject(&GPO, NULL, bExternalObj, hWnd)) )
//						break;
					extern BOOL TVIPS_GetImage(LPVOID pData);
					TVIPS_GetImage(NULL);
					break;
				}

			case IDM_F_TVIPS_GET_EM_VEC:
				{
//					if( NULL != g_pfnTVIPS_get_vec )
//					{
//						if( TRUE == g_pfnTVIPS_get_vec(NULL) )
//						{
//						}
//					}
					extern BOOL TVIPS_GetEmVector(LPVOID pData);
					TVIPS_GetEmVector(NULL);
					break;
				}

			case IDM_F_EXIT:
				if (bInExit) break;
				bInExit = TRUE;
				PostMessage(hWnd, WM_CLOSE, 0, 0);

			// Continue with closing all the windows
			case IDM_W_CLOSE:
				if ( hActive != NULL )
				{
					if ( hActive != hwndLive )
						DestroyWindow( hActive );
					else if(bGrabberActive == FALSE)
					{
						if( GB_USE_SHARK4 == nUseGrabber )
						{
							TerminateThread(hLiveThreadShark4, -1);
							hLiveThreadShark4 = NULL;
						}
						else if( GB_USE_MV40 == nUseGrabber )
						{
							TerminateMV40Capturing();
						}
						hwndLive = NULL;
						DestroyWindow( hActive );
					}
				}
				goto IniMnu;

			case IDM_W_CLOSEALL:
				{
					CloseAllWindows(MDIhWnd);
/*					register HWND hwndT;

/*					ShowWindow(MDIhWnd,SW_HIDE);
					while ( hwndT = GetWindow (MDIhWnd, GW_CHILD))
					{
						// Skip the icon title windows
						while (hwndT && GetWindow (hwndT, GW_OWNER))
							hwndT = GetWindow (hwndT, GW_HWNDNEXT);
						if (!hwndT)
							break;
						if (hwndT != hwndLive)
							DestroyWindow( hwndT );
					}
					ShowWindow(MDIhWnd,SW_SHOW);
*/
				}
				goto IniMnu;

IniMnu:			InitializeMenu( hWnd );
				break;

			case IDM_W_IARRANGE:	break;
			case IDM_F_PRINT:
				objPrintObject();
				break;

			case IDM_F_PRINTSET:
				prnPageSetup( hWnd, FALSE );
				break;

			case IDM_E_COPY:
				objCopyToCB();
				InitializeMenu( hWnd );
				break;

			case IDM_E_XCOPY:
				objCopyChildToCB();
				InitializeMenu( hWnd );
				break;

			case IDM_H_ABOUT: DialogBoxParam(hInst, "ABOUT", hWnd, AboutDlgProc, FALSE); break;
			case IDM_H_CONTENTS:
			case IDM_H_HELPONHELP:
				break;

			default:
				ReplyMessage( FALSE );
				tobSetState( LOWORD(wParam), TRUE );
				tobUpdateToolBarWnd();
				{
					HWND hChild = objCreateObject( LOWORD(wParam), NULL );
					if (hChild)
					{
						tobSetState( LOWORD(wParam), FALSE );
						tobUpdateToolBarWnd();
						ActivateObj( hChild );
						// added by Peter on 5 July 2006
						OBJ* pObj = OBJ::GetOBJ(hChild);
						if( (NULL != pObj) && (NULL != pObj->pfnPreActivate) )
						{
							(*(pObj->pfnPreActivate))(pObj, NULL);
						}
						break;
					}
				}
				tobSetState( LOWORD(wParam), FALSE );
				tobUpdateToolBarWnd();
				goto CallDFP;
		}
		break;

	case WM_MENUSELECT:
		if ( (LOWORD(lParam) & (MF_POPUP | MF_SEPARATOR | MF_SYSMENU)) )
			TMP[0]=0;
		else {
			if (wParam>=(WORD)IDM_W_CHILDS) wParam = IDM_W_CHILDS;
//			LoadString( hInst, wParam, TMP, sizeof(TMP));
		}
		tobSetInfoText( ID_INFO, TMP );
		break;

	case WM_CLOSE:
//		if (bLive) { bLive = FALSE; break; }
		DestroyWindow( hWnd );
		break;

	case WM_PALETTECHANGED:
		goto CallDFP;
		{
		static BOOL bInPalChange = FALSE;

		if (bInPalChange)
		break;

		bInPalChange = TRUE;
		SendMessageToAllChildren (MDIhWnd, message, wParam, lParam);
		bInPalChange = FALSE;
		break;
		}

	 // We get a QUERYNEWPALETTE message when our app gets the
	 //  focus.  We need to realize the currently active MDI
	 //  child's palette as the foreground palette for the app.
	 //  We do this by sending our MYWM_QUERYNEWPALETTE message
	 //  to the currently active child's window procedure.
	case WM_QUERYNEWPALETTE:
		if (bGrabberActive && hwndLive && IsWindow(hwndLive))
			SendMessage (hwndLive, WM_QUERYNEWPALETTE, 0, 0);
		goto CallDFP;

	case WM_SETFOCUS:
		InitializeMenu( hWnd );
		goto CallDFP;

	default:
CallDFP:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
	catch (std::exception& e)
	{
		Trace(Format(_T("MainWndProc() -> message '%1%': '%2%'")) % message % e.what());
	}
	return FALSE;
}
/****************************************************************************/
// Added by Peter on 29 June 2004 (My name DAY!!!)
void InitializeObjectDialog(HWND hDlg, OBJ* pObj)
{
	LONG lStyle;

	if( NULL == pObj )
	{
		return;
	}
	pObj->hWnd = hDlg;
	pObj->bDialog = TRUE;
	OBJ::SetOBJ( hDlg, pObj );

	SendMessage(hDlg, WM_SETICON, (WPARAM)FALSE,
		(LPARAM)LoadImage(hInst, "CALIDRIS", IMAGE_ICON,
		16, 16, LR_DEFAULTCOLOR));
	SetWindowPos( hDlg, NULL, uNewX0, uNewY0, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	// resource editor cannot set the flag WS_MINIMIZEBOX if the dialog has WS_CHILD
	lStyle = GetWindowLong( hDlg, GWL_STYLE );
	lStyle |= WS_MINIMIZEBOX;
	SetWindowLong( hDlg, GWL_STYLE, lStyle );
}

HWND CreateObjectDialog(HINSTANCE hInst, LPCTSTR pszDlgName, HWND hMainWnd, OBJ* pObj)
{
	HWND hDlg = NULL;
	try
	{
		// create the modeless dialog first
		if( pObj->ID != OT_NONE )
		{
			hDlg = CreateDialogParam( hInst, pszDlgName, hMainWnd,
				(DLGPROC)ObjectsWndProc[pObj->ID], (LPARAM)pObj );
			// check if valid
			if( NULL == hDlg )
			{
				::MessageBox(NULL, _T("Failed to create dialog"), _T("ERROR"), MB_OK);
				return NULL;
			}
		}
		else
		{
			::MessageBox(NULL, _T("Empty object function"), _T("ERROR"), MB_OK);
			return NULL;
		}
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("CreateObjectDialog() -> %s"), e.what()));
	}

	return hDlg;
}

/****************************************************************************/
LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = FALSE;
	LONG lStyle;

	try
	{
		OBJ*	O = OBJ::GetOBJ(hWnd);

		switch (message)
		{
		case WM_INITDIALOG:
			// THIS IS OBSOLETE, CANNOT BE REACHED!!!
			O = (OBJ*)lParam;
			O->hWnd = hWnd;
			O->bDialog = TRUE;
			OBJ::SetOBJ( hWnd, O);
			SendMessage(hWnd, WM_SETICON, (WPARAM)FALSE, (LPARAM)LoadImage(hInst, "CALIDRIS", IMAGE_ICON, 16,16,LR_DEFAULTCOLOR));
			SetWindowPos( hWnd, NULL, uNewX0, uNewY0, 0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			// resource editor cannot set the flag WS_MINIMIZEBOX if the dialog has WS_CHILD
			lStyle = GetWindowLong( hWnd, GWL_STYLE );
			lStyle |= WS_MINIMIZEBOX;
			SetWindowLong( hWnd, GWL_STYLE, lStyle );
			if (O->ID != OT_NONE)
			{
//				ObjectsWndProc[O->ID]( hWnd, message, wParam, lParam );
				SetWindowLong( hWnd, DWL_DLGPROC, (LONG)ObjectsWndProc[O->ID] );
			}
			return ObjectsWndProc[O->ID]( hWnd, message, wParam, lParam );

		case WM_NCCREATE:
			O = (OBJ*)(((LPCREATESTRUCT) lParam)->lpCreateParams);
			O->hWnd = hWnd;
			O->bDialog = FALSE;
			OBJ::SetOBJ( hWnd, O);
			if (O->ID != OT_NONE)
			{
				SetWindowLong( hWnd, GWL_WNDPROC, (LONG)ObjectsWndProc[O->ID] );
			}
			break;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("ChildWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
/****************************************************************************/
LRESULT CALLBACK BigPlotWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ(hWnd);

	switch (message)
	{
	case WM_NCCREATE:
		O = (OBJ*)(((LPCREATESTRUCT) lParam)->lpCreateParams);
		O->hBigPlot = hWnd;
		OBJ::SetOBJ(hWnd, O);
		break;
	case WM_PAINT:
		if (O->PlotPaint) O->PlotPaint( O );
		break;
	case WM_SIZE:
		InvalidateRect( hWnd, NULL, FALSE );
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
/****************************************************************************/
BOOL LoadAboutBmp( OBJ* O, LPSTR lpString )
{
	HBITMAP hbmp;
	DIBSECTION dbs;

	if( NULL == (hbmp = (HBITMAP)LoadImage(hInst, lpString, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION)) )
	{
		return FALSE;
	}
	GetObject( hbmp, sizeof(dbs), &dbs);
	O->ID = OT_IMG;
	O->x = dbs.dsBm.bmWidth;
	O->y = dbs.dsBm.bmHeight;
	O->xas = O->x;
	O->yas = O->y;
	O->npix = PIX_BYTE;
	O->imax = 0;
	O->imax = 256;
	O->maxcontr = FALSE;
#ifdef USE_XB_LENGTH
	O->xb = dbs.dsBm.bmWidthBytes;
	O->dp = (LPBYTE)calloc(1, O->xb*O->y);
#else
	O->stride = R4(dbs.dsBm.bmWidthBytes);
	O->dp = (LPBYTE)calloc(1, O->stride*O->y);
#endif
	if( NULL == O->dp )
		return FALSE;
	O->scu = 1;
	O->scd = 1;
	O->bright = 128;
	O->contrast = 128;
	O->palno = -1;
	strcpy( O->title, "" );
	strcpy( O->fname, "" );

	GetBitmapBits( hbmp, dbs.dsBm.bmWidthBytes*dbs.dsBm.bmHeight, O->dp );

	return TRUE;
}
/****************************************************************************/
static void SetAboutUserText( HWND hDlg )
{
	int serno;

	if( sscanf(szSerNo,"%d",&serno)==1 && serno!=0 )
	{
		sprintf(TMP,"%s\n%s\nS/N: %s", szUserName, szValidUntil, szSerNo);
	}
	else
	{
		sprintf(TMP,"\n%s\n%s", szUserName, szValidUntil);
	}
	SetDlgItemText( hDlg, IDC_ABOUT_TEXT, TMP);
}
/****************************************************************************/
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static	HFONT	hftimes, hfcourier;
	static	OBJ		O;
	static	BOOL bLogo;

	switch (message)
	{
		case WM_INITDIALOG:
		{
			HDC hdc = GetDC( hDlg );
			hftimes = CreateFont( MulDiv((-27), GetDeviceCaps(hdc, LOGPIXELSY), 72),
								  0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, DEFAULT_CHARSET,
								  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
								  FIXED_PITCH | FF_MODERN, "Times New Roman");

			hfcourier = CreateFont( MulDiv((-12), GetDeviceCaps(hdc, LOGPIXELSY), 72),
								  0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
								  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
								  FIXED_PITCH | FF_MODERN, "Courier New");

			SendDlgItemMessage( hDlg, IDC_ABOUT_NAME, WM_SETFONT, (WPARAM)hftimes, 0);
			SendDlgItemMessage( hDlg, IDC_ABOUT_TEXT, WM_SETFONT, (WPARAM)hfcourier, 0);
			sprintf(TMP, "Version %d.%d%c", iVersion, iRelease, cSubRel);
			SetDlgItemText( hDlg, IDC_ABOUT_VERSION, TMP);
			{ int i; for(i=0;i<sizeof(Copyright);i++) TMP[i]=Copyright[i] ^(5+(i%7)+(i%23));}
			SetDlgItemText( hDlg, IDC_ABOUT_COPYRIGHT, TMP);
			ReleaseDC( hDlg, hdc );
			LoadAboutBmp( &O, "bkg" );
			pntBitmapToScreen( hDlg, O.dp, O.x, O.y, 0, 0, O.scu, O.scd, &O, 0x8000);
			SetAboutUserText( hDlg );
			if (lParam) {	// As logo window
			RECT	rc;
				splashWnd = hDlg;
				SetTimer( hDlg, 0, 15000, NULL );
				bLogo = TRUE;
				GetWindowRect( hDlg, &rc );
				SetWindowPos( hDlg, HWND_TOPMOST, (GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.left))/2,
												  (GetSystemMetrics(SM_CYSCREEN)-(rc.bottom-rc.top))/2,
												  rc.right-rc.left,
												  rc.bottom-rc.top, SWP_NOACTIVATE);
			} else {
				bLogo = FALSE;
			}
		}
			return TRUE;

		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam==GetDlgItem(hDlg,IDC_A_ICON)) break;
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (BOOL)GetStockObject(NULL_BRUSH);

		case WM_ERASEBKGND:
			pntBitmapToScreen( hDlg, O.dp, O.x, O.y, 0, 0, O.scu, O.scd, &O, 0x8000);
			return 1;

		case WM_DESTROY:
			if (bLogo) KillTimer( hDlg, 0 );
			splashWnd = NULL;
			DeleteObject( hftimes ); hftimes = NULL;
			DeleteObject( hfcourier ); hfcourier = NULL;
			if (O.dp) free( O.dp );
			O.dp = NULL;
			return TRUE;

		case WM_TIMER:
		case WM_COMMAND:
		case WM_LBUTTONDOWN:
			if (bLogo)	DestroyWindow( hDlg );
			else		EndDialog(hDlg, TRUE);
			return TRUE;
	}
	return FALSE;
}
