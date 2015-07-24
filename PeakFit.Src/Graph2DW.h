#pragma once
// Graph2DW.h : header file
//

#include "PlotWnd.h"

#include "array.h"

/////////////////////////////////////////////////////////////////////////////
// represents a data-point

typedef array<float,2> CDataPt;

/////////////////////////////////////////////////////////////////////////////
// data to represent class

class CGraph2Data
{
public:
	typedef std::vector<CDataPt>	VectorPt;
	typedef VectorPt::iterator		VectorPtI;

public:
	CGraph2Data() {}
	~CGraph2Data() {}
	int size() const
		{ return m_vData.size(); }
	CDataPt& operator [] (size_t n)
		{ return m_vData[n]; };
	CDataPt& operator () (size_t n)
		{ return m_vData[n]; };
	void push_back( const CDataPt & x)
		{ m_vData.push_back(x); }
	bool empty()
		{ return m_vData.empty(); }
	VectorPtI begin()
		{ return m_vData.begin(); }
	VectorPtI end()
		{ return m_vData.end(); }
	void clear()
		{ m_vData.clear(); }
	void resize(UINT nNewSize)
		{ m_vData.resize(nNewSize); }

protected:
	VectorPt m_vData;		// data-points
};

class CGraph2Dpeak
{
public:
	CGraph2Dpeak()
	{
		m_dX0 = 0;
		m_dAmpl = 0;
		m_dFWHM = 0;
		m_bValid = true;
	}
	~CGraph2Dpeak()
	{
	}
	double m_dX0;
	double m_dAmpl;
	double m_dFWHM;
	bool m_bValid;
};

/////////////////////////////////////////////////////////////////////////////
// CGraph2DW window

class CGraph2DW : public CPlotWnd
{
// Construction
public:
	CGraph2DW();

// Attributes
public:

// Operations
public:
//	void		CleanDataSets();
	void		CreateDataSets(UINT nDataSets, size_t nSize);
	UINT		AddDataSet(size_t nSize);
	void		RemoveDataSet(UINT nDataSet);
	void		SetPoint(float x, float y, UINT nDataSet, size_t nPos);
	void		ResizeDataSet(UINT nDataSets, size_t nNewSize);
	void		Clear();
	virtual void	Draw();

	UINT		AddPeak(CGraph2Dpeak &peak);
	void		ResetPeaks();

	void		InverseYaxis(BOOL bInverse) { m_bInverseY = bInverse; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraph2DW)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGraph2DW();

	// Generated message map functions
public:
	CPoint		MapPoint(float x, float y);	// to window coordinates
	void		BackMapPoint(CPoint pt, float &x, float &y);	// to float coordinates

	void		SetZoomMode(CRect rcZoom);
	void		RestoreZoomMode();
//	void		ApplyZoomMode(CDC* pDC);

	void		CalculateLimits();

protected:

	BOOL		m_bNeedUpdate;
	COLORREF	m_clrBkColor;
	COLORREF	m_clrCurve;
	COLORREF	m_clrGrid;

	BOOL		m_bInverseY;

	BOOL		m_bZoomMode;
//	CRect		m_rcZoom;
//	CPoint		m_ptOrigin;

	std::vector<CGraph2Data*> m_vpData;	// multiple data vector

	typedef std::vector<CGraph2Data*>::iterator DataSetIt;

	typedef std::vector<CGraph2Dpeak>	DataPeaks;
	typedef DataPeaks::iterator			DataPeaksIt;

	DataPeaks	m_vPeaks;

	CFont		m_fntAxes;

	float		m_fminX,
				m_fminY,
				m_fmaxX,
				m_fmaxY;
	float		m_fOldminX,
				m_fOldminY,
				m_fOldmaxX,
				m_fOldmaxY;

	int			m_iXborderL,
				m_iXborderR,
				m_iYborderL,
				m_iYborderR;

	//{{AFX_MSG(CGraph2DW)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
