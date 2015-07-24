#if !defined(AFX_RAWDLG_H__80F4B7DF_039A_4EE8_AF6A_32C9031146A3__INCLUDED_)
#define AFX_RAWDLG_H__80F4B7DF_039A_4EE8_AF6A_32C9031146A3__INCLUDED_

#if defined USE_MFC

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RawDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// RawDlg dialog

class RawDlg : public CDialog
{
// Construction
public:
	RawDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(RawDlg)
	enum { IDD = IDD_IMPORT_RAW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RawDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(RawDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // USE_MFC

#endif // !defined(AFX_RAWDLG_H__80F4B7DF_039A_4EE8_AF6A_32C9031146A3__INCLUDED_)
