// SpinStar.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SpinStar.h"

#include "SpinningStar/SpinningStar.h"
#include "SpinningStar/SpinningStarStartDlg.h"

#include "commondef.h"
#include "objects.h"
#include "Eld.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HINSTANCE	hInst;			// Handle to instance.

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CSpinStarApp

BEGIN_MESSAGE_MAP(CSpinStarApp, CWinApp)
	//{{AFX_MSG_MAP(CSpinStarApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpinStarApp construction

CSpinStarApp::CSpinStarApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSpinStarApp object

CSpinStarApp theApp;

extern "C" BOOL __declspec(dllexport) CALLBACK Run_SpinStar(LPVOID pLists)
{
	hInst = GetModuleHandle(NULL);
//#ifdef _DEBUG
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
//#endif // _DEBUG
	CSpinningStarStartDlg dlg;
	
	// Assume LaueZone class here
	if( (NULL != pLists) && (FALSE == ::AfxIsValidAddress(pLists, sizeof(LPVOID)*2)) )
	{
		TRACE(_T("ERROR OBJECT LAUE ZONES.\n"));
	}
	LaueZone** laueZones = (LaueZone**)pLists;
	LaueZone *pLZ1 = laueZones[0];
	LaueZone *pLZ2 = laueZones[1];
	int autoOffset = (BYTE*)&pLZ1->bAuto-(BYTE*)pLZ1;
	int laueSize = sizeof(LaueZone);
	int vecSize = sizeof(EldReflVec);
	if( (NULL != pLZ1) && (FALSE == ::AfxIsValidAddress(pLZ1, sizeof(LaueZone))) )
	{
		TRACE(_T("ERROR OBJECT LAUE ZONE 1.\n"));
	}
	if( (NULL != pLZ2) && (FALSE == ::AfxIsValidAddress(pLZ2, sizeof(LaueZone))) )
	{
		TRACE(_T("ERROR OBJECT LAUE ZONE 2.\n"));
	}
	
	dlg.m_nDialogMode = (CSpinningStarStartDlg::DIALOG_MODE)
		(CSpinningStarStartDlg::HOLZ_MODE |
		CSpinningStarStartDlg::DISABLE_BROWSE_BTNS);
	
	if( (NULL == pLZ1) )
	{
		::MessageBox(NULL, "Zero Order Laue Zone has no estimated intensities.",
			"SPINNING STAR ERROR", MB_ICONERROR | MB_OK);
//		pDlg->DestroyWindow();
	}
	else
	{
		dlg.m_pLZ1 = pLZ1;
		dlg.m_pLZ2 = pLZ2;
		dlg.m_sZolzFile = CString(pLZ1->m_pszImgName);
		dlg.m_sZolzFilePath = dlg.m_sZolzFile;
		if( NULL != pLZ2 )
		{
			dlg.m_sFolzFile = CString(pLZ2->m_pszImgName);
			dlg.m_sFolzFilePath = dlg.m_sFolzFile;
		}
		dlg.DoModal();
// 		if( TRUE == dlg.Create(NULL) )
// 		{
// 			pDlg->ShowWindow(SW_SHOW);
// 		}
// 		else
// 		{
// 			pDlg->DestroyWindow();
// 		}
	}
	return TRUE;
}
/*
extern "C" BOOL __declspec(dllexport) CALLBACK Run_SpinStar(LPVOID pList1, LPVOID pList2)
{
//#ifdef _DEBUG
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
//#endif // _DEBUG
	CSpinningStarStartDlg *pDlg = new CSpinningStarStartDlg;

	if( NULL == pDlg )
	{
		return FALSE;
	}
	// Assume LaueZone class here
	if( (NULL != pList1) && (FALSE == ::AfxIsValidAddress(pList1, sizeof(LaueZone))) )
	{
		TRACE(_T("ERROR OBJECT LAUE ZONE 1.\n"));
	}
	if( (NULL != pList2) && (FALSE == ::AfxIsValidAddress(pList2, sizeof(LaueZone))) )
	{
		TRACE(_T("ERROR OBJECT LAUE ZONE 2.\n"));
	}

	pDlg->m_nDialogMode = (CSpinningStarStartDlg::DIALOG_MODE)
		(CSpinningStarStartDlg::HOLZ_MODE |
		CSpinningStarStartDlg::DISABLE_BROWSE_BTNS);

	LaueZone *pLZ1 = (LaueZone*)pList1;
	LaueZone *pLZ2 = (LaueZone*)pList2;
	if( (NULL == pLZ1) )
	{
		::MessageBox(NULL, "Zero Order Laue Zone has no estimated intensities.",
			"SPINNING STAR ERROR", MB_ICONERROR | MB_OK);
		pDlg->DestroyWindow();
	}
	else
	{
		pDlg->m_pLZ1 = pLZ1;
		pDlg->m_pLZ2 = pLZ2;
		pDlg->m_sZolzFile = CString(pLZ1->m_pszImgName);
		pDlg->m_sZolzFilePath = pDlg->m_sZolzFile;
		if( NULL != pLZ2 )
		{
			pDlg->m_sFolzFile = CString(pLZ2->m_pszImgName);
			pDlg->m_sFolzFilePath = pDlg->m_sFolzFile;
		}
		if( TRUE == pDlg->Create(NULL) )
		{
			pDlg->ShowWindow(SW_SHOW);
		}
		else
		{
			pDlg->DestroyWindow();
		}
	}

	return TRUE;
}
*/
