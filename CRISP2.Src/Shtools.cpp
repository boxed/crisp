/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRSIP2.shtools * by ML ***************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "objects.h"
#include "shtools.h"

LRESULT CALLBACK JoystickWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SpinEditWndProc(HWND, UINT, WPARAM, LPARAM);

static	HINSTANCE	hInst;

/**********************************************************************/
BOOL	InitSHTtools( HINSTANCE hInstance )
{
	WNDCLASS  wc;
	BOOL	b;

	hInst = hInstance;
	memset(&wc, 0, sizeof(wc));
// SHT_JOYSTICK
 	wc.style = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.lpfnWndProc = JoystickWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 8;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "SHT_JOYSTICK";
	b = RegisterClass(&wc);
// SHT_SPINEDIT
 	wc.style = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.lpfnWndProc = SpinEditWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 8;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "SHT_SPINEDIT";
	b = RegisterClass(&wc);
	return TRUE;
}
/**********************************************************************/
VOID CloseSHTtools( HINSTANCE hInst )
{
	UnregisterClass( "SHT_JOYSTICK", hInst );
	UnregisterClass( "SHT_SPINEDIT", hInst );
}

/**********************************************************************/
/**********************************************************************/
static VOID PaintArrowButton( HDC hDC, int x0, int y0, int  cx, int cy, int dir, BOOL pushed )
{
RECT	r;
POINT	pn[3];
HBRUSH	hob;
HPEN	hop;

	if (pushed) {
		SetRect(&r, x0, y0, x0+cx, y0+cy); FrameRect (hDC, &r, (HBRUSH)GetStockObject(GRAY_BRUSH));
		x0+=1; y0+=1;
	} else {
		SetRect(&r, x0+1, y0+1, x0+cx-2, y0+2); FillRect (hDC, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
		SetRect(&r, x0+1, y0+1, x0+2, y0+cy-2); FillRect (hDC, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
		SetRect(&r, x0+1, y0+cy-2, x0+cx-1, y0+cy-1); FillRect (hDC, &r, (HBRUSH)GetStockObject(GRAY_BRUSH));
		SetRect(&r, x0+cx-2, y0+1, x0+cx-1, y0+cy-1); FillRect (hDC, &r, (HBRUSH)GetStockObject(GRAY_BRUSH));
		SetRect(&r, x0,y0+cy-1,x0+cx,y0+cy); FillRect (hDC, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
		SetRect(&r, x0+cx-1,y0,x0+cx,y0+cy); FillRect (hDC, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	switch(dir) {
		case 0: pn[0].x=x0+cx-6; pn[0].y = y0+4; pn[1].x=x0+cx-6; pn[1].y = y0+cy-5; pn[2].x=x0+5; pn[2].y = y0+cy/2; break;
		case 1: pn[0].x=x0+5; pn[0].y = y0+4; pn[1].x=x0+5; pn[1].y = y0+cy-5; pn[2].x=x0+cx-6; pn[2].y = y0+cy/2; break;
		case 2: pn[0].x=x0+4; pn[0].y = y0+cy-6; pn[1].x=x0+cx-5; pn[1].y = y0+cy-6; pn[2].x=x0+cx/2; pn[2].y = y0+5; break;
		case 3: pn[0].x=x0+4; pn[0].y = y0+5; pn[1].x=x0+cx-5; pn[1].y = y0+5; pn[2].x=x0+cx/2; pn[2].y = y0+cy-6; break;
	}
	hob = SelectBrush(hDC, (HBRUSH)GetStockObject(BLACK_BRUSH));
	hop = SelectPen  (hDC, (HPEN)GetStockObject(BLACK_PEN));
	Polygon(hDC, pn, 3);
	if (hob) SelectBrush(hDC, hob);
	if (hop) SelectPen(hDC, hop);
}
/**********************************************************************/
static WORD WhereInJoystick( HWND hWnd )
{
RECT	r, rb[4];
POINT	p;
int		w,h,cx,cy;

	GetClientRect( hWnd, &r );
	w = r.right-r.left;
	h = r.bottom-r.top;
	GetCursorPos( &p );
	MapWindowPoints(NULL, hWnd, &p, 1);
	cx = w/3; cy=h/3;
	SetRect(&rb[0], 0,cy,cx,cy*2);
	SetRect(&rb[1], cx*2,cy,cx*3,cy*2);
	SetRect(&rb[2], cx,0,cx*2,cy);
	SetRect(&rb[3], cx,cy*2,cx*2,cy*3);
	if (PtInRect(&rb[0],p)) return JC_LEFT;
	if (PtInRect(&rb[1],p)) return JC_RIGHT;
	if (PtInRect(&rb[2],p)) return JC_UP;
	if (PtInRect(&rb[3],p)) return JC_DOWN;
	return 0;
}
/**********************************************************************/
static VOID PaintJoystick( HWND hWnd )
{
HDC		hDC;
int		cx, cy;
RECT	r, ru;
DWORD	state=0, oldstate = GetWindowLong(hWnd,0);

	GetClientRect( hWnd, &r );
	cx = (r.right-r.left)/3;
	cy = (r.bottom-r.top)/3;

	state = WhereInJoystick(hWnd);

	if ((state & JC_LEFT)  && ((oldstate&0x10)==0)) state&=~JC_LEFT;
	if ((state & JC_RIGHT) && ((oldstate&0x20)==0)) state&=~JC_RIGHT;
	if ((state & JC_UP)    && ((oldstate&0x40)==0)) state&=~JC_UP;
	if ((state & JC_DOWN)  && ((oldstate&0x80)==0)) state&=~JC_DOWN;
	
	if (((state&0xF) != (oldstate&0xF)) || GetUpdateRect(hWnd,&ru,FALSE)) {
		hDC = GetWindowDC(hWnd);
		if (hDC) {
			InvalidateRect( hWnd, NULL, FALSE );
			FillRect (hDC, &r, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
			PaintArrowButton( hDC, 0,    cy,    cx, cy, 0, (state & JC_LEFT) );		// left
			PaintArrowButton( hDC, cx*2, cy,    cx, cy, 1, (state & JC_RIGHT) );	// right
			PaintArrowButton( hDC, cx,   0,     cx, cy, 2, (state & JC_UP) );		// up
			PaintArrowButton( hDC, cx,   cy*2,  cx, cy, 3, (state & JC_DOWN) );		// down
			ReleaseDC( hWnd, hDC );
		}
		ValidateRect( hWnd, NULL );
	}
	state = (oldstate&0xFFF0) | state;
	SetWindowLong(hWnd, 0, state);
}
/**********************************************************************/
LRESULT CALLBACK JoystickWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
int state;

	switch(msg) {

		case WM_CREATE:
		{
		RECT	r;
		int		w,h;
			GetWindowRect( hWnd, &r );
			w = r.right-r.left;
			h = r.bottom-r.top;
			if (w<39) w=39; if (h<39) h=39;
			if ((w-39)%6) w = ((w-39)/6)*6+39;
			if ((h-39)%6) h = ((h-39)/6)*6+39;
			w = __min(w,h); h=w;
			SetWindowPos( hWnd, NULL, 0, 0, w, h, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

		}
			break;
		case WM_TIMER:
			if (wParam==0) {
				KillTimer(hWnd,0);
				SetTimer(hWnd,1,30,NULL);
			} else {
				state = WhereInJoystick(hWnd);
				if (state==((GetWindowLong(hWnd,0)>>4)&0xF))
					SendMessage(GetParent(hWnd),WM_COMMAND, MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), JM_MOVE), (LPARAM)state);
			}
			break;

		case WM_LBUTTONDOWN:
		 	state = WhereInJoystick(hWnd);
			if (state) {
				SendMessage(GetParent(hWnd),WM_COMMAND,	MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), JM_MOVE), (LPARAM)state);
				state = state<<4;
				SetWindowLong(hWnd,0,state);
				SetCapture( hWnd );
				PaintJoystick(hWnd);
				SetTimer(hWnd,0,400,NULL);
			}
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			SetWindowLong(hWnd,0,GetWindowLong(hWnd,0)&0xFF0F);
			PaintJoystick(hWnd);
			KillTimer(hWnd,0);
			KillTimer(hWnd,1);
			break;
			
		case WM_MOUSEMOVE:
			PaintJoystick(hWnd);
			break;

		case WM_PAINT:
			PaintJoystick( hWnd );
			return 0;

	}
	return DefWindowProc( hWnd, msg, wParam, lParam);
}                                    
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static WORD WhereInSpin( HWND hWnd )
{
RECT	r, rb[4];
POINT	p;
int		w,h;

	GetClientRect( hWnd, &r );
	w = r.right-r.left;
	h = r.bottom-r.top;
	GetCursorPos( &p );
	MapWindowPoints(NULL, hWnd, &p, 1);
	SetRect(&rb[2], w-11,0,w,h/2);
	SetRect(&rb[3], w-11,h/2,w,h);
	if (PtInRect(&rb[2],p)) return JC_UP;
	if (PtInRect(&rb[3],p)) return JC_DOWN;
	return 0;
}
/**********************************************************************/
static VOID PaintSpinButton( HDC hDC, int x0, int y0, int  cx, int cy, int dir, BOOL pushed )
{
RECT	r;
POINT	pn[3];
HBRUSH	hob;
HPEN	hop;

	if (pushed) {
		SetRect(&r, x0, y0, x0+cx, y0+cy); FrameRect (hDC, &r, (HBRUSH)GetStockObject(GRAY_BRUSH));
		x0+=1; y0+=1;
	} else {
/*		SetRect(&r, x0+1, y0+1, x0+cx-2, y0+2); FillRect (hDC, &r, GetStockObject(WHITE_BRUSH));
		SetRect(&r, x0+1, y0+1, x0+2, y0+cy-2); FillRect (hDC, &r, GetStockObject(WHITE_BRUSH));
		SetRect(&r, x0+1, y0+cy-2, x0+cx-1, y0+cy-1); FillRect (hDC, &r, GetStockObject(GRAY_BRUSH));
		SetRect(&r, x0+cx-2, y0+1, x0+cx-1, y0+cy-1); FillRect (hDC, &r, GetStockObject(GRAY_BRUSH));
		SetRect(&r, x0,y0+cy-1,x0+cx,y0+cy); FillRect (hDC, &r, GetStockObject(BLACK_BRUSH));
		SetRect(&r, x0+cx-1,y0,x0+cx,y0+cy); FillRect (hDC, &r, GetStockObject(BLACK_BRUSH));
*/
		SetRect(&r, x0,   y0,   x0+cx-1, y0+1); FillRect (hDC, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
		SetRect(&r, x0,   y0,   x0+1, y0+cy-1); FillRect (hDC, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
		SetRect(&r, x0,y0+cy-1,x0+cx,y0+cy); FillRect (hDC, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
		SetRect(&r, x0+cx-1,y0,x0+cx,y0+cy); FillRect (hDC, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	switch(dir) {
//		case 0: pn[0].x=x0+cx-6; pn[0].y = y0+4; pn[1].x=x0+cx-6; pn[1].y = y0+cy-5; pn[2].x=x0+5; pn[2].y = y0+cy/2; break;
//		case 1: pn[0].x=x0+5; pn[0].y = y0+4; pn[1].x=x0+5; pn[1].y = y0+cy-5; pn[2].x=x0+cx-6; pn[2].y = y0+cy/2; break;
		case 2: pn[0].x=x0+3; pn[0].y = y0+cy-4; pn[1].x=x0+cx-4; pn[1].y = y0+cy-4; pn[2].x=x0+cx/2; pn[2].y = y0+3; break;
		case 3: pn[0].x=x0+3; pn[0].y = y0+3; pn[1].x=x0+cx-4; pn[1].y = y0+3; pn[2].x=x0+cx/2; pn[2].y = y0+cy-4; break;
	}
	hob = SelectBrush(hDC, (HBRUSH)GetStockObject(BLACK_BRUSH));
	hop = SelectPen(hDC, (HBRUSH)GetStockObject(BLACK_PEN));
	Polygon(hDC, pn, 3);
	if (hob) SelectBrush(hDC, hob);
	if (hop) SelectPen(hDC, hop);
}
/**********************************************************************/
static VOID PaintSpinEdit( HWND hWnd )
{
HDC		hDC;
int		cx, cy, x0, y0;
RECT	r, ru;
DWORD	state=0, oldstate = GetWindowLong(hWnd,0);

	GetClientRect( hWnd, &r );
	cx = 11;
	cy = (r.bottom-r.top)/2;
	x0 = r.right-r.left-11;
	y0 = 0;

	state = WhereInSpin(hWnd);

	if ((state & JC_UP)   && ((oldstate&0x40)==0)) state&=~JC_UP;
	if ((state & JC_DOWN) && ((oldstate&0x80)==0)) state&=~JC_DOWN;
	
	r.left = x0;

	if (((state&0xF) != (oldstate&0xF)) || GetUpdateRect(hWnd,&ru,FALSE)) {
		hDC = GetWindowDC(hWnd);
		if (hDC) {
			InvalidateRect( hWnd, &r, FALSE );
			FillRect (hDC, &r, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
			if (IsWindowEnabled(hWnd)) {
				PaintSpinButton( hDC, x0,   y0,    cx, cy, 2, (state&JC_UP) );	// up
				PaintSpinButton( hDC, x0,   y0+cy, cx, cy, 3, (state&JC_DOWN) );	// down
			}
			ReleaseDC( hWnd, hDC );
		}
		ValidateRect( hWnd, &r );
	}
	state = (oldstate&0xFFF0) | state;
	SetWindowLong(hWnd, 0, state);
}
/**********************************************************************/
LRESULT CALLBACK SpinEditWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
int 	state;
HWND	hwe;

	hwe = (HWND)GetWindowLong( hWnd, 4 );
	switch(msg) {
		case WM_CREATE:
		{
		RECT	r;
		int		w,h;
			GetWindowRect( hWnd, &r );
			w = r.right-r.left;
			h = r.bottom-r.top;
			hwe = CreateWindowEx(WS_EX_NOPARENTNOTIFY, "Edit", "", WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER,
								 0, 0, w-11, h, hWnd, (HMENU)1234, hInst, NULL);
			SetWindowLong( hWnd, 4, (LONG)hwe );
		}
			break;

		case WM_ENABLE:
			InvalidateRect(hWnd,NULL,FALSE);
		case WM_SETTEXT:
		case WM_SETFONT:
		case WM_GETTEXT:
		case WM_SHOWWINDOW:
			if (hwe) return SendMessage( hwe, msg, wParam, lParam);
			break;

		case WM_COMMAND:
			if (LOWORD(wParam)==1234) 
				return SendMessage(GetParent(hWnd), msg, MAKEWPARAM(GetDlgCtrlID(hWnd),HIWORD(wParam)), (LPARAM)hWnd);
			else break;

//		case WM_CTLCOLOR:
//			return SendMessage( GetParent(hWnd), msg, wParam, (LPARAM)MAKELONG(hWnd,HIWORD(lParam)));

		case WM_TIMER:
			if (wParam==0) {
				KillTimer(hWnd,0);
				SetTimer(hWnd,1,30,NULL);
			} else {
				state = WhereInSpin(hWnd);
				if (state==((GetWindowLong(hWnd,0)>>4)&0xF))
					SendMessage(GetParent(hWnd),WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), SE_MOVE), (LPARAM)state);
			}
			break;

		case WM_LBUTTONDOWN:
		 	state = WhereInSpin(hWnd);
			if (state) {
				SendMessage(GetParent(hWnd),WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), SE_MOVE), (LPARAM)state);
				state = state<<4;
				SetWindowLong(hWnd,0,state);
				SetCapture( hWnd );
				PaintSpinEdit(hWnd);
				SetTimer(hWnd,0,400,NULL);
			}
			return 0;

		case WM_LBUTTONUP:
			ReleaseCapture();
			SetWindowLong(hWnd,0,GetWindowLong(hWnd,0)&0xFF0F);
			PaintSpinEdit(hWnd);
			KillTimer(hWnd,0);
			KillTimer(hWnd,1);
			return 0;
			
		case WM_MOUSEMOVE:
			PaintSpinEdit(hWnd);
			return 0;

		case WM_PAINT:
			PaintSpinEdit( hWnd );
			DefWindowProc( hWnd, msg, wParam, lParam);
			return 0;
	}
	return DefWindowProc( hWnd, msg, wParam, lParam);
}                                    
