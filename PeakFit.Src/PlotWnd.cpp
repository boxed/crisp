// PlotWnd.cpp : implementation file
//

#include "stdafx.h"

#include "PlotWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlotWnd

CPlotWnd::CPlotWnd()
{
}

CPlotWnd::~CPlotWnd()
{
	m_memDC.DeleteDC();
	m_bitmap.DeleteObject();
}

BEGIN_MESSAGE_MAP(CPlotWnd, CWnd)//CStatic)
	//{{AFX_MSG_MAP(CPlotWnd)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotWnd message handlers

HBRUSH CPlotWnd::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	// TODO: Return a non-NULL brush if the parent's handler should not be called
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

/////////////////////////////////////////////////////////////////////////////
// CPlotWnd functions

void CPlotWnd::SetupWindow()
{
//	GUARD(CPlotWnd::SetupWindow());

	int	  width, height, dx, dy;
	CWnd* pWnd = NULL;
	CDC*  pDC = NULL;

	if(!::IsWindow(m_hWnd))
		throw "Plot window was not Subclassed or was not created!";

	GetWindowRect(m_rcWindow);
	GetClientRect(m_rcClient);
	width = m_rcClient.Width();
	height = m_rcClient.Height();
	
	dx = m_rcWindow.Width() - width;
	dy = m_rcWindow.Height() - height;
	if(dx < 0) dx = 0; else dx >>= 1;
	if(dy < 0) dy = 0; else dy >>= 1;

	m_ptOffset = CPoint(dx, dy);

	pDC = GetWindowDC();

	m_memDC.DeleteDC();
	m_memDC.CreateCompatibleDC(pDC);

	if(pDC)
	{
		CBrush br;
		br.CreateStockObject(DKGRAY_BRUSH);

		m_bitmap.DeleteObject();
		m_bitmap.CreateCompatibleBitmap(pDC, width, height);
		m_memDC.SelectObject(m_bitmap);
		m_memDC.FillRect(m_rcClient, &br);
		ReleaseDC(pDC);
	}

//	UNGUARD;
}

void CPlotWnd::Draw()
{
//	GUARD(CPlotWnd::Draw());

	UINT  width, height;
	CDC*  pDC  = NULL;

	width  = m_rcClient.Width();
	height = m_rcClient.Height();

	if( (pDC = GetWindowDC()) != NULL )
	{
		pDC->BitBlt(m_ptOffset.x, m_ptOffset.y, width, height, &m_memDC, 0, 0, SRCCOPY);
		ReleaseDC(pDC);
	}

//	UNGUARD;
}

void CPlotWnd::PreSubclassWindow() 
{
//	GUARD(CLoadPreviewWnd::PreSubclassWindow());

//	CStatic::PreSubclassWindow();
	CWnd::PreSubclassWindow();

	SetupWindow();

//	UNGUARD;
}

void CPlotWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
//	GUARD(CPlotWnd::OnPaint());

	CRect rcClient;
	GetClientRect(m_rcClient);

	if( m_memDC.GetSafeHdc() != NULL )
	{
		dc.BitBlt(0, 0, m_rcClient.Width(), m_rcClient.Height(), &m_memDC, 0, 0, SRCCOPY);
	}

//	UNGUARD;
}

LRESULT CPlotWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT lRes = 0;
	__try
	{
		lRes = CWnd::WindowProc(message, wParam, lParam);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE(_T("Error -> in CPlotWnd::WindowProc() with message : 0x%08X\n"), message);
	}
	return lRes;
}
