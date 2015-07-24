/****************************************************************************/
/* Copyright© 2002 **********************************************************/
/****************************************************************************/
/* by Peter Oleynikov *******************************************************/
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
#include "rgconfig.h"

#include <mmsystem.h>
#include <process.h>

#include "..\MV40\mv40api.h"
#include "..\MV40\PixFilt.h"

#include "MV40Live.h"

#pragma comment(lib, "..\\MV40\\mv40api.lib")
#pragma comment(lib, "..\\MV40\\PixFilt.lib")

HANDLE	hLiveThreadMV40 = NULL;

UINT	nMV40Device = 0;

static BOOL bHaveData;
static UINT uiMode;
static DWORD dwBuffWidth;
static DWORD dwField1;
static DWORD dwPixSize;
static DWORD dwWidth;
static DWORD dwHeight;
static UINT uBufWidth;
static LPWORD pImageData = NULL;
BOOL		bStopGrabbingMV40 = FALSE;

//****************************************************************************
//							Exposure glide control
//****************************************************************************

LPSTR ExposureTimeText( int val )
{
static char t[32];
double tt = (double)val * 10.*100.;

	if      (tt<1000.)       { sprintf(t,"%.3gus",(tt)); }
	else if (tt<1000000.)    { sprintf(t,"%.3gms",(tt/1000.)); }
	else if (tt<1000000000.) { sprintf(t,"%.3gs",(tt/1000000.)); }
	else                     { sprintf(t,"%.3gs",(tt/1000000.)); }
	return t;
}

#define EXPOSURE_MAX		4999
//static SCROLLBAR sbDefExposure = { 10, 10, 0, 4999, 1, 10, IDC_MV_40_EXPOSURE_SLIDER};
static SCROLLBAR sbDefExposure = {	EXPOSURE_MAX-10, EXPOSURE_MAX-10, 0,
									EXPOSURE_MAX, 1, 10, IDC_MV_40_EXPOSURE_SLIDER};
static SCROLLBAR sbExposure;

BOOL CALLBACK MV40ControlDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
static void m12_16(LPWORD ip, LPWORD op, int n)
{
LPWORD	ep = op+n;
	_asm{
		mov		esi,	[ip]
		mov		edi,	[op]
t10:

		mov		eax,	[esi]
		mov		ecx,	eax
		mov		ebx,	[esi+4]

		shrd	edx,	eax,	4

		shrd	eax,	ebx,	16
		shrd	edx,	eax,	4
		and		eax,	0FFF0FFF0h
		shrd	edx,	ebx,	4
		shrd	edx,	ecx,	16
		and		edx,	0FFF0FFF0h
		mov		[edi],	edx
		mov		[edi+4],eax

		mov		eax,	[esi+8]
		mov		edx,	ebx
		and		ebx,	0FFF00000h
		shr		edx,	12
		mov		ecx,	eax
		and		edx,	00F0h
		shl		ecx,	8
		or		ebx,	edx
		mov		edx,	eax
		and		ecx,	0F00h
		shr		edx,	4
		or		ebx,	ecx
		and		edx,	0F000h
		and		eax,	0FFF0FFF0h
		or		ebx,	edx
		mov		[edi+12],eax
		and		ebx,	0FFF0FFF0h
		add		esi,	12
		mov		[edi+8],ebx
		add		edi,	16
		cmp		edi,	[ep]
		jc		t10
	}
}
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
static void mv40Unpack12( LPWORD pwDst, LPWORD pwSrc, DWORD dwBuffWidth, DWORD dwHeight, DWORD dwPixSize, DWORD dwField1, DWORD dwWidth)
{
DWORD	y;
LPWORD	op, ip;

	for(op = pwDst, y=0; y<dwHeight; y++, op+=dwWidth) {

		if (dwField1) ip = pwSrc + dwBuffWidth * (y/2 + (y&1)*dwField1);
		else          ip = pwSrc + dwBuffWidth * y;

		m12_16(ip, op, dwWidth);
	}
}
/*--------------------------------------------------------------*/
static void DoProcessingMV40( LPWORD pRawData )
{
	bHaveData = FALSE;
	mv40Unpack12(pImageData, pRawData, dwBuffWidth, dwHeight, dwPixSize, dwField1, uBufWidth);
	bHaveData = TRUE;
}
/*--------------------------------------------------------------*/
static HWND hMV40ControlWnd = NULL;

void StopMV40ControlPanel()
{
	SendMessage(hMV40ControlWnd, WM_CLOSE, 0, 0);
//	DestroyWindow(hMV40ControlWnd);
	hMV40ControlWnd = NULL;
}
/*--------------------------------------------------------------*/
void StartMV40ControlPanel()
{
	hMV40ControlWnd = CreateDialogParam(hInst, "MV40DLG", MainhWnd, (DLGPROC)MV40ControlDlgProc, (LPARAM)hwndLive );
	if( NULL != hMV40ControlWnd )
	{
		ShowWindow( hMV40ControlWnd, SW_NORMAL );
		UpdateWindow( hMV40ControlWnd );
	}
}
/*--------------------------------------------------------------*/
void TerminateMV40Capturing()
{
	HCURSOR hoc;

	if( (NULL == hLiveThreadMV40) || (FALSE == bGrabberActive) )
	{
		return;
	}
	hoc = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
	bGrabberActive = FALSE;
	while (WaitForSingleObject( hLiveThreadMV40, 100 ) == WAIT_TIMEOUT)
	{
//		TerminateThread(hLiveThreadMV40, -1);
	}
	hLiveThreadMV40 = NULL;

	if( NULL != hMV40ControlWnd )
	{
		StopMV40ControlPanel();
	}
	SetCursor(hoc);
}
/*--------------------------------------------------------------*/
static VOID _cdecl LiveProcMV40( LPVOID lpv )
{
	try
	{
		OBJ*	O = OBJ::GetOBJ((HWND) lpv);
		HANDLE	hMVlocal = INVALID_HANDLE_VALUE; // must be local!
		DWORD	dwSerNum;
		U32	devices;
		MV40_RETURN	mvret;
		LPWORD	pRawData;
		HCURSOR hoc;
		DWORD	t0, nf;
		char	tmp[100];

	hoc = SetCursor(hcWait);

	if( NULL == O )
		goto err_live;

	if( NULL == (pImageData = (LPWORD)O->dp) )
		goto err_live;

	// ADDED BY PETER on 2003 Jan 30
	O->maxcontr = TRUE;

	bStopGrabbingMV40 = FALSE;
	hwndLive = (HWND)lpv;
	t0 = timeGetTime();
	nf = 0;
	while( bGrabberActive )
	{
		if (hMVlocal == INVALID_HANDLE_VALUE)
		{
			mv40GetNumberDevices( szMV40_Name, &devices );
			// we choosed the device before, we do not need any critical section
			// here since the selectiong of the device was in MODAL dialog
			// and cannot be change during grabbing procedure
			if (nMV40Device < devices)
			{
				mv40GetDevice( nMV40Device, &dwSerNum );
				mv40Initialize( szMV40_Name, dwSerNum, &hMVlocal );
				mv40SetMode(hMVlocal, 1/*MODE*/);
				if (mv40StartImageAcquisition(hMVlocal) != MV40_OK)
					return;
			}
		}
		if ( hMVlocal != INVALID_HANDLE_VALUE )
		{
			while ( 1 )
			{
				if (!bGrabberActive)
					break;
				if( MV40_OK != mv40IsDeviceExist(hMVlocal) )
				{
					mv40Uninitialize( hMVlocal );
					hMVlocal = INVALID_HANDLE_VALUE;
					break;
				}
				if( MV40_OK == (mvret = mv40WaitForImage( hMVlocal, 1000 )) )
					break;
				Sleep(0);
				if ( mvret == MV40_INVALID_HANDLE )
				{
					mv40Uninitialize( hMVlocal );
					hMVlocal = INVALID_HANDLE_VALUE;
					break;
				}
				if ( MV40_NO_IMAGE == mvret  )
					continue;
//				if( MV40_TIMEOUT == mvret )
//				{
//					mv40StopImageAcquisition( hMVlocal );
//					mv40StartImageAcquisition( hMVlocal );
//					break;
//				}
			}
		}

		if (!bGrabberActive)
			break;

		if ( hMVlocal == INVALID_HANDLE_VALUE )
			continue;

		pRawData = NULL;

		if ( hMVlocal != INVALID_HANDLE_VALUE )
		{
			if( MV40_OK != mv40AcquireImage( hMVlocal, (PVOID*)&pRawData ) )
			{
				Beep(1000, 500);
				continue;
			}
		}
		if (pRawData == NULL)
			continue;

		if (1)
		{
			DWORD dwFrameNum = 0;
			if (!mv40IsBufferOK( pRawData, &dwFrameNum ))
			{
				Beep(1000, 10);
				continue;
			}
		}

		DoProcessingMV40( pRawData );

		nf++;
		if(nf == 10)
		{
			t0 = timeGetTime()-t0;
			if (t0)
			{
				sprintf(tmp,"%.1f fps", 1000.*10./(double)t0);
				tobSetInfoText( ID_DDB, tmp );
			}
			t0 = timeGetTime();
			nf = 0;
		}
		if (!bGrabberActive)
			break;

		if( bGrabberActive )
		{
			if (O->maxcontr) imgCalcMinMax( O );
			PostMessage(hwndLive, FM_Update, 0, 0);
		}
	}

	if( FALSE == bStopGrabbingMV40 )
	{
		imgCalcMinMax( O );
		O->maxcontr = TRUE;
		sprintf( O->title, "Grabbed image #%d", O->imageserial );
		sprintf(TMP, "%s - %dx%dx%d (%1d:%1d)",
			O->title,
			O->x,
			O->y,
			8*IMAGE::PixFmt2Bpp(O->npix),
			O->scu,
			O->scd);
		SetWindowText( hwndLive, TMP );
		InvalidateRect( hwndLive, NULL, FALSE );
	}
	O->flags &= ~OF_grabberON;	// Clear grabberON flag
	hwndLive = NULL;
err_live:
	if (hMVlocal != INVALID_HANDLE_VALUE)
	{
		mv40StopImageAcquisition( hMVlocal );
		mv40Uninitialize( hMVlocal );
	}
	Sleep(100);
	hLiveThreadMV40 = NULL;
	SetCursor(hoc);

	} // try
	catch (std::exception& e)
	{
		Trace(_FMT(_T("LiveWndProc() -> %s"), e.what()));
	}
	StopMV40ControlPanel();

	return;
}
/*--------------------------------------------------------------*/
void DoLiveMV40( HWND hWnd )
{
	SIZE	resolution;

	if( (hMV == INVALID_HANDLE_VALUE) ||
		(TRUE != bMV40) ||
		(GB_USE_MV40 != nUseGrabber) )
		return;

	bHaveData = FALSE;

	uiMode = 1;
	mv40SetModeEx( hMV, uiMode, 0, 0, 4096, 4096 );
	mv40GetResolution( hMV, &resolution, &dwBuffWidth, &dwField1, &dwPixSize );
	dwBuffWidth >>= 1;
	dwWidth  = resolution.cx;
	dwHeight = resolution.cy;

//	if (mv40StartImageAcquisition(hMV) != MV40_OK)
//		return;

	uBufWidth = ( dwWidth + 7 ) & (~7);	// rounded up buffer width
//	if( NULL != pImageData )
//		free(pImageData);
//	pImageData = (LPWORD) malloc( uBufWidth * (dwHeight + 2) << 1 );	// Image data buffer, +2 lines for filter routines

	StartMV40ControlPanel();

	hLiveThreadMV40 = (HANDLE)_beginthread( LiveProcMV40,  0, (LPVOID)hWnd );
	if (hLiveThreadMV40 == (HANDLE)-1L)
	{
		hLiveThreadMV40 = NULL;
		return;
	}

//	if( NULL != pImageData )
//		free(pImageData);
	while( !bGrabberActive )//&& !bHalted)
		Sleep(5);
}
/*--------------------------------------------------------------*/
BOOL CALLBACK ChooseGrabberDlgProc(HWND hDlg, UINT message,
								   WPARAM wParam, LPARAM lParam);

static GRABBER nGrabber = GB_USE_NONE;

GRABBER ChooseGrabber()
{
	int res;

	res = DialogBox( hInst, "GRABBERTYPE", MainhWnd, (DLGPROC)ChooseGrabberDlgProc );
	if( -1 == res )
	{
		::OutputDebugString("Failed to create modal dialog window with grabbers list");
		nGrabber = GB_USE_NONE;
	}

	return nGrabber;
}

static void InitGrabberDlg( HWND hDlg )
{
	// set font
#ifdef USE_FONT_MANAGER
	SendDlgItemMessage(hDlg, IDC_GRABBERS_LIST, WM_SETFONT, (WPARAM)FontManager::GetFont(FM_MEDIUM_FONT), 0);
#else
	SendDlgItemMessage(hDlg, IDC_GRABBERS_LIST, WM_SETFONT, (WPARAM)hfMedium, 0);
#endif
	// add all possible found grabbers
	if( TRUE == bShark4 )
	{
		sprintf(TMP, szShark4_Name );
    	SendDlgItemMessage(hDlg, IDC_GRABBERS_LIST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPSTR)TMP);
	}
	if( TRUE == bMV40 )
	{
		U32	devices;
		DWORD dwSerNum;
		// pass through the list of possible devices
		mv40GetNumberDevices( szMV40_Name, &devices );
		if (devices)
		{
			for(U32 i = 0; i < devices; i++)
			{
				mv40GetDevice( i, &dwSerNum );
				sprintf(TMP, "%s, device:%d, s/n:0x%08X", szMV40_Name, i+1, dwSerNum );
		    	SendDlgItemMessage(hDlg, IDC_GRABBERS_LIST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPSTR)TMP);
			}
		}
	}
	// point to none of them
	SendDlgItemMessage( hDlg, IDC_GRABBERS_LIST, LB_SETCURSEL, -1, 0 );
}

void GrabberFromString(char *pString)
{
	if( NULL == pString )
		return;
	if( NULL != strstr(pString, szShark4_Name) )
	{
		nGrabber = GB_USE_SHARK4;
	}
	else if( NULL != strstr(pString, szMV40_Name) )
	{
		nGrabber = GB_USE_MV40;
		// get device number also
		nMV40Device = 0;
		char *ptr = strstr(pString, "device:");
		if( NULL != ptr )
		{
			sscanf(ptr + strlen("device:"), "%d", &nMV40Device);
			nMV40Device--;
		}
	}
}

BOOL CALLBACK ChooseGrabberDlgProc(HWND hDlg, UINT message,
								   WPARAM wParam, LPARAM lParam)
{
	int i;
	switch (message)
	{
	case WM_INITDIALOG:
		InitGrabberDlg( hDlg );
		wndCenter( hDlg, MainhWnd, 0 );
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			i = (int)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
			SendMessage((HWND)lParam, LB_GETTEXT, (WPARAM)i, (LPARAM)TMP);
			GrabberFromString(TMP);
			EndDialog(hDlg, wParam);
			return TRUE;
		case IDCANCEL:
			nGrabber = GB_USE_NONE;
			EndDialog(hDlg, wParam);
			return TRUE;
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		break;
	}
	return FALSE;
}
//****************************************************************************
static	double base_tick = log(0.001);
static	double last_tick = log(4.999);
static	double tick_scale = (sbDefExposure.ma - sbDefExposure.mi) / (last_tick-base_tick);

static void SetExposureTicks( HWND hDlg )
{
	LONG lPosition;
	SendDlgItemMessage( hDlg, IDC_MV_40_EXPOSURE_SLIDER, TBM_CLEARTICS, 0, 0);
//	for(double t = 0.001; t <= 0.501; t += 0.5)
	for(double t = 1.001; t <= 5.0; t += 1.0)
	{
		double tick = log(t);
		lPosition = sbDefExposure.ma - (tick - base_tick) * tick_scale;
		SendDlgItemMessage( hDlg, IDC_MV_40_EXPOSURE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)lPosition);
	}
/*	for(double t = 0.501; t <= 1.001; t += 0.5)
	{
		double tick = log(t);
		lPosition = sbDefExposure.ma - (tick - base_tick) * tick_scale;
		SendDlgItemMessage( hDlg, IDC_MV_40_EXPOSURE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)lPosition);
	}
	for(double t = 1.001; t <= 5.0; t += 0.5)
	{
		double tick = log(t);
		lPosition = sbDefExposure.ma - (tick - base_tick) * tick_scale;
		SendDlgItemMessage( hDlg, IDC_MV_40_EXPOSURE_SLIDER, TBM_SETTIC, (WPARAM)0, (LPARAM)lPosition);
	}
*/
}

// nExposure - in milliseconds
static int ToTick( int nExposure )
{
	double dTick = log(nExposure / 1000.0);
//	double tick_scale = (sbDefExposure.ma - sbDefExposure.mi) / (last_tick-base_tick);
	return int((dTick - base_tick) * tick_scale);
}

static int ToExposure()
{
	int nTick = sbExposure.ma - sbExposure.p;
//	double tick_scale = (sbDefExposure.ma - sbDefExposure.mi) / (last_tick-base_tick);
	return 1000.0 * exp(nTick/tick_scale + base_tick);
}

static void SetExposureText(HWND hDlg)
{
	LPSTR pszText = ExposureTimeText( ToExposure() );
	SendDlgItemMessage(hDlg, IDC_MV_40_EXPOSURE_EDIT, WM_SETTEXT,
		(WPARAM)0, (LPARAM)pszText);
}

void InitMV40ContorlDlg( HWND hDlg )
{
	int nExposure;
	sbExposure = sbDefExposure;
	if (mv40GetExposure( hMV, (LPDWORD)&nExposure ) != MV40_OK)
	{
		Beep( 1000, 300 );
		sbExposure.p = sbExposure.ma;
	}
	sbExposure.p = sbExposure.ma - ToTick(nExposure/100);

	InitSliderCtl( hDlg, IDC_MV_40_EXPOSURE_SLIDER, &sbExposure );
	SetExposureText(hDlg);
	SetExposureTicks(hDlg);
}

BOOL CALLBACK MV40ControlDlgProc(HWND hDlg, UINT message,
								 WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		InitMV40ContorlDlg( hDlg );
		return TRUE;
	case WM_VSCROLL:
	case WM_HSCROLL:
		if( IDC_MV_40_EXPOSURE_SLIDER == GetDlgCtrlID((HWND)lParam) )
		{
			UpdateSliderCtl( hDlg, wParam, lParam, &sbExposure);
			// set the exposure
			if( MV40_OK != mv40IsDeviceExist(hMV) )
			{
				U32 dwSerNum = 0;
				mv40GetDevice( nMV40Device, &dwSerNum );
				mv40Initialize( szMV40_Name, dwSerNum, &hMV );
			}
			mv40SetExposure( hMV, ToExposure()*100 );
			SetExposureText(hDlg);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			nGrabber = GB_USE_NONE;
			DestroyWindow(hDlg);
			return TRUE;
		}
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;
	}
	return FALSE;
}
