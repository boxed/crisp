#ifndef __MARQUARDT_2D_H__
#define __MARQUARDT_2D_H__

#include "MathKrnl.h"

typedef std::vector<bool>	Vectorb;
//
// class NonlinearFitFunction for 2D fitting function
//
class NonlinearFit2Function : public NonlinearFitFunction
{
	G_DECLARE_DYNAMIC(NonlinearFit2Function);
public:
	virtual int GetNumParams(int nNumProfiles) = 0;
	virtual LPCTSTR GetName() = 0;
	virtual UINT GetType() = 0;
	
	// is not implemented for the abstract function
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) = 0;
	virtual double Calculate(double x, double y, double *a, double *dyda, int na) = 0;
	virtual void StepParameters(double *pdAtry, double *pdParams,
										   bool *pbParams, int nMA, double *pdDa);
	virtual bool AlphaBetaChisq(double *pdParams, bool *pbBoolParams,
								int nMA, int nMfit,
								double *pdData, int nDataX, int nDataY,
								double *pdCoordsX, double *pdCoordsY,
								CMatrix &mAlpha, CVector &vBeta,
								double &dChiSq);

protected:
	int _x;		// current step on the surface (_x, _y)
	int _y;
};

class NlGauss2D : public NonlinearFit2Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double y, double *a, double *dyda, int na);
	virtual int GetNumParams(int nNumProfiles) { return 6; }
	virtual LPCTSTR GetName() { return _T("Gauss2D"); }
	virtual UINT GetType() { return TYPE_2D; }
};

class NlLorentz2D : public NonlinearFit2Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double y, double *a, double *dyda, int na);
	virtual int GetNumParams(int nNumProfiles) { return 6; }
	virtual LPCTSTR GetName() { return _T("Lorentz2D"); }
	virtual UINT GetType() { return TYPE_2D; }
};

class NlGauss2DAs : public NonlinearFit2Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double y, double *a, double *dyda, int na);
	virtual int GetNumParams(int nNumProfiles) { return 8; }
	virtual LPCTSTR GetName() { return _T("Gauss2DAssym"); }
	virtual UINT GetType() { return TYPE_2D | TYPE_ASYMMETRIC; }
};

class NlLorentz2DAs : public NonlinearFit2Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double y, double *a, double *dyda, int na);
	virtual int GetNumParams(int nNumProfiles) { return 8; }
	virtual LPCTSTR GetName() { return _T("Lorentz2DAssym"); }
	virtual UINT GetType() { return TYPE_2D | TYPE_ASYMMETRIC; }
};

class NlPseudoVoigt2DAs : public NonlinearFit2Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double y, double *a, double *dyda, int na);
	virtual int GetNumParams(int nNumProfiles) { return 13; }
	virtual LPCTSTR GetName() { return _T("PseudoVoigt2DAssym"); }
	virtual UINT GetType() { return TYPE_2D | TYPE_ASYMMETRIC; }
};

// Special class for peak shape refinement
class NlPseudoVoigt2D_PS : public NonlinearFit2Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA);
	virtual double Calculate(double x, double y, double *a, double *dyda, int na);
	virtual int GetNumParams(int nNumProfiles) { return 15; }
	virtual LPCTSTR GetName() { return _T("PseudoVoigt2DAssym_PeakShape"); }
	virtual UINT GetType() { return TYPE_2D | TYPE_ASYMMETRIC; }

	virtual void StepParameters(double *pdAtry, double *pdParams,
										   bool *pbParams, int nMA, double *pdDa);

	// for centre missalignment
	std::vector<double> m_vAngle;		// angle values for each point _x
	std::vector<double> m_vSinTh;		// = A*sin(ang*_x + pha)
	std::vector<double> m_vCosTh;		// = A*cos(ang*_x + pha)
};

extern NlGauss2D			NL_Gauss2D;
extern NlLorentz2D			NL_Lorentz2D;

extern NlGauss2DAs			NL_Gauss2DAs;
extern NlLorentz2DAs		NL_Lorentz2DAs;
extern NlPseudoVoigt2DAs	NL_PseudoVoigt2DAs;

extern NlPseudoVoigt2D_PS	NL_PseudoVoigt2D_PS;

class CNonlinearFit2
{
public:
	CNonlinearFit2();
	virtual ~CNonlinearFit2();

	bool Fit(double *pdData, double *pdDataX, double *pdDataY, int nDataX, int nDataY,
						double *pdParams, const Vectorb &params, int nMA,
						NonlinearFit2Function *pfnFuncs2D);

	double GetChiSq() { return m_dChiSq; }
	double GetLamda() { return m_dALamda; }

private:
	double		m_dALamda;
	double		m_dChiSq;
	double		m_dOldChiSq;

	NonlinearFit2Function *m_pfnFuncs2D;
	double		*m_pdData;
	double		*m_pdXcoords;
	double		*m_pdYcoords;
	double		m_dStepX;
	double		m_dStepY;
	double		m_dStartX;
	double		m_dStartY;
	int			m_nDataX;
	int			m_nDataY;
	CVector		m_vParams;
	Vectorb		m_vBoolParams;
	int			m_nFit;				// number of parameters to be varied
	int			m_nMA;				// total number of parameters

	bool		mrqmin(CMatrix &mCovar, CMatrix &mAlpha, double *pdParams);
	bool		mrqcof(CMatrix &mAlpha, double *pdParams, CVector &vBeta);
	void		covsrt(CMatrix &mCovar, int nFit);
};

#endif // __MARQUARDT_2D_H__
