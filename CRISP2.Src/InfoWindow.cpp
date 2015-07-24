#include "stdafx.h"

#include "InfoWindow.h"
#include "globals.h"
#include "commondef.h"
#include "resource.h"

HBRUSH warningColorBrush = ::CreateSolidBrush(RGB(255, 255, 0));

LRESULT CALLBACK InfoWindowProc(      
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT r;
			GetClientRect(hWnd, &r);
			InflateRect(&r, -3, -3);
			TCHAR text[1024];
			GetWindowText(hWnd, text, 1024);
			SetTextColor(hdc, RGB(0, 0, 0));
			SetBkMode(hdc, TRANSPARENT);
			SelectObject(hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
			DrawText(hdc, text, -1, &r, DT_CENTER | DT_WORDBREAK);
			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_LBUTTONDOWN:
		{
			LONG commandID = GetWindowLong(hWnd, GWL_USERDATA);
			PostMessage(MainhWnd, WM_COMMAND, MAKEWPARAM(commandID, 0), 0);
		}
		return 0;
	case WM_ERASEBKGND:
		{
			HDC hdc = (HDC)wParam;
			RECT r;
			GetClientRect(hWnd, &r);
			FillRect(hdc, &r, warningColorBrush);
		}
		return 0;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

const TCHAR* GetInfoWindowClassName()
{
	static const TCHAR* classname = _T("Crisp Info Window");
	static bool classNameRegistered = false;
	if (!classNameRegistered)
	{
		WNDCLASS wndClass;
		ZeroMemory(&wndClass, sizeof(wndClass));
		wndClass.lpfnWndProc = InfoWindowProc;
		wndClass.hInstance = hInst;
		wndClass.hbrBackground = (HBRUSH)COLOR_BTNFACE;
		wndClass.lpszClassName = classname;
		wndClass.style = CS_NOCLOSE; 
		if (RegisterClass(&wndClass) == 0)
		{
			Trace(_FMT(_T("Failed to register window class: %d"), GetLastError()));
		}
		classNameRegistered = true;
	}
	return classname;
}

HWND CreateInfoWindow(const TCHAR* inMessage, DWORD inCommandOnClick, bool inChild)
{
	HWND w = CreateWindow(
		GetInfoWindowClassName(), 
		inMessage, 
		WS_VISIBLE | (inChild?WS_CHILD:WS_POPUP) | WS_CLIPSIBLINGS, 
		0, 0, 100, 30, // x, y, width, height 
		MDIhWnd,//inChild?MDIhWnd:NULL, 
		NULL,
		hInst, 
		(void*)0); // custom handle
	if (w == NULL)
	{
		Trace(_FMT(_T("Failed to create window: %d"), GetLastError()));
	}
	SetWindowLong(w, GWL_USERDATA, inCommandOnClick);
	ShowWindow(w, SW_SHOW);
	return w;
}