#pragma once

// SymmListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSymmListBox window

class CSymmListBox : public CListCtrl
{
	typedef struct
	{
		CString sSymm;
		CString sRA1;
		CString sRA2;

	} SymmEntry;

	typedef std::vector<SymmEntry> SymmVec;
	typedef std::vector<SymmEntry>::iterator SymmVecIt;
	typedef std::vector<SymmEntry>::const_iterator SymmVecCit;

	SymmVec m_vItemsList;
	int m_nBestChoises[2];

// Construction
public:
	CSymmListBox();

// Attributes
public:

// Operations
public:
	void SetBestChoice(int nBestChoice, int nRA)
	{
		if( (nRA >= 0) && (nRA < 2) )
		{
			m_nBestChoises[nRA] = nBestChoice;
		}
	}
	int GetBestChoice(int nRA)
	{
		if( (nRA >= 0) && (nRA < 2) )
		{
			return m_nBestChoises[nRA];
		}
		return 0;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSymmListBox)
	public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSymmListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSymmListBox)
	afx_msg LRESULT OnDeleteAllItems(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInsertItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetItem(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
