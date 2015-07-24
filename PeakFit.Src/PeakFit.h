// PeakFit.h : main header file for the PEAKFIT DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPeakFitApp
// See PeakFit.cpp for the implementation of this class
//

class CPeakFitApp : public CWinApp
{
public:
	CPeakFitApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPeakFitApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPeakFitApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
