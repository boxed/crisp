/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Common functions * by ML *************************************************/
/****************************************************************************/

//----------------------------------------------------------------------------
//- Common routines ----------------------------------------------------------
//----------------------------------------------------------------------------

//#define		SQR(x)		((x)*(x))

// Added by Peter on 29 June 2004
// THIS IS A MAIN FUNCTION WHICH EVERY DIALOG MUST CALL INSIDE WM_INITDIALOG MESSAGE!
void InitializeObjectDialog(HWND hDlg, OBJ* pObj);

// common.c
BOOL myError( int id );
BOOL dlgError( int id, HWND hdlg );

void ReportCoordinates( long lP, HWND hWnd );

VOID wndSetZoom             ( HWND hWnd, int scaleUp, int scaleDown );
BOOL wndDefSysMenuProc      ( HWND hWnd, WPARAM wParam, LPARAM lParam );
VOID wndCreateObjMenu       ( HWND hWnd, BOOL bTune, BOOL bZoom, BOOL bInvert, LPARAM lParam );
VOID wndSetScrollOrigin     ( HWND hWnd, BOOL bAround );
VOID wndSetInitialWindowSize( HWND hWnd );

//----------------------------------------------------------------------------
//- Choose a file ----------------------------------------------------------
//----------------------------------------------------------------------------
// choosef.c
typedef struct {
	LPSTR	szFilter;
	LPSTR	szDefExt;
	char	szDefDir[_MAX_FNAME];
	char	szDefFile[_MAX_FNAME];
} OFNAMES;
typedef OFNAMES * LPOFNAMES;

extern	OFNAMES	ofNames[];

#define	OFN_IMAGER	0
#define	OFN_IMAGEW	1
#define	OFN_HKLIST	2
#define	OFN_FILTER	3
#define	OFN_PALETTE	4
#define	OFN_CALIBR	5
#define	OFN_LIST	6
#define	OFN_CERIUS	7
#define	OFN_HKLLIST	8
#define	OFN_HKELIST	9
#define	OFN_ELDDATA	10
#define	OFN_BINARY	11

BOOL  fioGetFileNameDialog( HWND hWnd, LPSTR szDlgName, LPOFNAMES lpOFN, const TCHAR* szDefFile, BOOL save, UINT nFIndex=1);
LPSTR fioFileName( VOID );
LPSTR fioFileTitle( VOID );


//----------------------------------------------------------------------------
//- Painting routines --------------------------------------------------------
//----------------------------------------------------------------------------
// paint.c
#define		COLORS		256		// Number of colors
#define		GRAYSIZ		64		// Gray palette size
#define		USERSIZ		32		// User palette size
#define		MAXPAL		4		// Max number of user palettes
#define		RESISIZ		16		// Residual map palette size

typedef struct
{
	WORD	palVersion;
	WORD	palNumEntries;
	BYTE	palRGBF[COLORS*4];
} myLOGPAL;

typedef struct
{
	DWORD  biSize;
	LONG   biWidth;
	LONG   biHeight;
	WORD   biPlanes;
	WORD   biBitCount;
	DWORD  biCompression;
	DWORD  biSizeImage;
	LONG   biXPelsPerMeter;
	LONG   biYPelsPerMeter;
	DWORD  biClrUsed;
	DWORD  biClrImportant;
	WORD	index[COLORS];
} myBITMAPINFO;

VOID pntClose( VOID );
BOOL pntInit( VOID );

VOID pntBitmapToScreen( HWND, LPBYTE, int w, int h, int x0, int y0, int scu, int scd, OBJ* O, int flags );

__declspec(dllexport) HPALETTE pntGetPal( VOID );

BOOL	CALLBACK ColorsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern	HPALETTE	hMyPal;		// Handle to logical palette
extern	char		szPalName[MAXPAL][10];

#define	WM_INITCOLORS	(WM_USER+100)		// Message to update controls, lParam==hActive or NULL

//----------------------------------------------------------------------------
//- Image utilities  ---------------------------------------------------------
//----------------------------------------------------------------------------
// paint.c
__declspec(dllexport) void imgCalcMinMax( OBJ* O );

//----------------------------------------------------------------------------
//- Information panel --------------------------------------------------------
//----------------------------------------------------------------------------
// paint.c
#define	WM_INITINFO	(WM_USER+100)		// Message to update controls, lParam==hActive or NULL
BOOL CALLBACK InfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

double	CRISP_WaveLength(double u);
OBJ* _OpenShellFile(LPSTR pszFileName);

extern NEWINFO		NIDefault;