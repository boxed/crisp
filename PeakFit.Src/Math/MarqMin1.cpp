#include "StdAfx.h"

#include "MathKrnl.h"
#include "MarqMin.h"
#include "Gauss.h"

#include <math.h>

NlGauss1D				NL_Gauss1D;
NlLorentz1D				NL_Lorentz1D;
NlPseudoVoigt1D			NL_PseudoVoigt1D;
NlPseudoVoigt1DAs		NL_PseudoVoigt1DAs;
//NlPseudoVoigt1DExpAs	NL_PseudoVoigt1DExpAs;

G_IMPLEMENT_DYNAMIC(NonlinearFit1Function, NonlinearFitFunction);

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
// ratio M is stored in a[i]
// Gaussians are stored in consecutive locations of a:
//		a[i+1] = Ampl, a[i+2] = position, a[i+3] = Gauss left FWHM, a[i+4] = Gauss right FWHM
// Lorentzians are stored as: a[i+5] = Lorentz left FWHM, a[i+6] = Lorentz right FWHM
double NlPseudoVoigt1DAs::Calculate(double x, double *a, double *dyda, int na)
{
	int i, ind;
	double argG, argL, ex, inv, fac, inv_g, inv_l, rat, val, x0;

	val = a[0];		// background
	dyda[0] = 1.0;	// bkg derivative
	for (i = 1; i < na; i += 7)
	{
		dyda[i+3] = dyda[i+4] = dyda[i+5] = dyda[i+6] = 0.0;
		x0 = a[i+2];
		ind = ( x < x0 ) ? 3 : 4;
		// for Gauss
		inv_g = 1.0 / a[i+ind];
		inv_l = 1.0 / a[i+ind+2];
		rat = 1.0 - a[i];
		argG = (x - x0) * inv_g;
		ex = exp(-SQR(argG));
		fac = a[i+1] * ex * 2.0 * argG;
		// for Lorentz
		argL = (x - a[i+2]) * inv_l;
		inv = 1.0 / (4*SQR(argL) + 1);
		// derivatives
		dyda[i] = a[i+1] * (inv - ex);
		dyda[i+1] = rat * ex + a[i] * inv;
		dyda[i+2] = rat * fac * inv_g + a[i] * 8 * a[i+1] * argL * SQR(inv) * inv_l;
		dyda[i+ind] = rat * fac * argG * inv_g;
		dyda[i+ind+2] = a[i] * 8 * a[i+1] * SQR(argL * inv) * inv_l;
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
	double amp, tmp, arg, arg2, ex, inv, inv_w, rat, val, x0;

	val = a[0]+a[1]*x+a[2]*SQR(x);		// background
	dyda[0] = 1.0;		// bkg derivative
	dyda[1] = x;		// bkg derivative
	dyda[2] = SQR(x);	// bkg derivative
	for (i = 3; i < na; i += 5)
	{
		dyda[i+3] = dyda[i+4] = 0.0;
		x0 = a[i+2];
		amp = a[i+1];
		ind = ( x < x0 ) ? 3 : 4;
		// for Gauss
		inv_w = 1.0 / a[i+ind];
		rat = 1.0 - a[i];
		arg = (x - x0) * inv_w;
		arg2 = SQR(arg);
		ex = exp(-arg2);
		// for Lorentz
		inv = 1.0 / (arg2 + 1.0);
		// derivatives
		dyda[i] = amp * (ex - inv);
		dyda[i+1] = a[i]*ex + rat*inv;
		tmp = 2.0 * amp * arg * inv_w * (a[i]*ex + rat*SQR(inv));
		dyda[i+2] = tmp;
		dyda[i+ind] = tmp * arg;
		// common
		val += amp * dyda[i+1];
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
	double amp, U, V, W, /*Wexp, */x0, L, L_1, M, arg, arg2, ex, inv, fac, inv_w, val;

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
										   double &dChiSq)
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
		if( 0.0 != pdData[i] )
			sig2i = 1.0 / SQR(pdData[i]);
//		if( pdDataSig )
//			sig2i /= SQR(pdDataSig[i]);
		// NUMERIC DERIVATIVES
/*
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
				h = eps*FastAbs(temp);
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
*/
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

	m_dOldChiSq = 10e+12;
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

	if( m_nMA != vAtry.size() ) vAtry.resize(m_nMA);
	if( m_nMA != vBeta.size() ) vBeta.resize(m_nMA);
	if( m_nMA != vDa.size() ) vDa.resize(m_nMA);

	// initialize if Lamda < 0.0
	if( m_dALamda < 0.0 )
	{
		m_nFit = count_if(m_vBoolParams.begin(), m_vBoolParams.end(),
			bind2nd(not2(equal_to<bool>()), false) );
		vOneDA.resize(m_nFit);
		m_dALamda = 0.001;
		if( false == m_pfnFunc->AlphaBetaChisq(pdParams, (bool*)&*m_vBoolParams.begin(),
			m_nMA, m_nFit, m_pdData, m_nDataX, m_pdDataSig, m_pdXcoords,
			mAlpha, vBeta, m_dChiSq) )
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
//	if( false == GaussSolve::Solve(mCovar, vOneDA, m_nFit) )
	if( false == GaussSolve::SolveX(mCovar, vOneDA, m_nFit) )
	{
		TRACE(_T("1D FITTING: Gauss solver failed!\n"));
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
		mCovar, vDa, m_dChiSq) )
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
						NonlinearFit1Function *pfnFuncs1D, NLFITCALLBACK pfnCallBack,
						LPVOID pUserData)
{
	int i, k, iter, itst;
	double dOldChiSq;
	CMatrix mCovar(nMA, nMA), mAlpha(nMA, nMA);

	m_pfnCallBack = pfnCallBack;
	m_pUserData = pUserData;
	// save all data
	m_pfnFunc = pfnFuncs1D;
	m_pdData = pdData;
	m_pdXcoords = pdCoordsX;
	m_pdDataSig = pdDataSig;
	m_nDataX = nData;

	// copy parameters
	m_vParams.resize(nMA);
	m_vBoolParams.resize(nMA);
	m_vEsds.resize(nMA);
	copy(pdParams, pdParams+nMA, m_vParams.data());
	copy(pbParams, pbParams+nMA, m_vBoolParams.begin());

	m_nMA = nMA;
	// get the number of parameters to be varied
	m_nFit = count_if(m_vBoolParams.begin(), m_vBoolParams.end(),
		bind2nd(equal_to<bool>(), true));
	// iterations
	int iIter = 0;
	for(iter = 0; iter < 2; iter++)
	{
		m_dALamda = -1.0;
		mrqmin(mCovar, mAlpha, m_vParams.data());
		if( NULL != m_pfnCallBack )
		{
			if( false == m_pfnCallBack(m_vParams.data(), nMA, m_dChiSq, iIter++, m_pUserData) )
			{
				goto Error_Exit;
			}
		}
		k = 1;
		itst = 0;
		for( ; ; )
		{
			dOldChiSq = m_dChiSq;
			if( k++ >= 100 )
				goto ExitIterate;
			mrqmin(mCovar, mAlpha, m_vParams.data());
			iIter++;
			TRACE(_T("iteration: %d, Chi^2=%g.\n"), k, m_dChiSq);
			if( NULL != m_pfnCallBack )
			{
				if( false == m_pfnCallBack(m_vParams.data(), nMA, m_dChiSq, iIter, m_pUserData) )
				{
					goto Error_Exit;
				}
			}
			if( m_dChiSq > dOldChiSq )
				itst = 0;
			else if( fabs(dOldChiSq - m_dChiSq) < 1.0 )
				itst++;
			if (itst < 4)
				continue;
			m_dALamda = 0.0;
			mrqmin(mCovar, mAlpha, m_vParams.data());
			break;
		}
	}
ExitIterate :
	TRACE(_T("\ta[###]\tValue\tESD\n"));
	iter = 0;
	for(i = 0; i < nMA; i++)
	{
		pdParams[i] = m_vParams[i];
		if( true == m_vBoolParams[i] )
		{
//			m_vEsds[i] = FastSqrt(mCovar[iter][iter]);
			m_vEsds[i] = FastSqrt(mCovar[i][i]);
			iter++;
		}
//		TRACE(_T("\ta[%d]\t%g\t%g\n"), i, m_vParams[i], m_vEsds[iter-1]);
		TRACE(_T("\ta[%d]\t%g\t%g\n"), i, m_vParams[i], m_vEsds[i]);
	}
	TRACE(_T("\n"));
	return true;

Error_Exit:
	TRACE(_T("Calculations have been terminated...\n"));
	return false;
}
