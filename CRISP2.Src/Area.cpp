/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.Area * by ML ******************************************************/
/****************************************************************************/

#include "stdafx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"

#define	BOUND	4

/****************************************************************************/
void	UpdateArea( HWND hWnd, OBJ* O)
{
int		x,y,xs,ys;
OBJ*	par;

	par = OBJ::GetOBJ( O->ParentWnd ); // Should exists if it (parent) sends a messages
// Get new values from parent
	Cpyo(O, par, x);	Cpyo(O, par, y);
	Cpyo(O, par, xw);	Cpyo(O, par, yw);
	Cpyo(O, par, xo);	Cpyo(O, par, yo);
	Cpyo(O, par, scu);	Cpyo(O, par, scd);
	Cpyo(O, par, dp);
	Cpyo(O, par, NI);
	Cpyo(O, par, npix);
	Cpyo(O, par, imin);
	Cpyo(O, par, imax);
// Convert sizes and coord. to screen values
	x  = (O->xa - O->xo) * O->scu / O->scd;
	y  = (O->ya - O->yo) * O->scu / O->scd;
	xs = O->xas * O->scu / O->scd;
	ys = O->yas * O->scu / O->scd;
	SetWindowPos ( hWnd, 0, x, y, xs, ys, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
}
/****************************************************************************/
void PaintAreaBox( HWND hWnd, OBJ* O, BOOL active )
{
HPEN	hop, ar_pen;
HBRUSH	hob, ar_brush;
HDC		hdc;
int		rop, x,y,xs,ys, xm, ym;
RECT	r;

	if ( ! IsWindow(hWnd) ) return;
	ar_pen = CreatePen( PS_SOLID, 1, RGB(0,0,255));
	ar_brush = CreateSolidBrush( RGB(0,0,255));
	GetWindowRect( hWnd, &r);
	xs = r.right-r.left-1;
	ys = r.bottom-r.top-1;
	x = 0;
	y = 0;
	xm = x+(xs+1-BOUND)/2;
	ym = y+(ys+1-BOUND)/2;

	if ((hdc = GetWindowDC( hWnd ))!=NULL) {
		hop = SelectPen(hdc, ar_pen);
		hob = SelectBrush(hdc, (HBRUSH)GetStockObject( NULL_BRUSH) );
		if (active)	rop = SetROP2( hdc, R2_COPYPEN );
		else		rop = SetROP2( hdc, R2_WHITE );
		if (O->dummy[0]==0) {
			Rectangle( hdc, x,y,x+xs+1,y+ys+1 );
			SetROP2( hdc, R2_BLACK );
			Rectangle( hdc, x+1,y+1,x+xs,y+ys );
			if(active)	SetROP2( hdc, R2_COPYPEN ); // For markers, if any
			else		SetROP2( hdc, R2_WHITE );   // For markers, if any

			if ( O->ui == 0 ) {	// If free area size then drawing size change markers
				hob = SelectBrush(hdc, ar_brush );
				Rectangle( hdc, x+1,   y+1,  x+1+BOUND,  y+1+BOUND);
				Rectangle( hdc, x+xs,  y+1,  x+xs-BOUND, y+1+BOUND);
				Rectangle( hdc, x+xs,  y+ys, x+xs-BOUND, y+ys-BOUND);
				Rectangle( hdc, x+1,   y+ys, x+1+BOUND,  y+ys-BOUND);

				Rectangle( hdc, xm,    y+1,  xm+BOUND,   y+BOUND+1);
				Rectangle( hdc, xm,    y+ys, xm+BOUND,   y+ys-BOUND);
				Rectangle( hdc, x+1,   ym,   x+1+BOUND,  ym+BOUND);
				Rectangle( hdc, x+xs,  ym,   x+xs-BOUND, ym+BOUND);
			}
		} else {
			Ellipse( hdc, x,y,x+xs+1,y+ys+1 );
			Ellipse( hdc, x+1,y+1,x+xs,y+ys );
		}

		SetROP2( hdc, rop );
		if (hop) SelectPen(hdc, hop);
		if (hob) SelectBrush(hdc, hob);
		ReleaseDC( hWnd, hdc );
	}
	DeletePen( ar_pen );
	DeleteBrush( ar_brush );
}
/****************************************************************************/
BOOL InRect( int x0, int y0, int x1, int y1, LPPOINT p)
{
	if ( p->x>=x0 && p->x<=x1 && p->y>=y0 && p->y<=y1 ) return TRUE;
	else return FALSE;
}
/****************************************************************************/
/* Where in AREA window? All in screen coordinates !!!						*/
/****************************************************************************/
LONG	AreaHitTest( HWND hWnd, OBJ* O, LPARAM lParam)
{
RECT	r;
POINT	p;
int		xs, ys, x, y, xm, ym;

	p.x = LOWORD(lParam);
	p.y = HIWORD(lParam);
	if (O->ui) return HTCAPTION; /* Area with fixed size i.e. always in caption */

	GetWindowRect( hWnd, &r);
	xs = r.right -r.left-1;
	ys = r.bottom-r.top -1;
	x = r.left;
	y = r.top;
	xm = x+(xs+1-BOUND)/2;
	ym = y+(ys+1-BOUND)/2;

    if (InRect( x+1,    y+1,    x+1+BOUND,  y+1+BOUND, &p)) return HTTOPLEFT;
	if (InRect( x+xs-BOUND, y+1,    x+xs,   y+1+BOUND, &p)) return HTTOPRIGHT;
	if (InRect( x+xs-BOUND, y+ys-BOUND, x+xs,   y+ys,  &p)) return HTBOTTOMRIGHT;
	if (InRect( x+1,    y+ys-BOUND, x+1+BOUND,  y+ys,  &p)) return HTBOTTOMLEFT;

	if (InRect( xm,     y+1,    xm+BOUND,   y+BOUND+1, &p)) return HTTOP;
	if (InRect( xm,     y+ys-BOUND, xm+BOUND,   y+ys,  &p)) return HTBOTTOM;
	if (InRect( x+1,    ym,     x+1+BOUND,  ym+BOUND,  &p)) return HTLEFT;
	if (InRect( x+xs-BOUND, ym,     x+xs,   ym+BOUND,  &p)) return HTRIGHT;

	return	HTCAPTION;
}
/****************************************************************************/
LRESULT CALLBACK AreaWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ(hWnd);

	switch(message)
	{
	case WM_CREATE:
//		EnableFFTbutton( O );
		UpdateArea( hWnd, O );
		break;

	case WM_NCDESTROY:
		objFreeObj( hWnd );
		break;

	case WM_PAINT:
		ValidateRect( hWnd, NULL);
		return 0;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_GETMINMAXINFO:
		// Set minimum track size to 16
		((LPPOINT)lParam)[3].x = 16;
		((LPPOINT)lParam)[3].y = 16;
		// Set maximum track size to 32000
		((LPPOINT)lParam)[4].x = 32000;
		((LPPOINT)lParam)[4].y = 32000;
		return 0;

	case WM_NCPAINT:
		PaintAreaBox( hWnd, O , hActive==hWnd);
		return 0;

	case FM_Update:
		UpdateArea( hWnd, O );
		objInformChildren( hWnd, FM_Update, (WPARAM)O, 0);
		break;
	case FM_UpdateNI:
		O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
    	objInformChildren( hWnd, FM_UpdateNI, wParam, lParam);
    	break;
	case FM_ParentPaint:
		PaintAreaBox( hWnd, O , hActive==hWnd);
		break;

	case FM_ParentSize:
		UpdateArea( hWnd, O );
		break;

	case WM_NCACTIVATE:
		InvalidateRect( O->ParentWnd, NULL, 0 );
		return TRUE;

	case WM_NCHITTEST:
		{ POINT p = {LOWORD(lParam),HIWORD(lParam)};
		  WPARAM wp;
			ScreenToClient( O->ParentWnd, &p );
			wp = GetAsyncKeyState(VK_LBUTTON)?MK_LBUTTON:0 |
				 GetAsyncKeyState(VK_RBUTTON)?MK_RBUTTON:0 |
				 GetAsyncKeyState(VK_MBUTTON)?MK_MBUTTON:0 |
				 GetAsyncKeyState(VK_SHIFT)  ?MK_SHIFT:0 |
				 GetAsyncKeyState(VK_CONTROL)?MK_CONTROL:0;
			SendMessage( O->ParentWnd, WM_MOUSEMOVE, wp, MAKELONG((WORD)p.x, (WORD)p.y));
		}
		return AreaHitTest(hWnd, O, lParam);

	case WM_MOUSEMOVE:
	case WM_NCMOUSEMOVE:
		{
			// Added the point conversion to Client coordinates by Peter on 21 Mar 2006
			POINT p = {LOWORD(lParam),HIWORD(lParam)};
			//ScreenToClient( O->hWnd, &p );
			// end of addition by Peter on 21 Mar 2006
			ReportCoordinates( MAKELPARAM(p.x, p.y), O->hWnd );
		}
		break;

	case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS wp = (LPWINDOWPOS)lParam;
			int		x,y;

			// Setting this bit prevents(?) BitBlt while moving
			wp->flags |= SWP_NOCOPYBITS;
			// If nither moving nor sizing was happend, so nothing to remember
//				if ((wp->flags & (SWP_NOMOVE | SWP_NOSIZE)) == (SWP_NOMOVE | SWP_NOSIZE)) break;

			 // Patch to avoid recalculations while parent zooming or scrolling
			if (wp->flags & SWP_NOREDRAW) break;

			// Converting into real coordinates
			x  = wp->x  / O->scu * O->scd + O->xo;
			// Bounding
			if (x > O->x-10) x = O->x-10; if (x<0) x = 0;

			y  = wp->y  / O->scu * O->scd + O->yo;
			// Bounding
			if (y > O->y-10) y = O->y-10; if (y<0) y = 0;

			// Back to screen
			wp->x = (x - O->xo) * O->scu / O->scd;
			wp->y = (y - O->yo) * O->scu / O->scd;
		}
		return 1;

	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS wp = (LPWINDOWPOS)lParam;
			int		x,y,xs,ys;

			// Setting this bit prevents(?) BitBlt while moving
			wp->flags |= SWP_NOCOPYBITS;
			// If nither moving nor sizing was happend, so nothing to remember
//				if ((wp->flags & (SWP_NOMOVE | SWP_NOSIZE)) == (SWP_NOMOVE | SWP_NOSIZE)) break;

			 // Patch to avoid recalculations while parent zooming or scrolling
			if (wp->flags & SWP_NOREDRAW) break;
			x = O->xa; y = O->ya; xs = O->xas; ys = O->yas;

			// Converting into real coordinates
			O->xas = wp->cx / O->scu * O->scd;
			O->xa  = wp->x  / O->scu * O->scd + O->xo;
			// Bounding
			if (O->xa<0) O->xa = 0;


			O->yas = wp->cy / O->scu * O->scd;
			O->ya  = wp->y  / O->scu * O->scd + O->yo;
			// Bounding
			if (O->ya<0) O->ya = 0;

			// Back to screen
			wp->x = (O->xa - O->xo) * O->scu / O->scd;
			wp->y = (O->ya - O->yo) * O->scu / O->scd;
			if (x != O->xa || y != O->ya || xs != O->xas || ys != O->yas ) {
				objInformChildren( hWnd, FM_Update, 0, 0);
				sprintf(TMP, "Area %dx%d at (%d,%d)", O->xas, O->yas, O->xa, O->ya);
				tobSetInfoText(ID_INFO, TMP);
			}
		}
		return 0;

	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
	case WM_QUERYOPEN:
		ActivateObj( hWnd );
		return TRUE;

	case WM_NCRBUTTONUP:
	case WM_RBUTTONUP:
		ReplyMessage(DefWindowProc(hWnd, message, wParam, lParam));
		TrackPopupMenu( GetSystemMenu(hWnd, FALSE), 0, LOWORD(lParam), HIWORD(lParam), 0, hWnd, NULL);
		SetWindowPos( O->ParentWnd, NULL, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		return TRUE;

	case WM_INITMENU:
		CheckMenuItem( (HMENU)wParam, SC_myCIRCLE, MF_BYCOMMAND | (O->dummy[0]?MF_CHECKED:MF_UNCHECKED) );
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case SC_myCIRCLE:
				O->dummy[0] = ~O->dummy[0];
				InvalidateRect( O->ParentWnd, NULL, FALSE );
				objInformChildren( hWnd, FM_Update, 0, 0);
			return 0;

// Redirect all SC_xx commands to Def
			case SC_CLOSE:
			case SC_MOVE:
			case SC_SIZE:
				message = WM_SYSCOMMAND;
				break;

		}
		break;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
