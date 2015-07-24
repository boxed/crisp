/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* SHTools privat definitions * by ML ***************************************/
/****************************************************************************/
#pragma once

const int TMPSIZE = 65536;
extern char	TMP[TMPSIZE];
#define	pTMP	(&TMP[0])

extern HINSTANCE hInst;			// Handle to instance.                
extern HWND    	MainhWnd;		// Handle to main window.                
extern HACCEL	hAccel;			// Accelerator resource
            
extern char		szProg[];		// Program name
extern int		iVersion;
extern int		iRelease;
extern char		cSubRel;
extern char	 	szUserName[128];
extern char		szValidUntil[128];
extern char		szSerNo[128];
extern BOOL		bDemo;


extern HWND		MDIhWnd;		// Handle to MDI frame

extern int		iBPP, iPS;		// Number of bits per pixel, and pixel size in bytes

extern HWND		hActive;		// Active object box
extern HWND		hColors;		// Colors dialog box
extern HWND		hInfo;			// Information dialog box
extern HWND		hICalc;			// Image calculatr box
extern HWND		hSelectET;		// Select image edit tool
extern HFONT	hfInfo;			//
extern int		wFntW, wFntH;	//

#define	MAXRECENTFILES	10
extern char	szRecentFiles[MAXRECENTFILES][_MAX_PATH];
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// crisp2.c
extern	HPEN	RedPen;			// Red pen
extern	HPEN	BluePen;		// Blue pen
extern	HPEN	GreenPen;		// Green pen
extern	HPEN	GrayPen;		// Grey pen
extern	HPEN	YellowPen;		// Yellow pen
extern	HPEN	MagentaPen;		// Magenta pen
extern	HPEN	BluePen2;		// Blue pen 2 pixel width
extern	HPEN	RedPen2;		// Red pen 2 pixel width

// moved to FontManager
/*
extern	HFONT	hfSmall, hfMedium, hfLarge, hfNormal, hfGreek;
extern	int		fnts_w, fnts_h;
extern	int		fntm_w, fntm_h;
extern	int		fntn_w, fntn_h;
extern	int		fntg_w, fntg_h;
extern	int		fntl_w, fntl_h;
*/
extern	HCURSOR		hcWait;		// Hourglass cursor
extern	HCURSOR		hcFFT;		// FFT windows cursor
extern	HCURSOR		hcHeight;	// Histogram window cursor
extern	HCURSOR		hcAppStart;	// Arrow+Hourglass

extern	HBRUSH		hPlotBrush;	// Plot window bkg
extern	HPEN		hPlotPen;	// Pen used to draw plot bars
extern	COLORREF	rgbPlotText;// fg text color

// Added by Peter on 22 Aug 2002
// SoftHard Technology MV4.0 support
#define szShark4_Name		"Shark4 grabber"
typedef enum
{
	GB_USE_NONE,
	GB_USE_SHARK4,
	GB_USE_MV40,
} GRABBER;

extern	GRABBER	nUseGrabber;	// holds the type of grabber to use if multiple found
extern	BOOL	bMV40;			// SoftHard Technology MV4.0 Camera present
extern	UINT	nMV40Device;	// number of MV4.0 device if multiple found
extern	HANDLE	hMV;			// Handle to the camera device
extern	DWORD	dwSerial;		// Camera serial number
// end of addition
extern	BOOL	bShark4;		// Shark4 present
extern	BOOL	bGrabberActive;	// Grabber loop is active
extern	HWND	hwndLive;		// Window which displays a live image
extern	BOOL	bAbrupt;		// Grabber have to shut up

extern	BOOL	bInformChilds;	// Inform children loop is active

extern bool gRunTests;