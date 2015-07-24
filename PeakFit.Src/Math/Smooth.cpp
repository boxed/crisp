// Smooth.cpp: implementation of the CSmooth class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#include "Math\MathKrnl.h"
#include "LUDecomp.h"
#include "Smooth.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace Smooth
{

static float minarg1,minarg2;
#define FMIN(a,b) (minarg1=(a),minarg2=(b),(minarg1) < (minarg2) ? (minarg1) : (minarg2))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
Returns in c[1..np], in wrap-around order (N.B.!) consistent with the argument respns in
routine convlv, a set of Savitzky-Golay filter coe.cients. nl is the number of leftward (past)
data points used, while nr is the number of rightward (future) data points, making the total
number of data points used nl +nr +1. ld is the order of the derivative desired (e.g., ld = 0
for smoothed function). m is the order of the smoothing polynomial, also equal to the highest
conserved moment; usual values are m = 2 or m = 4.
*/
static BOOL savgol(CVector &c, int np, int nl, int nr, int ld, int m)
{
	int imj,i_pj,k,kk,mm;
	double d,fac,sum;
	vector<int> indx(0, m+1);
	CVector b(0.0, m+1);
	CMatrix a(m+1, m+1);

	if (np < nl + nr + 1 || nl < 0 || nr < 0 || ld > m || nl +nr < m)
	{
		TRACE0("bad args in savgol");
		return false;
	}
	for (i_pj = 0; i_pj <= (m << 1); i_pj++)		// Set up the normal equations of the desired
	{
		sum = (i_pj ? 0.0 : 1.0);				// least-squares fit.
		for (k = 1; k <= nr; k++)
			sum += pow((double)k,(double)i_pj);
		for (k = 1; k <= nl; k++)
			sum += pow((double)-k,(double)i_pj);
		mm = (int)FMIN((float)(i_pj), (float)(2*m-i_pj));
		for (imj = -mm; imj <= mm; imj += 2)
			a[(i_pj+imj)/2][(i_pj-imj)/2] = sum;
	}
	LUDecomp::LUDecomp(a, indx, d);			// Solve them: LU decomposition.

	b[ld] = 1.0;
	// Right-hand side vector is unit vector, depending on which derivative we want.
	LUDecomp::LUBacksub(a, indx, b);			// Get one row of the inverse matrix.

	for (k = -nl; k <= nr; k++)
	{
		sum = b[0];								// Each Savitzky-Golay coefficient is the dot
		fac = 1.0;								// product of powers of an integer with the
		for (mm = 0; mm < m; mm++)				// inverse matrix row.
			sum += b[mm+1]*(fac *= k);
		kk = ((np-k) % np);						// Store in wrap-around order.
		c[kk] = sum;
	}
	return true;
}

bool SavitzkyGolay(vector<double> &Ydata, vector<double> &OutYdata,
							int nPtsLeft, int nPtsRight, int nDeriv, bool bWrap)
{
	OutYdata.clear();
	if(Ydata.size() != OutYdata.size())
	{
		OutYdata.resize(Ydata.size());
	}

	return SavitzkyGolay((double*)&*Ydata.begin(), Ydata.size(),
		(double*)&*OutYdata.begin(), nPtsLeft, nPtsRight, nDeriv, bWrap);
//	return SavitzkyGolay(Ydata.begin(), Ydata.size(), OutYdata.begin(), nPtsLeft, nPtsRight, nDeriv, bWrap);
}

bool SavitzkyGolay(double *pYdata, int nPoints, double *pOutYdata,
							int nPtsLeft, int nPtsRight, int nDeriv, bool bWrap)
{
	if(nDeriv < 2 || nDeriv > 8)
	{
		TRACE1("CSmooth::SavitzkyGolay() -> nDeriv == %d is out of range.\n", nDeriv);
		return false;
	}
	if(nPtsLeft < 1 || nPtsLeft > 32)
	{
		TRACE1("CSmooth::SavitzkyGolay() -> nPtsLeft == %d is out of range.\n", nPtsLeft);
		return false;
	}
	if(nPtsRight < 1 || nPtsRight > 32)
	{
		TRACE1("CSmooth::SavitzkyGolay() -> nPtsRight == %d is out of range.\n", nPtsRight);
		return false;
	}
	if(nPoints < nPtsLeft+nPtsRight+1)
	{
		TRACE1("CSmooth::SavitzkyGolay() -> nPoints == %d is out of range.\n", nPoints);
		return false;
	}

	int np = nPtsLeft+nPtsRight+1;
	CVector c(0.0, np);

	// get Savitzky-Golay coefficients
	savgol(c, np, nPtsLeft, nPtsRight, 0, nDeriv);

	double val;
	int i, j, pos;
	for(i = 0; i != nPoints; ++i)
	{
		val = 0.0;
		if( bWrap == true )
		{
			for(j = nPtsLeft; j >= 0; j--)
			{
				pos = i - j;
				while( pos < 0 ) pos += nPoints;
				val += c[j + 1]*pYdata[pos];
			}
			for(j = 1; j <= nPtsRight; j++)
			{
				pos = i + j;
				while( pos >= nPoints ) pos -= nPoints;
				val += c[np - j + 1]*pYdata[pos];
			}
		}
		else
		{
			for(j = nPtsLeft; j >= 0; j--)
			{
				if(i - j >= 0)
					val += c[j + 1]*pYdata[i-j];
				else
					val += c[j + 1]*pYdata[0];
			}
			for(j = 1; j <= nPtsRight; j++)
			{
				if(i + j < nPoints)
					val += c[np - j + 1]*pYdata[i+j];
				else
					val += c[np - j + 1]*pYdata[nPoints-1];
			}
		}
		pOutYdata[i] = val;
	}

	return true;
}

}; // namespace Smooth
