// Graph2DW.cpp : implementation file
//

#include "stdafx.h"

#include "Graph2DW.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <algorithm>
#include <functional>
#include <limits>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CGraph2DW

CGraph2DW::CGraph2DW()
{
	m_clrBkColor = RGB(0, 0, 0);
	m_clrCurve = RGB(255, 0, 0);
	m_clrGrid = RGB(128, 128, 128);

	m_fminX = m_fminY = 0.0;//FLT_MAX;
	m_fmaxX = m_fmaxY = 0.0;//FLT_MIN;

	m_iXborderL = 5;
	m_iXborderR = 5;
	m_iYborderL = 5;
	m_iYborderR = 5;

	m_bInverseY = TRUE;

	m_bNeedUpdate = FALSE;

	m_bZoomMode = FALSE;

	m_fntAxes.CreateFont(15, 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN,
		_T("Times new roman"));
}

CGraph2DW::~CGraph2DW()
{
	Clear();
}

BEGIN_MESSAGE_MAP(CGraph2DW, CPlotWnd)
	//{{AFX_MSG_MAP(CGraph2DW)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	ON_WM_PAINT()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////

CPoint CGraph2DW::MapPoint(float x, float y)
{
	CPoint pt;

	if( (m_fminX == m_fmaxX) || (m_fminY == m_fmaxY) )
		return CPoint(0,0);

	pt.x = (x - m_fminX) * (double)(m_rcClient.Width() - (m_iXborderL + m_iXborderR)) /
		(m_fmaxX - m_fminX) + m_iXborderL;
	if(m_bInverseY)
	{
		pt.y = (m_fmaxY - y) * (double)(m_rcClient.Height() - (m_iYborderL + m_iYborderR)) /
			(m_fmaxY - m_fminY) + m_iYborderL;
	}
	else
	{
		pt.y = (y - m_fminY) * (double)(m_rcClient.Height() - (m_iYborderL + m_iYborderR)) /
			(m_fmaxY - m_fminY) + m_iYborderL;
	}
	return pt;
}

void CGraph2DW::BackMapPoint(CPoint pt, float &x, float &y)
{
	x = m_fminX + (double)(pt.x - m_iXborderL) * (m_fmaxX - m_fminX) /
		(double)(m_rcClient.Width() - (m_iXborderL + m_iXborderR));
	if(m_bInverseY)
	{
		y = m_fmaxY - (double)(pt.y - m_iYborderL) * (m_fmaxY - m_fminY) /
			(double)(m_rcClient.Height() - (m_iYborderL + m_iYborderR));
	}
	else
	{
		y = m_fminY + (double)(pt.y - m_iYborderL) * (m_fmaxY - m_fminY) /
			(double)(m_rcClient.Height() - (m_iYborderL + m_iYborderR));
	}
}

void CGraph2DW::CreateDataSets(UINT nDataSets, size_t nSize)
{
	CGraph2Data* pDataSet;

	for(size_t i = 0; i < nDataSets; i++)
	{
		pDataSet = new CGraph2Data;
		pDataSet->resize(nSize);
		m_vpData.push_back(pDataSet);
	}
}

UINT CGraph2DW::AddDataSet(size_t nSize)
{
	CGraph2Data* pDataSet = new CGraph2Data;

	if( pDataSet == NULL )
		throw "CGraph2DW::AddDataSet() -> cannot allocate memory for a new dataset";

	UINT nPos = m_vpData.size();
	pDataSet->resize(nSize);
	m_vpData.push_back(pDataSet);

	return nPos;
}

void CGraph2DW::RemoveDataSet(UINT nDataSet)
{
	if( nDataSet < m_vpData.size() )
	{
		delete m_vpData[nDataSet];
		m_vpData.erase(m_vpData.begin()+ nDataSet);
	}
	else
	{
		TRACE1("CGraph2DW::RemoveDataSet() -> cannot remove dataset: %d.\n", nDataSet);
	}
}

void CGraph2DW::SetPoint(float x, float y, UINT nDataSet, size_t nPos)
{
	if( nDataSet >= m_vpData.size() )
	{
		TRACE1("CGraph2DW::SetPoint() -> cannot add point to the dataset %d - it does not exist.\n", nDataSet);
		return;
	}

	CDataPt pt = {x, y};
	if( nPos <= m_vpData[nDataSet]->size() )
	{
		m_vpData[nDataSet]->operator[](nPos) = pt;
		// since the limits can cange
		m_bNeedUpdate = TRUE;
	}
	else
	{
		TRACE2("CGraph2DW::SetPoint() -> cannot set %d-th point to the %d-th dataset - index is out of bounds.\n", nPos, nDataSet);
		return;
	}
}

void CGraph2DW::ResizeDataSet(UINT nDataSet, size_t nNewSize)
{
	if( nDataSet >= m_vpData.size() )
	{
		TRACE1("CGraph2DW::SetPoint() -> cannot add point to the dataset %d - it does not exist.\n", nDataSet);
		return;
	}
	m_vpData[nDataSet]->resize(nNewSize);
}

void CGraph2DW::CalculateLimits()
{
	m_fminX = m_fminY = FLT_MAX;
	m_fmaxX = m_fmaxY = FLT_MIN;

//	CDataPt dMinX, dMaxX, dMinY, dMaxY;

	for(int i = 0; i < m_vpData.size(); i++)
	{
		for(CGraph2Data::VectorPtI pi = m_vpData[i]->begin(); pi != m_vpData[i]->end(); ++pi)
		{
			// min and max in the i-th dataset
/*			dMinX = *min_element(m_vpData[i]->begin(), m_vpData[i]->end(), cmp_x());
			dMaxX = *max_element(m_vpData[i]->begin(), m_vpData[i]->end(), cmp_x());
			dMinY = *min_element(m_vpData[i]->begin(), m_vpData[i]->end(), cmp_y());
			dMaxY = *max_element(m_vpData[i]->begin(), m_vpData[i]->end(), cmp_y());
			m_fminX = min(m_fminX,dMinX[0]);
			m_fmaxX = max(m_fmaxX,dMaxX[0]);
			m_fminY = min(m_fminY,dMinY[1]);
			m_fmaxY = max(m_fmaxY,dMaxY[1]);
/*/
			if( m_fminX > (*pi)[0] )
				m_fminX = (*pi)[0];
			if( m_fminY > (*pi)[1] )
				m_fminY = (*pi)[1];
			if( m_fmaxX < (*pi)[0] )
				m_fmaxX = (*pi)[0];
			if( m_fmaxY < (*pi)[1] )
				m_fmaxY = (*pi)[1];
//*/
		}
	}
	CDC dc;
	CRect rc(0, 0, 0, 0);
	CString str;

	// estimate the sizes of the font
	dc.CreateCompatibleDC(NULL);

	dc.SelectObject(&m_fntAxes);

	// the hight of the font to use with X-axis
	str.Format(_T("%d"), (int)m_fmaxX);
	dc.DrawText(str, &rc, DT_CALCRECT);
	m_iYborderR += rc.Height() * 2;
	
	// the width of the font to use with Y-axis
	str.Format(_T("%.2f"), m_fmaxY);
	dc.DrawText(str, &rc, DT_CALCRECT);
	m_iXborderL += rc.Width();
	
	dc.DeleteDC();
	m_bNeedUpdate = FALSE;
}

void CGraph2DW::Clear()
{
	for(int i = 0; i < m_vpData.size(); i++)
	{
		delete m_vpData[i];
	}
	m_vpData.clear();

	m_fminX = m_fminY = 0.0;//FLT_MAX;
	m_fmaxX = m_fmaxY = 0.0;//FLT_MIN;

	m_bNeedUpdate = TRUE;
}

#define MAX_DATA_COLORS		18

static DWORD dataColors[MAX_DATA_COLORS] =
{
	RGB(255, 128, 128),
	RGB(128, 255, 128),
	RGB(128, 128, 255),
	RGB(255, 255, 0),
	RGB(255, 0, 255),
	RGB(0, 255, 255),
	RGB(192, 0, 0),
	RGB(0, 192, 0),
	RGB(0, 0, 192),
	RGB(192, 192, 0),
	RGB(192, 0, 192),
	RGB(0, 192, 192),
	RGB(128, 0, 0),
	RGB(0, 128, 0),
	RGB(0, 0, 128),
	RGB(128, 128, 0),
	RGB(128, 0, 128),
	RGB(0, 128, 128),
};

void CGraph2DW::RestoreZoomMode()
{
	if( TRUE == m_bZoomMode )
	{
		m_fminX = m_fOldminX;
		m_fmaxX = m_fOldmaxX;
		m_fminY = m_fOldminY;
		m_fmaxY = m_fOldmaxY;
		m_bZoomMode = FALSE;
	}
	Draw();
}

void CGraph2DW::SetZoomMode(CRect rcZoom)
{
	float x, y;

	if( FALSE == m_bZoomMode )
	{
		m_fOldminX = m_fminX;
		m_fOldmaxX = m_fmaxX;
		m_fOldminY = m_fminY;
		m_fOldmaxY = m_fmaxY;
	}

	BackMapPoint(rcZoom.TopLeft(), x, y);
	m_fminX = x;
	m_fmaxY = y;
	BackMapPoint(rcZoom.BottomRight(), x, y);
	m_fmaxX = x;
	m_fminY = y;
	m_bZoomMode = TRUE;

	Draw();
}

void CGraph2DW::Draw()
{
	CDC		*pDC = NULL;
	CBrush	*pOldBrush, br;
	CPen	*pOldPen, pen;
	CRgn	rgn;

	try
	{
		if(m_vpData.empty() == true)
			return;

		if( (pDC = GetMemDC()) == NULL )
		{
			return;
		}
		if( NULL == pDC->GetSafeHdc() )
		{
			return;
		}

		rgn.CreateRectRgn(m_rcClient.left, m_rcClient.top, m_rcClient.right, m_rcClient.bottom);
		pDC->SelectClipRgn(&rgn);

		br.CreateSolidBrush(m_clrBkColor);
		pOldBrush = pDC->SelectObject(&br);
		pDC->FillRect(m_rcClient, &br);

//		if( m_bNeedUpdate == TRUE )
//			CalculateLimits();

		DataSetIt di;
		CDataPt pt;
		CPoint LogPt;
		int nCurColor = 0;
		BOOL bVisible1, bVisible2;

		// for each dataset
		for(di = m_vpData.begin(); di < m_vpData.end(); ++di)
		{
			CGraph2Data &cur_set = *(*di);
			int nSize = cur_set.size();
			if( /*(*di)->size()*/nSize < 2 )
				continue;
			pen.CreatePen(PS_SOLID, 1, dataColors[nCurColor++]);
			nCurColor %= MAX_DATA_COLORS;
			pOldPen = pDC->SelectObject(&pen);
//			pt = (*di)->operator[](0);
			pt = cur_set[0];
			LogPt = MapPoint(pt[0], pt[1]);
			pDC->MoveTo(LogPt);
			// initialize
			bVisible1 = m_rcClient.PtInRect(LogPt);
			for(int nPt = 1; nPt < nSize; nPt++)
			{
				pt = cur_set[nPt];
				LogPt = MapPoint(pt[0], pt[1]);
				bVisible2 = m_rcClient.PtInRect(LogPt);
				// if previous or/and current are visible then draw
				if( bVisible1 || bVisible2 )
				{
					pDC->LineTo(LogPt);
				}
				else
				{
					pDC->MoveTo(LogPt);
				}
				bVisible1 = bVisible2;
			}
/*
			pDC->MoveTo(LogPt);
			for(int nPt = 1; nPt < (*di)->size(); nPt++)
			{
				pt = (*di)->operator[](nPt);
				LogPt = MapPoint(pt[0], pt[1]);
//				if( m_rcClient.PtInRect(LogPt) )
					pDC->LineTo(LogPt);
//				else
//					pDC->MoveTo(LogPt);
			}
*/
			// restore the old pen
			if(pOldPen)
				pDC->SelectObject(pOldPen);
			// delete the pen
			pen.DeleteObject();
		}

		pen.CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
		pOldPen = pDC->SelectObject(&pen);
		for(DataPeaksIt pi = m_vPeaks.begin(); pi != m_vPeaks.end(); ++pi)
		{
			if( (0 == pi->m_dAmpl) || (0 == pi->m_dFWHM) )
			{
				LogPt = MapPoint(pi->m_dX0, 0);
				pDC->MoveTo(LogPt);
				LogPt = MapPoint(pi->m_dX0, m_fmaxY);
				pDC->LineTo(LogPt);
			}
			else
			{
				// TODO: Draw some peak-shape
				LogPt = MapPoint(pi->m_dX0, 0);
				pDC->MoveTo(LogPt);
				LogPt = MapPoint(pi->m_dX0, m_fmaxY);
				pDC->LineTo(LogPt);
			}
		}
		// draw axes
		pDC->SelectObject(&br);
		pDC->SelectObject(&m_fntAxes);
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkColor(m_clrBkColor);
		CString str;
		const int iInterv = 10;
		int nHeight = m_rcClient.Height();
		int nWidth = m_rcClient.Width();
		int i;
		// Y-axis
		double dStep = (m_fmaxY - m_fminY) / (iInterv - 1.0);
		int iStep = (int)((double)(nHeight - (m_iYborderL + m_iYborderR)) / (iInterv - 1.0));
		for(i = 0; i < iInterv; i++)
		{
			CRect rc(0, 0, 0, 0);
			str.Format(_T("%.2f"), m_fminY + i*dStep);
			pDC->DrawText(str, rc, DT_CALCRECT | DT_RIGHT | DT_VCENTER);
			rc.OffsetRect(m_iXborderL - rc.Width(), nHeight - (m_iYborderR + iStep*i + (rc.Height()>>1)));
			pDC->DrawText(str, rc, DT_RIGHT | DT_VCENTER);
		}
		// X-axis
		dStep = (m_fmaxX - m_fminX) / (iInterv - 1.0);
		iStep = (int)((double)(nWidth - (m_iXborderL + m_iXborderR)) / (iInterv - 1.0));
		for(i = 0; i < iInterv; i++)
		{
			CRect rc(0, 0, 0, 0);
			str.Format(_T("%.2f"), m_fminX + i*dStep);
			pDC->DrawText(str, rc, DT_CALCRECT | DT_RIGHT | DT_VCENTER);
			rc.OffsetRect(m_iXborderL + iStep*i - (rc.Width()>>1), nHeight - (m_iYborderL + rc.Height() - 1));
			pDC->DrawText(str, rc, DT_RIGHT | DT_VCENTER);
		}
		// restore states
		if(pOldPen)
			pDC->SelectObject(pOldPen);
		pen.DeleteObject();
		if(pOldBrush)
			pDC->SelectObject(pOldBrush);

		CPlotWnd::Draw();
	}
	catch(...)
	{
		TRACE0("CGraph2DW::Draw() -> unknown error.\n");
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGraph2DW message handlers

void CGraph2DW::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CPlotWnd::OnLButtonDblClk(nFlags, point);
}

void CGraph2DW::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CPlotWnd::OnRButtonDblClk(nFlags, point);
}

BOOL CGraph2DW::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;	
}

void CGraph2DW::OnSize(UINT nType, int cx, int cy) 
{
	CPlotWnd::OnSize(nType, cx, cy);

	SetupWindow();
	Draw();
}

void CGraph2DW::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CPlotWnd::OnMouseMove(nFlags, point);
}

void CGraph2DW::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CPlotWnd::OnLButtonDown(nFlags, point);
}

void CGraph2DW::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CPlotWnd::OnLButtonUp(nFlags, point);
}

void CGraph2DW::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CPlotWnd::OnRButtonDown(nFlags, point);
}

void CGraph2DW::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CPlotWnd::OnRButtonUp(nFlags, point);
}

void CGraph2DW::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CPlotWnd::OnPaint() for painting messages
	Draw();
}

UINT CGraph2DW::AddPeak(CGraph2Dpeak &peak)
{
	m_vPeaks.push_back(peak);
	return m_vPeaks.size() - 1;
}

void CGraph2DW::ResetPeaks()
{
	m_vPeaks.clear();
}

LRESULT CGraph2DW::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT lRes = 0;
	__try
	{
		lRes = CPlotWnd::WindowProc(message, wParam, lParam);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE(_T("Error -> in CGraph2DW::WindowProc() with message : 0x%08X\n"), message);
	}
	return lRes;
}
