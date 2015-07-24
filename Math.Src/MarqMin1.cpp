//#include "StdAfx.h"

#include "StdAfx.h"

#include "MathKrnl.h"
#include "MarqMin.h"
#include "Gauss.h"
#include <algorithm>
#include <functional>
#include "../CRISP2.Src/trace.h"

using namespace std;

NlGauss1D				NL_Gauss1D;
NlLorentz1D				NL_Lorentz1D;
NlPseudoVoigt1D			NL_PseudoVoigt1D;
NlPseudoVoigt1DAs		NL_PseudoVoigt1DAs;
NlPseudoVoigt1DAs2		NL_PseudoVoigt1DAs2;
NlPseudoVoigt1DExpAs	NL_PseudoVoigt1DExpAs;

G_IMPLEMENT_DYNAMIC(NonlinearFit1Function, NonlinearFitFunction);

static char msg[1024];

// first is always background
// pdDataY(pdDataX; a) is the sum of na/3 Gaussians.
// The amplitude, center, and width of the
// Gaussians are stored in consecutive locations of a: a[i] = Bk, a[i+1] = Ek, a[i+2] = Gk,
// k = 1; :::; na/3. The dimensions of the arrays are a[1..na], dyda[1..na].
double NlGauss1D::Calculate(double x, double *a, double *dyda, int na)
{
	int i;
	double fac, ex, arg, inv, val;

	val = a[0];
	dyda[0] = 1.0;
	for (i = 1; i < na; i += 3)
	{
		inv = 1.0 / a[i+2];
		arg=(x-a[i+1])*inv;
		ex = exp(-SQR(arg));
		fac = a[i]*ex*2.0*arg;
		val += a[i]*ex;
		dyda[i]=ex;
		dyda[i+1]=fac*inv;
		dyda[i+2]=fac*arg*inv;
	}
	return val;
}
// Lorentzians are stored in consecutive locations of a: a[i] = Bk, a[i+1] = Ek, a[i+2] = Gk,
double NlLorentz1D::Calculate(double x, double *a, double *dyda, int na)
{
	int i;
	double arg, inv, inv1, val;

	val = a[0];
	dyda[0] = 1.0;
	for (i = 1; i < na; i += 3)
	{
		inv1 = 1.0/a[i+2];
		arg = (x-a[i+1])*inv1;
		inv = 1.0 / (4*SQR(arg) + 1);
		val += a[i] * inv;
		dyda[i] = inv;
		dyda[i+1] = 8*a[i]*arg*SQR(inv)*inv1;
		dyda[i+2] = 8*a[i]*SQR(arg*inv)*inv1;
	}
	return val;
}
// ratio M is stored in a[i]
// Gaussians and Lorentzians are stored in consecutive locations of a:
//		a[i+1] = Ampl, a[i+2] = position, a[i+3] = FWHM,
double NlPseudoVoigt1D::Calculate(double x, double *a, double *dyda, int na)
{
	int i;
	double amp, arg, ex, inv, fac, inv_w, rat, val, tmp;

	val = a[0];
	dyda[0] = 1.0;
	for (i = 1; i < na; i += 4)
	{
		amp = a[i+1];
		inv_w = 1.0 / a[i+3];
		rat = 1.0 - a[i];
		arg = (x - a[i+2]) * inv_w;
		// for Gauss
		ex = exp(-SQR(arg));
		fac = amp * ex * 2.0 * arg;
		// for Lorentz
		inv = 1.0 / (4*SQR(arg) + 1);
		// derivatives
		dyda[i] = amp * (inv - ex);
		dyda[i+1] = rat * ex + a[i] * inv;
		tmp = 2.0*amp*arg*inv_w*(rat*ex + 4.0*a[i]*SQR(inv));
		dyda[i+2] = tmp;
		dyda[i+3] = tmp * arg;
		// common
		val += amp * (rat * ex + a[i] * inv);
	}
	return val;
}
/*
// ratio M is stored in a[i]
// Gaussians are stored in consecutive locations of a:
//		a[i+1] = Ampl, a[i+2] = position, a[i+3] = Gauss FWHM,
// Lorentzians are stored in consecutive locations of a: a[i+4] = Lorentz FWHM,
double NlPseudoVoigt1D::Calculate(double x, double *a, double *dyda, int na)
{
	int i;
	double argG, argL, ex, inv, fac, inv_g, inv_l, rat, val;

	val = a[0];
	dyda[0] = 1.0;
	for (i = 1; i < na; i += 5)
	{
		// for Gauss
		inv_g = 1.0 / a[i+3];
		inv_l = 1.0 / a[i+4];
		rat = 1.0 - a[i];
		argG = (x - a[i+2]) * inv_g;
		ex = exp(-SQR(argG));
		fac = a[i+1] * ex * 2.0 * argG;
		// for Lorentz
		argL = (x - a[i+2]) * inv_l;
		inv = 1.0 / (4*SQR(argL) + 1);
		// derivatives
		dyda[i] = a[i+1] * (inv - ex);
		dyda[i+1] = rat * ex + a[i] * inv;
		dyda[i+2] = rat * fac * inv_g + a[i] * 8 * a[i+1] * argL * SQR(inv) * inv_l;
		dyda[i+3] = rat * fac * argG * inv_g;
		dyda[i+4] = a[i] * 8 * a[i+1] * SQR(argL * inv) * inv_l;
		// common
		val += a[i+1] * (rat * ex + a[i] * inv);
	}
	return val;
}
*/
// ratio M is stored in a[i]
// Voigtians are stored in consecutive locations of a:
//		a[i+1] = Ampl, a[i+2] = position, a[i+3] = left FWHM, a[i+4] = right FWHM
double NlPseudoVoigt1DAs::Calculate(double x, double *a, double *dyda, int na)
{
	int i, ind;
	double amp, arg, arg2, ex, inv, inv_w, rat, val, x0;

	val = a[0]+a[1]*x+a[2]*SQR(x);		// background
//	dyda[0] = 1.0;		// bkg derivative
//	dyda[1] = x;		// bkg derivative
//	dyda[2] = SQR(x);	// bkg derivative
	for (i = 3; i < na; i += 5)
	{
//		dyda[i+3] = dyda[i+4] = 0.0;
		amp = a[i+1];
		x0 = a[i+2];
		ind = ( x < x0 ) ? 3 : 4;
		// for Gauss
		inv_w = 1.0 / a[i+ind];
		rat = 1.0 - a[i];
		arg = (x - x0) * inv_w;
		arg2 = SQR(arg);
		ex = exp(-arg2);
		// for Lorentz
		inv = 1.0 / (4*arg2 + 1.0);
		// derivatives
/*
		dyda[i] = amp * (ex - inv);
		dyda[i+1] = a[i]*ex + rat*inv;
		tmp = 2.0 * amp * arg * inv_w * (a[i]*ex + rat*SQR(inv));
		dyda[i+2] = tmp;
		dyda[i+ind] = tmp * arg;
		// common
		val += amp * dyda[i+1];
*/
		val += amp * (a[i]*ex + rat*inv);
	}
	return val;
}

// ratio M-left is stored in a[i], a[i+1] = M-right
// Voigtians are stored in consecutive locations of a:
//		a[i+2] = Ampl, a[i+3] = position, a[i+4] = left FWHM, a[i+5] = right FWHM
double NlPseudoVoigt1DAs2::Calculate(double x, double *a, double *dyda, int na)
{
	int i, ind;
	double amp, arg, arg2, ex, inv, inv_w, rat, val, x0;

	val = a[0]+a[1]*x+a[2]*SQR(x);		// background
//	dyda[0] = 1.0;		// bkg derivative
//	dyda[1] = x;		// bkg derivative
//	dyda[2] = SQR(x);	// bkg derivative
	for (i = 3; i < na; i += 6)
	{
//		dyda[i+3] = dyda[i+4] = 0.0;
		amp = a[i+2];
		x0 = a[i+3];
		ind = ( x < x0 ) ? 0 : 1;//4 : 5;
		// for Gauss
		inv_w = 1.0 / a[i+ind+4];
		rat = 1.0 - a[i+ind];
		arg = (x - x0) * inv_w;
		arg2 = SQR(arg);
		ex = exp(-arg2);
		// for Lorentz
		inv = 1.0 / (arg2 + 1.0);
		// derivatives
/*
		dyda[i] = amp * (ex - inv);
		dyda[i+1] = a[i]*ex + rat*inv;
		tmp = 2.0 * amp * arg * inv_w * (a[i]*ex + rat*SQR(inv));
		dyda[i+2] = tmp;
		dyda[i+ind] = tmp * arg;
		// common
		val += amp * dyda[i+1];
*/
		val += amp * (a[i+ind]*ex + rat*inv);
	}
	return val;
}

// BKG is stored in a[0]
// U, V and W are stored in a[1], a[2] and a[3]
// Gaussians and Lorentzians are stored in consecutive locations of a:
//		a[i] = Ampl, a[i+1] = position
//		ratio L is stored in a[i+2]
//		Lorentz coeff M is stored in a[i+3]
double NlPseudoVoigt1DExpAs::Calculate(double x, double *a, double *dyda, int na)
{
	int i;
	double amp, U, V, W, x0, L, L_1, M, arg, arg2, ex, inv, fac, inv_w, val;

	val = a[0];
	U = a[1];
	V = a[2];
	W = a[3];
	dyda[0] = 1.0;					// BKG
	dyda[1] =
	dyda[2] =
	dyda[3] = 0.0;					// U, V, W in the begining
//	Wexp = FastExp(-W*SQR(x));
//	inv_w = 1.0 / (U + V*Wexp);
	inv_w = 1.0 / (U + V*x + W*SQR(x));
	for (i = 4; i < na; i += 4)
	{
		amp = a[i];
		x0 = a[i+1];
		L = a[i+2];
		M = a[i+3];
		L_1 = 1.0 - L;
		arg = (x - x0) * inv_w;
		arg2 = SQR(arg);
		// for Gauss
		ex = exp(-arg2);
		// for Lorentz
		inv = 1.0 / (M*arg2 + 1.0);
		// derivatives
		fac = 2.0*amp*arg*inv_w*(L_1*ex + M*L*SQR(inv));
		dyda[i] = L_1*ex + L*inv;						// AMPLITUDE
		dyda[i+1] = fac;		// X0
		dyda[i+2] = amp*(inv - ex);						// L
		dyda[i+3] = -amp*L*arg2*SQR(inv);				// M

		fac *= arg;
		dyda[1] += fac;						// U
//		dyda[2] += fac*Wexp;				// V
//		dyda[3] -= fac*V*SQR(x)*Wexp;		// W
		dyda[2] += fac*x;					// V
		dyda[3] += fac*SQR(x);				// W
		// common
		val += amp * (L_1 * ex + L * inv);
	}
	return val;
}
////////////////////////////////////////////////////////////////////////////////////

void NonlinearFit1Function::StepParameters(double *pdAtry, double *pdParams,
										   bool *pbParams,
										   int nMA, double *pdDelta)
{
	int l, j;
	// make new params as A = A + dA
	for(j = 0, l = 0; l < nMA; l++)
	{
		if( pbParams[l] )
		{
			pdAtry[l] = pdParams[l] + pdDelta[j++];
		}
	}
}

bool NonlinearFit1Function::AlphaBetaChisq(double *pdParams, bool *pbBoolParams,
										   int nMA, int nMfit,
										   double *pdData, int nDataX,
										   double *pdDataSig, double *pdCoordsX,
										   CMatrix &mAlpha, CVector &vBeta,
										   double &dChiSq, bool bUseSigma)
{
	int i, j, k, l, m;
	double ymod, wt, sig2i, dy;
	CVector vDyDa(nMA);

	Precalculate(pdParams, pbBoolParams, nMA);
	// initialize (symmetric) data in alpha, beta
	for (j = 0; j < nMfit; j++)
	{
		for (k = 0; k <= j; k++)
			mAlpha[j][k] = 0.0;
		vBeta[j] = 0.0;
	}
	// initialize Chi^2
	dChiSq = 0.0;
	// loop over the data
	for(i = 0; i < nDataX; i++)
	{
		_x = i;
		ymod = Calculate(pdCoordsX[i], pdParams, vDyDa.data(), nMA);
		sig2i = 1.0;
		if( true == bUseSigma )
		{
			if( 0.0 != pdData[i] )
				sig2i = 1.0 / pdData[i];
			if( pdDataSig )
				sig2i /= SQR(pdDataSig[i]);
		}
		// NUMERIC DERIVATIVES
		{
			int iDer;
			double temp, h, eps = 1e-6, val;
			for(iDer = 0; iDer < nMA; iDer++)
			{
				if( false == pbBoolParams[iDer] )
				{
					vDyDa[iDer] = 0.0;
					continue;
				}
				// save the value of the variable
				temp = pdParams[iDer];
				// step for the parameter
				h = eps*fabs(temp);
				if( 0.0 == h )
					h = eps;
				// step the variable
				pdParams[iDer] += h;
				// calculate the new function value
				val = Calculate(pdCoordsX[i], pdParams, vDyDa.data(), nMA);
				// calculate the derivative
				vDyDa[iDer] = (val - ymod) / h;
				// restore the value of the variable
				pdParams[iDer] = temp;
			}
		}
		// END NUMERIC DERIVATIVES
		dy = pdData[i] - ymod;
		// find chi^2
		dChiSq += dy*dy*sig2i;
		for(j = 0, l = 0; l < nMA; l++)
		{
			if( pbBoolParams[l] )
			{
				wt = vDyDa[l]*sig2i;
				for(k = 0, m = 0; m <= l; m++)
				{
					if( pbBoolParams[m] )
						mAlpha[j][k++] += wt * vDyDa[m];
				}
				vBeta[j++] += dy * wt;
			}
		}
	}
	// fill the symmetric side
	for(j = 1; j < nMfit; j++)
	{
		for(k = 0; k < j; k++)
			mAlpha[k][j] = mAlpha[j][k];
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////

CNonlinearFit::CNonlinearFit()
{
	m_dALamda = -1;

	m_pfnFunc = NULL;
	m_pdData = NULL;
	m_pdXcoords = NULL;
	m_pdDataSig = NULL;
	m_nDataX = 0;
	m_nMA = 0;
	m_nFit = 0;

	m_dChiSq = m_dOldChiSq = 10e+12;

	m_bNumDeriv = false;
	m_bUseSigma = false;
}

CNonlinearFit::~CNonlinearFit()
{
}

void CNonlinearFit::covsrt(CMatrix &mCovar, int nFit)
{
	int i, j, k;

	for(i = nFit; i < m_nMA; i++)
	{
		for(j = 0; j < i; j++)
		{
			mCovar[i][j] = mCovar[j][i] = 0.0;
		}
	}
	k = nFit - 1;
	for(j = m_nMA - 1; j >= 0; j--)
	{
		if(m_vBoolParams[j])
		{
			for (i = 0; i < m_nMA; i++) swap(mCovar[i][k], mCovar[i][j]);
			for (i = 0; i < m_nMA; i++) swap(mCovar[k][i], mCovar[j][i]);
			k--;
		}
	}
}
/*
bool CNonlinearFit::mrqcof(CMatrix &mAlpha, double *pdParams, CVector &vBeta)
{
	int i,j,k,l,m;
	double ymod,wt,sig2i,dy;
	CVector dyda(m_nMA);

	// initialize (symmetric) data in alpha, beta
	for (j = 0; j < m_nFit; j++)
	{
		for (k = 0; k <= j; k++)
			mAlpha[j][k] = 0.0;
		vBeta[j] = 0.0;
	}
	// initialize Chi^2
	m_dChiSq=0.0;
	// loop over the data
	for (i = 0; i < m_nDataX; i++)
	{
		(*m_pfnFuncs)(m_pdDataX[i], pdParams, &ymod, &dyda[0], m_nMA);
		sig2i = 1.0;
		if(m_pdDataSig)
			sig2i /= SQR(m_pdDataSig[i]);
		dy = m_pdDataY[i] - ymod;
		// find chi^2
		m_dChiSq += dy*dy*sig2i;
		for (j = 0, l = 0; l < m_nMA; l++)
		{
			if (m_piParams[l])
			{
				wt = dyda[l]*sig2i;
				for (k = 0, m = 0; m <= l; m++)
				{
					if (m_piParams[m])
						mAlpha[j][k++] += wt * dyda[m];
				}
				vBeta[j++] += dy * wt;
			}
		}
	}
	// fill the symmetric side
	for (j = 1; j < m_nFit; j++)
	{
		for (k = 0; k < j; k++)
			mAlpha[k][j] = mAlpha[j][k];
	}

	return true;
}
*/
bool CNonlinearFit::mrqmin(CMatrix &mCovar, CMatrix &mAlpha, double *pdParams)
{
	int j, k, l;
	static CVector vAtry(m_nMA), vBeta(m_nMA), vDa(m_nMA), vOneDA;

	// initialize if Lamda < 0.0
	if( m_dALamda < 0.0 )
	{
		m_nFit = count_if(m_vBoolParams.begin(), m_vBoolParams.end(),
			bind2nd(not2(equal_to<bool>()), false) );
		vOneDA.resize(m_nFit);
		m_dALamda = 0.001;
		if( false == m_pfnFunc->AlphaBetaChisq(pdParams, (bool*)&*m_vBoolParams.begin(),
			m_nMA, m_nFit, m_pdData, m_nDataX, m_pdDataSig, m_pdXcoords,
			mAlpha, vBeta, m_dChiSq, m_bUseSigma) )
			return false;
		m_dOldChiSq = m_dChiSq;
		for(j = 0; j < m_nMA; j++)
		{
			vAtry[j] = pdParams[j];
		}
	}
	// create new matrix with AlphaTry(j,j) = Alpha(j,j)*(1 + Lamda)
	//					and   AlphaTry(j,k) = Alpha(j,k)	(j != k)
	for(j = 0; j < m_nFit; j++)
	{
		for(k = 0; k < m_nFit; k++)
		{
			mCovar[j][k] = mAlpha[j][k];
		}
		mCovar[j][j] = mAlpha[j][j] * (1.0 + m_dALamda);
		vOneDA[j] = vBeta[j];
	}
	// solve the system of linear equations for
	// Alpha * dA = Beta to find dA
	if( false == GaussSolve::Solve(mCovar, vOneDA, m_nFit) )
	{
		::OutputDebugString("1D FITTING: Gauss solver failed!\n");
		return false;
	}
	// copy
	for( j = 0; j < m_nFit; j++ )
	{
		vDa[j] = vOneDA[j];
	}
	// finish calculations
	if( 0.0 == m_dALamda )
	{
		covsrt(mCovar, m_nFit);
		return true;
	}
	// step parameters
	m_pfnFunc->StepParameters(vAtry.data(), pdParams,
		(bool*)&*m_vBoolParams.begin(), m_nMA, vOneDA.data());
	// calculate Chi square
	if( false == m_pfnFunc->AlphaBetaChisq(vAtry.data(), (bool*)&*m_vBoolParams.begin(),
		m_nMA, m_nFit, m_pdData, m_nDataX, m_pdDataSig, m_pdXcoords,
		mCovar, vDa, m_dChiSq, m_bUseSigma) )
	{
		return false;
	}
	// increase Chi square by 10
	if (m_dChiSq >= m_dOldChiSq)
	{
		m_dALamda *= 10.0;
		if(m_dALamda > 1e15)
			m_dALamda /= 3e14;
		m_dChiSq = m_dOldChiSq;
	}
	// decrease Chi square by 10
	// update trial solution a = a + da
	else
	{
		m_dALamda *= 0.1;
		if(m_dALamda < 1e-15)
			m_dALamda *= 3e14;
		m_dOldChiSq = m_dChiSq;
		for(j = 0; j < m_nFit; j++)
		{
			for(k = 0; k < m_nFit; k++)
			{
				mAlpha[j][k] = mCovar[j][k];
			}
			vBeta[j] = vDa[j];
		}
		for(l = 0; l < m_nMA; l++)
		{
			pdParams[l] = vAtry[l];
		}
	}

	return true;
}
// x - the point to calculate at
// y - parameters vector of m-length
// g - gradient vector of m-length
// s - gradient step
static double GradR(double x, double *y, double *g, size_t m, double s,
				  bool *pbParams, NonlinearFit1Function *pfnFuncs1D)
{
	register size_t i, j;
	double temp, h, val, ymod;
	std::vector<double> dy(m);
	double *grd = &*dy.begin();

	// initial value
	ymod = pfnFuncs1D->Calculate(x, y, grd, m);
	for(i = 0, j = 0; i < m; i++)
	{
		if( false == pbParams[i] )
			continue;
		// remember the value of the variable
		temp = y[i];
		// step for the parameter
		h = s*fabs(temp);
		// is that 0?
		if( 0.0 == h )
			h = s;
		// step the variable
		y[i] += h;
		// calculate the new function value
		val = pfnFuncs1D->Calculate(x, y, grd, m);
		// calculate the derivative
		g[j] = (val - ymod) / h;
		// restore the value of the variable
		y[i] = temp;
		j++;
	}
	return ymod;
}

static double GradL(double x, double *y, double *g, size_t m, double s,
				  bool *pbParams, NonlinearFit1Function *pfnFuncs1D)
{
	register size_t i, j;
	double temp, h, val, ymod;
	std::vector<double> dy(m);
	double *grd = &*dy.begin();

	// initial value
	ymod = pfnFuncs1D->Calculate(x, y, grd, m);
	for(i = 0; i < m; i++)
	{
		if( false == pbParams[i] )
			continue;
		// remember the value of the variable
		temp = y[i];
		// step for the parameter
		h = s*fabs(temp);
		// is that 0?
		if( 0.0 == h )
			h = s;
		// step the variable
		y[i] -= h;
		// calculate the new function value
		val = pfnFuncs1D->Calculate(x, y, grd, m);
		// calculate the derivative
		g[j] = (val - ymod) / h;
		// restore the value of the variable
		y[i] = temp;
		j++;
	}
	return ymod;
}

static double GradC(double x, double *y, double *g, size_t m, double s,
				  bool *pbParams, NonlinearFit1Function *pfnFuncs1D)
{
	register size_t i, j;
	double temp, h, val1, val2;
	std::vector<double> dy(m);
	double *grd = &*dy.begin();

	for(i = 0; i < m; i++)
	{
		if( false == pbParams[i] )
			continue;
		// remember the value of the variable
		temp = y[i];
		// step for the parameter
		h = s*fabs(temp);
		// is that 0?
		if( 0.0 == h )
			h = s;
		// step the variable
		y[i] += h;
		// calculate the new function value
		val1 = pfnFuncs1D->Calculate(x, y, grd, m);
		y[i] -= 2.0*h;
		val2 = pfnFuncs1D->Calculate(x, y, grd, m);
		// calculate the derivative
		g[j] = (val1 - val2) / (2.0*h);
		// restore the value of the variable
		y[i] = temp;
		j++;
	}
	return (val1 + val2)*0.5;
}
// x - the point to calculate at
// y - parameters vector of m-length
// mHesse - the Hesse matrix of second derivatives
// s - gradient step
static void HesseR(double x, double *y, size_t m, CMatrix &mHesse, double s,
				   bool *pbParams, NonlinearFit1Function *pfnFuncs1D)
{
	double temp1, temp2, h1, h2, ymod, deriv;
	register size_t i, j;
	size_t k, l;
	std::vector<double> dy(m);
	double *g = &*dy.begin();

	// initial value
	ymod = pfnFuncs1D->Calculate(x, y, g, m);
	// upper triangle of the Hesse matrix
	for(i = 0, l = 0; i < m; i++)
	{
		if( false == pbParams[i] )
			continue;
		for(j = 0, k = 0; j <= i; j++)
		{
			if( false == pbParams[j] )
				continue;
			// remember the value of the variable
			temp1 = y[i];
			temp2 = y[j];
			// step for the parameters
			h1 = s*fabs(temp1);
			h2 = s*fabs(temp2);
			// is that 0?
			if( 0.0 == h1 )
				h1 = s;
			if( 0.0 == h2 )
				h2 = s;
			// step the variable
			y[i] += h1;
			deriv = ymod - pfnFuncs1D->Calculate(x, y, g, m);
			// step the variable
			y[j] += h2;
			deriv += pfnFuncs1D->Calculate(x, y, g, m);

			y[i] = temp1;
			deriv -= pfnFuncs1D->Calculate(x, y, g, m);

			y[j] = temp2;
			// derivative value
			mHesse[l][k] = deriv / (2.0*s);

			k++;
		}
		l++;
	}
	// lower triangle of the Hesse matrix
	for(i = 1; i < m; i++)
	{
		for(j = 0; j < i; j++)
			mHesse[j][i] = mHesse[i][j];
	}
}

bool CNonlinearFit::NewRaphs(double *pdData, double *pdCoordsX, double *pdDataSig,
							 int nData, double *pdParams, bool *pbParams, int nMA,
							 NonlinearFit1Function *pfnFuncs1D, int &nMaxIter)
{
	register int i, j;	// cycle counters
	int iIter;			// iterations count

	// save all data
	m_pfnFunc = pfnFuncs1D;
	m_pdData = pdData;
	m_pdXcoords = pdCoordsX;
	m_pdDataSig = pdDataSig;
	m_nDataX = nData;

	// steps during processing the array
	// copy parameters
	m_vParams.resize(nMA);
	m_vBoolParams.resize(nMA);
	copy(pdParams, pdParams+nMA, m_vParams.data());
	copy(pbParams, pbParams+nMA, m_vBoolParams.begin());

	m_nMA = nMA;

	// get the number of parameters to be varied
	m_nFit = count_if(m_vBoolParams.begin(), m_vBoolParams.end(),
		bind2nd(equal_to<bool>(), true));

	CMatrix 	mHesse(m_nMA, m_nMA), mInvHesse(m_nMA, m_nMA);
	CVector		vGrad(m_nMA), vStep(m_nMA);
	double		dNorm;

	// start iterations
	iIter = 0;
	while( iIter++ <= nMaxIter )
	{
/*		for(i = 0; i < m_nMA; i++)
		{
			vGrad[i] = 0.0;
			vStep[i] = 0.0;
		}
		m_dChiSq = 0.0;
		for(i = 0; i < m_nDataX; i++)
		{
			val = GradR(m_pdXcoords[i], m_vParams.data(), vStep.data(), m_nMA, 1e-6, pbParams, m_pfnFunc);
			coeff = (pdData[i] - val);
			if( 0.0 != pdData[i] )
				coeff /= pdData[i];
			m_dChiSq += SQR(coeff);
//			coeff *= 2.0;
			for(j = 0; j < m_nFit; j++)
			{
				vGrad[j] += coeff*vStep[j];
//				vGrad[j] -= coeff*vStep[j];
			}
		}
		for(i = 0; i < m_nFit; i++)
		{
			for(j = 0; j < m_nFit; j++)
			{
				mHesse[i][j] = 0.0;
				mInvHesse[i][j] = 0.0;
			}
		}
		for(i = 0; i < m_nDataX; i++)
		{
			HesseR(m_pdXcoords[i], m_vParams.data(), m_nMA, mInvHesse, 1e-6, pbParams, m_pfnFunc);
			mHesse += mInvHesse;
		}
*/
		// gradient and Hesse matrix
		if( false == m_pfnFunc->AlphaBetaChisq(m_vParams.data(),
			(bool*)&*m_vBoolParams.begin(), m_nMA, m_nFit,
			m_pdData, m_nDataX, m_pdDataSig, m_pdXcoords,
			mHesse, vGrad, m_dChiSq, m_bUseSigma) )
			return false;
		// Invert Hesse matrix

		try
		{
			mInvHesse.Inverse(mHesse, m_nFit);
		}
		catch (std::exception& e)
		{
			Trace(_FMT(_T("Hesse matrix error: %s"), e.what()));
//			cerr << "Hesse matrix: " << str << "." << endl;
		}

		for(i = 0; i < m_nFit; i++)
		{
			dNorm = 0.0;
			for(j = 0; j < m_nFit; j++)
			{
				dNorm += mInvHesse[i][j] * vGrad[j];
			}
			vStep[i] = dNorm;
		}

		bool *pbFlags = (bool*)&*m_vBoolParams.begin();
		double *pdParams = m_vParams.data();
		// calculate the norm and step parameters
		dNorm = 0.0;
		for(i = 0, j = 0; i < m_nMA; i++)
		{
			if( pbFlags[i] )
			{
				dNorm += SQR(vStep[j]);
				pdParams[i] += vStep[j];
				j++;
			}
		}
		sprintf(msg, "iteration: %d, Chi^2=%g, norm=%g.\n", iIter, m_dChiSq, dNorm);
		::OutputDebugString(msg);
		if( dNorm <= 1e-6 )
		{
			break;
		}
	}
	nMaxIter = iIter;
/*	for(i = 0; i < nMA; i++)
		::OutputDebugString("      a[") << i << _T("]\t");
	::OutputDebugString("\n");
*/	for(i = 0; i < nMA; i++)
	{
//		cerr << m_vParams[i] << _T("\t");
		pdParams[i] = m_vParams[i];
	}
/*	::OutputDebugString("\n");
*/
	return true;
}
//
// Nonlinear method of reducing Chi-square of a fit btw a set of data
//		pdDataX and pdDataY - data points in the range [0, nData)
//		pdDataSig - individual std deviations in the range [0, nData)
//		nData - number of data points
//		pdParams - array with parameters (some of them may has initial values)
//		piParams - array of integers, each value indicates:
//				[j] value is non-zero - [j] parameter has to be fitted for the value from pdParams[j]
//				[j] value is zero     - [j] parameter has fixed value from pdParams[j]
//		nMA - number of parameters
//		pfnFuncs - user defined function, which depends on some parameters
//
bool CNonlinearFit::Fit(double *pdData, double *pdCoordsX, double *pdDataSig, int nData,
						double *pdParams, bool *pbParams, int nMA,
						NonlinearFit1Function *pfnFuncs1D, bool bUseSigma /*= true*/)
{
	int i, k, iter, itst;
	double dOldChiSq;
	CMatrix mCovar(nMA, nMA), mAlpha(nMA, nMA);

	// save all data
	m_pfnFunc = pfnFuncs1D;
	m_pdData = pdData;
	m_pdXcoords = pdCoordsX;
	m_pdDataSig = pdDataSig;
	m_nDataX = nData;

	m_bUseSigma = bUseSigma;

	// copy parameters
	m_vParams.resize(nMA);
	m_vBoolParams.resize(nMA);
	copy(pdParams, pdParams+nMA, m_vParams.data());
	copy(pbParams, pbParams+nMA, m_vBoolParams.begin());

	m_nMA = nMA;
	// get the number of parameters to be varied
	m_nFit = count_if(m_vBoolParams.begin(), m_vBoolParams.end(),
		bind2nd(equal_to<bool>(), true));
	// iterations
	for(iter = 0; iter < 5; iter++)
	{
		m_dALamda = -1.0;
		mrqmin(mCovar, mAlpha, m_vParams.data());
		k = 1;
//		sprintf(msg, "iteration: %d, Chi^2=%g.\n", k, m_dChiSq);
//		::OutputDebugString(msg);
		itst = 0;
		while( 1 )
		{
			dOldChiSq = m_dChiSq;
			if( k++ >= 100 )
				goto ExitIterate;
			mrqmin(mCovar, mAlpha, m_vParams.data());
//			sprintf(msg, "iteration: %d, Chi^2=%g.\n", k, m_dChiSq);
//			::OutputDebugString(msg);
			if( m_dChiSq > dOldChiSq )
				itst = 0;
			else if( fabs(dOldChiSq - m_dChiSq) < 0.0001 )
				itst++;
			if (itst < 14)
				continue;
			m_dALamda = 0.0;
			mrqmin(mCovar, mAlpha, m_vParams.data());
			break;
		}
	}
ExitIterate :

	sprintf(msg, "Chi^2=%g.\n", m_dChiSq);
	::OutputDebugString(msg);
/*
	for(i = 0; i < nMA; i++)
		::OutputDebugString("      a[") << i << _T("]\t");
	::OutputDebugString("\n");
*/
	for(i = 0; i < nMA; i++)
	{
//		cerr << m_vParams[i] << _T("\t");
		pdParams[i] = m_vParams[i];
	}
/*
	::OutputDebugString("\n");
	::OutputDebugString("\nUncertainties:\n");
	for(i = 0; i < nMA; i++)
		cerr << sqrt(mCovar[i][i]) << _T("\t");
	::OutputDebugString("\n");
*/
	return true;
}
