#pragma once

#include "commondef.h"

#ifndef __CRISP_OBJECTS_H__
#define __CRISP_OBJECTS_H__
/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Object types definitions * by ML *****************************************/
/****************************************************************************/

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*- New Project Info -------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#include "Image.h"
#include <map>

#pragma pack(push, 1)
struct NEWINFO
{
	char	logo[2];			// Identificator always "NI"
// Microscope data
	short	EM_ID;				// Electron Microscope num. in ELMICRO.TBL
	float	Vacc;				// Acceleration voltage  ( in Volts )
	float	Cs;					// Spherical aberration  ( in m )
	float	Defocus0;			// Smallest Defocus value( in m )
	float	Defocus1;			// Highest Defocus value ( in m )
	float	Azimuth;			// Angle between X and defucus0 direction (in Radians)
	float	Def_Spread;			// Spread of defocus     ( in m )
	float	Conver;				// Convergence -- semi-angle at specimen (in Radians)
// Other data
	float	Scale;				// Pixel size along X    ( in m )
	float	Beam_Tilt;			// Beam tilt             ( in Radians )
	float	Spec_Tilt;			// Specimen tilt         ( in Radians )
	float	Thickness;			// Cristal thickness    ( in m )
	WORD	Flags;				// Flags
#define	NI_DIFFRACTION_PATTERN	1				// Image is Diffraction pattern
	float	Magnif;				// Image magnification   ( x )
	float	CamLength;			// Camera length         ( m )
	float	Digit;				// Digitalization along X (pixel/m)
	float	AspectXY;			// Aspect ratio X/Y
	BYTE	DUMMY[40];			// Reserve

};

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//- Object types -----------------------------------------------------------
//--------------------------------------------------------------------------

#define	OT_NONE	-1		// Object is Grand Parent
#define	OT_IMG	0		// Object is Image
#define	OT_AREA	1		// Object is Area
#define	OT_FFT	2		// Object is FFT
#define	OT_HIST	3		// Object is Histogram
#define	OT_CORR	4		// Object is CCD correction
#define	OT_DIGI	5		// Object is Digital map
#define	OT_IFFT	6		// Object is IFFT
#define	OT_FILT	7		// Object is Filter

#define	OT_LREF	8		// Object is Lattice refinement
#define	OT_OREF	9		// Object is Origin refinement
#define	OT_DMAP	10		// Object is Density map
#define	OT_AMTR	11		// Object is Atomic meter

#define	OT_ONED	12		// Object is One D tools

#define	OT_ELD	13		// Object is ELD
#define	OT_CAL	14		// Object is ELD calibration
#define	OT_PHIDO 15		// Object is Phase identification
#define	OT_EDRD	16		// Object is ED	rad. distribution

#define OT_AXTL 17		// Object is Artificial Xtall

#define OT_UBEND 18		// Object is Unbending

// Added by Peter on [5/4/2006]
#define	CRISP_EMPTY_IMG		19
// end of addition by Peter [5/4/2006]

#define	MAXOBJECTS	20	// Number of object types


class OBJ;

// OBJ *GetOBJ(HWND hWnd);
// void SetOBJ(HWND hWnd, OBJ *pObj);
//#define	GetOBJ(hWnd)	((OBJ*)GetWindowLong( hWnd, GWL_USERDATA))
//#define	SetOBJ(hWnd,O)	SetWindowLong( hWnd, GWL_USERDATA, (LONG)O)

typedef BOOL (*PFN_PRE_ACTIVATE_OBJECT)(OBJ *pObj, LPVOID pExtraData);

// abstract iteration position
#ifndef _MFC_VER
struct __POSITION { };
typedef __POSITION* POSITION;
#endif

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//- Object main structure --------------------------------------------------
//--------------------------------------------------------------------------

typedef std::vector<HWND> HwndVec;
typedef HwndVec::iterator HwndVecIt;
typedef HwndVec::const_iterator HwndVecCit;

#define	OF_exists		0x0001	// Window exists
#define	OF_active		0x0002	// Window is active
#define	OF_tobesaved	0x0004	// Image should be saved
#define	OF_inedit		0x0008	// Object in edit mode
#define	OF_wasedit		0x0010	// Object was edited
#define	OF_indrag		0x0020	// Object in drag mode
#define	OF_grabberON	0x0040	// Grabber is ON
#define	OF_grabberFast	0x0080	// Grabber is in Fast mode

class OBJ
{
public:
	OBJ();
	~OBJ();

	static OBJ *GetOBJ(HWND hWnd);
	static void SetOBJ(HWND hWnd, OBJ *pObj);
	//////////////////////////////////////////////////////////////////////////
	OBJ &operator = (const OBJ &);
	POSITION FindFirstChild(int nID);
	OBJ *FindNextChild(POSITION &CurPos, int nID);
	HwndVecIt FindByWindow(HWND hWnd);

	LPVOID GetDataRow(int y)
	{
		return LPVOID(LPBYTE(dp) + y*stride);
	}
	LPVOID GetDataPtr(int _x, int _y)
	{
		return LPVOID(LPBYTE(dp) + _y*stride + _x*IMAGE::PixFmt2Bpp(npix));
	}
	static LPVOID GetDataPtr(LPVOID _ptr, PIXEL_FORMAT _npix, int _x, int _stride, int _y)
	{
		return LPVOID(LPBYTE(_ptr) + _y*_stride + _x*IMAGE::PixFmt2Bpp(_npix));
	}
	int CalculateStride()
	{
		return OBJ::CalculateStride(npix, x);
	}
	static int CalculateStride(PIXEL_FORMAT _npix, int _x)
	{
		return R4(IMAGE::PixFmt2Bpp(_npix) * _x);
	}

	int		ID;					// Object type
	HWND	ParentWnd;			// myParent HWND
	HwndVec	m_vChildren;		// myChild HWNDs
//	HWND	Chi[MAXCHI];		// myChild HWNDs
	HWND	hWnd;				// Self hWnd
	HTREEITEM	hTree;			// Handle in Navigator window
	BOOL	bDialog;			// This object is a dialog

	PFN_PRE_ACTIVATE_OBJECT pfnPreActivate;

	int		x,  y;				// Data size
	double	imin, imax;			// 5% thresh. min/max intensity in image
	double	imina, imaxa;		// Absolute min/max intensity in image
	LPBYTE	dp;					// Data memory pointer
	PIXEL_FORMAT	npix;		// Pixel type
//	float	x, y;
	int		xw, yw;				// Window client area size
//	float	xw, yw;				// Window client area size
	int		xo, yo;				// Window offsets
	int		xa, ya;				// Area position
	int		xas,yas;			// Area size
//	float	xas,yas;			// Area size
	int		scd;				// Scale down (zoom out)
	int		scu;				// Scale up (zoom in)
	int		stride;				// Image buffer stride

	HBITMAP hbit;				// BITMAP handle
	LPBYTE	bp;					// BitMap mem ptr
	LPBYTE	dp000;				// Original Data mem ptr, used while grabbing
	LPBYTE	ebp;				// Edit BitMap mem ptr
	LPBYTE	edp;				// Edit Data mem ptr
	HBITMAP hBme;				// Edit BITMAP handle
	BOOL	maxcontr;			// Maximize contrast
	DWORD	flags;				// Object flags

	char	title[_MAX_PATH];	// Image window title
	char	fname[_MAX_PATH];	// Image file path
	char	extra[_MAX_PATH];	// extra file path
	HMENU	sysmenu;			// System menu copy
	HWND	hDlg;				// Dialog handle
	HWND	hBigPlot;			// Big Plot window
	void	(*PlotPaint)( void * O );	// Plot paint function
	BYTE	bright;				// Brightness (Gray mode)
	BYTE	contrast;			// Contrast (Gray mode)
	int		palno;				// Palette no
	NEWINFO	NI;					// Project Info struct
	int		imageserial;		// Image serial number
	POINT	clickpoint;			// Click point coordinates

	HANDLE	hCalcTask;			// Calculation thread handle
	BOOL	bDoItAgain;			// Something has been changed, so do it again

	int		ui;					// Simple user int
	BYTE	dummy[4096];		// Dummy place for communication packages

	UINT	nMouseMessage;		// shows in case of image which message was for the mouse

	void AddWarningMessage(const TCHAR* inMessage, int inCommandOnClick = 0);
	void RemoveWarningMessage(const TCHAR* inMessage);
	void HandleWarningMessage(const TCHAR* inMessage, bool inShowMessage, int inCommandOnClick = 0);

	void UpdateInfoWindowsPos();
private:
	typedef std::map<std::string, HWND> InfoWindows;
	InfoWindows *infoWindows;

	InfoWindows& GetInfoWindows();
};

OBJ* GetBaseObject(OBJ* inObject);

typedef std::vector<OBJ*> vObjectsVec;
typedef vObjectsVec::iterator vObjectsVecIt;

typedef BOOL (*pfnCREATEOBJ_CALLBACK)(OBJ*, LPVOID);

#define	Cpyo(o,p,x) o->x=p->x

VOID ActivateObj( HWND hWnd );
void InitializeMenu( HWND hWnd );

// if bDestroyDMap == TRUE then DMap will be also destroyed
void	objFreeObj       ( HWND ohw, BOOL bDestroyDMap = FALSE );
void	objInformChildren( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
// Modified by Peter on [5/4/2006], added the PFN_CREATEOBJ_CALLBACK function
HWND	objCreateObject  ( UINT uType, OBJ* OE, pfnCREATEOBJ_CALLBACK pfnPreCreate = NULL,
						  LPVOID pExtraData = NULL );
// end of modification by Peter [5/4/2006]
HMENU	objModifySysMenu ( HWND hWnd );
HWND	objFindParent    ( OBJ* O, int ID );

#define	FM_Update			(WM_USER+1000)
#define	FM_ParentPaint		(WM_USER+1001)
#define	FM_ParentSize		(WM_USER+1002)
#define	FM_ParentInspect	(WM_USER+1003)
#define	FM_ParentLBUTTONUP	(WM_USER+1004)
#define	FM_UpdateNI			(WM_USER+1005)
#define	FM_Update2			(WM_USER+1006)

LRESULT CALLBACK ImageWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AreaWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FFTWndProc  (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HistWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CorrWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DigiWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK IFFTWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FiltWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LRefWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ORefWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DMapWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AmtrWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OneDWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ELDWndProc  (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EdRDWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CalWndProc  (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CorrWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PhidoWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK UBendWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AXtalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#pragma pack(pop)

#endif // __CRISP_OBJECTS_H__