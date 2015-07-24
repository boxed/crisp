/****************************************************************************/
/* Copyright(c) 1998 SoftHard Technology, Ltd. ******************************/
/****************************************************************************/
/* CRISP2.Histo * by ML******************************************************/
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

#include "numeric_traits.h"

#include "Histogram.h"

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

static SCROLLBAR	SB0_Left  = {   0,  0,   0,   256, 1, 16, 0};	// Left edge
static SCROLLBAR	SB0_Right = {  20, 20,   0,   256, 1, 16, 0};	// Right edge
static SCROLLBAR	SB0_Top   = { 100,100,   0,   100, 1, 10, 0};	// Top edge

/****************************************************************************/
/* Histogram window *********************************************************/
/****************************************************************************/
#define	BRD	0		// Border width, was 4
/****************************************************************************/

void Histogram::UpdateHist(HWND hWnd, OBJ* pObj)
{
	int		x, y, xs, ys;
	OBJ*	par;

	par = OBJ::GetOBJ( pObj->ParentWnd ); // Should exists if it (parent) sends a messages
	Cpyo(pObj, par, x);		Cpyo(pObj, par, y);
	Cpyo(pObj, par, xa);	Cpyo(pObj, par, ya);
	Cpyo(pObj, par, xas);	Cpyo(pObj, par, yas);
	Cpyo(pObj, par, dp);	Cpyo(pObj, par, npix);

	ys = pObj->yas;
	xs = pObj->xas;
	y  = pObj->ya;
	x  = pObj->xa;
	if( x+xs >= pObj->x ) xs = pObj->x - x;
	if( y+ys >= pObj->y ) ys = pObj->y - y;
	if( x < 0 ) { xs += x; x = 0; }
	if( y < 0 ) { ys += y; y = 0; }
	switch( pObj->npix )
	{
	case PIX_BYTE:		UpdateHist_t((LPBYTE)pObj->dp, pObj, x, y, xs, ys); break;
	case PIX_CHAR:		UpdateHist_t((LPSTR)pObj->dp, pObj, x, y, xs, ys); break;
	case PIX_WORD:		UpdateHist_t((LPWORD)pObj->dp, pObj, x, y, xs, ys); break;
	case PIX_SHORT:		UpdateHist_t((SHORT*)pObj->dp, pObj, x, y, xs, ys); break;
	case PIX_DWORD:		UpdateHist_t((LPDWORD)pObj->dp, pObj, x, y, xs, ys); break;
	case PIX_LONG:		UpdateHist_t((LPLONG)pObj->dp, pObj, x, y, xs, ys); break;
	case PIX_FLOAT:		UpdateHist_t((LPFLOAT)pObj->dp, pObj, x, y, xs, ys); break;
	case PIX_DOUBLE:	UpdateHist_t((LPDOUBLE)pObj->dp, pObj, x, y, xs, ys); break;
	}
	if( _left == _right && 0 == _left )
	{
		_left = dMinX;
		_right = dMaxX;
	}
}
/****************************************************************************/
void Histogram::PaintHist(HWND hWnd, OBJ* O)
{
	HDC 	hDC = GetDC( hWnd ), hMemDC;
	int	 	i, x, xs, ys, orop;
	double	l, r;
	OBJ*	pPar = OBJ::GetOBJ(O->ParentWnd);
	double	w, acc;
	RECT	rr;
 	double	scale = dScaleY;
	HBRUSH	hb = CreateSolidBrush( GetSysColor(COLOR_BTNFACE) ), hob;
	HBITMAP	hbmp, hobmp;
	HFONT	hof;
	SIZE	size;
	HPEN	hop;

	hMemDC = CreateCompatibleDC( hDC );
	GetClientRect( hWnd, &rr );
	xs = rr.right-BRD*2;
	ys = rr.bottom-BRD*2;
	if (ys<=BRD*2) ys = BRD*2;
	if (xs<=BRD*2) xs = BRD*2;
	hbmp = CreateCompatibleBitmap( hDC, rr.right, rr.bottom );
	hobmp = SelectBitmap( hMemDC, hbmp );

	hob = SelectBrush( hMemDC, hb );
	PatBlt( hMemDC, 0, 0, rr.right, rr.bottom, PATCOPY);

	PatBlt( hMemDC, BRD, BRD, xs, ys, WHITENESS);

	if( bZoomY ) scale /= dZoomY;

	scale = fabs(scale);
	if( scale > 0. )
	{
		orop = SetROP2( hMemDC, R2_BLACK );
		if( bZoomX )
		{
			l = __max(dMinX, _left);
			r = __min(dMaxX, _right);
			if( r < l )
				r = l + 1;
		}
		else
		{
			l = dMinX;
			r = dMaxX;
		}
		int iLeft = ConvertFromX(l);
		int iRight = ConvertFromX(r);
		w = ((double)xs)/(double)(iRight - iLeft - 1);
// 		w = ((double)xs)/(double)(r - l - 1);
		for(i = iLeft; i < iRight; )
		{
			x = (int)((i-iLeft)*w)+BRD;
			acc = hist[i];
			while (x==(int)((i-iLeft)*w)+BRD && i<iRight)
			{
				acc=__max(acc, hist[i]);
				i++;
			}
			do {
				_MoveTo(hMemDC, x, ys+BRD );
				LineTo(hMemDC, x, __max(BRD,ys-1-(int)( acc / scale * (double)(ys-1))+BRD) );
				x++;
			} while (x < (int)((i+0-iLeft)*w)+BRD);
		}
	}
	if( (NULL != pPar) && (pPar->ID == OT_IMG || pPar->ID==OT_AREA) )
	{
		SetROP2( hMemDC, R2_COPYPEN );
		hop=SelectPen( hMemDC, BluePen );
		x = (pPar->imin - l)*w+BRD+w/2;
		if (x>=0 && x<xs)
		{
			_MoveTo(hMemDC, x, ys+BRD );
			LineTo(hMemDC, x,    BRD );
		}
		SelectObject( hMemDC, RedPen );
		x = (pPar->imax - l)*w+BRD+w/2;
		if (x>=0 && x<xs)
		{
			_MoveTo(hMemDC, x, ys+BRD );
			LineTo(hMemDC, x,    BRD );
		}
		SelectObject( hMemDC, hop );
	}

	SetROP2( hMemDC, orop );
	SetBkMode( hMemDC, TRANSPARENT );
	SetTextColor( hMemDC, RGB(255,0,0));
	hof = SelectFont( hMemDC, hfInfo );

	sprintf(TMP, "%g", l);
	GetTextExtentPoint( hMemDC, TMP, strlen(TMP), &size );
	TextOut( hMemDC, 5, rr.bottom-5-size.cy, TMP, strlen(TMP) );

	sprintf(TMP, "%g", r);
	GetTextExtentPoint( hMemDC, TMP, strlen(TMP), &size );
	TextOut( hMemDC, rr.right-5-size.cx, rr.bottom-5-size.cy, TMP, strlen(TMP) );

	if( scale > 0 )
	{
		// Smoothing performed on 9 pixels, so divide scale in this case
		if (bZoomY) sprintf(TMP, "%d%%, %d", (int)(100/dZoomY), bSmooth?(int)(scale/9):(int)scale );
		else           sprintf(TMP, "%d%%, %d", 100           , bSmooth?(int)(scale/9):(int)scale );
		TextOut( hMemDC, 5, 5, TMP, strlen(TMP) );
	}
	sprintf(TMP,"RMS=%.1f", rms);				TextOut( hMemDC, 5, 5+size.cy  , TMP, strlen(TMP) );
	sprintf(TMP,"MAD=%.1f", dva);				TextOut( hMemDC, 5, 5+size.cy*2, TMP, strlen(TMP) );
	sprintf(TMP,"MEAN=%g", mean);				TextOut( hMemDC, 5, 5+size.cy*3, TMP, strlen(TMP) );
	sprintf(TMP,"FWID=%d", ConvertX(fwidth));	TextOut( hMemDC, 5, 5+size.cy*4, TMP, strlen(TMP) );

	SelectObject( hMemDC, hof );

	BitBlt( hDC, 0,0, rr.right,rr.bottom, hMemDC, 0,0, SRCCOPY );

	SelectObject( hMemDC, hobmp );
	SelectObject( hMemDC, hob );
	DeleteDC( hMemDC );
	DeleteObject( hbmp );
	DeleteObject( hb );
	ReleaseDC(hWnd, hDC);
}
/****************************************************************************/
static BOOL CALLBACK HistDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND ParentWnd = (HWND)GetWindowLong( hDlg, DWL_USER );
	OBJ*	O;
	LPHIST H = NULL;
	if (ParentWnd)
	{
		O = OBJ::GetOBJ(ParentWnd);
		H = GetHIST(O);
	}

	switch (message)
	{
		case WM_INITDIALOG:
			O = OBJ::GetOBJ((HWND)lParam);
			H = GetHIST(O);
			SetWindowLong( hDlg, DWL_USER, lParam);
//			InitSliderCtl( hDlg, IDC_FT_PAR, &S->SB_Par);
//			InitSliderCtl( hDlg, IDC_FT_LIN, &S->SB_Lin);
//			InitSliderCtl( hDlg, IDC_FT_DC,  &S->SB_DC);
//			InitSliderCtl( hDlg, IDC_FT_SC,  &S->SB_Sc);
//			CheckDlgButton( hDlg, IDC_FT_LOG,  S->flag & FF_Log );
//			CheckDlgButton( hDlg, IDC_FT_RESL, S->flag & FF_Res );
			wndPlaceToolWindow( (HWND)lParam, hDlg );
			return TRUE;
/*
		case WM_HSCROLL:
			switch (GetDlgCtrlID ( (HWND)lParam ))
			{
				case IDC_FT_PAR: UpdateSliderCtl ( hDlg, wParam,lParam, &S->SB_Par); break;
				case IDC_FT_LIN: UpdateSliderCtl ( hDlg, wParam,lParam, &S->SB_Lin); break;
				case IDC_FT_DC:  UpdateSliderCtl ( hDlg, wParam,lParam, &S->SB_DC);  break;
				case IDC_FT_SC:  UpdateSliderCtl ( hDlg, wParam,lParam, &S->SB_Sc);  break;
			}
*/
//			ReplyMessage( TRUE );
//			if(wParam == SB_THUMBTRACK) return TRUE;
//			if (!GetAsyncKeyState(VK_LBUTTON)) {
//			return TRUE;

		case WM_DESTROY:
			O->hDlg = 0;
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					O->hDlg = 0;
					DestroyWindow( hDlg );
					return TRUE;
/*
				case IDC_FT_DEF:
					S->SB_DC.p  = S->SB_DC.def;  S->SB_Sc.p  = S->SB_Sc.def;
					S->SB_Lin.p = S->SB_Lin.def; S->SB_Par.p = S->SB_Par.def;
					InitSliderCtl( hDlg, IDC_FT_PAR, &S->SB_Par);
					InitSliderCtl( hDlg, IDC_FT_LIN, &S->SB_Lin);
					InitSliderCtl( hDlg, IDC_FT_DC,  &S->SB_DC);
					InitSliderCtl( hDlg, IDC_FT_SC,  &S->SB_Sc);
					goto xx;
*/			}
			break;
		case WM_ACTIVATE:
			if (ParentWnd && LOWORD(wParam)==WA_CLICKACTIVE) ActivateObj( ParentWnd );
			break;

	}
	return FALSE;
}
/****************************************************************************/
BOOL InitHistObject( HWND hWnd, OBJ* O )
{
	RECT	r;
	int		hsize;
	LPHIST	H = GetHIST(O);

	switch( O->npix )
	{
	case PIX_BYTE:
	case PIX_CHAR:
		{
			hsize = 256;
			break;
		}
	case PIX_WORD:
	case PIX_SHORT:
	case PIX_DWORD:
	case PIX_LONG:
	case PIX_FLOAT:
	case PIX_DOUBLE:
		{
			hsize = 65536;
			break;
		}
	default: return FALSE;
	}
	// Histo data memory
	O->bp = (LPBYTE)calloc(hsize, sizeof(long));
	if( NULL == O->bp )
	{
		return FALSE;
	}
	memset(O->bp, 0, hsize * sizeof(long));
	H->hist = (LPLONG)O->bp;
	H->_left = 0;
	H->_right = 0;
	H->dZoomY = 1;
	H->bZoomX = FALSE;
	H->bZoomY = FALSE;
	H->hsize = hsize;
	H->bSmooth = FALSE;

	SetRect(&r, 0, 0, 256+BRD*2, 192+BRD*2);
	AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
	SetWindowPos ( hWnd, 0, 0, 0, r.right-r.left, r.bottom-r.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	return	TRUE;
}
/****************************************************************************/
void Histogram::TrackHistoMenu(HWND hWnd)
{
	HMENU	hm;
	POINT	p, p1;

	ReplyMessage( 0 );
	hm = LoadMenu(hInst, "HISTO");
	GetCursorPos(&p);
	p1 = p;
	ScreenToClient(hWnd, &p1);
	this->x = p1.x-BRD;
	this->y = p1.y-BRD;

	TrackPopupMenu( GetSubMenu(hm,0), TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL );
	DestroyMenu( hm );
}
/****************************************************************************/
LRESULT CALLBACK HistWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ(hWnd);
	LPHIST	H;
	if( O != NULL )
	{
		H = GetHIST(O);
	}
	switch(message)
	{
	case WM_CREATE:
		if( !InitHistObject( hWnd, O ) )
			return -1;

	// Continue with update
	case FM_Update:
		H->UpdateHist(hWnd, O);
		InvalidateRect(hWnd, NULL, 0);
		UpdateWindow(hWnd);
		break;

	case WM_NCDESTROY:
		objFreeObj( hWnd );
		goto CallDCP;

	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
    	objInformChildren( hWnd, FM_UpdateNI, wParam, lParam);
    	break;

	case WM_DESTROY:
		if( NULL != O->bp )
		{
			free( O->bp );
		}
		O->bp = NULL;
		H->hist = NULL;
		break;

	case WM_SETCURSOR:
		if (LOWORD(lParam)==HTCLIENT)
		{
			SetCursor( hcHeight );
			return TRUE;
		}
		goto CallDCP;

	case WM_PAINT:
		H->PaintHist( hWnd, O );
		ValidateRect( hWnd, NULL );
		break;

	case WM_SIZE:
		InvalidateRect( hWnd, NULL, FALSE );
		H->xw = LOWORD(lParam)-BRD*2;
		H->yw = HIWORD(lParam)-BRD*2;
		break;

	case WM_MOUSEMOVE:
	{
		int		xpos = LOWORD(lParam) - BRD;
		double	x;
		LPLONG	his = H->hist;
		if( H->xw <= 0 ) break;
		if( H->bZoomX )
		{
			x = xpos * (H->_right - H->_left)/(double)H->xw + H->_left;
			if (x<H->_left || x>H->_right) break;
		}
		else
		{
 			x = xpos * (H->dMaxX - H->dMinX)/(double)H->xw + H->dMinX;
		}
		xpos = (x - H->dMinX) * H->hsize / (H->dMaxX - H->dMinX);
//		xpos = H->ConvertFromX(x);
		sprintf(TMP, "HIS:%d - %3d%%", (int)x, (int)((double)his[xpos]/fabs(H->dScaleY)*100.));
		tobSetInfoText( ID_INFO, TMP);
	}
		break;


	case WM_RBUTTONUP:
		H->TrackHistoMenu(hWnd);
		return TRUE;

	case WM_INITMENU:
		CheckMenuItem( (HMENU)wParam, IDHM_ZOOMX,  MF_BYCOMMAND | (H->bZoomX  ?MF_CHECKED:MF_UNCHECKED) );
		CheckMenuItem( (HMENU)wParam, IDHM_ZOOMY,  MF_BYCOMMAND | (H->bZoomY  ?MF_CHECKED:MF_UNCHECKED) );
		CheckMenuItem( (HMENU)wParam, IDHM_SMOOTH, MF_BYCOMMAND | (H->bSmooth ?MF_CHECKED:MF_UNCHECKED) );
		break;

	case WM_COMMAND:
		{
			int xx=0;
			double zz=-1;
			if( H->xw > 0)
			{
				if( H->bZoomX ) xx = (int)(H->x*(double)(H->_right - H->_left)/(double)H->xw + H->_left);
				else           xx = (int)(H->x*(double)(H->dMaxX - H->dMinX)/(double)H->xw + H->dMinX);
			}
			if (H->yw>0 && H->y>0 && H->y<H->yw)
			{
				if (H->bZoomY) zz = H->dZoomY*(double)H->yw/(double)(H->yw-H->y);
				else           zz =           (double)H->yw/(double)(H->yw-H->y);
			}
			switch(LOWORD(wParam))
			{
			case IDHM_SMOOTH: H->bSmooth = !H->bSmooth; H->UpdateHist( hWnd, O); break;
			case IDHM_FULL: H->bZoomX = FALSE; H->bZoomY=FALSE; H->_left=0; H->_right=H->hsize; H->dZoomY = 1; break;
			case IDHM_ZOOMX: H->bZoomX = !H->bZoomX; break;
			case IDHM_ZOOMY: H->bZoomY = !H->bZoomY; break;
			case IDHM_LEFT:  if (xx>0) { H->bZoomX = TRUE; H->_left = xx;  } break;
			case IDHM_RIGHT: if (xx>0) { H->bZoomX = TRUE; H->_right = xx; } break;
			case IDHM_TOP:   if (zz>0) { H->bZoomY = TRUE; H->dZoomY = zz;} break;
			case IDHM_TUNE:
				if  (O->hDlg)  BringWindowToTop(O->hDlg);
				else O->hDlg = CreateDialogParam(hInst, "HIST", MainhWnd, HistDlgProc, (LPARAM)hWnd);
				break;
			}
			if( H->_left > H->_right )
			{
				xx = H->_left;
				H->_left = H->_right;
				H->_right = xx;
			}
			InvalidateRect( hWnd, NULL, FALSE );
		}
		return TRUE;
	case WM_ERASEBKGND:
		return TRUE;

	case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS wp = (LPWINDOWPOS)lParam;
			// Setting this bit prevents(?) BitBlt while moving
			wp->flags |= SWP_NOCOPYBITS;
		}
		goto CallDCP;

	case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
	case WM_QUERYOPEN:
	case WM_MOUSEACTIVATE:
		ActivateObj( hWnd );
		goto CallDCP;

	default:
CallDCP:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
