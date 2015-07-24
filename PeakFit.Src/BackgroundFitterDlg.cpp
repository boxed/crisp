// BackgroundFitterDlg.cpp : implementation file
//

#include "stdafx.h"

#include "LinearProfile.h"

#include "resource.h"
#include "BackgroundFitterDlg.h"

#include "Math\PeakSearch.h"

#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define USE_BKG_COEEFS				12

#define	CHART_PROFILE_DATA			0
#define	CHART_SELECTED_DATA			1
#define	CHART_FITTED_DATA			2
#define	CHART_DIFFERENCE_DATA		3
#define	CHART_BACKGROUND_DATA		4

#define	CURRENT_PROFILE_NUM_FMT		_T("Profile No.: %d, %s")

//////////////////////////////////////////////////////////////////////////
double PseudoVoight(double x, double dRatio, double dFWHM)
{
	double arg2 = SQR(x / dFWHM);
	return dRatio * FastExp(-arg2) + (1.0 - dRatio) / (1 + 4.0 * arg2);
}

double EstimatePseudoVoightFWHM(double dFWHM, double dRatio)
{
	double x_left, x_right, x_cur, f_left, f_right, f_cur;
	int nMaxIter = 1000;

	x_left = 0.0;
	x_right = dFWHM;
	do {
		x_cur = 0.5 * (x_left + x_right);
		f_cur   = PseudoVoight(x_cur, dRatio, dFWHM);
		f_left  = PseudoVoight(x_left, dRatio, dFWHM);
		f_right = PseudoVoight(x_right, dRatio, dFWHM);
		if( (f_left < 0.5 && f_cur > 0.5) || (f_left > 0.5 && f_cur < 0.5) )
		{
			x_right = x_cur;
		}
		else
		{
			x_left = x_cur;
		}
		nMaxIter--;
		if( nMaxIter < 0 )
		{
			break;
		}
	} while( FastAbs(f_cur - 0.5) > 1e-10 );
	return x_cur;
}

//////////////////////////////////////////////////////////////////////////

// CBackgroundFitterDlg dialog

IMPLEMENT_DYNAMIC(CBackgroundFitterDlg, CDialog)

CBackgroundFitterDlg::CBackgroundFitterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBackgroundFitterDlg::IDD, pParent)
	, m_pParent(pParent)
{
	//{{AFX_DATA_INIT(CBackgroundFitterDlg)
	m_bFitBkg = FALSE;
	m_bFitPeakPos = FALSE;
	m_bFitPeakShape = FALSE;
	m_bFitPeakAmpl = FALSE;
	m_bFWHMparams = FALSE;
	m_bPeakAsymmetry = FALSE;
	m_sChiSq = _T("");
	m_nBkgPolyN = 6;
	m_sRadiusD = _T("");
	//}}AFX_DATA_INIT
	m_nBorder_x = 0;
	m_nBorder_y = 0;
	m_nChartPos_x = 0;
	m_nChartPos_y = 0;
	m_nMaxSize = 0;
	m_bMouseDown = FALSE;
	m_nFitterType = 0;
	m_nMA = 0;
	m_hThread = 0;

	m_nMA = m_fitter.GetNumParams(0);
	m_vdParams.resize(m_nMA, 0);
	m_vdDerivs.resize(m_nMA, 0);
	m_vbParams.resize(m_nMA, false);

	m_hEventReady = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	m_nIntStart = -1;
	m_nIntEnd = -1;

	m_dScaleRad = -1;

	m_bInitialized = false;
	m_bAbortThread = false;
	m_bHaveFit = false;
}

CBackgroundFitterDlg::~CBackgroundFitterDlg()
{
	::CloseHandle(m_hEventReady);
}

BOOL CBackgroundFitterDlg::Create(CWnd* pParentWnd)
{
	return CDialog::Create(CBackgroundFitterDlg::IDD, m_pParent);
}

void CBackgroundFitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBackgroundFitterDlg)
	DDX_Check(pDX, IDC_PROF_FIT_BKG, m_bFitBkg);
	DDX_Check(pDX, IDC_PROF_FIT_PEAK_POS, m_bFitPeakPos);
	DDX_Check(pDX, IDC_PROF_FIT_PEAK_SHAPE, m_bFitPeakShape);
	DDX_Check(pDX, IDC_PROF_FIT_PEAK_AMPL, m_bFitPeakAmpl);
	DDX_Check(pDX, IDC_PROF_FIT_FWHM_PARAMS, m_bFWHMparams);
	DDX_Check(pDX, IDC_PROF_FIT_PEAK_ASYMMETRY, m_bPeakAsymmetry);
	DDX_Text(pDX, IDC_PEAK_FIT_CHI_SQ, m_sChiSq);
	DDX_CBIndex(pDX, IDC_FIT_BKG_POLY_N, m_nBkgPolyN);
	DDX_Text(pDX, IDC_RADIUS_D_VALUE, m_sRadiusD);
	//}}AFX_DATA_MAP
//	DDX_Control(pDX, IDC_TP_BKG_FITTER_CTRL, m_ctrlFitterType);
}

BEGIN_MESSAGE_MAP(CBackgroundFitterDlg, CDialog)
	//{{AFX_MSG_MAP(CBackgroundFitterDlg)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_TP_BKG_FIT_POINTS, CBackgroundFitterDlg::OnFitBkgPoints)
	ON_BN_CLICKED(IDC_FIND_PEAKS, CBackgroundFitterDlg::OnFindPeaks)
	ON_BN_CLICKED(IDC_AUTO_FIT_POINTS, CBackgroundFitterDlg::OnAutoFitPoints)
	ON_BN_CLICKED(IDC_RESET_PARAMS, CBackgroundFitterDlg::OnResetParams)
	ON_BN_CLICKED(IDC_ABORT_FITTING, CBackgroundFitterDlg::OnAbortFitting)
	ON_CBN_SELCHANGE(IDC_FIT_BKG_POLY_N, CBackgroundFitterDlg::OnSelchangeFitBkgPolyN)
	ON_MESSAGE(UM_UPDATE_FITTING, OnUpdateFitting)
	ON_MESSAGE(UM_FINNISHED_FITTING, OnTerminated)
// 	ON_THREAD_MESSAGE(UM_UPDATE_FITTING, OnUpdateFitting)
// 	ON_THREAD_MESSAGE(UM_FINNISHED_FITTING, OnTerminated)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBackgroundFitterDlg::ModifyControlStyle(UINT uiCtrlID, DWORD dwRemove /*= 0*/, DWORD dwAdd /*= 0*/)
{
	CWnd *pWnd;

	if( (pWnd = GetDlgItem(uiCtrlID)) != NULL )
	{
		pWnd->ModifyStyle(dwRemove, dwAdd);
		pWnd->RedrawWindow();
	}
}

// CBackgroundFitterDlg message handlers

BOOL CBackgroundFitterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
/*
	int iItem;
	UINT nType;

	// initialize fitters list
	NonlinearFitFunction *pType = NonlinearFitFunction::GetFirstFitter();
	while( pType )
	{
		// check the fitter
		nType = pType->GetType();
		if( (nType & pType->TYPE_1D) )//&& !(nType & pType->TYPE_ASYMMETRIC) )
		{
			iItem = m_ctrlFitterType.AddString(pType->GetName());
			if( iItem >= 0 )
			{
				if( CB_ERR == (iItem = m_ctrlFitterType.SetItemData(iItem, (DWORD)pType)) )
				{
					TRACE(_T("CBackgroundFitterDlg::OnInitDialog() -> failed to associate fitter "
						"with control string.\n"));
				}
			}
		}
		pType = pType->GetNextFitter();
	}
//	// now add some polynom fitter
//	m_ctrlFitterType.AddString(_T("Polynom"));
	// set the very first as current
	m_ctrlFitterType.SetCurSel(0);
*/
	// initialize the rect for the chart
	CRect rc, rcdlg;
	CWnd *pWnd = GetDlgItem(IDC_TP_BKG_CHART);

	if( NULL == pWnd )
	{
		return FALSE;
	}
	pWnd->GetClientRect(rc);
	pWnd->ClientToScreen(rc);
	ScreenToClient(rc);
	GetClientRect(rcdlg);

	m_nBorder_x = rcdlg.right - rc.right;
	m_nBorder_y = rcdlg.bottom - rc.bottom;

	m_nChartPos_x = rc.left - rcdlg.left;
	m_nChartPos_y = rc.top - rcdlg.top;

	VERIFY(m_wndChart.SubclassDlgItem(IDC_TP_BKG_CHART, this));

	WINDOWPLACEMENT wp;

	GetWindowPlacement(&wp);
	wp.length = sizeof(wp);

	wp.showCmd = SW_SHOWMAXIMIZED;
	SetWindowPlacement(&wp);

	ProfileToChart();

	UpdateControls();

	return TRUE;
}

void CBackgroundFitterDlg::SetProfile(double *pPoints, int nPoints)
{
	try
	{
		if( NULL == pPoints )
		{
			return;
		}
		m_Profile.resize(nPoints);
		copy(pPoints, pPoints+nPoints, m_Profile.begin());

		// get min & max elements
		m_dMinVal = *min_element(m_Profile.begin(), m_Profile.end());
		m_dMaxVal = *max_element(m_Profile.begin(), m_Profile.end());
		// set the interval to 0
		m_nMaxSize = m_Profile.size();
		m_vInterval.resize(m_nMaxSize);
		fill_n(m_vInterval.begin(), m_nMaxSize * sizeof(bool), false);
		// update the chart data sets
		m_wndChart.CreateDataSets(5, m_nMaxSize);

		PrepareProfile();
		InitializeParameters();
	}
	catch( std::exception &exc )
	{
		Trace(Format("CBackgroundFitterDlg::SetProfile() -> exception: '%1%'.") % exc.what());
	}
}

bool fnNLFITCALLBACK(double *pdParams, int nParams, double dChiSq, int nIter, LPVOID pUserData)
{
	if( NULL == pUserData )
	{
		return true;
	}
	CBackgroundFitterDlg *pDlg = (CBackgroundFitterDlg*)pUserData;
	if( true == pDlg->m_bAbortThread )
	{
		return false;
	}
	else
	{
		copy(pdParams, pdParams + nParams, pDlg->m_vdParams.begin());
		pDlg->FittedDataToChart();
		// redraw the chart
		pDlg->m_wndChart.Draw();
		pDlg->m_wndChart.InvalidateRect(NULL);
		pDlg->m_wndChart.RedrawWindow();

		pDlg->m_sChiSq.Format(_T("#%d: Chi^2=%g"), nIter, dChiSq);
		::PostMessage(pDlg->GetSafeHwnd(), UM_UPDATE_FITTING, 0, 0);
	}
	return true;
}

void CBackgroundFitterDlg::InitializeParameters()
{
	double *pdParams = &*m_vdParams.begin();
	bool *pbParams = (bool*)&*m_vbParams.begin();
	if( false == m_bInitialized )
	{
		size_t i;
		// background
		for(i = 0; i < NL_PROFILE_BKG; i++)
		{
			pdParams[i] = 0.0;
		}
		pdParams[nUL] = 5.0;				// FWHM left
		pdParams[nVL] = 0.0;
		pdParams[nWL] = 0.0;
		pdParams[nUR] = 5.0;				// FWHM right
		pdParams[nVR] = 0.0;
		pdParams[nWR] = 0.0;
		pdParams[nW]  = 0.5;				// Gauss-Lorentz factor
		for(i = 0; i < m_vPeaks.size(); i++)
		{
			pdParams[NL_PROFILE_PEAKS_START+  (i<<1)] = m_vPeaks[i].m_dX0;
			pdParams[NL_PROFILE_PEAKS_START+1+(i<<1)] = m_Profile[(int)m_vPeaks[i].m_dX0];
		}
		m_bInitialized = true;
	}
}

void CBackgroundFitterDlg::PrepareProfile()
{
	int i, iCount, nProfSize, nIntervSize;
	double dMinX, dMaxX;
	try
	{
		// sizes
		nProfSize = m_Profile.size();
		nIntervSize = m_vInterval.size();
		iCount = 0;
		// count the data
		for(i = 0; i < __min(nIntervSize,nProfSize); i++)
		{
			if( true == m_vInterval[i] )
				iCount++;
		}
		
		// allocate the data points
		m_vXcoords.resize(iCount);
		m_vYcoords.resize(iCount);
		// fill in data
		iCount = 0;
		// fill them up
		for(i = 0; i < __min(nIntervSize,nProfSize); i++)
		{
			if( true == m_vInterval[i] )
			{
				m_vXcoords[iCount] = (double)i;
				m_vYcoords[iCount] = (double)m_Profile[i];
				iCount++;
			}
		}
		
		// find min & max value of X
		if( false == m_vXcoords.empty() )
		{
			dMinX = *min_element(m_vXcoords.begin(), m_vXcoords.end());
			dMaxX = *max_element(m_vXcoords.begin(), m_vXcoords.end());
		}
		else
		{
			dMinX = 0;
			dMaxX = nProfSize;
		}
		m_dMinX = dMinX;
		m_dScaleX = 2 / (dMaxX - dMinX);
		m_fitter.m_dMinX = dMinX;
		m_fitter.m_dScale = 2 / (dMaxX - dMinX);
	}
	catch(...)
	{
		TRACE(_T("CBackgroundFitterDlg::PrepareProfile() -> unhandled exception.\n"));
	}
}

void CBackgroundFitterDlg::UpdateControls()
{
	BOOL bEnable = m_hThread == NULL;

	static UINT auiCointrols[] =
	{
		IDC_FIND_PEAKS, IDC_TP_BKG_FIT_POINTS, IDC_AUTO_FIT_POINTS, IDC_RESET_PARAMS,
		IDC_PROF_FIT_BKG, IDC_PROF_FIT_PEAK_SHAPE, IDC_PROF_FIT_PEAK_POS,
		IDC_PROF_FIT_PEAK_AMPL, IDC_PROF_FIT_FWHM_PARAMS, IDC_PROF_FIT_PEAK_ASYMMETRY,
		IDC_FIT_BKG_POLY_N, IDOK, IDCANCEL,
	};
	DWORD dwRemove = (TRUE == bEnable) ? WS_DISABLED : 0;
	DWORD dwAdd = (TRUE == bEnable) ? 0 : WS_DISABLED;
	for(int i = 0; i < sizeof(auiCointrols) / sizeof(UINT); i++)
	{
		CWnd *pWnd = GetDlgItem(auiCointrols[i]);
		if( NULL != pWnd )
		{
			pWnd->ModifyStyle(dwRemove, dwAdd);
			pWnd->UpdateWindow();
			pWnd->RedrawWindow();
		}
	}
	GetDlgItem(IDC_ABORT_FITTING)->ShowWindow(bEnable ? SW_HIDE: SW_SHOW);
}

unsigned long __stdcall ThreadFitBkgPoints(LPVOID pParams)
{
	int i;
	CBackgroundFitterDlg *pDlg = (CBackgroundFitterDlg*)pParams;

	if( NULL == pDlg )
	{
		goto Error_Exit;
	}
	try
	{
		if( TRUE != pDlg->IsKindOf(RUNTIME_CLASS(CBackgroundFitterDlg)) )
		{
			throw _T("the thread pointer is not of 'CBackgroundFitterDlg' class type");
		}
		if( true == pDlg->m_bAbortThread )
		{
			throw _T("the thread has been aborted");
		}
		// if not in auto-fit mode
		if( false == pDlg->m_bAutoFit )
		{
			// set up the initial paramters
			pDlg->m_fitter.m_vInitialParams.resize(pDlg->m_vdParams.size());
			copy(pDlg->m_vdParams.begin(), pDlg->m_vdParams.end(), pDlg->m_fitter.m_vInitialParams.begin());
		}
		int nParamsNum = pDlg->m_vPeaks.size();
		// NON-LINEAR FITTING
		double *pdParams = &*pDlg->m_vdParams.begin();
		bool *pbParams = (bool*)&*pDlg->m_vbParams.begin();

		bool bFitBackground = (TRUE == pDlg->m_bFitBkg) ? true : false;
		bool bFitPeakShape  = (TRUE == pDlg->m_bFitPeakShape) ? true : false;
		bool bFitFWHM    = (TRUE == pDlg->m_bFWHMparams) ? true : false;
		bool bFitAsymm   = (TRUE == pDlg->m_bPeakAsymmetry) ? true : false;
		bool bFitPeakPos = (TRUE == pDlg->m_bFitPeakPos) ? true : false;
		bool bFitPeakAmp = (TRUE == pDlg->m_bFitPeakAmpl) ? true : false;
		// background
		for(i = 0; i < pDlg->m_nBkgPolyN/*USE_BKG_COEEFS/*NL_PROFILE_BKG*/; i++)
		{
			pbParams[i] = bFitBackground;
		}
		for(; i < USE_BKG_COEEFS; i++)
		{
			pdParams[i] = 0;
			pbParams[i] = false;
		}
		// Peak asymmetry
		pDlg->m_fitter.bUseAsymm = bFitAsymm;
		pbParams[nUL] = bFitPeakShape;
		pbParams[nVL] = pbParams[nWL] = bFitPeakShape && bFitFWHM;
		pbParams[nUR] = bFitPeakShape && bFitAsymm;
		// FWHM, peak is asymmetric
		pbParams[nVR] = pbParams[nWR] = bFitPeakShape && bFitFWHM && bFitAsymm;
		// FWHM & Gauss-Lorentz factor
		pbParams[nW] = bFitPeakShape;
//		TRACE(_T("-------------------------\n"));
//		TRACE(_T("refining:\n"));
//		TRACE(_T("\tUl:%s\n"), pbParams[nUL] ? _T("true") : _T("false"));
//		TRACE(_T("\tVl:%s\n"), pbParams[nVL] ? _T("true") : _T("false"));
//		TRACE(_T("\tWl:%s\n"), pbParams[nWL] ? _T("true") : _T("false"));
//		TRACE(_T("\tUr:%s\n"), pbParams[nUR] ? _T("true") : _T("false"));
//		TRACE(_T("\tVr:%s\n"), pbParams[nVR] ? _T("true") : _T("false"));
//		TRACE(_T("\tWr:%s\n"), pbParams[nWR] ? _T("true") : _T("false"));
//		TRACE(_T("\tLG:%s\n"), pbParams[nW]  ? _T("true") : _T("false"));
//		TRACE(_T("-------------------------\n"));
		// peaks pos and amplitude
		for(i = 0; i < nParamsNum; i++)
		{
			pbParams[NL_PROFILE_PEAKS_START+  (i<<1)] = bFitPeakPos;
			pbParams[NL_PROFILE_PEAKS_START+1+(i<<1)] = bFitPeakAmp;
		}
		// count the parameters
		if( 0 == count_if(pbParams, pbParams + pDlg->m_nMA, bind2nd(equal_to<bool>(), true)) )
		{
			MessageBox(pDlg->GetSafeHwnd(), _T("Specify parameters to refine!"),
				_T("PeakFit warning"), MB_OK);
			goto Error_Exit;
		}
		try
		{
			bool bOldHaveFit = pDlg->m_bHaveFit;
			pDlg->m_bHaveFit = true;
			if( true == pDlg->m_nlfit.Fit(&*pDlg->m_vYcoords.begin(), &*pDlg->m_vXcoords.begin(),
				NULL, pDlg->m_vXcoords.size(), pdParams, pbParams, pDlg->m_nMA, &pDlg->m_fitter,
				fnNLFITCALLBACK, (LPVOID)pDlg) )
			{
				pDlg->m_bHaveFit = true;
				::PostMessage(pDlg->GetSafeHwnd(), UM_UPDATE_FITTING, 1, 0);
			}
			else
			{
				pDlg->m_bHaveFit = bOldHaveFit;
			}
		}
		catch(...)
		{
			TRACE(_T("CBackgroundFitterDlg::OnFitBkgPoints() -> unhandled exception.\n"));
		}
//		if( false == pDlg->m_bAutoFit )
//		{
//			::PostMessage(pDlg->GetSafeHwnd(), UM_FINNISHED_FITTING, 0, 0);
//		}
	}
	catch( TCHAR *pszError )
	{
		Trace(Format("CBackgroundFitterDlg::OnFitBkgPoints() -> exception '%1%'.") % pszError);
	}
	catch( ... )
	{
		Trace(_T("CBackgroundFitterDlg::OnFitBkgPoints() -> unhandled exception (2)."));
	}
Error_Exit:
	if( false == pDlg->m_bAutoFit )
	{
		::PostMessage(pDlg->GetSafeHwnd(), UM_FINNISHED_FITTING, 0, 0);
		// ask the thread to quit
		VERIFY(SetEvent(pDlg->m_hEventReady));
		ExitThread(0);
	}
	TRACE(_T("Fitting thread has finnished.\n"));
	return 0;
}

unsigned long __stdcall ThreadAutoFit(LPVOID pParams)
{
	CBackgroundFitterDlg *pDlg = (CBackgroundFitterDlg*)pParams;
	const int POSSIBLE_PARAMS = 6;
	static bool abRefPlan[] = {
		//    BCKGR   SHAPE    POSIT   AMPL    FWHM     ASYMM
//		/*0*/ TRUE,   FALSE,   FALSE,  FALSE,  FALSE,   FALSE,
		/*1*/ TRUE,   FALSE,   FALSE,  TRUE,   FALSE,   FALSE,
		/*2*/ TRUE,   TRUE,    FALSE,  TRUE,   FALSE,   FALSE,
		/*3*/ TRUE,   FALSE,   TRUE,   FALSE,  FALSE,   FALSE,
		/*4*/ TRUE,   TRUE,    FALSE,  TRUE,   FALSE,   FALSE,
		/*4*/ TRUE,   FALSE,   TRUE,   FALSE,  FALSE,   FALSE,
		/*5*/ TRUE,   TRUE,    FALSE,  TRUE,   FALSE,   FALSE,
		/*6*/ TRUE,   TRUE,    TRUE,   TRUE,   FALSE,   FALSE,
		/*7*/ TRUE,   TRUE,    FALSE,  TRUE,   TRUE,    FALSE,
		/*8*/ TRUE,   TRUE,    TRUE,   TRUE,   TRUE,    FALSE,
		/*9*/ TRUE,   TRUE,    TRUE,   TRUE,   TRUE,    TRUE,
	};
	int iStep, nSteps;

	if( NULL == pDlg )
	{
		goto Error_Exit;
	}
	try
	{
		if( TRUE != pDlg->IsKindOf(RUNTIME_CLASS(CBackgroundFitterDlg)) )
		{
			throw _T("the thread pointer is not of 'CBackgroundFitterDlg' class type");
		}
		pDlg->m_bAutoFit = true;

		nSteps = sizeof(abRefPlan) / sizeof(bool);
		// set up the initial paramters
		pDlg->m_fitter.m_vInitialParams.resize(pDlg->m_vdParams.size());
		copy(pDlg->m_vdParams.begin(), pDlg->m_vdParams.end(), pDlg->m_fitter.m_vInitialParams.begin());
		// iterate automatically
		for(iStep = 0; iStep < nSteps; iStep += POSSIBLE_PARAMS)
		{
			pDlg->m_bFitBkg = abRefPlan[iStep];
			pDlg->m_bFitPeakShape = abRefPlan[iStep+1];
			pDlg->m_bFitPeakPos = abRefPlan[iStep+2];
			pDlg->m_bFitPeakAmpl = abRefPlan[iStep+3];
			pDlg->m_bFWHMparams = abRefPlan[iStep+4];
			pDlg->m_bPeakAsymmetry = abRefPlan[iStep+5];
			try
			{
				ThreadFitBkgPoints(pParams);
			}
			catch( TCHAR *pszError )
			{
				Trace(Format("ThreadAutoFit -> exception (1) '%1%'.") % pszError);
			}
			catch( ... )
			{
				Trace(_T("ThreadAutoFit -> unhandled exception (1)."));
			}
		}
		::PostMessage(pDlg->GetSafeHwnd(), UM_FINNISHED_FITTING, 0, 0);
	}
	catch( TCHAR *pszError )
	{
		Trace(Format("ThreadAutoFit -> exception (2) '%1%'.") % pszError);
	}
	catch( ... )
	{
		Trace(_T("ThreadAutoFit -> unhandled exception (2)."));
	}
Error_Exit:
	// ask the thread to quit
	SetEvent(pDlg->m_hEventReady);
	ExitThread(0);
	return 0;
}

void CBackgroundFitterDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if( (0 == m_nBorder_x) | (0 == m_nBorder_y) )
	{
		return;
	}

	CWnd *pWnd = GetDlgItem(IDC_TP_BKG_CHART);

	if( NULL == pWnd )
	{
		return;
	}

	CRect rcdlg;
	GetClientRect(rcdlg);

	cx = rcdlg.right - m_nBorder_x - m_nChartPos_x;
	cy = rcdlg.bottom - m_nBorder_y - m_nChartPos_y;

	if( cx < m_nChartPos_x )
		cx = m_nChartPos_x;
	if( cy < m_nChartPos_y )
		cy = m_nChartPos_y;

	pWnd->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE  | SWP_SHOWWINDOW);
	pWnd->RedrawWindow();
}

void CBackgroundFitterDlg::FittedDataToChart()
{
	float val;
	double y, *pdParams;
	size_t nProfSize = m_Profile.size();
	
	m_fitter.m_dMinX = m_dMinX;
	m_fitter.m_dScale = m_dScaleX;
	// successfully fitted
	pdParams = &*m_vdParams.begin();

	// calculate the fitted function and the difference
	for(size_t i = 0; i < m_nMaxSize; i++)
	{
		if( false == m_vInterval[i] )
		{
			m_wndChart.SetPoint((float)i, (float)0, 2, i);
			m_wndChart.SetPoint((float)i, (float)0, 3, i);
			m_wndChart.SetPoint((float)i, (float)0, 4, i);
			continue;
		}
		if( i < nProfSize )
			val = (float)m_Profile[i];
		if( true == m_bHaveFit )
		{
			y = m_fitter.Calculate(i, pdParams, &*m_vdDerivs.begin(), m_vdParams.size());
			if( y >= m_dMaxVal )
				y = m_dMaxVal;
			m_wndChart.SetPoint((float)i, (float)y, 2, i);
			y = val - y;
			m_wndChart.SetPoint((float)i, (float)y, 3, i);
			y = m_fitter.CalculateBkg(i, pdParams);
			if( y >= m_dMaxVal )
				y = m_dMaxVal;
			m_wndChart.SetPoint((float)i, (float)y, 4, i);
		}
	}
}

void CBackgroundFitterDlg::ProfileToChart()
{
	UINT nSize;

	// profile size
	nSize = m_Profile.size();
	// fast access data
	double *pData = &*m_Profile.begin();
	float val;
	size_t i;

	if( 0 )//true == m_ProfFit.m_bFitted )
	{
		// fill-up the chart
		for(i = 0; i < nSize; i++)
		{
			val = (float)pData[i];
			m_wndChart.SetPoint((float)i, val, 0, i);
			if( m_vInterval[i] )
				m_wndChart.SetPoint((float)i, val, 1, i);
			else
				m_wndChart.SetPoint((float)i, 0, 1, i);
		}
		// fill by the last value
		for(; i < m_nMaxSize; i++)
		{
			m_wndChart.SetPoint((float)i, val, 0, i);
			if( m_vInterval[i] )
				m_wndChart.SetPoint((float)i, val, 1, i);
			else
				m_wndChart.SetPoint((float)i, 0, 1, i);
		}
/*
		NonlinearFitFunction *pFitter = NonlinearFitFunction::GetFirstFitter();
		while( pFitter )
		{
			if( pFitter == m_ProfFit.m_pfnFuncs )
			{
				break;
			}
			pFitter = pFitter->GetNextFitter();
		}
		// found the fitter - set it active in the combo box
		if( pFitter )
		{
			for(int i = 0; i < m_ctrlFitterType.GetCount(); i++)
			{
				if( pFitter == (NonlinearFitFunction*)
					m_ctrlFitterType.GetItemDataPtr(i) )
				{
					m_ctrlFitterType.SetCurSel(i);
					break;
				}
			}
		}
*/
		// update fitted data
		FittedDataToChart();
	}
	else
	{
		// fill-up the chart
		for(i = 0; i < nSize; i++)
		{
			val = (float)pData[i];
			m_wndChart.SetPoint((float)i, val, 0, i);
			m_wndChart.SetPoint((float)i, 0, 1, i);
			m_wndChart.SetPoint((float)i, 0, 2, i);
			m_wndChart.SetPoint((float)i, 0, 3, i);
			m_wndChart.SetPoint((float)i, 0, 4, i);
		}
		// fill by the last value
		for(; i < m_nMaxSize; i++)
		{
			m_wndChart.SetPoint((float)i, val, 0, i);
			m_wndChart.SetPoint((float)i, 0, 1, i);
			m_wndChart.SetPoint((float)i, 0, 2, i);
			m_wndChart.SetPoint((float)i, 0, 3, i);
			m_wndChart.SetPoint((float)i, 0, 4, i);
		}
	}
	// redraw the chart
	m_wndChart.Draw();
	m_wndChart.CalculateLimits();
	m_wndChart.InvalidateRect(NULL);
	m_wndChart.RedrawWindow();
}

BOOL CBackgroundFitterDlg::CheckChartRect(CPoint point)
{
	CRect rc(0, 0, 0, 0);
	try
	{
		CWnd *pWnd = GetDlgItem(IDC_TP_BKG_CHART);

		if( NULL == pWnd )
		{
			return FALSE;
		}
		pWnd->GetClientRect(rc);
		pWnd->ClientToScreen(rc);
		ClientToScreen(&point);
	}
	catch( ... )
	{
		TRACE(_T("CBackgroundFitterDlg::CheckChartRect() -> unhandled exception.\n"));
	}
	return rc.PtInRect(point);
}

void CBackgroundFitterDlg::DrawZoomRect(CPoint ptFirst, CPoint ptLast)
{
	try
	{
		CWnd *pWnd = GetDlgItem(IDC_TP_BKG_CHART);
		CRect rc, rc1, rc2;

		if( NULL == pWnd )
		{
			return;
		}
		pWnd->GetClientRect(rc1);

		CClientDC dc(pWnd);

		if( ptFirst.x > ptLast.x )
			swap(ptFirst.x, ptLast.x);
		if( ptFirst.y > ptLast.y )
			swap(ptFirst.y, ptLast.y);

		rc2 = CRect(ptFirst, ptLast);

		rc.IntersectRect(rc1, rc2);

		int nOldMode = dc.SetROP2(R2_NOT);

		ptFirst = rc.TopLeft();
		ptLast = rc.BottomRight();
		dc.MoveTo(ptFirst);
		dc.LineTo(ptFirst.x, ptLast.y);
		dc.LineTo(ptLast);
		dc.LineTo(ptLast.x, ptFirst.y);
		dc.LineTo(ptFirst);

		dc.SetROP2(nOldMode);
	}
	catch( ... )
	{
		TRACE(_T("CBackgroundFitterDlg::DrawZoomRect() -> unhandled exception.\n"));
	}
}

void CBackgroundFitterDlg::DrawSelectionRect(CPoint ptFirst, CPoint ptLast)
{
	try
	{
		CWnd *pWnd = GetDlgItem(IDC_TP_BKG_CHART);
		CRect rc;

		if( NULL == pWnd )
		{
			return;
		}
		pWnd->GetClientRect(rc);

		CClientDC dc(pWnd);

		if( rc.left > ptFirst.x )
			ptFirst.x = rc.left;
		if( rc.right < ptFirst.x )
			ptFirst.x = rc.right;
		if( rc.left > ptLast.x )
			ptLast.x = rc.left;
		if( rc.right < ptLast.x )
			ptLast.x = rc.right;

		int nOldMode = dc.SetROP2(R2_NOT);
		dc.MoveTo(ptFirst.x, rc.top);
		dc.LineTo(ptFirst.x, rc.bottom);
		dc.MoveTo(ptLast.x, rc.top);
		dc.LineTo(ptLast.x, rc.bottom);

		dc.SetROP2(nOldMode);
	}
	catch( ... )
	{
		TRACE(_T("CBackgroundFitterDlg::DrawSelectionRect() -> unhandled exception.\n"));
	}
}

void CBackgroundFitterDlg::AddInterval(CVector2f &vfMouseDown,
									   CVector2f &vfMouseUp, bool bFill)
{
	try
	{
		float val;
		int left = (int)floor(vfMouseDown.x + 0.5f);
		int right = (int)floor(vfMouseUp.x + 0.5f);

		if( left > right )
			swap(left, right);
		if( left == right )
			return;

		// fast access data
		double *pData = &*m_Profile.begin();
		size_t nSize = m_Profile.size();
		for(size_t i = (size_t)__max(0,left); i <= (size_t)__min(right,m_nMaxSize-1); i++)
		{
			m_vInterval[i] = bFill;
			if( i >= nSize )
				val = (float)pData[nSize - 1];
			else
				val = (float)pData[i];
			if( false == bFill )
				val = 0.0f;
			m_wndChart.SetPoint((float)i, val, 1, i);
		}
		UpdateIntervalEnds();
	}
	catch( ... )
	{
		TRACE(_T("CBackgroundFitterDlg::AddInterval() -> unhandled exception.\n"));
	}
}

void CBackgroundFitterDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd *pWnd = GetDlgItem(IDC_TP_BKG_CHART);
	CRect rc;

	pWnd->GetClientRect(rc);
	pWnd->ClientToScreen(rc);

	CPoint LogPt = point;
	ClientToScreen(&LogPt);

	if( FALSE == rc.PtInRect(LogPt) )
		return;

	pWnd->ScreenToClient(&LogPt);

	CDC *pDC = pWnd->GetDC();
	pDC->DPtoLP(&LogPt);
	ReleaseDC(pDC);

	m_ptFirst = m_ptCurrent = LogPt;
	m_bMouseDown = FALSE;
	if( TRUE == CheckChartRect(point) )
	{
		m_bMouseDown = TRUE;
		m_wndChart.BackMapPoint(LogPt, m_vfMouseDown.x, m_vfMouseDown.y);
	}

	ClipCursor(rc);

	// we need to redraw window
	m_wndChart.Draw();
	m_wndChart.RedrawWindow();

	CDialog::OnLButtonDown(nFlags, point);
}

void CBackgroundFitterDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CWnd *pWnd = GetDlgItem(IDC_TP_BKG_CHART);
	CRect rc;

	CPoint LogPt = point;
	ClientToScreen(&LogPt);
	pWnd->ScreenToClient(&LogPt);
	pWnd->GetClientRect(rc);

	if( FALSE == rc.PtInRect(LogPt) )
		return;

	CDC *pDC = pWnd->GetDC();
	pDC->DPtoLP(&LogPt);
	ReleaseDC(pDC);

	if( TRUE == m_bMouseDown )
	{
		m_bMouseDown = TRUE;
		m_wndChart.BackMapPoint(LogPt, m_vfMouseUp.x, m_vfMouseUp.y);
		if( (!(nFlags & MK_CONTROL)) && (!(nFlags & MK_SHIFT)) )
		{
			// do the processing ONLY if the thread is NULL
			if( NULL == m_hThread )
			{
				// add interval
				AddInterval(m_vfMouseDown, m_vfMouseUp, true);
			}
		}
		else if( (nFlags & MK_CONTROL) && (!(nFlags & MK_SHIFT)) )
		{
			// do the processing ONLY if the thread is NULL
			if( NULL == m_hThread )
			{
				// remove interval
				AddInterval(m_vfMouseDown, m_vfMouseUp, false);
			}
		}
		else if( (nFlags & MK_SHIFT) && (!(nFlags & MK_CONTROL)) )
		{
			// zoom the image
			DrawZoomRect(m_ptFirst, m_ptCurrent);
			m_wndChart.SetZoomMode(CRect(m_ptFirst, m_ptCurrent));
		}
	}
	m_bMouseDown = FALSE;

	ClipCursor(NULL);

	// we need to redraw window
	m_wndChart.Draw();
	m_wndChart.RedrawWindow();

	CDialog::OnLButtonUp(nFlags, point);
}

void CBackgroundFitterDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CWnd *pWnd = GetDlgItem(IDC_TP_BKG_CHART);

	CPoint LogPt = point;
	ClientToScreen(&LogPt);
	pWnd->ScreenToClient(&LogPt);

	CDC *pDC = pWnd->GetDC();
	pDC->DPtoLP(&LogPt);
	ReleaseDC(pDC);

	UpdateData();
	if( TRUE == m_bMouseDown )
	{
		if( nFlags & MK_SHIFT )
		{
			// selection rect
			DrawZoomRect(m_ptFirst, m_ptCurrent);
			m_ptCurrent = LogPt;
			DrawZoomRect(m_ptFirst, m_ptCurrent);
		}
		else// if( nFlags & MK_CONTROL )
		{
			// do the processing ONLY if the thread is NULL
			if( NULL == m_hThread )
			{
				// add/remove interval
				DrawSelectionRect(m_ptFirst, m_ptCurrent);
				m_ptCurrent = LogPt;
				DrawSelectionRect(m_ptFirst, m_ptCurrent);
			}
		}
	}
	// update the coordinates
	CPoint pt;
	CRect rc;
	float x, y;
	m_wndChart.GetWindowRect(rc);
	if( TRUE == rc.PtInRect(point) )
	{
		pt.x = point.x - rc.left;
		pt.y = point.y - rc.top;
		m_wndChart.BackMapPoint(pt, x, y);
		if( x > 0 )
		{
			if( -1 != m_dScaleRad )
			{
				m_sRadiusD.Format(_T("R = %5d, d = %7.3fÅ"), (int)x, m_dScaleRad / x);
			}
			else
			{
				m_sRadiusD.Format(_T("R = %5d, d = ---"), (int)x);
			}
		}
	}
	UpdateData(FALSE);
	CDialog::OnMouseMove(nFlags, point);
}

void CBackgroundFitterDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CDialog::OnRButtonDown(nFlags, point);
}

void CBackgroundFitterDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_wndChart.RestoreZoomMode();
	
	CDialog::OnRButtonUp(nFlags, point);
}

LRESULT CBackgroundFitterDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT lRes = 0;

	__try
	{
		lRes = CDialog::WindowProc(message, wParam, lParam);
	}
	__except(EXCEPTION_EXECUTE_HANDLER )
	{
		TRACE(_T("CBackgroundFitterDlg::WindowProc() -> unhandled exception.\n"));
	}
	return lRes;
}

void CBackgroundFitterDlg::UpdateIntervalEnds()
{
	CProfileFitter::IntervalIt it;

	m_nIntStart = m_nIntEnd = -1;
	if( m_vInterval.end() == (it = find(m_vInterval.begin(), m_vInterval.end(), true)) )
	{
		return;
	}
	m_nIntStart = m_nIntEnd	= it - m_vInterval.begin();
	// find the end of the WHOLE interval (the end of the rightmost part)
	for(size_t i = 0; i < m_vInterval.size(); i++)
	{
		if( true == m_vInterval[i] )
		{
			m_nIntEnd = i;
		}
	}
}

void CBackgroundFitterDlg::OnFindPeaks() 
{
	CProfile peaks;
//	CString str;

	// check if we tried to select some interval
	if( m_vInterval.end() == find(m_vInterval.begin(), m_vInterval.end(), true) )
	{
		MessageBox(_T("No area selected. Select region using the mouse"),
			_T("WARNING!"));
		return;
	}
	UpdateIntervalEnds();
	if( true == PeakSearch::FindPeaksSmoothDeriv(m_Profile, peaks, 10, true) )
	{
		m_vPeaks.clear();
		m_wndChart.ResetPeaks();
//		str.Format(_T("Found %d peaks"), peaks.size());
//		MessageBox(str, _T("Peaks"));
		for(size_t i = 0; i < peaks.size(); i++)
		{
			if( (peaks[i] >= m_nIntStart) && (peaks[i] <= m_nIntEnd) )
			{
				AddPeak(peaks[i]);
			}
		}
		Peaks2Params();
	}
	m_wndChart.InvalidateRect(NULL);
	m_wndChart.RedrawWindow();
}

void CBackgroundFitterDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// do the processing ONLY if the thread is NULL
	if( NULL == m_hThread )
	{
		CRect rc;
		CPoint pt;
		bool bFound = false;
		float x, y;

		m_wndChart.GetWindowRect(rc);
		if( TRUE == rc.PtInRect(point) )
		{
			size_t i;
			pt.x = point.x - rc.left;
			pt.y = point.y - rc.top;
			m_wndChart.BackMapPoint(pt, x, y);
			for(i = 0; i < m_vPeaks.size(); i++)
			{
				if( FastAbs(m_vPeaks[i].m_dX0 - x) < 3 )
				{
					bFound = true;
					break;
				}
			}
			if( true == bFound )
			{
				// erase it
				RemovePeak(i);
			}
			else
			{
				// add it
				AddPeak(x);
			}
			Peaks2Params();
			m_wndChart.InvalidateRect(NULL);
			m_wndChart.RedrawWindow();
		}
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CBackgroundFitterDlg::AddPeak(double x)
{
	vector<double> temp(m_vdParams.size(), 0);
	double bkg = m_fitter.Calculate(x, &*m_vdParams.begin(), &*temp.begin(), m_vdParams.size());
	double ampl = m_Profile[(int)x] - bkg;
	CGraph2Dpeak peak;
	peak.m_dX0 = x;
	peak.m_dFWHM = 0;
	peak.m_dAmpl = (ampl > 0) ? ampl : 0.01;
	m_vPeaks.push_back(peak);
	Peaks2Params();
	FittedDataToChart();
}

void CBackgroundFitterDlg::RemovePeak(int iPos)
{
	m_vPeaks.erase(m_vPeaks.begin() + iPos);
}

void CBackgroundFitterDlg::Params2Peaks()
{
	CGraph2Dpeak peak;

	m_wndChart.ResetPeaks();
	m_vPeaks.clear();
	for(size_t i = NL_PROFILE_PEAKS_START; i < m_vdParams.size(); i += 2)
	{
		peak.m_dFWHM = 0;
		peak.m_dX0 = m_vdParams[i];
		peak.m_dAmpl = m_vdParams[i+1];
		m_wndChart.AddPeak(peak);
		m_vPeaks.push_back(peak);
	}
}

void CBackgroundFitterDlg::Peaks2Params()
{
	std::vector<double> temp(m_vdParams);

	m_nMA = m_fitter.GetNumParams(m_vPeaks.size());
	m_vdParams.resize(m_nMA, 0);
	m_vdDerivs.resize(m_nMA, 0);
	m_vbParams.resize(m_nMA, false);
	// copy initial parameters
	copy(temp.begin(), temp.begin() + NL_PROFILE_PEAKS_START, m_vdParams.begin());
	// fill in the peaks
	m_wndChart.ResetPeaks();
	for(size_t i = 0; i < m_vPeaks.size(); i++)
	{
		m_vdParams[NL_PROFILE_PEAKS_START+  (i<<1)] = m_vPeaks[i].m_dX0;
		m_vdParams[NL_PROFILE_PEAKS_START+1+(i<<1)] = m_vPeaks[i].m_dAmpl;
		m_wndChart.AddPeak(m_vPeaks[i]);
	}
}
/*
void CBackgroundFitterDlg::OnCancel() 
{
	m_bAbortThread = true;
	if( NULL != m_hThread )
	{
		while( WAIT_TIMEOUT == ::WaitForSingleObject(m_hEventReady, 250) )
		{
			Sleep(250);
		}
	}
	CDialog::OnCancel();
}
*/
void CBackgroundFitterDlg::OnDestroy() 
{
	m_bAbortThread = true;
	if( NULL != m_hThread )
	{
		while( WAIT_TIMEOUT == ::WaitForSingleObject(m_hEventReady, 50) )
		{
			Sleep(50);
		}
	}
	CDialog::OnDestroy();
}

void CBackgroundFitterDlg::OnOK() 
{
	if( 0 != m_hThread )
	{
		m_bAbortThread = true;
		while( WAIT_TIMEOUT == ::WaitForSingleObject(m_hEventReady, 250) )
		{
			Sleep(250);
		}
	}
	double *a = &*m_vdParams.begin();
	for(size_t i = 0; i < m_vPeaks.size(); i++)
	{
		EDRD_REFL refl;
		refl.dAmpl = m_vPeaks[i].m_dAmpl;
		refl.dX0 = m_vPeaks[i].m_dX0;
		refl.dIntens = m_fitter.IntegrInt(a, i);
		refl.dSigma = 0;
		refl.dFWHM = m_fitter.FWHM(a, i);
		m_vOutputRefls.push_back(refl);
	}
	CDialog::OnOK();
}

void CBackgroundFitterDlg::OnFitBkgPoints()
{
	UpdateData();
	if( NULL != m_hThread )
	{
		return;
	}
	// check if we tried to select some interval
	if( m_vInterval.end() == find(m_vInterval.begin(), m_vInterval.end(), true) )
	{
		MessageBox(_T("No area selected. Select region using the mouse"),
			_T("WARNING!"));
		return;
	}
	if( false == m_bInitialized )
	{
		InitializeParameters();
	}
	PrepareProfile();
	m_bAbortThread = false;
	DWORD dwId;
	m_bAutoFit = false;
	m_hThread = CreateThread(NULL, 0, ThreadFitBkgPoints, this, 0, &dwId);
	SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST);
	UpdateControls();
}

void CBackgroundFitterDlg::OnAutoFitPoints()
{
	UpdateData();
	if( NULL != m_hThread )
	{
		return;
	}
	// check if we tried to select some interval
	if( m_vInterval.end() == find(m_vInterval.begin(), m_vInterval.end(), true) )
	{
		MessageBox(_T("No area selected. Select region using the mouse"), _T("WARNING!"));
		return;
	}
	if( m_vPeaks.empty() )
	{
		MessageBox(_T("No peaks selected. Select peaks using the mouse"), _T("WARNING!"));
		return;
	}
	if( false == m_bInitialized )
	{
		InitializeParameters();
	}
	PrepareProfile();
	m_bAbortThread = false;
	DWORD dwId;
	m_hThread = CreateThread(NULL, 0, ThreadAutoFit, this, 0, &dwId);
	SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST);
	UpdateControls();
}

void CBackgroundFitterDlg::OnResetParams()
{
	m_bInitialized = false;
	Peaks2Params();
	InitializeParameters();
	Params2Peaks();
	m_bHaveFit = false;
	for(size_t i = 0; i < m_nMaxSize; i++)
	{
		m_wndChart.SetPoint((float)i, (float)0, 2, i);
		m_wndChart.SetPoint((float)i, (float)0, 3, i);
		m_wndChart.SetPoint((float)i, (float)0, 4, i);
	}
	FittedDataToChart();
	// redraw the chart
	m_wndChart.Draw();
	m_wndChart.RedrawWindow();
}

LRESULT CBackgroundFitterDlg::OnTerminated(WPARAM wParam, LPARAM lParam)
{
	TRACE(_T("CBackgroundFitterDlg::OnTerminated() -> thread has been terminated.\n"));
	m_hThread = NULL;
	UpdateControls();
	return 0;
}

LRESULT CBackgroundFitterDlg::OnUpdateFitting(WPARAM wParam, LPARAM lParam)
{
	TRACE(_T("CBackgroundFitterDlg::OnUpdateFitting() -> has been called.\n"));
	if( false == m_bAbortThread )
	{
		if( 1 == wParam )
		{
			Params2Peaks();
			FittedDataToChart();
			// redraw the chart
			m_wndChart.Draw();
			m_wndChart.RedrawWindow();
		}
		UpdateData(FALSE);
	}
	return 0;
}

void CBackgroundFitterDlg::OnAbortFitting() 
{
	m_bAbortThread = true;
}

void CBackgroundFitterDlg::OnSelchangeFitBkgPolyN() 
{
	UpdateData();
	if( m_nBkgPolyN < 0 )
	{
		m_nBkgPolyN = 0;
	}
	if( m_nBkgPolyN > 12 )
	{
		m_nBkgPolyN = 12;
	}
	UpdateData(FALSE);
}
