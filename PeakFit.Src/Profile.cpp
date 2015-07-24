// Profile.cpp: implementation of the CProfile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#include "Math\MathDef.h"
//#include "Math\MathKrnl.h"
#include "Math\Smooth.h"
//#include "Math\Vector.h"

#include "Profile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////
//
// bool GetBaseLine()
// IN:	int nLevel - 0 = Level only
//					 1 = Level and Zero
//
// OUT: double &dBaseLine - value of BASELINE
//
// RETURN:	true - if BASELINE detected ok, false overwise
//
/////////////////////////////////////////////////////////////////////////////////////
bool CProfile::GetBaseLine(int nLevel)
{
	CPolynomFit lfit;
	CVector out;
	vector<int> flag(size(), 1);
	double sumx, sumy, sumx2, sumxy, sumy2, a, b, d, mean, stddev, thresh;
	int n2 = 0, nGdPts, k = 1, maxit = 20;
	CProfile copyprof(*this);
	size_t t, n;

	while( k < maxit)
	{
		sumx = sumy = sumx2 = sumxy = 0.0;
		for(t = 0; t < size(); t++)
		{
			sumx  += t * flag[t];
			sumy  += copyprof[t] * flag[t];
			sumx2 += SQR(t) * flag[t];
			sumxy += t * copyprof[t] * flag[t];
		}

		nGdPts = 0;
		nGdPts = accumulate(flag.begin(), flag.end(), nGdPts); // number of non-peak points

		d = (nGdPts*sumx2)-(sumx*sumx);
		a = ((nGdPts*sumxy)-(sumy*sumx))/d;		// calculate slope
		b = ((sumx2*sumy)-(sumx*sumxy))/d; 		// and intercept

		if( k == 1 )
			mean = a*(size()-1)/2 + b;	// Calculate only on original trace (401MCB)
		// subtract best fit line
		for(t = 0; t < size(); t++)
		{
			copyprof[t] -= a*t + b;
		}

		n = count_if(copyprof.begin(), copyprof.end(), bind2nd(less<double>(), 0.0)); // number of points below BFL

		// termination condition
		if( n < (size()/2) )
		{
			// Put offset back in if no zeroing
			if( nLevel == 0 )
			{
				transform(copyprof.begin(), copyprof.end(), copyprof.begin(), bind2nd(plus<double>(), mean));
			}
			copy(copyprof.begin(), copyprof.end(), begin());
			return true;
		}
		sumy2 = 0.0;
		for(t = 0; t < size(); t++)
		{
			if( copyprof[t] < 0.0 )
			{
				sumy2 += SQR(copyprof[t]);		// calculate std dev of lower half
			}
		}
		stddev = FastSqrt(sumy2 / n);
		if( n != n2 )
		{
			thresh = 1.8*stddev;
			n2 = n;
		}
		for(t = 0; t < size(); t++)
		{
			flag[t] = flag[t] * ((copyprof[t] < thresh) ? 1 : 0); // ignore values above thresh
		}
		if( nLevel == 0 )
		{
			// Put offset back in if no zeroing
			transform(copyprof.begin(), copyprof.end(), copyprof.begin(), bind2nd(plus<double>(), mean));
		}
		k++;
	}

	// No convergence
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////
//
// bool GetBackground()
// IN:	double dGrad - value of the max gradient between neigbour BKG points
//		double dAvg  - average value of BKG points must be less then this value
//
// OUT: double &dBkg - value of BKG
//		int &nBkgSize - number of points in the BKG sequence
//
// RETURN:	true - if BKG detected ok, false overwise
//
/////////////////////////////////////////////////////////////////////////////////////
bool CProfile::GetBackground(double dGrad, double dAvg, double &dBkg, int &nBkgSize)
{
	vector<double> diff;
	vector<double>::iterator it1, it2, large/* = NULL*/;
	int nLargeSize = 0, nStart, nSize;
	double dSumm = 0.0, dMin;

	dBkg = 0.0;
	nBkgSize = 0;

	dMin = *min_element(begin(), end());
	diff.resize(this->size());
	adjacent_difference(this->begin(), this->end(), diff.begin());
	// first element of diff is not a difference!
	// 	it1 = it2 = large = diff.begin();
	it1 = it2 = diff.begin();
	large = diff.end();
	while(1)
	{
		if( (it1 = find_if(it2, diff.end(), bind2nd(less_equal<double>(), dGrad))) == diff.end())
			break;
		if( (it2 = find_if(it1, diff.end(), bind2nd(greater<double>(), dGrad))) == diff.end())
			break;
		// average must be close to the 0
		// overwise it is slowly growing sequence (with small gradient < or > 0)
		nSize = (it2 - it1);
		dSumm = FastAbs(accumulate(it1, it2, 0.0) / nSize);
		// TODO: change the limit to the dSumm
		if( (nSize > nLargeSize) && (dSumm < dAvg) )
		{
			nLargeSize = nSize;
			large = it1;
		}
	}
	// no background found
//	if( large == NULL )
	if( diff.end() == large )
	{
		dBkg = dMin;
		nBkgSize = size();
		return false;
	}

	// get the average
	nStart = large - diff.begin();
	if( nStart <= 0 )
		nStart = 1;

	dSumm = accumulate(this->begin() + nStart - 1, this->begin() + nStart + nLargeSize, 0.0);
	dBkg = dSumm / nLargeSize;

	nBkgSize = nLargeSize;

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////
//
// bool Smooth()
// IN:	none
//
// OUT: none
//
// RETURN:	true - if smmothed ok, false overwise
//
/////////////////////////////////////////////////////////////////////////////////////
bool CProfile::Smooth()
{
	// smooth the profile
	size_t X = 32, der = 8;
	if( X > ((size()-1) >> 1) ) X = (size()-1)>>1;
	if( X < 1 )  X = 1;
	// nothing to smooth
	if( der > ((size()-1) >> 1) )
		return false;

	vector<double> temp(*this);

	return Smooth::SavitzkyGolay(temp, *this, X, X, der, true);
//	return Smooth::SavitzkyGolay((double*)&*begin(), size(), (double*)&*begin(), X, X, der, true);
}
//////////////////////////////////////////////////////////////////////////////////////
//
// bool GetPoints()
// IN:	none
//
// OUT: Vector &pts - a vector, which fills with coordinates for points lies on the
//		vector perpendicular to the profile path, number of the points is m_nWidth.
//
// RETURN:	none
//
/////////////////////////////////////////////////////////////////////////////////////
void CProfile::GetPoints(Vector &pts)
{
	// do nothing
	throw _T("CProfile::GetPoints() -> function is not implemented");
}
//////////////////////////////////////////////////////////////////////////////////////
//
// bool SingleStep() - performs a single step along the profile
// IN:	none
//
// OUT: none
//
// RETURN:	none
//
/////////////////////////////////////////////////////////////////////////////////////
void CProfile::SingleStep()
{
	// do nothing
	throw _T("CProfile::SingleStep() -> function is not implemented");
}

bool CProfile::Initialize()
{
	return (m_bInitialized = true);
}

void CProfile::Reset()
{
	// do nothing
	throw _T("CProfile::Reset() -> function is not implemented");
}
