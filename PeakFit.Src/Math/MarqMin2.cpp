#include "StdAfx.h"

#include "MarqMin.h"
#include "Gauss.h"

NlGauss2D	NL_Gauss2D;
NlLorentz2D NL_Lorentz2D;

NlGauss2DAs			NL_Gauss2DAs;
NlLorentz2DAs		NL_Lorentz2DAs;
NlPseudoVoigt2DAs	NL_PseudoVoigt2DAs;

NlPseudoVoigt2D_PS	NL_PseudoVoigt2D_PS;

G_IMPLEMENT_DYNAMIC(NonlinearFit2Function, NonlinearFitFunction);

////////////////////////////////////////////////////////////////////////////////////
//							Symmetric functions
////////////////////////////////////////////////////////////////////////////////////
//	f(u,v) = a4*exp(-((u-a0)/a2)^2 - ((v-a1)/a3)^2 ) + a5
// m_pdParams two dimensional gaussian
double NlGauss2D::Calculate(double x, double y, double *pdParams, double *dyda, int na)
{
	double fac, expon, arg0, arg1, inv_prm2, inv_prm3;

	inv_prm2 = 1.0 / pdParams[2];
	inv_prm3 = 1.0 / pdParams[3];
	arg0 = (x - pdParams[0]) * inv_prm2;
	arg1 = (y - pdParams[1]) * inv_prm3;
	expon = exp( - SQR(arg0) - SQR(arg1) );
	fac = 2.0 * pdParams[4] * expon;

	dyda[0] = arg0 * fac * inv_prm2;
	dyda[1] = arg1 * fac * inv_prm3;
	dyda[2] = SQR(arg0) * fac * inv_prm2;
	dyda[3] = SQR(arg1) * fac * inv_prm3;
	dyda[4] = expon;
	dyda[5] = 1;

	return pdParams[4] * expon + pdParams[5];
}
////////////////////////////////////////////////////////////////////////////////////
// f(x,y) = a4 / [ (4*((x-a0)/a2)^2 + 1) * (4*((y-a1)/a3)^2 + 1) ] + a5
// pdParams two dimensional lorentz
double NlLorentz2D::Calculate(double x, double y, double *pdParams, double *dyda, int na)
{
	double arg0, arg1, wx, wy, inv_denom, inv_den1, inv_den2, amp, mul;

	wx = 1.0 / pdParams[2];
	wy = 1.0 / pdParams[3];
	amp = pdParams[4];
	arg0 = (x - pdParams[0]) * wx;
	arg1 = (y - pdParams[1]) * wy;
	inv_den1 = 1.0 / (4.0 * SQR(arg0) + 1.0);
	inv_den2 = 1.0 / (4.0 * SQR(arg1) + 1.0);
	inv_denom = inv_den1 * inv_den2;

	dyda[5] = 1;		// dBkg
	dyda[4] = inv_denom;		// dAmp
	mul = 8.0 * amp * inv_denom;
	dyda[3] = mul * inv_den2 * SQR(arg1) * wy;	// dwy
	dyda[2] = mul * inv_den1 * SQR(arg0) * wx;	// dwx
	dyda[1] = mul * inv_den2 * arg1 * wy;	// dy0
	dyda[0] = mul * inv_den1 * arg0 * wx;	// dx0
	return amp * inv_denom + pdParams[5];
}
////////////////////////////////////////////////////////////////////////////////////
//							Asymmetric functions
////////////////////////////////////////////////////////////////////////////////////
//	if(x < a0)  f(u,v) = a6*exp(-((u-a0)/a2)^2 - ((v-a1)/a4)^2 ) + a7
//	if(x >= a0) f(u,v) = a6*exp(-((u-a0)/a3)^2 - ((v-a1)/a4)^2 ) + a7
//	if(y < a1)  f(u,v) = a6*exp(-((u-a0)/a2)^2 - ((v-a1)/a4)^2 ) + a7
//	if(y >= a1) f(u,v) = a6*exp(-((u-a0)/a2)^2 - ((v-a1)/a5)^2 ) + a7
// m_pdParams two dimensional gaussian
double NlGauss2DAs::Calculate(double x, double y, double *pdParams, double *dyda, int na)
{
	double fac, expon, arg0, arg1, inv_prm1, inv_prm2;
	int ind1, ind2;

	dyda[2] = dyda[3] = dyda[4] = dyda[5] = 0.0;
	ind1 = ( x < pdParams[0] ) ? 2 : 3;
	ind2 = ( y < pdParams[1] ) ? 4 : 5;
	inv_prm1 = 1.0 / pdParams[ind1];
	inv_prm2 = 1.0 / pdParams[ind2];
	arg0 = (x - pdParams[0]) * inv_prm1;
	arg1 = (y - pdParams[1]) * inv_prm2;
	expon = exp( - SQR(arg0) - SQR(arg1) );
	fac = 2.0 * pdParams[6] * expon;
	// derivatives
	dyda[0] = arg0 * fac * inv_prm1;
	dyda[ind1] = arg0 * dyda[0];
	dyda[1] = arg1 * fac * inv_prm2;
	dyda[ind2] = arg1 * dyda[1];
	dyda[6] = expon;
	dyda[7] = 1;
	// function value
	return pdParams[6] * expon + pdParams[7];
}
////////////////////////////////////////////////////////////////////////////////////
// if(x < a0)  f(x,y) = a5 / [ (4*((x-a0)/a2)^2 + 1) * (4*((y-a1)/a4)^2 + 1) ] + a6
// if(x >= a0) f(x,y) = a5 / [ (4*((x-a0)/a3)^2 + 1) * (4*((y-a1)/a4)^2 + 1) ] + a6
// pdParams two dimensional lorentz
double NlLorentz2DAs::Calculate(double x, double y, double *pdParams, double *dyda, int na)
{
	double arg0, arg1, wx, wy, inv_denom, inv_den1, inv_den2, amp, mul;
	int ind1, ind2;

	dyda[2] = dyda[3] = dyda[4] = dyda[5] = 0.0;
	ind1 = ( x < pdParams[0] ) ? 2 : 3;
	ind2 = ( y < pdParams[1] ) ? 4 : 5;
	wx = 1.0 / pdParams[ind1];
	wy = 1.0 / pdParams[ind2];
	amp = pdParams[6];
	arg0 = (x - pdParams[0]) * wx;
	arg1 = (y - pdParams[1]) * wy;
	inv_den1 = 1.0 / (4.0 * SQR(arg0) + 1.0);
	inv_den2 = 1.0 / (4.0 * SQR(arg1) + 1.0);
	inv_denom = inv_den1 * inv_den2;
	// derivatives
	dyda[7] = 1;		// dBkg
	dyda[6] = inv_denom;		// dAmp
	mul = 8.0 * amp * inv_denom;
	dyda[ind1] = arg0 * (dyda[0] = mul * inv_den1 * arg0 * wx);	// dwx
	dyda[ind2] = arg1 * (dyda[1] = mul * inv_den2 * arg1 * wy);	// dwy
	// function value
	return amp * inv_denom + pdParams[7];
}
////////////////////////////////////////////////////////////////////////////////////
// f(x,y) = a12 + a11 * [ a10*exp(-((u-a0)/a2,3)^2 - ((v-a1)/a4,5)^2) +
//						 (1-a10)/(4*((x-a0)/a6,7)^2+1)*(4*((y-a1)/a8,9)^2+1) ]
// pdParams two dimensional lorentz
double NlPseudoVoigt2DAs::Calculate(double x, double y, double *pdParams, double *dyda, int na)
{
	double x0, y0, wx, wy, amp, K, dx, dy, expon, fac, arg0, arg1, inv_den1, inv_den2, mul, inv_denom;
	int ind1, ind2;

	dyda[2] = dyda[3] = dyda[4] = dyda[5] = dyda[6] = dyda[7] = dyda[8] = dyda[9] = 0.0;

	x0 = pdParams[0];
	y0 = pdParams[1];
	amp = pdParams[11];
	K = pdParams[10];

	dx = x - x0;
	dy = y - y0;

	ind1 = ( x < x0 ) ? 2 : 3;
	ind2 = ( y < y0 ) ? 4 : 5;

	// GAUSS part
	wx = 1.0 / pdParams[ind1];
	wy = 1.0 / pdParams[ind2];
	arg0 = dx * wx;
	arg1 = dy * wy;
	expon = exp( - SQR(arg0) - SQR(arg1) );
	fac = 2.0 * amp * expon;
	// derivatives
	dyda[0] = arg0 * fac * wx * K;
	dyda[ind1] = arg0 * dyda[0];
	dyda[1] = arg1 * fac * wy * K;
	dyda[ind2] = arg1 * dyda[1];
	dyda[11] = expon*K;					// dAmp
	dyda[12] = 1;						// dBkg
	dyda[10] = amp * expon;
	// LORENTZ part
	ind1 += 4;
	ind2 += 4;
	wx = 1.0 / pdParams[ind1];
	wy = 1.0 / pdParams[ind2];
	arg0 = dx * wx;
	arg1 = dy * wy;
	inv_den1 = 1.0 / ( 4.0 * SQR(arg0) + 1.0);
	inv_den2 = 1.0 / ( 4.0 * SQR(arg1) + 1.0);
	inv_denom = inv_den1 * inv_den2;
	// derivatives
	dyda[11] += inv_denom * (1-K);				// dAmp
	mul = 8.0 * amp * inv_denom;
	fac = mul * inv_den1 * arg0 * wx * (1-K);
	dyda[0] += fac;
	dyda[ind1] = arg0 * fac;	// dwx
	fac = mul * inv_den2 * arg1 * wy * (1-K);
	dyda[1] += fac;
	dyda[ind2] = arg1 * fac;	// dwy
	dyda[10] -= amp * inv_denom;

	return amp * (K * expon + (1 - K) * inv_denom) + pdParams[12];
}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
#define X0			0
#define Y0			1
#define A_SIN		2		// A_SIN*sin(S_SIN*theta+B_SIN)
#define	S_SIN		3
#define WX_LEFT		4
#define WX_RIGHT	5
#define WY_LEFT		6
#define WY_RIGHT	7
#define M_COEFF		8		// Lorentz coeffs
#define N_COEFF		9
#define L_COEFF		10		// L*Gauss + (1-L)*Lorentz
#define AMPL		11
#define K_COEFF		12		// background k*y+b
#define B_COEFF		13
#define	B_SIN		14
void NlPseudoVoigt2D_PS::StepParameters(double *pdAtry, double *pdParams,
										bool *pbParams, int nMA, double *pdDelta)
{
	int l, j;
	double tmp;

	// make new params as A = A + dA
	for(j = 0, l = 0; l < nMA; l++)
	{
		if( pbParams[l] )
		{
			tmp = pdParams[l] + pdDelta[j++];
			// phase shift
			if( B_SIN == l )
			{
				while( tmp > M_2PI )
					tmp -= M_2PI;
				while( tmp < 0.0 )
					tmp += M_2PI;
				pdAtry[l] = tmp;
			}
			if( L_COEFF == l )
			{
				if( tmp < 0.0 )
					tmp = 0.0;
				else if( tmp > 1.0 )
					tmp = 1.0;
				pdAtry[l] = tmp;
			}
			else
				pdAtry[l] = tmp;
		}
	}
	// look up the ration between Lorentzian and Gaussian:
	// if K == 1 -> pure Gaussian, Lorentzian overwise
/*	if( 1.0 == pdParams[L_COEFF] )
	{
		// GAUSS only, disable Lorentz parameters
		pbParams[M_COEFF] = false;
		pbParams[N_COEFF] = false;
	}
	else
	{
		pbParams[M_COEFF] = true;
		pbParams[N_COEFF] = true;
//		for(j = 4; j <= 13; j++)
//			pbParams[j] = true;
	}
*/
}
void NlPseudoVoigt2D_PS::Precalculate(double *pdParams, bool *pbBoolParams, int nMA)
{
	double sc, pha;

	sc = pdParams[S_SIN];
	pha = pdParams[B_SIN];
	if( m_vSinTh.empty() )
		m_vSinTh.resize( m_vAngle.size() );
	if( m_vCosTh.empty() )
		m_vCosTh.resize( m_vAngle.size() );
	// precalculate sin
	for(size_t i = 0; i < m_vSinTh.size(); i++)
	{
		FastSinCos(sc*m_vAngle[i] + pha, m_vSinTh[i], m_vCosTh[i]);
	}
}
// f(x,y) = a16 + a17*y + a15 * [ a14*exp(-((u-a0)/a4,5)^2 - ((v-y0)/a6,7)^2) +
//								 (1-a14)/(a12*((x-a0)/a8,9)^2+1)*(a13*((y-y0)/a10,11)^2+1) ]
// where	y0 = a1 - a2*Sin(a3*T+a18)
double NlPseudoVoigt2D_PS::Calculate(double x, double y, double *pdParams, double *dyda, int na)
{
	double x0, y0, wx, wy, amp, K, K_1, dx, dy, expon, fac1, fac2, M, N, SA, Th;
	double arg0, arg1, inv_den1, inv_den2, mul, inv_denom, dSin, dCos;
	int ind1, ind2;

	dyda[WX_LEFT] = dyda[WX_RIGHT] = dyda[WY_LEFT] = dyda[WY_RIGHT] = 0.0;

	x0 = pdParams[X0];
	y0 = pdParams[Y0];
	M = pdParams[M_COEFF];
	N = pdParams[N_COEFF];
	K = pdParams[L_COEFF];
	K_1 = 1.0 - K;
	amp = pdParams[AMPL];

	SA = pdParams[A_SIN];
	if( !m_vAngle.empty() )
	{
		dSin = m_vSinTh[_x];
		dCos = m_vCosTh[_x];
		Th = m_vAngle[_x];
	}
	else
		dSin = dCos = Th = 0.0;
	dx = x - x0;
	dy = y - y0 + SA*dSin;

	ind1 = ( x < x0 ) ? 4 : 5;
	ind2 = ( y < y0 ) ? 6 : 7;

	wx = 1.0 / pdParams[ind1];
	wy = 1.0 / pdParams[ind2];
	arg0 = dx * wx;
	arg1 = dy * wy;
	// GAUSS part
	expon = exp( - SQR(arg0) - SQR(arg1) );
	fac1 = 2.0 * amp * expon;
	// derivatives
	dyda[X0] = arg0 * fac1 * wx * K;
	dyda[Y0] = arg1 * fac1 * wy * K;
	dyda[ind1] = arg0 * dyda[X0];
	dyda[ind2] = arg1 * dyda[Y0];
	dyda[L_COEFF] = amp * expon;				// dK
	dyda[AMPL] = expon*K;					// dAmp
	dyda[A_SIN] = -2.0*amp*dSin * K * expon * arg1 * wy;			// sine amp
	dyda[B_SIN] = -2.0*amp*SA*dCos * K * expon * arg1 * wy;
	dyda[S_SIN] = dyda[B_SIN] * Th;	// sine angle scale
	// LORENTZ part
	inv_den1 = 1.0 / (M * SQR(arg0) + 1.0);
	inv_den2 = 1.0 / (N * SQR(arg1) + 1.0);
	inv_denom = inv_den1 * inv_den2;
	dyda[A_SIN] += -2.0 * amp * dSin * K_1*inv_denom*inv_den2*N*arg1*wy;	// sine amp
	fac2 = -2.0*amp*SA*dCos * K_1*inv_denom*inv_den2*N*arg1*wy;
	dyda[B_SIN] += fac2;
	dyda[S_SIN] += fac2 * Th;				// sine angle scale
	// derivatives
	dyda[AMPL] += inv_denom * K_1;		// dAmp
	mul = 2.0 * amp * inv_denom;
	fac2 = M * mul * inv_den1 * arg0 * wx * K_1;
	dyda[X0] += fac2;
	dyda[ind1] += arg0 * fac2;			// dwx
	fac2 = N * mul * inv_den2 * arg1 * wy * K_1;
	dyda[Y0] += fac2;
	dyda[ind2] += arg1 * fac2;			// dwy
	dyda[L_COEFF] -= amp * inv_denom;		// dK
	// constant derivatives
	dyda[M_COEFF] = -amp * K_1 * SQR(arg0) * inv_denom * inv_den1;		// dM
	dyda[N_COEFF] = -amp * K_1 * SQR(arg1) * inv_denom * inv_den2;		// dN
	dyda[K_COEFF] = 1;						// dBkg
	dyda[B_COEFF] = y;						// dBkg

	return amp * (K * expon + K_1 * inv_denom) + pdParams[K_COEFF] + pdParams[B_COEFF]*y;
}
/*
double NlPseudoVoigt2D_PS::Calculate(double x, double y, double *pdParams, double *dyda, int na)
{
	double x0, y0, wx, wy, amp, K, K_1, dx, dy, expon, fac1, fac2, M, N, SA, Th;
	double arg0, arg1, inv_den1, inv_den2, mul, inv_denom, dSin, dCos;
	int ind1, ind2;

	dyda[4] = dyda[5] = dyda[6] = dyda[7] = dyda[8] =
		dyda[9] = dyda[10] = dyda[11] = 0.0;

	x0 = pdParams[0];
	y0 = pdParams[1];
	M = pdParams[12];
	N = pdParams[13];
	K = pdParams[14];
	K_1 = 1.0 - K;
	amp = pdParams[15];

	SA = pdParams[2];
	if( !m_vAngle.empty() )
	{
		dSin = m_vSinTh[_x];
		dCos = m_vCosTh[_x];
		Th = m_vAngle[_x];
	}
	else
		dSin = dCos = Th = 0.0;
	dx = x - x0;
	dy = y - y0 + SA*dSin;

	ind1 = ( x < x0 ) ? 4 : 5;
	ind2 = ( y < y0 ) ? 6 : 7;

	// GAUSS part
	wx = 1.0 / pdParams[ind1];
	wy = 1.0 / pdParams[ind2];
	arg0 = dx * wx;
	arg1 = dy * wy;
	expon = exp( - SQR(arg0) - SQR(arg1) );
	fac1 = 2.0 * amp * expon;
	// derivatives
	dyda[0] = arg0 * fac1 * wx * K;
	dyda[1] = arg1 * fac1 * wy * K;
	dyda[ind1] = arg0 * dyda[0];
	dyda[ind2] = arg1 * dyda[1];
	dyda[14] = amp * expon;				// dK
	dyda[15] = expon*K;					// dAmp
	dyda[2] = -2.0*amp*dSin * K * expon * arg1 * wy;			// sine amp
	dyda[18] = -2.0*amp*SA*dCos * K * expon * arg1 * wy;
	dyda[3] = dyda[18] * Th;	// sine angle scale
	// LORENTZ part
	ind1 += 4;
	ind2 += 4;
	wx = 1.0 / pdParams[ind1];
	wy = 1.0 / pdParams[ind2];
	arg0 = dx * wx;
	arg1 = dy * wy;
	inv_den1 = 1.0 / (M * SQR(arg0) + 1.0);
	inv_den2 = 1.0 / (N * SQR(arg1) + 1.0);
	inv_denom = inv_den1 * inv_den2;
	dyda[2] += -2.0 * amp * dSin * K_1*inv_denom*inv_den2*N*arg1*wy;	// sine amp
	fac2 = -2.0*amp*SA*dCos * K_1*inv_denom*inv_den2*N*arg1*wy;
	dyda[18] += fac2;
	dyda[3] += fac2 * Th;				// sine angle scale
	// derivatives
	dyda[15] += inv_denom * K_1;		// dAmp
	mul = 2.0 * amp * inv_denom;
	fac2 = M * mul * inv_den1 * arg0 * wx * K_1;
	dyda[0] += fac2;
	dyda[ind1] = arg0 * fac2;			// dwx
	fac2 = N * mul * inv_den2 * arg1 * wy * K_1;
	dyda[1] += fac2;
	dyda[ind2] = arg1 * fac2;			// dwy
	dyda[14] -= amp * inv_denom;		// dK
	// constant derivatives
	dyda[12] = -amp * K_1 * SQR(arg0) * inv_denom * inv_den1;		// dM
	dyda[13] = -amp * K_1 * SQR(arg1) * inv_denom * inv_den2;		// dN
	dyda[16] = 1;						// dBkg
	dyda[17] = _y;						// dBkg

	return amp * (K * expon + K_1 * inv_denom) + pdParams[16] + pdParams[17]*y;
}
*/
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool NonlinearFit2Function::AlphaBetaChisq(double *pdParams, bool *pbBoolParams,
										   int nMA, int nMfit,
										   double *pdData, int nDataX, int nDataY,
										   double *pdCoordsX, double *pdCoordsY,
										   CMatrix &mAlpha, CVector &vBeta,
										   double &dChiSq)
{
	int j, k, l, m, x, y;
	double *z, zmod, wt, dz, *ptr_x, *ptr_y, dY;
	CVector		vDyDa(nMA);

	// some optimizations can be done in this overloaded function
	Precalculate(pdParams, pbBoolParams, nMA);
	// initialize (symmetric) alpha and beta
	for(j = 0; j < nMfit; j++)
	{
		for(k = 0; k <= j; k++)
			mAlpha[j][k] = 0.0;
		vBeta[j] = 0.0;
	}
	// set Chi^2 to 0
	dChiSq = 0.0;
	// for all data points perform a summation
	z = pdData;
	ptr_y = pdCoordsY;
	for(y = 0; y < nDataY; y++)
	{
		_y = y;
		ptr_x = pdCoordsX;
		dY = *ptr_y++;
		for(x = 0; x < nDataX; x++)
		{
			_x = x;
			// get Zcalc
			zmod = Calculate(*ptr_x++, dY, pdParams, vDyDa.data(), nMA);

			// calculate delta
			dz = (*z++) - zmod;
			// Chi square = SUM(Yobs - Ycalc)^2
			dChiSq += SQR(dz);
			// make Alpha matrix: Alpha(k,l) = SUM(dy/dA(k) * dy/dA(l));
			for(j = 0, l = 0; l < nMA; l++)
			{
				if( pbBoolParams[l] )
				{
					wt = vDyDa[l];
					for(k = 0, m = 0; m <= l; m++)
					{
						if( pbBoolParams[m] )
							mAlpha[j][k++] += wt*vDyDa[m];
					}
					vBeta[j++] += dz*wt;
				}
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

void NonlinearFit2Function::StepParameters(double *pdAtry, double *pdParams,
										   bool *pbParams, int nMA, double *pdDelta)
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

////////////////////////////////////////////////////////////////////////////////////

CNonlinearFit2::CNonlinearFit2()
{
	// start Lamda small like 0.001
	m_dALamda = 1e-3;

	m_pfnFuncs2D = NULL;
	m_pdData = NULL;
	m_pdXcoords = NULL;
	m_pdYcoords = NULL;
	m_nDataX = 0;
	m_nDataY = 0;
	m_dStepX = 0.;
	m_dStepY = 0.;
	m_dStartX = 0.;
	m_dStartY = 0.;

	m_nFit = m_nMA = 0;

	// very big number
	m_dOldChiSq = 10e+12;
}

CNonlinearFit2::~CNonlinearFit2()
{
}

bool CNonlinearFit2::mrqmin(CMatrix &mCovar, CMatrix &mAlpha, double *pdParams)
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
		if( false == m_pfnFuncs2D->AlphaBetaChisq(pdParams, &*m_vBoolParams.begin(),
			m_nMA, m_nFit, m_pdData, m_nDataX, m_nDataY, m_pdXcoords, m_pdYcoords,
			mAlpha, vBeta, m_dChiSq) )
			return false;
		m_dOldChiSq = m_dChiSq;
		for(j = 0; j < m_nMA; j++)
		{
			vAtry[j] = pdParams[j];
		}
	}
	//	loop specified number of iterations
//	for( i = 0; i <= 3; i++ )
	{
//		m_dOldChiSq = m_dChiSq;

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
			CERR << _T("2D FITTING: Gauss solver failed!");
			return false;
		}
		// copy
		for(j = 0; j < m_nFit; j++)
		{
			vDa[j] = vOneDA[j];
		}
		// finish calculations
		if( 0.0 == m_dALamda )
		{
			// cosvrt
			return true;
		}
		// step parameters
		m_pfnFuncs2D->StepParameters(vAtry.data(), pdParams,
			&*m_vBoolParams.begin(), m_nMA, vOneDA.data());
		// calculate Chi square
		if( false == m_pfnFuncs2D->AlphaBetaChisq(vAtry.data(), &*m_vBoolParams.begin(),
			m_nMA, m_nFit, m_pdData, m_nDataX, m_nDataY, m_pdXcoords, m_pdYcoords,
			mCovar, vDa, m_dChiSq) )
			// TODO: check vDa or vBeta
		{
			return false;
		}
		// increase Chi square by 10
		if(m_dChiSq >= m_dOldChiSq)
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
	}

	return true;
}
//
// Nonlinear method of reducing Chi-square of m_pdParams fit btw m_pdParams set of m_ppdData
//		pdDataX and pdDataY - m_ppdData points in the range [0, nData)
//		pdDataSig - individual std deviations in the range [0, nData)
//		nData - number of m_ppdData points
//		pdParams - array with parameters (some of them may has initial values)
//		piParams - array of integers, each value indicates:
//				[j] value is non-zero - [j] parameter has to be fitted for the value from pdParams[j]
//				[j] value is zero     - [j] parameter has fixed value from pdParams[j]
//		nMA - number of parameters
//		pfnFuncs - user defined function, which depends on some parameters
//
bool CNonlinearFit2::Fit(double *pdData, double *pdDataX, double *pdDataY,
						 int nDataX, int nDataY,
						 double *pdParams, const Vectorb &vBoolParams, int nMA,
						 NonlinearFit2Function *pfnFuncs2D)
{
	int i, k, iter, itst;
	double dOldChiSq;
	CMatrix mAlpha(nMA, nMA), mCovar(nMA, nMA);

	// save all m_ppdData
	m_pfnFuncs2D = pfnFuncs2D;
	m_pdData = pdData;
	m_nDataX = nDataX;
	m_nDataY = nDataY;
	m_pdXcoords = pdDataX;
	m_pdYcoords = pdDataY;

	// steps during processing the array
	m_dStepX = (pdDataX[nDataX-1] - pdDataX[0])/(nDataX-1);
	m_dStepY = (pdDataY[nDataY-1] - pdDataY[0])/(nDataY-1);
	m_dStartX = pdDataX[0];
	m_dStartY = pdDataY[0];

	// copy parameters
	m_vParams.resize(nMA);
	m_vBoolParams.resize(nMA);
	copy(pdParams, pdParams + nMA, m_vParams.data());
	copy(vBoolParams.begin(), vBoolParams.end(), m_vBoolParams.begin());

	m_nMA = nMA;
	// get the number of parameters to be varied
	m_nFit = count_if(vBoolParams.begin(), vBoolParams.end(),
		bind2nd(equal_to<bool>(), true));

	// iterations
	for(iter = 0; iter < 2; iter++)
	{
		m_dALamda = -1.0;
		mrqmin(mCovar, mAlpha, m_vParams.data());
		k = 1;
		itst = 0;
		for( ; ; )
		{
			dOldChiSq = m_dChiSq;
			if( k++ >= 100 )
				goto ExitIterate;
			mrqmin(mCovar, mAlpha, m_vParams.data());
			CERR << _T("iteration: ") << k << _T(", Chi^2=") << m_dChiSq << _T(".\n");
			if( m_dChiSq > dOldChiSq )
				itst = 0;
			else if( fabs(dOldChiSq - m_dChiSq) < 0.01 )
				itst++;
			if (itst < 4)
				continue;
			m_dALamda = 0.0;
			mrqmin(mCovar, mAlpha, m_vParams.data());
			break;
		}
	}
ExitIterate :
//	TRACE("\n%s %2d %17s %10.4f %10s %9.2e\n","Iterations #", k, "chi^2=", dOldChiSq, "lamda=", m_dALamda);

	for(i = 0; i < nMA; i++)
		CERR << _T("      a[") << i << _T("]\t");
//		TRACE("%5s[%d]\t", "a", i);
	CERR << _T("\n");
	for(i = 0; i < nMA; i++)
	{
		CERR << m_vParams[i] << _T("\t");
//		TRACE("%9.4f\t", m_vParams[i]);
		pdParams[i] = m_vParams[i];
	}
	CERR << _T("\n");

	return true;
}
