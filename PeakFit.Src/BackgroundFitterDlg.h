#pragma once

#include "afxwin.h"

#include "Profile.h"
#include "Graph2DW.h"
#include "ProfileFitter.h"

typedef double* LPDOUBLE;
#include "../Crisp2.src/EDRD.h"

#define UM_FINNISHED_FITTING		WM_USER+100
#define UM_UPDATE_FITTING			WM_USER+101

#define NL_PROFILE_BKG				12
#define NL_PROFILE_PEAKS_START		(NL_PROFILE_BKG+7)

const int nUL = NL_PROFILE_BKG;
const int nVL = NL_PROFILE_BKG+1;
const int nWL = NL_PROFILE_BKG+2;
const int nUR = NL_PROFILE_BKG+3;
const int nVR = NL_PROFILE_BKG+4;
const int nWR = NL_PROFILE_BKG+5;
const int nW  = NL_PROFILE_BKG+6;

double EstimatePseudoVoightFWHM(double dFWHM, double dRatio);

class NlProfile1D : public NonlinearFit1Function
{
public:
	NlProfile1D()
	{
		m_dMinX = 0;
		m_dScale = 0;
		bUseAsymm = false;
	}
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA)
	{
	}
	virtual double CalculateBkg(double x, double *a)
	{
		double x2, _x, val;
		_x = m_dScale * (x - m_dMinX) - 1.0;
		x2 = _x;
		val = a[0];
		for(int i = 1; i < NL_PROFILE_BKG; i++)
		{
			val += a[i]*x2;
			x2 *= _x;
		}
		return val;
	}
	virtual void StepParameters(double *pdAtry, double *pdParams,
								bool *pbParams, int nMA, double *pdDelta)
	{
		double old_pos;
		bool bCheckOffset = false;
		NonlinearFit1Function::StepParameters(pdAtry, pdParams, pbParams, nMA, pdDelta);

		if( true == pbParams[nW] )
		{
			if( pdAtry[nW] < 0.0 )
			{
				pdAtry[nW] = 0.0;
			}
			if( pdAtry[nW] > 1.0 )
			{
				pdAtry[nW] = 1.0;
			}
		}
		if( (false == pbParams[nUR]) && (true == pbParams[nUL]) )
		{
			pdParams[nUR] = pdParams[nUL];
			pdAtry[nUR] = pdAtry[nUL];
		}
		if( (false == pbParams[nVR]) && (true == pbParams[nVL]) )
		{
			pdParams[nVR] = pdParams[nVL];
			pdAtry[nVR] = pdAtry[nVL];
		}
		if( (false == pbParams[nWR]) && (true == pbParams[nWL]) )
		{
			pdParams[nWR] = pdParams[nWL];
			pdAtry[nWR] = pdAtry[nWL];
		}
		// test the maximum offset of each peak - must not be more than Half-width
		if( nMA == m_vInitialParams.size() )
		{
			bCheckOffset = true;
		}
		double dHW = pdAtry[nUL];
		for(int i = NL_PROFILE_PEAKS_START; i < nMA; i += 2)
		{
			double pos = pdAtry[i];
			double ampl = pdAtry[i+1];
			if( ampl < 1e-3 )
			{
				pdAtry[i+1] = 0.01;
			}
			if( true == bCheckOffset )
			{
				old_pos = m_vInitialParams[i];
				if( FastAbs(pos - old_pos) > dHW )
				{
					// the candidate
					pdAtry[i] = pdParams[i];//(pos < old_pos) ? old_pos - d;
				}
			}
		}
	}
	virtual double Calculate(double x, double *a, double *dyda, int na)
	{
		double ampl, arg, arg2, val, w, w_1, tmp;
		double fwhm, fwhm_l, fwhm_r, x2, _x, peak, x0, gauss, lorentz;
		int i, fwhm_i;

		_x = m_dScale * (x - m_dMinX) - 1.0;
		val = a[0];
		dyda[0] = 1.0;
		x2 = _x;
		for(i = 1; i < NL_PROFILE_BKG; i++)
		{
			val += a[i]*x2;
			dyda[i] = x2;
			x2 *= _x;
		}
		for(i = NL_PROFILE_BKG; i < NL_PROFILE_PEAKS_START; i++)
		{
			dyda[i] = 0;
		}
		x2 = _x*_x;
		fwhm_l = 1.0 / FastAbs(a[nUL] + FastAbs(a[nVL])*_x + FastAbs(a[nWL])*x2);
		if( true == bUseAsymm )
		{
			fwhm_r = 1.0 / FastAbs(a[nUR] + FastAbs(a[nVR])*_x + FastAbs(a[nWR])*x2);
		}
		else
		{
			fwhm_r = fwhm_l;
		}
		w = a[nW];
		w_1 = 1.0 - w;
		for(i = NL_PROFILE_PEAKS_START; i < na; i += 2)
		{
			x0 = a[i];
			ampl = FastAbs(a[i+1]);
//			if( x0 <= 0.0 ) continue;
			fwhm = fwhm_l;
			fwhm_i = nUL;
			if( (true == bUseAsymm) && (x >= x0) )
			{
				fwhm = fwhm_r;
				fwhm_i = nUR;
			}
			arg = (x - x0) * fwhm;
			arg2 = arg*arg;
			gauss = FastExp(-arg2);
			lorentz = 1.0 / (1.0 + 4.0*arg2);
			peak = w*gauss + w_1*lorentz;
			val += ampl*peak;
			tmp = 2.0*ampl*arg*fwhm*(w*gauss + 4.0*w_1*lorentz*lorentz);
			dyda[i] = tmp;
			dyda[i+1] = peak;
			dyda[nW] += ampl*(gauss - lorentz);
			tmp *= arg;
			dyda[fwhm_i]   += tmp;
			dyda[fwhm_i+1] += tmp*_x;
			dyda[fwhm_i+2] += tmp*x2;
		}
		return val;
	}
	double IntegrInt(double *a, int peakN)
	{
		double w = a[nW];
		double x = a[NL_PROFILE_PEAKS_START + (peakN << 1)];
		double ampl = a[NL_PROFILE_PEAKS_START + (peakN << 1) + 1];
		double _x = m_dScale * (x - m_dMinX) - 1.0;
		double fwhm_l = FastAbs(a[nUL] + (FastAbs(a[nVL]) + FastAbs(a[nWL])*_x)*_x);
		double fwhm_r = FastAbs(a[nUR] + (FastAbs(a[nVR]) + FastAbs(a[nWR])*_x)*_x);
		double w1 = EstimatePseudoVoightFWHM(fwhm_l, w);
		double w2 = EstimatePseudoVoightFWHM(fwhm_r, w);

		// scaled 100 times
		return FastAbs(ampl * 0.5 * (w1 + w2) * (w * FastSqrt(M_PI) + (1 - w) * M_PI_2));
	}
	double FWHM(double *a, int peakN)
	{
		double w = a[nW];
		double x = a[NL_PROFILE_PEAKS_START + (peakN << 1)];
		double _x = m_dScale * (x - m_dMinX) - 1.0;
		double fwhm_l = FastAbs(a[nUL] + (FastAbs(a[nVL]) + FastAbs(a[nWL])*_x)*_x);
		double fwhm_r = FastAbs(a[nUR] + (FastAbs(a[nVR]) + FastAbs(a[nWR])*_x)*_x);
		double w1 = EstimatePseudoVoightFWHM(fwhm_l, w);
		double w2 = EstimatePseudoVoightFWHM(fwhm_r, w);
		return FastAbs(0.5 * (w1 + w2));
	}
	// background + FWHM + N*(Amplitude + position)
	virtual int GetNumParams(int nNumProfiles)
	{
		return NL_PROFILE_PEAKS_START + 2*nNumProfiles;
	}
	virtual LPCTSTR GetName() { return _T("1D profile with bkg"); }
	virtual UINT GetType() { return TYPE_1D | TYPE_MULTIPROFILE; }
	
	double m_dMinX;
	double m_dScale;
	bool bUseAsymm;
	std::vector<double> m_vInitialParams;
};

unsigned long __stdcall ThreadFitBkgPoints(LPVOID pParams);

// CBackgroundFitterDlg dialog

class CBackgroundFitterDlg : public CDialog
{
	DECLARE_DYNAMIC(CBackgroundFitterDlg)

public:
	CBackgroundFitterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBackgroundFitterDlg();

	BOOL Create(CWnd* pParentWnd);

// Dialog Data
	//{{AFX_DATA(CBackgroundFitterDlg)
	enum { IDD = IDD_BKG_FITTING_DLG };
	BOOL	m_bFitBkg;
	BOOL	m_bFitPeakPos;
	BOOL	m_bFitPeakShape;
	BOOL	m_bFitPeakAmpl;
	BOOL	m_bFWHMparams;
	BOOL	m_bPeakAsymmetry;
	CString	m_sChiSq;
	int		m_nBkgPolyN;
	CString	m_sRadiusD;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CBackgroundFitterDlg)
	public:
	virtual BOOL OnInitDialog();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
//	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

public:
	// The number of sections for background fitting
	UINT m_nFitterType;
	int m_nBorder_x;
	int m_nBorder_y;
	int m_nChartPos_x;
	int m_nChartPos_y;
	UINT m_nMaxSize;
	BOOL m_bMouseDown;
	CVector2f m_vfMouseDown;
	CVector2f m_vfMouseUp;

	double m_dScaleRad;					// if we have some scale factor to make Rad --> Angtr

	CPoint m_ptFirst, m_ptCurrent;		// mouse press & current points

	CWnd *m_pParent;

//	CComboBox m_ctrlFitterType;

	CGraph2DW m_wndChart;

	CProfile m_Profile;
	NlProfile1D m_fitter;
	CNonlinearFit m_nlfit;

	double m_dMaxVal, m_dMinVal;
	double m_dMinX, m_dScaleX;
	CProfileFitter::Interval m_vInterval;
	std::vector<double> m_vdParams;
	std::vector<double> m_vdDerivs;
	std::vector<bool> m_vbParams;
	int m_nMA;

	std::vector<double> m_vXcoords;
	std::vector<double> m_vYcoords;

	bool m_bInitialized;
	bool m_bAbortThread;
	HANDLE m_hThread;
	HANDLE m_hEventReady;

	int		m_nIntStart;		// the start of the interval
	int		m_nIntEnd;			// the end of the interval

	bool	m_bAutoFit;
	bool	m_bHaveFit;			// true if we did any successful fitting

	typedef std::vector<CGraph2Dpeak> DataPeaks;
	typedef std::vector<CGraph2Dpeak>::iterator DataPeaksIt;

	DataPeaks m_vPeaks;

	std::vector<EDRD_REFL> m_vOutputRefls;

	//{{AFX_MSG(CBackgroundFitterDlg)
	afx_msg void OnFitBkgPoints();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnFindPeaks();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnAutoFitPoints();
	afx_msg void OnResetParams();
	afx_msg LRESULT OnUpdateFitting(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTerminated(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
//	virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnAbortFitting();
	afx_msg void OnSelchangeFitBkgPolyN();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetProfile(double *pPoints, int nPoints);

	void AddPeak(double x);
	void RemovePeak(int iPos);
	void Params2Peaks();
	void Peaks2Params();

	void UpdateIntervalEnds();
	void UpdateControls();

//protected:
	void ModifyControlStyle(UINT uiCtrlID, DWORD dwRemove = 0, DWORD dwAdd = 0);
	void ProfileToChart();
	BOOL CheckChartRect(CPoint point);
	void DrawSelectionRect(CPoint ptFirst, CPoint ptLast);
	void DrawZoomRect(CPoint ptFirst, CPoint ptLast);
	void AddInterval(CVector2f &vfMouseDown, CVector2f &vfMouseUp, bool bFill);
	void FittedDataToChart();

	void InitializeParameters();
	void PrepareProfile();
};
