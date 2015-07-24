// PeakFit.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"

#include "PeakFit.h"
#include "BackgroundFitterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HINSTANCE hInst = NULL;

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
// CPeakFitApp

BEGIN_MESSAGE_MAP(CPeakFitApp, CWinApp)
	//{{AFX_MSG_MAP(CPeakFitApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPeakFitApp construction

CPeakFitApp::CPeakFitApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPeakFitApp object

CPeakFitApp theApp;

extern "C" BOOL __declspec(dllexport) CALLBACK Free_PeakFit(LPVOID pData)
{
	__try
	{
		if( NULL != pData )
		{
			free(pData);
		}
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
		MessageBox(NULL, _T("Free_PeakFit() -> Unhandled exception!"), _T("EDRD ERROR"), MB_OK);
	}
	return TRUE;
}

int _cdecl sort_arc_R(const void *pEl1, const void *pEl2)
{
	EDRD_REFL *pArc1 = (EDRD_REFL*)pEl1, *pArc2 = (EDRD_REFL*)pEl2;
	
	if( pArc1->dX0 < pArc2->dX0 )		return -1;
	else if( pArc1->dX0 == pArc2->dX0 )	return  0;
	return  1;
}

static double FindAmplScale(EDRD_REFL *pData, int nRefls)
{
	double dScale = 1.0;
	double dMax = 0.0;
	
	for(int i = 0; i < nRefls; i++)
	{
		if( pData[i].dIntens > dMax )
		{
			dMax = pData[i].dIntens;
		}
	}
	if( dMax > 0.0 )
	{
		dScale = 10000.0 / dMax;
	}
	
	return dScale;
}

extern "C" BOOL __declspec(dllexport) CALLBACK Run_PeakFit(LPVOID pPtr, double dScale)
{
	LPEDRD pEdrd = (LPEDRD)pPtr;
	if( NULL == pEdrd )
	{
		return FALSE;
	}
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	try
	{
		CBackgroundFitterDlg dlg;
		dlg.SetProfile(pEdrd->lpRDF, pEdrd->r);
		dlg.m_dScaleRad = dScale;
		if( IDOK == dlg.DoModal() )
		{
			double dScaleI;
			LPARC lpa = pEdrd->lpArc;
			int nRefls = dlg.m_vOutputRefls.size();
			pEdrd->arccnt = 0;
			qsort(&*dlg.m_vOutputRefls.begin(), dlg.m_vOutputRefls.size(), sizeof(EDRD_REFL), sort_arc_R);
			dScaleI = FindAmplScale(&*dlg.m_vOutputRefls.begin(), dlg.m_vOutputRefls.size());
			for(int i = 0; i < nRefls; i++)
			{
				lpa[i].arcno = i;
				lpa[i].intens = dlg.m_vOutputRefls[i].dIntens;
				lpa[i].io = dlg.m_vOutputRefls[i].dIntens * dScaleI;
				lpa[i].rad = dlg.m_vOutputRefls[i].dX0;
				lpa[i].wid = dlg.m_vOutputRefls[i].dFWHM;
				if( (0 == pEdrd->ae0) && (0 == pEdrd->ae1) )
				{
					lpa[i].a0 = 0;
					lpa[i].a1 = M_2PI;
				}
				else
				{
					lpa[i].a0 = pEdrd->ae0;
					lpa[i].a1 = pEdrd->ae1;
				}
				CString str;
				str.Format(_T("reflection %d, I = %g\n"), i+1, lpa[i].intens);
				::OutputDebugString(str);
//				pEdrd->arccnt++;
			}
			pEdrd->arccnt = nRefls;
//			if( nRefls > 0 )
//			{
//				pData = malloc(nRefls * sizeof(EDRD_REFL));
//				std::copy(dlg.m_vOutputRefls.begin(), dlg.m_vOutputRefls.end(), (EDRD_REFL*)pData);
//			}
		}
	}
	catch(...)
	{
		MessageBox(NULL, _T("Run_PeakFit() -> Unhandled exception!"), _T("EDRD ERROR"), MB_OK);
	}
	return TRUE;
}
/*
extern "C" LPVOID __declspec(dllexport) CALLBACK Run_PeakFit(LPVOID pList1, LPVOID pList2,
															 LONG &nRefls, double dScale)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	LPVOID pData = NULL;
	try
	{
		CBackgroundFitterDlg dlg;
//		FILE *pFile = _tfopen(_T("G:\\My downloads\\Math\\fityk-0.7.2\\samples\\sic_zn.dat"), _T("rt"));
//		FILE *pFile = _tfopen(_T("G:\\My downloads\\Math\\fityk-0.7.2\\samples\\nacl01.dat"), _T("rt"));
//		vector<double> vX, vY;
//		if( pFile )
//		{
//			double x, y;
//			char str[256];
//			while( fgets(str, 256, pFile) )
//			{
//				sscanf(str, "%lf %lf", &x, &y);
//				vX.push_back(x);
//				vY.push_back(y);
//			}
//			fclose(pFile);
//			dlg.SetProfile(vY.begin(), vY.size());
//		}
//		else
//		{
		dlg.SetProfile((double*)pList1, *(int*)pList2);
//		}
		dlg.m_dScaleRad = dScale;
		if( IDOK == dlg.DoModal() )
		{
			nRefls = dlg.m_vOutputRefls.size();
//			for(int i = 0; i < nRefls; i++)
//			{
//				CString str;
//				str.Format(_T("reflection %d, I = %g"), i+1, dlg.m_vOutputRefls[i].dIntens);
//				AfxMessageBox(str);
//			}
			if( nRefls > 0 )
			{
				pData = malloc(nRefls * sizeof(EDRD_REFL));
				std::copy(dlg.m_vOutputRefls.begin(), dlg.m_vOutputRefls.end(), (EDRD_REFL*)pData);
			}
		}
	}
	catch(...)
	{
		MessageBox(NULL, _T("Run_PeakFit() -> Unhandled exception!"), _T("EDRD ERROR"), MB_OK);
	}
	return pData;
}
*/
