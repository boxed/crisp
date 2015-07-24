#ifndef __MARQUARDT_1D_H__
#define __MARQUARDT_1D_H__

#include "MathKrnl.h"

typedef std::vector<bool>	Vectorb;
//
// class NonlinearFitFunction for 1D fitting function
//
class NonlinearFit1Function : public NonlinearFitFunction
{
	G_DECLARE_DYNAMIC(NonlinearFit1Function);
public:
	virtual int GetNumParams(int nNumProfiles) = 0;
	virtual LPCTSTR GetName() = 0;
	virtual UINT GetType() = 0;

	// is not implemented for the abstract function
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) = 0;
	virtual double Calculate(double x, double *a, double *dyda, int na) = 0;
	virtual void StepParameters(double *pdAtry, double *pdParams,
								bool *pbParams,
								int nMA, double *pdDelta);
	virtual bool AlphaBetaChisq(double *pdParams, bool *pbBoolParams,
								int nMA, int nMfit,
								double *pdData, int nDataX,
							    double *pdDataSig, double *pdCoordsX,
								CMatrix &mAlpha, CVector &vBeta,
								double &dChiSq, bool bUseSigma);

protected:
	NonlinearFit1Function *m_pNext;
	int _x;		// current step on the curve during AlphaBetaChisq
};

class NlGauss1D : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na);
	// background + N*(Amplitude + position + FWHM)
	virtual int GetNumParams(int nNumProfiles) { return 1 + 3*nNumProfiles; }
	virtual LPCTSTR GetName() { return "Gauss"; }
	virtual UINT GetType() { return TYPE_1D | TYPE_MULTIPROFILE; }
};

class NlLorentz1D : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na);
	// background + N*(Amplitude + position + FWHM)
	virtual int GetNumParams(int nNumProfiles) { return 1 + 3*nNumProfiles; }
	virtual LPCTSTR GetName() { return "Lorentz"; }
	virtual UINT GetType() { return TYPE_1D | TYPE_MULTIPROFILE; }
};

class NlPseudoVoigt1D : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na);
	// background + N*(Ratio + Amplitude + position + FWHM_Gauss + FWHM_Lorentz)
	virtual int GetNumParams(int nNumProfiles) { return 1 + 4*nNumProfiles; }
	virtual LPCTSTR GetName() { return "Pseudo-Voigt"; }
	virtual UINT GetType() { return TYPE_1D | TYPE_MULTIPROFILE; }
};

class NlPseudoVoigt1DAs : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na);
	// background + N*(Ratio + Amplitude + position + FWHM_Gauss + FWHM_Lorentz)
	virtual int GetNumParams(int nNumProfiles) { return 3 + 5*nNumProfiles; }
	virtual LPCTSTR GetName() { return "Pseudo-Voigt Asymm"; }
	virtual UINT GetType() { return TYPE_1D | TYPE_MULTIPROFILE | TYPE_ASYMMETRIC; }
};

class NlPseudoVoigt1DAs2 : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na);
	// background + N*(Ratio + Amplitude + position + FWHM_Gauss + FWHM_Lorentz)
	virtual int GetNumParams(int nNumProfiles) { return 3 + 6*nNumProfiles; }
	virtual LPCTSTR GetName() { return "Pseudo-Voigt Asymm"; }
	virtual UINT GetType() { return TYPE_1D | TYPE_MULTIPROFILE | TYPE_ASYMMETRIC; }
};

class NlPseudoVoigt1DExpAs : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na);
	// background + N*(Ratio + Amplitude + position + FWHM_Gauss + FWHM_Lorentz)
	virtual int GetNumParams(int nNumProfiles) { return 4 + 4*nNumProfiles; }
	virtual LPCTSTR GetName() { return "Pseudo-Voigt Exp Asymm"; }
	virtual UINT GetType() { return TYPE_1D | TYPE_MULTIPROFILE | TYPE_ASYMMETRIC; }
};

// fitters themself
extern NlGauss1D			NL_Gauss1D;
extern NlLorentz1D			NL_Lorentz1D;
extern NlPseudoVoigt1D		NL_PseudoVoigt1D;
extern NlPseudoVoigt1DAs	NL_PseudoVoigt1DAs;
extern NlPseudoVoigt1DAs2	NL_PseudoVoigt1DAs2;
extern NlPseudoVoigt1DExpAs	NL_PseudoVoigt1DExpAs;

class CNonlinearFit
{
public:
	CNonlinearFit();
	virtual ~CNonlinearFit();

	bool Fit(double *pdData, double *pdCoordsX, double *pdDataSig, int nData,
			 double *pdParams, bool *piParams, int nMA,
			 NonlinearFit1Function *pfnFuncs1D, bool bUseSigma = true);

	bool NewRaphs(double *pdData, double *pdCoordsX, double *pdDataSig, int nData,
			 double *pdParams, bool *piParams, int nMA,
			 NonlinearFit1Function *pfnFuncs1D, int &nMaxIter);

	double GetChiSq() { return m_dChiSq; }
	double GetLamda() { return m_dALamda; }

private:
	double		m_dALamda;
	double		m_dChiSq;
	double		m_dOldChiSq;
	bool		m_bNumDeriv;
	bool		m_bUseSigma;

	NonlinearFit1Function *m_pfnFunc;
	double		*m_pdData;
	double		*m_pdXcoords;
	double		*m_pdDataSig;
	int			m_nDataX;
	CVector		m_vParams;
	Vectorb		m_vBoolParams;
	int			m_nFit;				// number of parameters to be varied
	int			m_nMA;				// total number of parameters

	bool		mrqmin(CMatrix &mCovar, CMatrix &mAlpha, double *pdParams);
	void		covsrt(CMatrix &mCovar, int nFit);
};

#endif // __MARQUARDT_1D_H__
