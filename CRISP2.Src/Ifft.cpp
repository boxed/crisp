/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.IFFT * by ML*******************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"

#include "fft.h"
#pragma optimize ("",off)
/****************************************************************************/
/* IFFT object **************************************************************/
/****************************************************************************/
static void UpdateIFFT( HWND hWnd, OBJ* O )
{
	OBJ*	pr  = OBJ::GetOBJ(O->ParentWnd);
	LPFFT	S   = GetFFT(O);
	LPFFT	Spr = GetFFT(pr);

	S->ft2 = Spr->ft2;	// Copy of FFT
	S->ft2.IFT();		// IFFT' it

	switch(S->npix)
	{
	case PIX_BYTE:
		{
			BM8	bmdst(S->fs,S->fs,O->dp);
			S->ft2.DisplayIFT(bmdst);
			bmdst.P = NULL;
		}
		break;
	case PIX_CHAR:
		{
			BM8S	bmdst(S->fs,S->fs, (LPSTR)O->dp);
			S->ft2.DisplayIFT(bmdst);
			bmdst.P = NULL;
		}
		break;
	case PIX_WORD:
		{
			BM16	bmdst(S->fs,S->fs,(LPWORD)O->dp);
			S->ft2.DisplayIFT(bmdst);
			bmdst.P = NULL;
		}
		break;
	case PIX_SHORT:
		{
			BM16S	bmdst(S->fs,S->fs,(SHORT*)O->dp);
			S->ft2.DisplayIFT(bmdst);
			bmdst.P = NULL;
		}
		break;
	case PIX_DWORD:
		{
			BM32	bmdst(S->fs,S->fs,(LPDWORD)O->dp);
			S->ft2.DisplayIFT(bmdst);
			bmdst.P = NULL;
		}
		break;
	case PIX_LONG:
		{
			BM32S	bmdst(S->fs,S->fs,(LPLONG)O->dp);
			S->ft2.DisplayIFT(bmdst);
			bmdst.P = NULL;
		}
		break;
	case PIX_FLOAT:
		break;
	case PIX_DOUBLE:
		break;
	}

	InvalidateRect(hWnd, NULL, FALSE);
}
/****************************************************************************/
BOOL	InitIFFTObject( HWND hWnd, OBJ* O )
{
	OBJ*	pr  = OBJ::GetOBJ(O->ParentWnd);
	LPFFT	S   = GetFFT(O);
	LPFFT	Spr = GetFFT(pr);
/*
	LPFFT	Spr;
	HWND	hWndFFT;

	hWndFFT = objFindParent( O, OT_FFT );
	OBJ* pFft = OBJ::GetOBJ( hWndFFT );
	Spr = GetFFT(pFft);
*/
	S->src = S->img = NULL;
	S->fs = Spr->fs;
	S->ft2.FT2R::FT2R(S->fs);	// FFT data

	O->bp  = O->dp  = NULL;	// To avoid freeing of parent memory which handles was copied
	O->ebp = O->edp = NULL;
//	O->npix = (1 == S->pix) ? PIX_BYTE : PIX_WORD;		// Pixel depth from the parent's FFT object
	O->npix = S->npix;

// FFT bitmap memory
	if ((O->dp = (LPBYTE)calloc(1, S->fs*S->fs*IMAGE::PixFmt2Bpp(O->npix))) == NULL) return FALSE;
	UpdateIFFT( hWnd, O );
	return TRUE;
}
/****************************************************************************/
LRESULT CALLBACK IFFTWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ(hWnd);
	LPFFT	S;

/*
	HWND hWndFFT = objFindParent( O, OT_FFT );
	OBJ* pFft = OBJ::GetOBJ( hWndFFT );
	S = GetFFT(pFft);
*/
	if (O)
		S = GetFFT(O);

	switch(message)
	{
	case WM_CREATE:
		if ( ! InitIFFTObject( hWnd, O )) return -1;
		wndSetInitialWindowSize( hWnd );
		break;

	case WM_NCDESTROY:
		// TODO: check this!!!
//		S->ft2.FT2R::~FT2R(); // Free FFT storage
		if (O->dp) free( O->dp ); O->dp=NULL;
		objFreeObj( hWnd );
		break;

	case WM_MOUSEMOVE:
		ReportCoordinates( lParam, hWnd );
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if (wParam & (MK_LBUTTON | MK_MBUTTON)) objInformChildren(hWnd, FM_ParentInspect, wParam, lParam);
		break;

	case WM_RBUTTONUP:
		wndCreateObjMenu(hWnd, FALSE, TRUE, FALSE, lParam);
		break;

	case WM_HSCROLL:
		wndUpdateScrollBars( hWnd, wParam, lParam, SB_HORZ);
		objInformChildren( hWnd, FM_ParentSize, (WPARAM)O, 0);
		break;

	case WM_VSCROLL:
		wndUpdateScrollBars( hWnd, wParam, lParam, SB_VERT);
		objInformChildren( hWnd, FM_ParentSize, (WPARAM)O, 0);
		break;

	case WM_NCCALCSIZE:
		if ( O && wParam ) wndTrackWindow( hWnd, O, (LPRECT)lParam );
		goto CallDCP;

	case WM_SIZE:
		// Update new window size in case of "normal" sizes
		DefWindowProc(hWnd, message, wParam, lParam);
		if ((wParam == SIZENORMAL)||(wParam == SIZEFULLSCREEN))
		{
			O->xw = LOWORD(lParam);
			O->yw = HIWORD(lParam);
			O->xo = (O->x - O->xw * O->scd / O->scu)/2;	// Centering picture
			O->yo = (O->y - O->yw * O->scd / O->scu)/2;
			wndInitScrollBars( hWnd );
			objInformChildren( hWnd, FM_ParentSize, (WPARAM)O, 0);
		}
		break;

	case WM_ERASEBKGND:
		return FALSE;

	case WM_GETMINMAXINFO:
		// Max track size
		if ( O ) {
			RECT r;
			SetRect(&r, 0, 0, O->x * O->scu / O->scd, O->y * O->scu / O->scd);
			AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
			((LPPOINT)lParam)[4].x = r.right-r.left;
			((LPPOINT)lParam)[4].y = r.bottom-r.top;
		}
		break;

	case WM_SYSCOMMAND:
		if (wndDefSysMenuProc(hWnd, wParam, lParam)) break;
		goto CallDCP;

	case WM_COMMAND:
		if (wndDefSysMenuProc(hWnd, wParam, lParam)) break;
		goto CallDCP;

	case FM_Update:
		UpdateIFFT( hWnd, O );
		objInformChildren( hWnd, message, wParam, lParam);
    	break;

	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
		objInformChildren( hWnd, message, wParam, lParam);
    	break;

	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
	case WM_QUERYOPEN:
		ActivateObj(hWnd);
		goto CallDCP;

	case WM_PALETTECHANGED:
		if (hWnd==(HWND)wParam) break;
		{
		HDC hDC;
		HPALETTE hOldPal;
		hDC     = GetDC (hWnd);
		hOldPal = SelectPalette (hDC, pntGetPal(), TRUE);

		RealizePalette (hDC);
		UpdateColors (hDC);

		if (hOldPal) SelectPalette (hDC, hOldPal, FALSE);

		ReleaseDC (hWnd, hDC);
		}
		break;

	case WM_PAINT:
		PaintImage(hWnd, O);
		objInformChildren( hWnd, FM_ParentPaint, (WPARAM)O, 0 );
		break;

	case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS wp = (LPWINDOWPOS)lParam;
			// Setting this bit prevents(?) BitBlt while moving
			wp->flags |= SWP_NOCOPYBITS;
		}
		goto CallDCP;

	case WM_INITMENUPOPUP:
		if (HIWORD(lParam)==0)
			CheckMenuItem((HMENU)wParam, SC_myZOOM | (O->scu<<8) | (O->scd<<4), MF_BYCOMMAND | MF_CHECKED);
		break;

	default:
CallDCP:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return FALSE;
}
