#pragma once
// PlotWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlotWnd window

class CPlotWnd : public CWnd
{
// Construction
public:
	CPlotWnd();

// Attributes
protected:
	CBitmap		m_bitmap;		// bitmap we are drawing in
	CDC			m_memDC;
	CRect		m_rcWindow;
	CRect		m_rcClient;
	CPoint		m_ptOffset;

// Operations
public:
	virtual void	Draw();
	
	void		SetupWindow();
	CDC			*GetMemDC() { return &m_memDC; }
	CBitmap		*GetBitmap() { return &m_bitmap; }
	void		GetClientRc(LPRECT lpRect) { *lpRect  = m_rcClient; }
	void		GetWindowRc(LPRECT lpRect) { *lpRect  = m_rcWindow; }
	void		GetOffset(LPPOINT lpPoint)   { *lpPoint = m_ptOffset; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlotWnd)
	protected:
	virtual void PreSubclassWindow();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPlotWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlotWnd)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
