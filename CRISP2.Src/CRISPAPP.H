// CrispApp.h: interface for the CCrispApp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

class CCrispApp : public CWinApp  
{
public:
	CCrispApp();

// Overrides
	// ClassWizard generated virtual function overrides
public:
	virtual BOOL InitInstance();
	virtual BOOL InitApplication();
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
//	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};
