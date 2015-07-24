// PeakSearch.cpp: implementation of the PeakSearch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PeakSearch.h"
#include "Smooth.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace PeakSearch
{

template<class T>
struct absolute_plus : binary_function<T, T, T>
{
	T operator()(const T& _X, const T& _Y) const { return (_X + FastAbs(_Y)); }
};

template<class T>
struct absolute_less : binary_function<T, T, bool>
{
	bool operator()(const T& _X, const T& _Y) const { return (FastAbs(_X) < FastAbs(_Y)); }
};

template<class T>
struct absolute_greater : binary_function<T, T, bool>
{
	bool operator()(const T& _X, const T& _Y) const { return (FastAbs(_X) > FastAbs(_Y)); }
};

static int ScanInterval(double *begin, double *end, int m, int beta, bool bReverse)
{
	int p, n;
	double bkg, mul, val, prev_val, sum_I;
	double *cur;

	n = end - begin;
	prev_val = 10e+30;
	double inv_beta = 1.0 / (double) beta;
	cur = begin + m;
	sum_I = *cur;
	mul = 2.0 * inv_beta;
	if( !bReverse )
		p = m + 1;
	else
		p = m - 1;
	while( 1 )
	{
		if( !bReverse )
		{
			if( p > n - beta )
				break;
			cur++;
			sum_I += *cur;
			bkg = accumulate(cur + 1, cur + beta + 1, 0.0);
			p++;
		}
		else
		{
			if( p < beta )
				break;
			cur--;
			sum_I += *cur;
			bkg = accumulate(cur - beta, cur, 0.0);
			p--;
		}
		val = (sum_I + SQR(mul)*bkg) / (sum_I - mul*bkg);
		if( val > prev_val )
		{
			break;
		}
//		TRACE1("%f\n", val);
		prev_val = val;
		mul += inv_beta;
	}
//	TRACE0("---------\n");
//	TRACE1("min: %f\n", prev_val);
//	TRACE0("---------\n");
	return (bReverse) ? p+1 : p-1;
}

bool ScanForPeaks(double *begin, double *end, vector<PEAKSEARCH> &vPeaks, int iStart)
{
	double *max_el;
	int m, startP, endP;
	PEAKSEARCH	pi;

/*
	vector<float> detail;

	ResolveProfile(profile, detail, 3);
	// 1. find maximum
	max_el = max_element(detail.begin(), detail.end());
	m = max_el - detail.begin();	// point of maximum counts

	// scan left interval
	start = ScanInterval(detail.begin(), detail.end(), m, 1, TRUE);
	// scan right interval
	end = ScanInterval(detail.begin(), detail.end(), m, 1, FALSE);
*/
	// 1. find maximum
	max_el = max_element(begin, end);
	if(*max_el < 1)
		return false;
	m = max_el - begin;	// point of maximum counts

	if( (m == 0) || (max_el == end-1) )
		return false;
	// scan left interval
	startP = ScanInterval(begin, end, m, 3, TRUE);
	// scan right interval
	endP = ScanInterval(begin, end, m, 3, FALSE);
	pi.piStart = startP+iStart;
	pi.piPos = m+iStart;
	pi.piEnd = endP+iStart;
	TRACE3("peak: start:%d\tend:%d\tpos:%d\n", pi.piStart, pi.piEnd, (int)pi.piPos);
	vPeaks.push_back(pi);
	ScanForPeaks(begin, begin+startP, vPeaks, iStart);
	ScanForPeaks(begin+endP, end, vPeaks, endP+iStart);

	return true;
}

bool GradScanForPeaks(double *pBegin, double *pEnd, vector<PEAKSEARCH> &vPeaks, int iStart)
{
	double noise, I, x;
	int nSize = pEnd - pBegin;
	vector<double> deltaY(nSize);
// 	vector<double>::iterator start, end, last, cur, first;
	vector<double>::iterator start, end, last, first;
	double *cur;
	PEAKSEARCH pi;

	// clear peak array
	vPeaks.clear();

	// Calculate deltaY for all the profile
	adjacent_difference(pBegin, pEnd, deltaY.begin());
	deltaY[0] = deltaY[1];

	// Determine average value - "noise"
	noise = accumulate( deltaY.begin(), deltaY.end(), 0.0, absolute_plus<double>() ) / deltaY.size();

	noise *= 0.5;
	first = start = end = deltaY.begin();
	last = deltaY.end();
	while( 1 )
	{
		// 2. Look through array and determine deltaY >= noise
		start = find_if( end, last, bind2nd(greater_equal<double>(), noise) );
		if( start == last )
			break;
		// 3. Look for maximum value
		for(cur = pBegin + (start - first); cur < pEnd - 1; cur++)
		{
			if( *(cur-1) < *cur && *cur > *(cur+1) )
				break;
		}
		if( cur == pEnd - 2)
			break;
		
		// 4. look for the end
		end = find_if( first + (cur - pBegin), last, bind2nd(absolute_greater<double>(), noise) );
		if( end == start || end == last )
			break;
		end = find_if( end, last, bind2nd(absolute_less<double>(), noise) );
		if( end == start || end == last )
			break;
		// 5. mass centre of the peak
		I = x = 0.0f;
		for( cur = pBegin + (start - first); cur != pBegin + (end - first); cur++)
		{
			I += *cur;
			x += (*cur) * (cur - pBegin);
		}
		pi.piPos = x / I;
		// fill in peak structure
		pi.piStart = start - first;
		pi.piEnd   = end   - first;
		// add peak
		vPeaks.push_back(pi);
//		TRACE1("%f\n", pi.piPos);
	}

	return true;
}

//static FILE *pFile = NULL;

bool SmoothDeriv(CProfile &profile, CProfile &smooth, int min_rad, int nSmooth)
{
	double _x, _y, _a, _b, _inv;
	CProfile x_coords;
	int i, nSize = profile.size();
	bool bRes = false;

	if( (min_rad >= nSize) || (nSize < min_rad + 2*nSmooth + 1) )
	{
		TRACE0(_T("SmoothDeriv() -> too little points in the profile for smoothing.\n"));
		return false;
	}
	x_coords.resize(nSize, 0);
	smooth.resize(nSize, 0);
	for(i = 0; i < nSize; i++)
	{
		x_coords[i] = i;
	}
	_inv = 1.0 / double(2*nSmooth + 1);
	try
	{
		for(i = min_rad + nSmooth; i < nSize - nSmooth - 1; i++)
		{
			_x = accumulate(x_coords.begin() + i - nSmooth, x_coords.begin() + i + nSmooth + 1, 0);
			_y = accumulate(profile.begin() + i - nSmooth, profile.begin() + i + nSmooth + 1, 0);
			_x *= _inv;
			_y *= _inv;
			_a = _b = 0.0;
			for(int j = -nSmooth; j <= nSmooth; j++)
			{
				_a += (x_coords[i + j] - _x)*(profile[i + j] - _y);
				_b += SQR(x_coords[i + j] - _x);
			}
			smooth[i] = _a / _b;
		}
		bRes = true;
	}
	catch( ... )
	{
		TRACE("SmoothDeriv -> unhandled exception with:\n");
		TRACE("\tmin_rad = %d, i = %d, nSize = %d, nSmooth = %d\n",
			min_rad, i, nSize, nSmooth);
		TRACE("\tprofile.size() = %d, smooth.size() = %d\tx_coords.size() = %d\n",
			profile.size(), smooth.size(), x_coords.size());
//		if( pFile )
//		{
//			fprintf(pFile, "SmoothDeriv -> unhandled exception with:\n");
//			fprintf(pFile, "\tmin_rad = %d, i = %d, nSize = %d, nSmooth = %d\n",
//				min_rad, i, nSize, nSmooth);
//			fprintf(pFile, "\tprofile.size() = %d, smooth.size() = %d\tx_coords.size() = %d\n",
//				profile.size(), smooth.size(), x_coords.size());
//		}
	}
	TRACE("SmoothDeriv -> successful with:\n");
	TRACE("\tmin_rad = %d, i = %d, nSize = %d, nSmooth = %d\n",
		min_rad, i, nSize, nSmooth);
	TRACE("\tprofile.size() = %d, smooth.size() = %d\tx_coords.size() = %d\n",
		profile.size(), smooth.size(), x_coords.size());
//	if( pFile && bRes )
//	{
//		fprintf(pFile, "SmoothDeriv -> successful with:\n");
//		fprintf(pFile, "\tmin_rad = %d, i = %d, nSize = %d, nSmooth = %d\n",
//			min_rad, i, nSize, nSmooth);
//		fprintf(pFile, "\tprofile.size() = %d, smooth.size() = %d\tx_coords.size() = %d\n",
//			profile.size(), smooth.size(), x_coords.size());
//	}
	return bRes;
}

bool FindPeaksSmoothDeriv(CProfile &profile, CProfile &peaks,
						  double min_rad, bool bSmooth)
{
	CProfile::iterator begin, end, start, finish;
	CProfile smooth(profile);
	bool bRes = false;
	double peak_pos;

//	pFile = fopen("d:\\peaksrch.txt", "wt");
	
	peaks.clear();
//	if( pFile ) fprintf(pFile, "calculating the smoothed derivative...");
	try
	{
		if( false == SmoothDeriv(profile, smooth, (int)min_rad, 7) )
		{
			return false;
		}
	}
	catch( ... )
	{
//		if( pFile ) fprintf(pFile, "unhandled exception occured\n");
		return false;
	}
//	if( pFile ) fprintf(pFile, "...done\n");
	try
	{
		// initialize the pointers
		start = (finish = (begin = smooth.begin()));
		end = smooth.end();
//		if( pFile )
//		{
//			// vector main pointers
//			fprintf(pFile, "smooth: begin=0x%08X, end=0x%08X, start=0x%08X, finish=0x%08X\n",
//				begin, end, start, finish);
//		}
		// iterate
		while( 1 )
		{
			// look for the zero-cross
			for( ; start < end-1; ++start )
			{
				// break if we've found the fall of the first derivative down
				if( (*start > 0) && (*(start+1) <= 0) )
					break;
			}
			// break if at the end
			if( start >= end )
				break;
			// look for the end of the zero-cross
			for(finish = start+1; finish < end-1; ++finish )
			{
				// break if we've found fall down
				if( (*(finish-1) >= 0) && (*finish < 0) )
					break;
			}
			// break if at the end
			if( finish >= end )
				break;
			// interpolate x coordinate
			peak_pos = (finish - start) * (*start) / (*start - *finish) + (start-begin);
			if( peak_pos >= min_rad )
			{
				peaks.push_back(peak_pos);
//				if( pFile ) fprintf(pFile, "found peak %d at %g...\n", peaks.size(), peak_pos);
			}
			start = finish;
		}
		bRes = true;
	}
	catch( ... )
	{
//		if( pFile ) fprintf(pFile, "peak search -> unhandled exception occured\n");
		bRes = true;
	}
//	if( pFile )
//	{
//		fprintf(pFile, "peak search has finished\n");
//		fclose(pFile);
//		pFile = NULL;
//	}
	return bRes;
}

}; // namespace PeakSearch
