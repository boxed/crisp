/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Window management * by ML ************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "objects.h"
#include "commondef.h"
#include "shtools.h"
#include "const.h"
#include "globals.h"
#include "common.h"
#include "resource.h"
#include "fft.h"

#include "numeric_traits.h"

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

/****************************************************************************/
/* Set window zoom and update its caption ***********************************/
/****************************************************************************/
BOOL myError( int id )
{
	LoadString( hInst, id, pTMP, 200);
	MessageBox ( GetFocus(), pTMP, szProg, ((id&0x8000) ? MB_ICONASTERISK : MB_ICONSTOP) | MB_OK);
	return FALSE;
}
/****************************************************************************/
BOOL dlgError( int id, HWND hdlg )
{
	LoadString( hInst, id, TMP, 200);
	MessageBox ( hdlg, TMP, szProg,((id&0x8000) ? MB_ICONASTERISK : MB_ICONSTOP) | MB_OK);
	return FALSE;
}

/****************************************************************************/
double	fftDval( double h, double k, int size, OBJ* O )
{
	return sqrt((double)h*(double)h+(double)k*(double)k)/(double)(size)/(O->NI.Scale);
}
/****************************************************************************/
template <typename DisplayType, typename _Tx>
void ReportXYVal_t(_Tx *ptr, int x, int stride, int y)
{
	ptr = (_Tx*)(((LPBYTE)ptr) + stride * y);
	//sprintf(TMP, "(%d, %d) value=%u", x, y, ptr[x]);
	strcpy(TMP, _FMT(_T("(%d, %d) value=%g"), x, y, (double)(DisplayType)ptr[x]).c_str());
}
/****************************************************************************/
void ReportCoordinates( LPARAM lP, HWND hWnd )
{
	OBJ*	originalObject = OBJ::GetOBJ(hWnd);
	if( NULL == originalObject )
	{
		return;
	}
	OBJ*	pObj = GetBaseObject(originalObject);
	int		x, y;
#ifdef USE_XB_LENGTH
	double	r;
	int		ofs;
#endif
	double	a, p;

	pObj = (originalObject);

	if( pObj == NULL )
		return;
	switch( pObj->ID )
	{
	case OT_AREA:
	case OT_IMG:
	case OT_IFFT:
		if( NULL != pObj->dp )
		{
			x = LOWORD(lP)*pObj->scd/pObj->scu + pObj->xo;
			y = HIWORD(lP)*pObj->scd/pObj->scu + pObj->yo;

			if( (x >= pObj->x) || (y >= pObj->y) ) return;
#ifdef USE_XB_LENGTH
			ofs  = pObj->xb*y + x;

			if (originalObject->ID == OT_AREA)
			{
				POINT p = {x, y};
				ScreenToClient( originalObject->hWnd, &p );

				x = p.x;
				y = p.y;
			}

			switch( pObj->npix )
			{
			case PIX_BYTE:
				{
					r = *(pObj->dp + ofs);
					sprintf(TMP, "(%d,%d) value=%u", x, y, (BYTE)r);
					break;
				}
			case PIX_CHAR:
				{
					r = *(pObj->dp + ofs);
					sprintf(TMP,"(%d,%d) value=%d", x, y, (CHAR)r);
					break;
				}
			case PIX_WORD:
				{
					r = *((LPWORD)(pObj->dp + ofs*2));
					sprintf(TMP,"(%d,%d) value=%u", x, y, (WORD)r);
					break;
				}
			case PIX_SHORT:
				{
					r = *((LPWORD)(pObj->dp + ofs*2));
					sprintf(TMP,"(%d,%d) value=%d", x, y, (SHORT)r);
					break;
				}
			case PIX_DWORD:
				{
					r = *((LPDWORD)(pObj->dp + ofs*4));
					sprintf(TMP,"(%d,%d) value=%u", x, y, (DWORD)r);
					break;
				}
			case PIX_LONG:
				{
					r = *((LPDWORD)(pObj->dp + ofs*4));
					sprintf(TMP,"(%d,%d) value=%d", x, y, (LONG)r);
					break;
				}
			case PIX_FLOAT:
				{
					r = *((float*)(pObj->dp + ofs*4));
					sprintf(TMP,"(%d,%d) value=%g", x, y, r);
					break;
				}
			case PIX_DOUBLE:
				{
					r = *((double*)(pObj->dp + ofs*8));
					sprintf(TMP,"(%d,%d) value=%g", x, y, r);
					break;
				}
			}
#else
			switch( pObj->npix )
			{
			case PIX_BYTE:		ReportXYVal_t<int>((LPBYTE)pObj->dp, x, pObj->stride, y); break;
			case PIX_CHAR:		ReportXYVal_t<int>((LPSTR)pObj->dp, x, pObj->stride, y); break;
			case PIX_WORD:		ReportXYVal_t<int>((LPWORD)pObj->dp, x, pObj->stride, y); break;
			case PIX_SHORT:		ReportXYVal_t<int>((SHORT*)pObj->dp, x, pObj->stride, y); break;
			case PIX_DWORD:		ReportXYVal_t<DWORD>((LPDWORD)pObj->dp, x, pObj->stride, y); break;
			case PIX_LONG:		ReportXYVal_t<LONG>((LPLONG)pObj->dp, x, pObj->stride, y); break;
			case PIX_FLOAT:		ReportXYVal_t<FLOAT>((LPFLOAT)pObj->dp, x, pObj->stride, y); break;
			case PIX_DOUBLE:	ReportXYVal_t<DOUBLE>((LPDOUBLE)pObj->dp, x, pObj->stride, y); break;
			}
#endif
		}
		else
		{
			x = LOWORD(lP) + pObj->xo;
			y = HIWORD(lP) + pObj->yo;
			sprintf(TMP,"(%d,%d)",x,y);
		}
		break;
	case OT_FFT:
		{
			LPFFT F = GetFFT(pObj);

			x = LOWORD(lP)*pObj->scd/pObj->scu + pObj->xo - pObj->x/2;
			y = HIWORD(lP)*pObj->scd/pObj->scu + pObj->yo - pObj->y/2;

			if ((abs(x)>=pObj->x/2)||(abs(y)>=pObj->y/2)) return;
			{FCOMPLEX fontWidth = F->ft2.GetVal(x,y); a = abs(fontWidth); p=atan2(fontWidth); }

			if ( F->flag & FF_RES ) sprintf(TMP,"%5.3f 1/Angst, A=%.0f P=%.f", fftDval(x, y, F->fs, pObj)*1e-10, a/1000., R2D(p) );
			else					sprintf(TMP,"(%d,%d) A=%.0f P=%.f",x,y,a/1000.,R2D(p));

			break;
		}
	}
	tobSetInfoText(ID_INFO, TMP);
}
/****************************************************************************/
/* Set window zoom and update its caption ***********************************/
/****************************************************************************/
void wndSetScrollOrigin( HWND hWnd, BOOL bAround )
{
	int mx, my, x, y;
	OBJ* O = OBJ::GetOBJ( hWnd );

	if (bAround) {
		x = O->clickpoint.x;
		y = O->clickpoint.y;
	} else {
		x = O->x/2;
		y = O->y/2;
	}
	mx = O->x - (O->xw * O->scd + O->scu-1) / O->scu;
	my = O->y - (O->yw * O->scd + O->scu-1) / O->scu;
	if (mx>0 && my>0) {
		O->xo = __max(0,__min(x-((O->xw/2)*O->scd)/O->scu, mx));
		O->yo = __max(0,__min(y-((O->yw/2)*O->scd)/O->scu, my));
	} else {
		O->xo = 0;
		O->yo = 0;
	}
}
/****************************************************************************/
/* Set window zoom and update its caption ***********************************/
/****************************************************************************/
VOID wndSetZoom( HWND hWnd, int scu, int scd )
{
OBJ* O = OBJ::GetOBJ( hWnd );
RECT	rb, rw;
int		xs, ys, __max, may;
int		cxadd, cyadd;

	switch (O->ID) {
	case OT_IMG: case OT_FFT: case OT_IFFT:
		break;
	default: return;
	}
	SetRect(&rw, 0, 0, 0, 0);
	AdjustWindowRectEx( &rw, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
	cxadd = rw.right - rw.left;
	cyadd = rw.bottom - rw.top;

	GetWindowRect(GetParent(hWnd), &rb);
	GetWindowRect(hWnd, &rw);
	if ((__max = rb.right  - rw.left - cxadd) < 0 ) __max = 100;
	if ((may = rb.bottom - rw.top  - cyadd) < 0 ) may = 100;

	O->scd = scd;
	O->scu = scu;

	if (rw.right-rw.left == GetSystemMetrics( SM_CXMIN ))
		xs = O->x*scu/scd;
	else xs = __min(rw.right-rw.left-cxadd, O->x*scu/scd);

	if (rw.bottom-rw.top == GetSystemMetrics( SM_CYMIN ))
		ys = O->y*scu/scd;
	else ys = __min(rw.bottom-rw.top-cyadd, O->y*scu/scd);
	xs = __min(xs,__max);
	ys = __min(ys,may);

	switch(O->ID)
	{
		case OT_IMG:
		case OT_IFFT:
			sprintf(TMP, "%s - %dx%dx%d (%1d:%1d)",
				O->title,
				O->x,
				O->y,
				8*IMAGE::PixFmt2Bpp(O->npix),
				O->scu,
				O->scd);
			break;
		case OT_FFT:
			{
			OBJ* P;
			if (O->ParentWnd && (P=OBJ::GetOBJ(O->ParentWnd))!=NULL)
			{
				sprintf(TMP, "%s from %s, %dx%d (%1d:%1d)",
					O->title,
					P->title,
					O->x,
					O->y,
					O->scu,
					O->scd);
			}
			break;
		}
	}
	SetWindowText( hWnd, TMP);
	if (IsIconic(hWnd)) ShowWindow( hWnd, SW_RESTORE );
	SetWindowPos ( hWnd, 0, 0, 0, xs+cxadd, ys+cyadd, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	SetRect( &rw, 0,0,xs+cxadd,ys+cyadd);
	wndTrackWindow( hWnd, O, &rw );
	wndInitScrollBars( hWnd );
	SetWindowPos( hWnd, NULL,0,0,0,0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_DRAWFRAME );
	wndSetScrollOrigin( hWnd, TRUE );
	InvalidateRect( hWnd, NULL, FALSE);
}
/****************************************************************************/
/****************************************************************************/
void wndSetInitialWindowSize( HWND hWnd )
{
	RECT r;
	int scdx=1,scdy=1;
	OBJ*	pObj = OBJ::GetOBJ( hWnd );

	// Scale window down to fit into current back_hwnd client size
	GetClientRect(GetParent(hWnd), &r);
	pObj->clickpoint.x = pObj->x/2;
	pObj->clickpoint.y = pObj->y/2;
//	SetWindowPos( hWnd, NULL, 0,0,
//				GetSystemMetrics( SM_CXMAXTRACK ),GetSystemMetrics( SM_CYMAXTRACK ),
//				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	if( (pObj->x > r.right) && (r.right > 0) )
	{
		scdx = pObj->x / r.right;
		if( pObj->x % r.right ) scdx++;
	}
	if( (pObj->y > r.bottom) && (r.bottom > 0) )
	{
		scdy = pObj->y / r.bottom;
		if( pObj->y % r.bottom ) scdy++;
	}
	if (pObj->x/scdx > 1024) scdx = (pObj->x+1023)/1024;
	if (pObj->y/scdy > 1024) scdy = (pObj->y+1023)/1024;
	wndSetZoom( hWnd, 1, __max(scdx,scdy) );
}
/****************************************************************************/
/****************************************************************************/
void wndCreateObjMenu( HWND hWnd, BOOL bTune, BOOL bZoom, BOOL bInvert, LPARAM lParam )
{
	HMENU	hm;
	POINT	p;
	OBJ*	O = OBJ::GetOBJ( hWnd );

	O->clickpoint.x = LOWORD(lParam)*O->scd/O->scu + O->xo;
	O->clickpoint.y = HIWORD(lParam)*O->scd/O->scu + O->yo;

	ReplyMessage( 0 );
	if (bZoom) hm = LoadMenu(hInst, "OBJMENUZOOM");
	else       hm = LoadMenu(hInst, "OBJMENU");
	if (!bTune)   DeleteMenu( hm, IDM_OM_TUNE,   MF_BYCOMMAND );
	if (!bInvert) DeleteMenu( hm, IDM_OM_INVERT, MF_BYCOMMAND );
	ModifyMenu( hm, IDM_OM_PALGRAY, MF_BYCOMMAND          | (O->palno==-1?MF_CHECKED:0), IDM_OM_PALGRAY, "Gray" );
	ModifyMenu( hm, IDM_OM_PAL1, MF_BYCOMMAND | MF_STRING | (O->palno== 0?MF_CHECKED:0), IDM_OM_PAL1, szPalName[0] );
	ModifyMenu( hm, IDM_OM_PAL2, MF_BYCOMMAND | MF_STRING | (O->palno== 1?MF_CHECKED:0), IDM_OM_PAL2, szPalName[1] );
	ModifyMenu( hm, IDM_OM_PAL3, MF_BYCOMMAND | MF_STRING | (O->palno== 2?MF_CHECKED:0), IDM_OM_PAL3, szPalName[2] );
	ModifyMenu( hm, IDM_OM_PAL4, MF_BYCOMMAND | MF_STRING | (O->palno== 3?MF_CHECKED:0), IDM_OM_PAL4, szPalName[3] );
	GetCursorPos(&p);
	TrackPopupMenu( GetSubMenu(hm,0), TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL );
	DestroyMenu( hm );
}
/***************************************************************************/
BOOL wndDefSysMenuProc( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	OBJ* O = OBJ::GetOBJ(hWnd);

	if( SC_myZOOM == (wParam & 0xF000) )
	{
		wndSetZoom( hWnd, (wParam>>8)&0xF, (wParam>>4)&0xF );
		return TRUE;
	}

	switch (wParam & 0xFFF0)
	{
		case SC_MAXIMIZE:
			if (IsIconic(hWnd))
				//DefWindowProc(hWnd, WM_SYSCOMMAND, SC_RESTORE, lParam);
				ShowWindow( hWnd, SW_RESTORE );
			if ( O ) {
				RECT r,rp;
				SetRect(&r, 0, 0, O->x * O->scu / O->scd, O->y * O->scu / O->scd);
				AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
				GetClientRect( GetParent(hWnd), &rp );
				SetWindowPos( hWnd, NULL, 0,0,
							  __min(r.right-r.left,rp.right-rp.left),
							  __min(r.bottom-r.top,rp.bottom-rp.top),
							  SWP_NOMOVE|SWP_NOZORDER);
			}
			return TRUE;

/*
		case SC_myINFO: {
			FARPROC lpProc;
			BOOL	ret;
			lpProc = MakeProcInstance(InfoDlgProc, hInst);
			ret = DialogBoxParam(hInst, "Information", hWnd, lpProc, (LPARAM)hWnd );
			FreeProcInstance(lpProc);
			if (ret) InformChilds( hWnd, O, FM_UpdateNI);
			return TRUE;
		}
*/
	}
	switch (wParam)
	{
		case IDM_OM_INVERT:
			{
#ifdef USE_XB_LENGTH
				IMAGE::InvertImage(O->dp, O->npix, O->xb*IMAGE::PixFmt2Bpp(O->npix), O->x, O->y);
#else
				IMAGE::InvertImage(O->dp, O->npix, O->stride, O->x, O->y);
#endif
				imgCalcMinMax( O );
				InvalidateRect( hWnd, NULL, FALSE );
				ActivateObj(hWnd);	// To update all opened info dialogs
				// Added by Peter on 30 Mar 2006
				objInformChildren(hWnd, WM_SYSCOMMAND, wParam, lParam);
				// end of addition by Peter on 30 mar 2006
			}
			return TRUE;
		case IDM_OM_PALGRAY: O->palno = -1; goto redraw;
		case IDM_OM_PAL1:    O->palno =  0; goto redraw;
		case IDM_OM_PAL2:    O->palno =  1; goto redraw;
		case IDM_OM_PAL3:    O->palno =  2; goto redraw;
		case IDM_OM_PAL4:    O->palno =  3; goto redraw;

redraw:		if (O->hbit) { DeleteObject( O->hbit ); O->hbit = NULL; }
			InvalidateRect( hWnd, NULL, FALSE );
			return TRUE;
	}
	return FALSE;
}

static BOOL PreCreateImage(OBJ* pObj, LPVOID pExtraData)
{
	BOOL bRes = FALSE;
	try
	{
		if( (NULL == pExtraData) || (NULL == pObj) )
		{
			throw std::exception("Image or Object pointer is NULL");
		}
		extern BOOL CRISP_InitObjFromImage(OBJ* pObj, SAVEIMAGE &S);
		CRISP_InitObjFromImage(pObj, *(LPSAVEIMAGE)pExtraData);
		bRes = TRUE;
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("PreCreateImage() -> %s"), e.what()));
	}
	return bRes;
}

OBJ* _OpenShellFile(LPSTR pszFileName)
{
	PSTR pStr;
	HWND hWnd;
	OBJ* pObj;
	SAVEIMAGE Img;
	std::vector<OBJ*> vObjects;

	if( FALSE == fioOpenImageFile( pszFileName, NULL, &Img, vObjects ) )
	{
		return NULL;
	}
	// We don't get OpenTitle like we do from GetOpenFileName; need to
	// figure this out for ourselves
	pStr = pszFileName + lstrlen(pszFileName) - 1;

	while( (pStr >= pszFileName) && *pStr != '/' && *pStr != '\\' && *pStr != ':')
		pStr--;

	pStr++;
	hWnd = objCreateObject(CRISP_EMPTY_IMG, NULL, PreCreateImage, &Img);
	if( NULL == hWnd )
	{
		return NULL;
	}
	if( NULL != (pObj = OBJ::GetOBJ(hWnd)) )
	{
		memcpy(&pObj->NI, &NIDefault, sizeof(NIDefault));
		// microscope is not known yet
		pObj->NI.EM_ID = -1;
		strcpy(pObj->title, pStr);
		wndSetInitialWindowSize(hWnd);
		ActivateObj(hWnd);
	}
	return pObj;
}