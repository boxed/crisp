// MainFrm.cpp : implementation file
//

#include "StdAfx.h"

#if defined USE_MFC

#include "stdafx.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CMDIFrameWnd)

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_QUIT, OnQuit)
	ON_WM_CHAR()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_SETFOCUS()
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

void CMainFrame::OnDestroy() 
{
	CMDIFrameWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

LPARAM CMainFrame::OnQuit(WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your message handler code here

	return 0;
}

void CMainFrame::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CMDIFrameWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIFrameWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIFrameWnd::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	
}

void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd) 
{
	CMDIFrameWnd::OnPaletteChanged(pFocusWnd);
	
}

// We get a QUERYNEWPALETTE message when our app gets the
//  focus.  We need to realize the currently active MDI
//  child's palette as the foreground palette for the app.
//  We do this by sending our MYWM_QUERYNEWPALETTE message
//  to the currently active child's window procedure.
BOOL CMainFrame::OnQueryNewPalette()
{
/*
		if (bGrabberActive && hwndLive && IsWindow(hwndLive)) 
			SendMessage (hwndLive, WM_QUERYNEWPALETTE, 0, 0);
		goto CallDFP;
		return FALSE;
*/
	return CMDIFrameWnd::OnQueryNewPalette();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame functions

#endif
