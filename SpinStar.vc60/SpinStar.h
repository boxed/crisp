// SpinStar.h : main header file for the SPINSTAR DLL
//

#if !defined(AFX_SPINSTAR_H__0A0DC63C_512D_4A81_A390_564E2AC26563__INCLUDED_)
#define AFX_SPINSTAR_H__0A0DC63C_512D_4A81_A390_564E2AC26563__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSpinStarApp
// See SpinStar.cpp for the implementation of this class
//

class CSpinStarApp : public CWinApp
{
public:
	CSpinStarApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpinStarApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSpinStarApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPINSTAR_H__0A0DC63C_512D_4A81_A390_564E2AC26563__INCLUDED_)
