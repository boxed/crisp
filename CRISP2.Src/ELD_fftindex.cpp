// FftIndex.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"

#include "commondef.h"
#include "objects.h"
#include "ft32util.h"
#include "eld.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char TMP[65536];

int s_nTab = 1;
static FILE *s_pLog = NULL;

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

using namespace std;

void ft1CalcReal( double *data, int size )
{
	FFTD1R(data, 0, size, 1);
}

void ft1CalcInverseReal( double *data, int size )
{
	IFFTD1R(data, 0, size, 1);
}

typedef vector<double> VectorD;

typedef struct
{
	double x;
	double y;
	double _x;
	double A;
	int n;
} PEAK;

#define MAX_FFT_PEAKS	3

typedef struct
{
//	double dPos;
//	double dAmpl;
	double dSumAmpl;
	int iPos[MAX_FFT_PEAKS];
	double dPos[MAX_FFT_PEAKS];
	double dAmpl[MAX_FFT_PEAKS];
	double dAngle;
	int nFftSize;
	int nSuccessPeaks;		// always < MAX_FFT_PEAKS

} FFTPEAK;

struct sort_pos : binary_function<bool, FFTPEAK, FFTPEAK>
{
	sort_pos(int nPos)
	{
		if( (_nPos = nPos) >= MAX_FFT_PEAKS ) _nPos = 0;
	}
	bool operator()(const FFTPEAK &p1, const FFTPEAK &p2) const
	{
		return (p1.dPos[_nPos] < p2.dPos[_nPos]) ? true : false;
//		return (p1.dPos < p2.dPos) ? true : false;
	}
	int _nPos;
};

struct sort_ampl : binary_function<bool, FFTPEAK, FFTPEAK>
{
	sort_ampl(int nPos)
	{
		if( (_nPos = nPos) >= MAX_FFT_PEAKS ) _nPos = 0;
	}
	bool operator()(const FFTPEAK &p1, const FFTPEAK &p2) const
	{
		if( _nPos < 0 )
		{
			return (p1.dSumAmpl > p2.dSumAmpl) ? true : false;
		}
		return (p1.dAmpl[_nPos] > p2.dAmpl[_nPos]) ? true : false;
	}
	int _nPos;
};

typedef vector<PEAK> PeakVec;
typedef vector<PEAK>::iterator PeakVecIt;
typedef vector<PEAK>::const_iterator PeakVecCit;

typedef vector<FFTPEAK> FftPeakVec;
typedef vector<FFTPEAK>::iterator FftPeakVecIt;
typedef vector<FFTPEAK>::const_iterator FftPeakVecCit;

static void DoRotation(PeakVec &vPeaks, double dAngle)
{

	PeakVecIt pit;
	double dSin, dCos;

	dSin = FastSin(dAngle);
	dCos = FastCos(dAngle);

	for(pit = vPeaks.begin(); pit != vPeaks.end(); ++pit)
	{
		pit->_x = pit->x * dCos - pit->y * dSin;
//		pit->_x = pit->x * dCos + pit->y * dSin;
	}

}
// calculate the power of 2 number closest (>=) to nLen
static int Po2(int nLen)
{

	int iRes = 1;
	while( iRes < nLen )
	{
		iRes <<= 1;
	}
	return iRes;

}

static void FindMinMaxX(const PeakVec &vPeaks, double &dMin, double &dMax, PEAK *pPeakMin)
{

	PeakVecCit pit;
	dMin =  1e+30;
	dMax = -1e+30;
	for(pit = vPeaks.begin(); pit != vPeaks.end(); ++pit)
	{
		if( dMax < pit->_x ) dMax = pit->_x;
		if( dMin > pit->_x ) { dMin = pit->_x; *pPeakMin = *pit; }
	}

}

static void MakePofile(const PeakVec &vPeaks, VectorD &vProf)
{

	PEAK peak_min;
	PeakVecCit pit;
	double dMinX, dMaxX, dLen/*, delta*/, dPos, dA;
	int nLen, iPos;

	FindMinMaxX(vPeaks, dMinX, dMaxX, &peak_min);
	if( dMaxX == dMinX )
	{
		return;
	}
	dLen = dMaxX - dMinX;
	nLen = (int)::floor(dLen + 0.5);
	nLen = Po2(nLen);
//	if( vProf.size() != nLen )
//	{
		vProf.resize(nLen, 0);
//	}
	memset(&*vProf.begin(), 0, sizeof(double)*vProf.size());
	// add peaks
	for(pit = vPeaks.begin(); pit != vPeaks.end(); ++pit)
	{
		dPos = pit->_x - dMinX;
		iPos = (int)::floor(dPos + 0.5);
//		iPos = (int)::floor(dPos);
		if( iPos < nLen )
		{
//			dA = 1;
			dA = pit->A;
//			if( iPos < nLen-1 )
//			{
//				delta = dPos - iPos;
//				vProf[iPos]   += dA * (1.0 - delta);
//				vProf[iPos+1] += dA * delta;
//			}
//			else
			{
				vProf[iPos]   += dA;
			}
		}
//		iPos = (int)::floor(pit->_x - dMinX + 0.5);
//		if( iPos < nLen )
//		{
//			vProf[iPos] += pit->A;
//		}
	}

}
// The FFT has the first complex number:
//	Real part is 0-element (always real)
//	Complex part is the last element (also real)
static bool PowerSpectrum(VectorD &vProf, VectorD &vSpectr)
{

	double *dst, *src;
	int nSize, i;

	nSize = vProf.size() >> 1;
	if( nSize < 2 )
	{
		return false;
	}
	vSpectr.clear();
	vSpectr.resize(nSize);
	memset(&*vSpectr.begin(), 0, sizeof(double)*nSize);
	dst = &*vSpectr.begin();
	src = &*vProf.begin();
	for(i = 0; i < nSize; i++, src+=2)
	{
		dst[i] = FastSqrt(src[0]*src[0] + src[1]*src[1]);
	}
//	dst[0] = src[0];
	return true;

}

template<class T>
struct absolute_plus : binary_function<T, T, T>
{
	T operator()(const T& _X, const T& _Y) const { return (_X + FastAbs(_Y)); }
};

static double GetNoiseLevel(double *pProf, int nSize)
{

	double dNoise = 0.0;

	if( nSize > 1 )
	{
		for(int i = 1; i < nSize; i++)
		{
			dNoise += FastAbs(pProf[i] - pProf[i-1]);
		}
		// Determine average value - "noise"
		dNoise /= nSize;
	}
	dNoise *= 0.5;
	return dNoise;

}

static bool RefinePos(double *pdX, double *pdY, double &dPos, int &iPos)
{

	CPolynomFit pfit;
	CVector res;
	bool bRes = false;

	iPos = -1;
	dPos = -1.0;
	if( (NULL == pdX) || (NULL == pdY) )
	{
		return false;
	}
	if( TRUE == pfit.Solve(pdX, pdY, 3, 3, res) )
	{
		if( res[2] != 0.0 )
		{
			dPos = -res[1] / (2.0 * res[2]);
			iPos = (int)::floor(dPos + 0.5);
			bRes = true;
		}
	}
	return bRes;

}

static bool AnalyzeSpectrum(VectorD &vProf, VectorD &vSpectr,
							FFTPEAK &fftpeak, bool bLookMax = false,
							bool bDetailedReport = false)
{

	double dMax, dThresh, *ptr, /**ptr2, */*pFirstAbove, *pdMax;
	double val, d, dNoise, adX[3];
	bool bRet = false;
	int i, nSize;

	// some initializations
	nSize = vSpectr.size();
	ptr = &*vSpectr.begin();
	if( bDetailedReport && s_pLog ) fprintf(s_pLog, "AnalyzeSpectrumBUG() -> profile size %d.\n", nSize);
	memset(&fftpeak, 0, sizeof(fftpeak));
	// set initial peak positions to -1
	for(i = 0; i < MAX_FFT_PEAKS; i++)
	{
		fftpeak.iPos[i] = -1;
	}
	// zero size of the profile? - quit
	if( 0 == nSize )
	{
		return false;
	}
	dMax = vSpectr[0];
	dThresh = 0.5 * dMax;
	if( bDetailedReport && s_pLog ) fprintf(s_pLog, "AnalyzeSpectrumBUG() -> max = %.2f.\n", dMax);
	// skip the first maximum
	for(i = 1; i < nSize; i++)
	{
		if( ptr[i] > ptr[i-1]/*dThresh*/ ) break;
	}
	// control the position
	if( i >= nSize - 1 )
	{
		return false;
	}
	// get the average noise level
	dNoise = GetNoiseLevel(ptr + i, nSize - i);
	if( bDetailedReport && s_pLog )
		fprintf(s_pLog, "AnalyzeSpectrumBUG() -> peak pos = %d, noise %.2f.\n", i, dNoise);
	// find a peak which is higher that the 3*noise level
	pFirstAbove = find_if(ptr + i, ptr + nSize, bind2nd(greater_equal<double>(), 3.0 * dNoise));
	if( pFirstAbove == ptr + nSize )
	{
		if( bDetailedReport && s_pLog ) fprintf(s_pLog, "AnalyzeSpectrumBUG() -> no peaks were found.\n");
		return false;
	}
	if( true == bLookMax )
	{
		if( bDetailedReport && s_pLog ) fprintf(s_pLog, "AnalyzeSpectrumBUG() -> looking for max...");
		pdMax = max_element(ptr + i, ptr + nSize);
		fftpeak.iPos[0] = pdMax - ptr;
	}
	else
	{
		if( bDetailedReport && s_pLog ) fprintf(s_pLog, "AnalyzeSpectrumBUG() -> not looking for max...");
		dMax = *max_element(ptr + i, ptr + nSize);
		// if the maximum value (except 0-th element)
//		if( dMax < 0.3 * vSpectr[0] )
		if( (dMax < 10.0 * dNoise) && (dMax < 0.1 * vSpectr[0]) )
		{
			return false;
		}
		dThresh = 0.5 * dMax;
		// look for a first value above the threshold
		for(; i < nSize; i++)
		{
			if( ptr[i] > dThresh ) break;
		}
		// look for a maximum
		if( i == nSize )
		{
			return false;
		}
		fftpeak.iPos[0] = i;
		val = ptr[i++];
		for(; i < nSize; i++)
		{
			d = ptr[i];
			if( d > val ) { val = d; fftpeak.iPos[0] = i; }
			else break;
		}
		if( bDetailedReport && s_pLog ) fprintf(s_pLog, "ok\n");
	}
	// get the maximum using parabolic fitting of 3 points
	if( (fftpeak.iPos[0] > 1) && (fftpeak.iPos[0] < nSize - 1) )
	{
		if( bDetailedReport && s_pLog )
			fprintf(s_pLog, "AnalyzeSpectrumBUG() -> zero pos = %d.\n", fftpeak.iPos[0]);
		adX[0] = fftpeak.iPos[0] - 1;
		adX[1] = fftpeak.iPos[0];
		adX[2] = fftpeak.iPos[0] + 1;
		if( true == RefinePos(adX, ptr + fftpeak.iPos[0] - 1, fftpeak.dPos[0], fftpeak.iPos[0]) )
		{
			if( (fftpeak.iPos[0] < nSize) && (fftpeak.iPos[0] > 1) )
			{
				fftpeak.dAmpl[0] = ptr[fftpeak.iPos[0]];
				fftpeak.nSuccessPeaks++;
			}
			else
			{
				return false;
			}
			if( bDetailedReport && s_pLog )
				fprintf(s_pLog, "AnalyzeSpectrumBUG() -> refined pos = %.2f (%d), ampl = %.2f.\n",
				fftpeak.dPos[0], fftpeak.iPos[0], fftpeak.dAmpl[0]);
/*
			// try to get the 3d reflection, if not possible try to get 2nd
			// then refine the position of the 3d (2nd) reflection and compare to the 1st
			if( fftpeak.dAmpl[0] > 0.0 )
			{
				for(i = 1; i < MAX_FFT_PEAKS; i++)
				{
					fftpeak.iPos[i] = (int)::floor((i + 1) * fftpeak.dPos[0] + 0.5);
					if( bDetailedReport && s_pLog )
						fprintf(s_pLog, "AnalyzeSpectrumBUG() -> iPos[%d] = %d.\n",
						i, fftpeak.iPos[i]);
					if( (fftpeak.iPos[i] < nSize - (i + 2)) && (fftpeak.iPos[i] > i + 1) )
					{
						// find closest maximum within +/- (i+1) pixels
						ptr2 = max_element(ptr + fftpeak.iPos[i] - (i + 1),
							ptr + fftpeak.iPos[i] + (i + 2));
						if( *ptr2 > dThresh )
						{
							fftpeak.iPos[i] = ptr2 - ptr;
							adX[0] = fftpeak.iPos[i] - 1;
							adX[1] = fftpeak.iPos[i];
							adX[2] = fftpeak.iPos[i] + 1;
							if( true == RefinePos(adX, ptr + fftpeak.iPos[i] - 1, fftpeak.dPos[i], fftpeak.iPos[i]) )
							{
								// number of successfull peaks
								if( fftpeak.iPos[i] < nSize )
								{
									fftpeak.dAmpl[i] = ptr[fftpeak.iPos[i]];
									fftpeak.nSuccessPeaks++;
								}
							}
						}
					}
				}
			}
*/
		}
	}
	fftpeak.dSumAmpl = accumulate(fftpeak.dAmpl, &fftpeak.dAmpl[MAX_FFT_PEAKS], 0.0);
	if( bDetailedReport && s_pLog )
		fprintf(s_pLog, "AnalyzeSpectrumBUG() -> sum ampl = %.2f.\n", fftpeak.dSumAmpl);
	return true;

}

static void RefineAxis(double &dAngle, double dStep, double &dPos, int &nFftSize,
					   double &dAmpl, PeakVec &vPeaks)
{

	double dCurAngle, dCurAmpl, dCurPos, dBestPos;
	VectorD vProf, vSpectr;
	int i, iBestStep, iPos;
	FFTPEAK fftpeak;

	if( s_pLog ) fprintf(s_pLog, "Refining the axis at %.2f angle, %.5f step.\n", dAngle, dStep);
	dBestPos = dPos;
	iBestStep = 0;
	for(i = -5; i <= 5; i++)
	{
		dCurAngle = dAngle + i * dStep;
		DoRotation(vPeaks, dCurAngle);
		MakePofile(vPeaks, vProf);
		// 1D fft
		ft1CalcReal(&*vProf.begin(), vProf.size());
		if( true == PowerSpectrum(vProf, vSpectr) )
		{
			AnalyzeSpectrum(vProf, vSpectr, fftpeak);
			dCurPos = fftpeak.dPos[0];
			dCurAmpl = fftpeak.dAmpl[0];
			if( FastAbs(dCurPos - dPos) < 1.0 )
			{
				iPos = (int)::floor(dCurPos + 0.5);
				if( iPos < vProf.size() )
				{
					// TODO: check it is better to compare the result from returned by AnalyzeSpectrum
					dCurAmpl = vSpectr[iPos];
					if( dCurAmpl > dAmpl )
					{
						dAmpl = dCurAmpl;
						dBestPos = dCurPos;
						iBestStep = i;
						nFftSize = vProf.size();
					}
				}
			}
		}
	}
	dAngle += iBestStep * dStep;
	dPos = dBestPos;
	if( s_pLog ) fprintf(s_pLog, "\t\tok...at %.2f angle, %.2f pos.\n", dAngle, dPos);

}

static bool FilteredFft(VectorD &vProf, double &dDist, double &dMaxPos)
{

	VectorD vTmpProf(vProf), vSpectr;
	double dMax, dPos, dThresh, dThresh2, dAmpl2;
	double ax, ay;
	double *src, *pdMax;
	int i, iLeft, iRight, nMaxPos;
	FFTPEAK fftpeak;

	// initial values (negative shows failure)
	dDist = -1;
	dMaxPos = -1;
	// calculate the FFT
	ft1CalcReal(&*vTmpProf.begin(), vTmpProf.size());
	if( false == PowerSpectrum(vTmpProf, vSpectr) )
	{
		return false;
	}
	if( false == AnalyzeSpectrum(vTmpProf, vSpectr, fftpeak) )
	{
		return false;
	}
	dPos = fftpeak.dPos[0];
	dAmpl2 = fftpeak.dAmpl[0];
	if( (dPos < 1) || (dPos >= vProf.size()) )
	{
		return false;
	}
	dDist = vProf.size() / dPos;
	dMax = vTmpProf[0];
	dThresh2 = SQR(dMax * 0.5);
	src = &*vTmpProf.begin();
	vTmpProf[1] = 0.0;
	// cut out all weak points (below the threshold)
	for(i = 2; i < vTmpProf.size(); i+=2)
	{
		dAmpl2 = SQR(src[i]) + SQR(src[i+1]);
		if( dAmpl2 < dThresh2 )
		{
			src[i] = src[i+1] = 0.0;
		}
	}
	ft1CalcInverseReal(&*vTmpProf.begin(), vTmpProf.size());
	// find maximum here
	pdMax = &*max_element(vTmpProf.begin(), vTmpProf.end());
	nMaxPos = pdMax - &*vTmpProf.begin();
	dMax = *pdMax;
	// in some perfect case we will have several peaks with MAX amplitude
	pdMax = &*vTmpProf.begin();
	VectorD vMaxVals;
	for(i = 0; i < vTmpProf.size(); i++, pdMax++)
	{
		if( *pdMax == dMax )
		{
			vMaxVals.push_back(i);
		}
	}
	int nMid = vMaxVals.size() >> 1;
	nMaxPos = vMaxVals[nMid];
	// the threshold
	dThresh = 0.5 * dMax;
	// clip to 0 all values below the threshold
	src = &*vTmpProf.begin();
	for(i = 0; i < vTmpProf.size(); i++)
	{
		if( src[i] < dThresh ) src[i] = 0.0;
	}
	// find a mass center of the values around dMax
	// - find left position
	for(iLeft = nMaxPos; iLeft >= 0; iLeft--)
	{
		if( 0.0 == src[iLeft] ) break;
	}
	// - find right position
	for(iRight = nMaxPos; iRight >= 0; iRight++)
	{
		if( 0.0 == src[iRight] ) break;
	}
	ax = ay = 0.0;
	for(i = iLeft+1; i < iRight; i++)
	{
		ax += src[i];
		ay += src[i] * i;
	}
	if( ax != 0.0 )
	{
		dMaxPos = ay / ax;
	}
	return true;

}

static bool AnalyzeFfts(PeakVec &vPeaks, FftPeakVec &vFftPeaks,
						LatCellVec &vRedCells, int nCellsMax)
{

	LATTICE_CELL BaseCell, ReducedCell;
	FftPeakVecIt pit;
	int j, nFftSize_a;
	double dInvSin, dAngle_a, dAngle_b, a_len, b_len, dAngDiff, dPos_a, dAmpl_a;
	CVector2d a_axis, b_axis, a_dir, b_dir;
	LatCellVecIt cit;
	double dThreshA = 0.05, dThreshAng = 0.05;
	PeakVec vPeaksOut;
	double dDistK, dDistH;
	VectorD vProf;
	double dDist1, dDist2, dMinX, dMaxX;
	PEAK peak_min1, peak_min2;

	if( true == vFftPeaks.empty() )
	{
		if( s_pLog ) fprintf(s_pLog, "No good peaks were found in FFTs\n");
		return false;
	}
	// sort by the dAmpl field
//	sort(vFftPeaks.begin(), vFftPeaks.end(), sort_ampl(-1));
	sort(vFftPeaks.begin(), vFftPeaks.end(), sort_ampl(0));
//	sort(vFftPeaks.begin(), vFftPeaks.end()+20, sort_ampl(-1));
//	sort(vFftPeaks.begin(), vFftPeaks.begin()+20, sort_pos());
	// select the strongest (first) vector as the 1st lattice vector
	dAngle_a = vFftPeaks[0].dAngle;
	dPos_a = vFftPeaks[0].dPos[0];
	nFftSize_a = vFftPeaks[0].nFftSize;
	dAmpl_a = vFftPeaks[0].dAmpl[0];

	if( s_pLog )
	{
		fprintf(s_pLog, "a-axis was found to be at\n");
		fprintf(s_pLog, "\tangle = %.5f\n", dAngle_a);
		fprintf(s_pLog, "\tpos   = %.5f\n", dPos_a);
		fprintf(s_pLog, "\tFFT size = %d\n", nFftSize_a);
		fprintf(s_pLog, "\tAmpl  = %.1f\n", dAmpl_a);
		fprintf(s_pLog, "--------------------------\n");
		fprintf(s_pLog, "FFT peaks found = %d\n\n", vFftPeaks.size());
		for(int j = 0; j < vFftPeaks.size(); j++)
		{
			fprintf(s_pLog, "%10.2f\t%10.2f\t%5.1f\t%10.2f\t%5d\t%10.2f\n",
				vFftPeaks[j].dSumAmpl, vFftPeaks[j].dAmpl[0], RAD2DEG(vFftPeaks[j].dAngle),
				vFftPeaks[j].dPos[0], vFftPeaks[j].nFftSize,
				vFftPeaks[j].nFftSize/vFftPeaks[j].dPos[0]);
		}
	}
	// refine the best axis
	RefineAxis(dAngle_a, DEG2RAD(0.1), dPos_a, nFftSize_a, dAmpl_a, vPeaks);
	// find the reflections close to the lattice lines for the given best axis
	dDistH = nFftSize_a / dPos_a;
	dPos_a = nFftSize_a / dDistH;

	{
		DoRotation(vPeaks, dAngle_a);
		MakePofile(vPeaks, vProf);
		FindMinMaxX(vPeaks, dMinX, dMaxX, &peak_min1);
		// perform filtered FFT to find the maximum
		if( false == FilteredFft(vProf, dDistH, dDist1) )
		{
			return false;
		}
	}

	a_dir = CVector2d(FastCos(dAngle_a), FastSin(dAngle_a));
	a_len = (double)nFftSize_a / dPos_a;
	a_axis = a_len * a_dir;
	if( NULL != s_pLog )
	{
		fprintf(s_pLog, "--------------------------\n");
		fprintf(s_pLog, "Reduced cells for different choices of the b-axis\n\n");
		fprintf(s_pLog, "%10.1f\t%10.1f\t%5.1f\t%10.2f\t%4d\t%10.2f\t<---- the refined a-vector\n",
			0.0, dAmpl_a, RAD2DEG(dAngle_a), dPos_a, nFftSize_a, nFftSize_a / dPos_a);
	}
	// try the rest of vectors as a second axis
	j = 1;
	for(pit = vFftPeaks.begin() + 1; pit != vFftPeaks.end(); ++pit, j++)
	{
		dAngle_b = pit->dAngle;
		// check if the angle is too close
		// should be more than 30 degrees and less than 150.0 away from 1st axis
		dAngDiff = FastAbs(RAD2DEG(dAngle_a - dAngle_b));
		if( (dAngDiff <= 30.0) || (dAngDiff > 150.0) )
		{
			continue;
		}
		b_len = (double)pit->nFftSize / pit->dPos[0];
		b_dir = CVector2d(FastCos(dAngle_b), FastSin(dAngle_b));
		b_axis = b_len * b_dir;
		// TODO: check this with Sin
		dInvSin = 1.0 / FastAbs(FastSin(DEG2RAD(dAngDiff)));
		BaseCell.cell[0] = a_len * dInvSin;
		BaseCell.cell[1] = b_len * dInvSin;
		BaseCell.cell[2] = __max(a_len, b_len) * 2.3456; // just big enough
		BaseCell.cell[3] = 90.0;
		BaseCell.cell[4] = 90.0;
		BaseCell.cell[5] = dAngDiff;
		// reduce the cell
		Buerger_ReduceCell(BaseCell, ReducedCell);
		if( NULL != s_pLog )
		{
			fprintf(s_pLog, "%10.1f\t%10.1f\t%5.1f\t%10.2f\t%4d\t%10.2f\t<----\t%10.2f\t%10.2f\t%6.2f\n",
				vFftPeaks[j].dSumAmpl, vFftPeaks[j].dAmpl[0], RAD2DEG(vFftPeaks[j].dAngle),
				vFftPeaks[j].dPos[0], vFftPeaks[j].nFftSize,
				vFftPeaks[j].nFftSize / vFftPeaks[j].dPos[0],
				ReducedCell.cell[0], ReducedCell.cell[1], RAD2DEG(ReducedCell.cell[5]));
		}
		// look through the list and try to find same reduced cell
		for(cit = vRedCells.begin(); cit != vRedCells.end(); ++cit)
		{
			int k;
			// compare cell parameters
			for(k = 0; k < 2; k++)
			{
				if( FastAbs(cit->cell[k] - ReducedCell.cell[k]) > ReducedCell.cell[k]*dThreshA )
				{
					break;
				}
			}
			if( k != 2 )
				continue;
			if( FastAbs(cit->cell[5] - ReducedCell.cell[5]) > ReducedCell.cell[5]*dThreshAng )
			{
				continue;
			}
			break;
		}
		// not found
		if( cit == vRedCells.end() )
		{
			CVector2d vO1, vO2, center;
			double det, L1, dSinA, dCosA, dSinB, dCosB;

			// Calculate the center
			DoRotation(vPeaks, dAngle_b);
			MakePofile(vPeaks, vProf);
			FindMinMaxX(vPeaks, dMinX, dMaxX, &peak_min2);
			// perform filtered FFT to find the maximum
			if( false == FilteredFft(vProf, dDistK, dDist2) )
			{
				continue;
			}

			// det cannot be 0 because we checked that dAngle_a - dAngle_b > 30.0 degrees
			det = FastSin(dAngle_a - dAngle_b);
			FastSinCos(dAngle_a, dSinA, dCosA);
			FastSinCos(dAngle_b, dSinB, dCosB);
			// ---------------------------
//			vO1.x = peak_min1.x + dDist1 * FastCos( dAngle_a);
//			vO1.y = peak_min1.y + dDist1 * FastSin(-dAngle_a);
//			vO2.x = peak_min2.x + dDist2 * FastCos( dAngle_b);
//			vO2.y = peak_min2.y + dDist2 * FastSin(-dAngle_b);
//			L1 = ((vO2.x - vO1.x)*FastCos(dAngle_b) +
//				(vO1.y - vO2.y)*FastSin(dAngle_b)) / det;
//			center.x = vO1.x + L1*FastCos(M_PI_2 - dAngle_a);
//			center.y = vO1.y + L1*FastSin(M_PI_2 - dAngle_a);
			// ---------------------------
			vO1 = CVector2d(peak_min1.x, peak_min1.y) + dDist1 * CVector2d(dCosA, -dSinA);
			vO2 = CVector2d(peak_min2.x, peak_min2.y) + dDist2 * CVector2d(dCosB, -dSinB);
			L1 = ((vO2.x - vO1.x)*dCosB + (vO1.y - vO2.y)*dSinB) / det;
			center = vO1 + L1*CVector2d(dSinA, dCosA);
//			sprintf(TMP, "center: (%.2f, %.2f)\n", vCentre.x, vCentre.y);
//			::OutputDebugString(TMP);
			// save the reduced cell
			ReducedCell.center = center;
//			ReducedCell.h_dir = CVector2d(a_dir.y, -a_dir.x);
//			ReducedCell.k_dir = CVector2d(b_dir.y, -b_dir.x);
			ReducedCell.h_dir = a_dir;
			ReducedCell.k_dir = b_dir;
			ReducedCell.h_dir = CVector2d(a_dir.x, -a_dir.y);
			ReducedCell.k_dir = CVector2d(b_dir.x, -b_dir.y);
			ReducedCell.a_axis = a_len * dInvSin;
			ReducedCell.b_axis = b_len * dInvSin;
//			ReducedCell.a_axis = a_len;
//			ReducedCell.b_axis = b_len;
			ReducedCell.a_angle = dAngle_a;
			ReducedCell.b_angle = dAngle_b;
			ReducedCell.nCount = 1;
			// add it to the list
			vRedCells.push_back(ReducedCell);
			// esceeded maximum size?
			if( vRedCells.size() == nCellsMax )
			{
				break;
			}
		}
		else
		{
			// found
			cit->nCount++;
		}
	}
	if( NULL != s_pLog )
	{
		fprintf(s_pLog, "--------------------------\n");
		fprintf(s_pLog, "Selected reduced cells = %d\n\n", vRedCells.size());
		j = 1;
		for(cit = vRedCells.begin(); cit != vRedCells.end(); ++cit, j++)
		{
			fprintf(s_pLog, "Cell %d:\t%.2f\t%.2f\t%.2f\t, center=(%.2f, %.2f), cnt = %d\n",
				j, cit->cell[0], cit->cell[1], RAD2DEG(cit->cell[5]),
				cit->center.x, cit->center.y, cit->nCount);
		}
	}
	return true;

}

bool ELD_FftIndex(const EldAritemVec &vEldPeaks, LatCellVec &vRedCells, int nRotations)
{

	PeakVec vPeaks;
	FftPeakVec vFftPeaks;
	PEAK peak;
	FFTPEAK fftpeak;
	EldAritemVecCit eit;
	VectorD vProf, vSpectr;
	bool bRes = false;
	int i;

	s_pLog = fopen("d:\\working\\eld\\fft_index_log.txt", "wt");

	try
	{
		int nSaveProf = -1;//74;
		// copy all the peaks
		if( nSaveProf >= 0 )
		{
			fprintf(s_pLog, "--------------------------\n");
			fprintf(s_pLog, "The reflections list\n");
		}
		vPeaks.reserve(vEldPeaks.size());
		for(eit = vEldPeaks.begin(); eit != vEldPeaks.end(); ++eit)
		{
			peak.x = eit->x;
			peak.y = eit->y;
			peak.A = eit->as;
			peak.n = eit->n;
			vPeaks.push_back(peak);
			if( nSaveProf >= 0 )
			{
				fprintf(s_pLog, "%.2f\t%.2f\t%.1f\n", eit->x, eit->y, eit->as);
			}
		}
		// rotate
		double dAngle = 0;
		double dAngleStep = M_PI / (double)nRotations;
		for(i = 0; i < nRotations; i++)
		{
//			if( NULL != s_pLog )
//			{
//				fprintf(s_pLog, "--------------------------\n");
//				fprintf(s_pLog, "projection %d\n", i);
//			}
			try
			{
				vProf.clear();
				vSpectr.clear();
				DoRotation(vPeaks, dAngle);
				MakePofile(vPeaks, vProf);
				if( i == nSaveProf )
				{
					if( NULL != s_pLog )
					{
						fprintf(s_pLog, "--------------------------\n");
						fprintf(s_pLog, "The projection at the tilt = %.1f\n", RAD2DEG(dAngle));
						for(int j = 0; j < vProf.size(); j++)
						{
							fprintf(s_pLog, "%d\t%g\n", j, vProf[j]);
						}
					}
				}
				ft1CalcReal(&*vProf.begin(), vProf.size());
				if( false == PowerSpectrum(vProf, vSpectr) )
				{
					goto Continue_Next;
				}
				if( i == nSaveProf )
				{
					if( NULL != s_pLog )
					{
						fprintf(s_pLog, "--------------------------\n");
						fprintf(s_pLog, "FFT power spectrum for the tilt = %.1f\n", RAD2DEG(dAngle));
						for(int j = 0; j < vSpectr.size(); j++)
						{
							fprintf(s_pLog, "%d\t%g\n", j, vSpectr[j]);
						}
					}
				}
			}
			catch (std::exception& e)
			{
				Trace(_FMT(_T("ELD_FftIndex() -> FFT %d step exception: %s"), i, e.what()));
				if( s_pLog )
				{
					fprintf(s_pLog, _FMT(_T("ELD_FftIndex() -> FFT %d step exception: %s"), i, e.what()).c_str());
				}
			}
			try
			{
				if( true == AnalyzeSpectrum(vProf, vSpectr, fftpeak, false/*, true*/) )
				{
					fftpeak.dAngle = dAngle;
					fftpeak.nFftSize = vProf.size();
					vFftPeaks.push_back(fftpeak);
				}
			}
			catch (std::exception& e)
			{
				Trace(_FMT(_T("ELD_FftIndex() -> %d step exception: %s"), i, e.what()));
				if( s_pLog ) fprintf(s_pLog, _FMT(_T("ELD_FftIndex() -> %d step exception: %s"), i, e.what()).c_str());
				AnalyzeSpectrum(vProf, vSpectr, fftpeak, false/*, true*/);
				if( NULL != s_pLog )
				{
					fprintf(s_pLog, "--------------------------\n");
					fprintf(s_pLog, "FFT power spectrum for the tilt = %.1f\n", RAD2DEG(dAngle));
					for(int j = 0; j < vSpectr.size(); j++)
					{
						fprintf(s_pLog, "%d\t%g\n", j, vSpectr[j]);
					}
				}
			}
Continue_Next:
			dAngle += dAngleStep;
		}
		// analyze all selected FFT peaks
		bRes = AnalyzeFfts(vPeaks, vFftPeaks, vRedCells, 10);
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("ELD_FftIndex() -> %s"), e.what()));
		if( s_pLog ) fprintf(s_pLog, _FMT(_T("ELD_FftIndex() -> %s"), e.what()).c_str());
		bRes = false;
	}
	// close the log file
	if( NULL != s_pLog )
	{
		fclose(s_pLog);
		s_pLog = NULL;
	}
	return bRes;

}

/*
static bool AnalyzeSpectrum(VectorD &vProf, VectorD &vSpectr, FFTPEAK &fftpeak, bool bLookMax = false)
{
	int i, / *iPos, * /nSize;
	double dMax, dThresh;
	double *ptr;
	double val, d/ *, dPos* /;
	bool bRet = false;

	nSize = vSpectr.size();
	ptr = vSpectr.begin();
//	dAmpl = 0;
//	dPos = -1;
	memset(&fftpeak, 0, sizeof(fftpeak));
	if( 0 == nSize )
	{
		return false;//-1;
	}
	dMax = vSpectr[0];
	dThresh = 0.5 * dMax;
	// skip the first maximum
	for(i = 1; i < nSize; i++)
	{
		if( ptr[i] > ptr[i-1]/ *dThresh* / ) break;
	}
	// get the average noise level
	double dNoise = GetNoiseLevel(ptr + i, nSize - i);
	// find a peak which is higher that the 3*noise level
	double *pFirstAbove = find_if(ptr + i, ptr + nSize, bind2nd(greater_equal<double>(), 3.0 * dNoise));
	if( pFirstAbove == ptr+nSize )
	{
		return false;//-1;
	}
	if( true == bLookMax )
	{
		double *pdMax = max_element(ptr+i, ptr + nSize);
		iPos = pdMax - ptr;
	}
	else
	{
		dMax = *max_element(ptr+i, ptr + nSize);
		// if the maximum value (except 0-th element)
//		if( dMax < 0.3 * vSpectr[0] )
		if( (dMax < 10.0 * dNoise) && (dMax < 0.1 * vSpectr[0]) )
		{
			return false;//-1;
		}
		dThresh = 0.5 * dMax;
		// look for a first value above the threshold
		for(; i < nSize; i++)
		{
			if( ptr[i] > dThresh ) break;
		}
		// look for a maximum
		if( i == nSize )
		{
			return -1;//dPos;
		}
		iPos = i;
		val = ptr[i++];
		for(; i < nSize; i++)
		{
			d = ptr[i];
			if( d > val ) { val = d; iPos = i; }
			else break;
		}
	}
	// get the maximum using parabolic fitting of 3 points
	double adX[3] = {iPos-1, iPos, iPos+1};
	if( true == RefinePos(adX, ptr + iPos - 1, dPos, iPos) )
	{
		if( iPos < nSize )
		{
			dAmpl = ptr[iPos];
		}
		// try to get the 3d reflection, if not possible try to get 2nd
		// then refine the position of the 3d (2nd) reflection and compare to the 1st
		if( dAmpl > 0.0 )
		{
			int iPos3, iPos2;
			double dPos2, dPos3, *ptr2, *ptr3, dAmpl2, dAmpl3;

			iPos2 = (int)::floor(2.0 * dPos + 0.5);
			iPos3 = (int)::floor(3.0 * dPos + 0.5);
			if( (iPos2 < nSize+2) && (iPos2 > 2) )
			{
				// find closest maximum within +/- 2 pixels
				ptr2 = max_element(ptr + iPos2 - 2, ptr + iPos2 + 3);
				if( *ptr2 > dThresh )
				{
					iPos2 = ptr2 - ptr;
					adX[0] = iPos2 - 1, adX[1] = iPos2, adX[2] = iPos2 + 1;
					if( true == RefinePos(adX, ptr + iPos2 - 1, dPos2, iPos2) )
					{
						if( iPos2 < nSize )
						{
							dAmpl2 = ptr[iPos2];
						}
					}
				}
			}
			if( (iPos3 < nSize+3) && (iPos3 > 3) )
			{
				// find closest maximum within +/- 3 pixels
				ptr3 = max_element(ptr + iPos3 - 3, ptr + iPos3 + 4);
				if( *ptr3 > dThresh )
				{
					iPos3 = ptr3 - ptr;
					adX[0] = iPos3 - 1, adX[1] = iPos3, adX[2] = iPos3 + 1;
					if( true == RefinePos(adX, ptr + iPos3 - 1, dPos3, iPos3) )
					{
						if( iPos3 < nSize )
						{
							dAmpl3 = ptr[iPos3];
						}
					}
				}
			}
		}
	}
	return dPos;
/ *
	double dNewPos, dRefPos;
	dAmpl = ptr[iPos];
	dRefPos = dPos;
	for(i = 2; i < nSize / dPos; i++)
	{
		dNewPos = i*dRefPos;
		iPos = (int)::floor(dNewPos + 0.5);
		if( iPos >= nSize-1 )
		{
			break;
		}
		adX[0] = iPos-1, adX[1] = iPos, adX[2] = iPos+1;
		if( TRUE == pfit.Solve(adX, ptr + iPos - 1, 3, 3, res) )
		{
			if( res[2] != 0.0 )
			{
				dNewPos = -res[1] / (2.0 * res[2]);
				if( FastAbs(dNewPos - i*dRefPos) < 2.0 )
				{
//					dRefPos = dNewPos / (double)i;
					dRefPos = dPos;
					iPos = (int)::floor(dNewPos + 0.5);
					if( ptr[iPos] > 0.5*dMax )
					{
						dAmpl += ptr[iPos];
					}
				}
			}
		}
	}
	return dPos;
* /
}
*/
/*
static void FindRowOrigin(PeakVec &vPeaksIn, double &dAngle, double &dDist, CVector2d &vOrigin)
{
	PEAK peak_min;
	PeakVecCit pit;
	PeakVec vPeaksOut;
	double dPos, dMinX, dMaxX, ax, ay, dThresh, dCnt, dMassCentre, dOffset, dPhase;
	VectorD vProf, vSpectr;
	int nCnt, iPos;

	vOrigin = -1;

	DoRotation(vPeaksIn, -M_PI_2 + dAngle);
	if( NULL != s_pLog )
	{
		fprintf(s_pLog, "---------------\n");
		fprintf(s_pLog, "Row reflections (%d)\n", vPeaksIn.size());
		for(pit = vPeaksIn.begin(); pit != vPeaksIn.end(); ++pit)
		{
			fprintf(s_pLog, "%10.2f\t%10.2f\t%10.2f\n", pit->x, pit->y, pit->_x);
		}
	}
	MakePofile(vPeaksIn, vProf);
	// perform a filtered FFT over the row
	double dDistK = -1;
	FilteredFft(vProf, dDist, dDistK);

//	ft1CalcReal(vProf.begin(), vProf.size());
//	PowerSpectrum(vProf, vSpectr);
//	dDist = -1;
//	dPos = AnalyzeSpectrum(vProf, vSpectr);
//	if( dPos <= 0.0 )
//	{
//		return;
//	}
//	iPos = 2*(int)::floor(dPos + 0.5);
//	// the real spacing
//	dDist = vProf.size() / dPos;
//	dPhase = FastAcos(vProf[iPos+1] / FastSqrt(SQR(vProf[iPos]) + SQR(vProf[iPos+1])));
//	dOffset = (M_PI_2 - dPhase) * vProf.size() / (M_2PI * (iPos>>1));
	FindMinMaxX(vPeaksIn, dMinX, dMaxX, &peak_min);
	if( dMinX == dMaxX )
	{
		return;
	}
	// the threshold
	dThresh = dDist * 0.05;
	if( dThresh < 1.0 )
	{
		dThresh = 1.0;
	}
	if( NULL != s_pLog )
	{
		fprintf(s_pLog, "------------------------\n");
		fprintf(s_pLog, "Closest peaks on the selected row are spaced "
			"with dist = %.2f with maximum at %.2f\n\n", dDist, dDistK);
	}
//	ax = ay = 0.0;
	VectorD vX, vY;
	for(pit = vPeaksIn.begin(); pit != vPeaksIn.end(); ++pit)
	{
		dPos = pit->_x - dMinX;
		dCnt = (dPos - dDistK) / dDist;
		nCnt = (int)::floor(dCnt + 0.5);
		if( FastAbs((dPos - dDistK) - nCnt*dDist) < dThresh )
		{
			vPeaksOut.push_back(*pit);
			vX.push_back(pit->x);
			vY.push_back(pit->y);
			if( NULL != s_pLog )
			{
				fprintf(s_pLog, "%10.2f\t%10.2f\t%10.2f\n", pit->x, pit->y, pit->_x);
			}
		}
	}
	CPolynomFit pfit;
	CVector res;
	if( TRUE == pfit.Solve(vX.begin(), vY.begin(), vX.size(), 2, res) )
	{
		dAngle = FastAtan(res[1]);
		vOrigin.x = peak_min.x + dDistK * FastCos(dAngle);
		vOrigin.y = peak_min.y + dDistK * FastSin(dAngle);
	}
//	if( ax != 0 )
//	{
//		dMassCentre = ay / ax;
//		dCnt = dMassCentre / dDist;
//		nOrigin = (int)::floor(dCnt + 0.5);
//		if( NULL != s_pLog )
//		{
//			fprintf(s_pLog, "\nAccepted K-origin = No.%d reflection\n", nOrigin);
//		}
//	}
//	else if( NULL != s_pLog )
//	{
//		fprintf(s_pLog, "\nNO ORIGIN WAS ACCEPTED!\n");
//		nOrigin = -1;
//	}
}

static void GetClosestRefls(PeakVec &vPeaksIn, PeakVec &vPeaksOut,
							double &dAngle, double &dDistH, double &dDistK,
							CVector2d &vCentre)
{
	double dMinX, dMaxX, dPos, dCnt, dThresh, ax, ay, dMassCentre;
	double dDist, dPhase, dOffset;
	PeakVecIt pit;
	PeakVec vRowPeaks;
	VectorD vProf, vSpectr;
	int nCnt, iPos;
	PEAK peak_min;

	DoRotation(vPeaksIn, dAngle);
	MakePofile(vPeaksIn, vProf);
	// perform filtered FFT to find the maximum
	FilteredFft(vProf, dDistH, dDist);
	if( NULL != s_pLog )
	{
		fprintf(s_pLog, "------------------------\n");
		fprintf(s_pLog, "Closest peaks to the direction = %.2fdeg and dist = %.2f, spacing = %.2f\n\n",
			RAD2DEG(dAngle), dDist, dDistH);
	}
	// find min-max
	FindMinMaxX(vPeaksIn, dMinX, dMaxX, &peak_min);
	if( dMinX == dMaxX )
	{
		return;
	}
	// the threshold
	dThresh = dDistH * 0.04;
	if( dThresh < 1.0 )
	{
		dThresh = 1.0;
	}
	// find peaks closest to the direction lines spaced on the distance dDistH from the main direction
	for(pit = vPeaksIn.begin(); pit != vPeaksIn.end(); ++pit)
	{
		dPos = pit->_x - dMinX;
		dCnt = (dPos - dDist) / dDistH;
		nCnt = (int)::floor(dCnt + 0.5);
		if( FastAbs((dPos - dDist) - nCnt*dDistH) < dThresh )
		{
			vPeaksOut.push_back(*pit);
			if( NULL != s_pLog )
			{
				fprintf(s_pLog, "%10.2f\t%10.2f\n", pit->x, pit->y);
			}
		}
		// reflections closest to the row
		if( FastAbs(dPos - dDist) < dThresh )
		{
			vRowPeaks.push_back(*pit);
		}
	}
	FindRowOrigin(vRowPeaks, dAngle, dDistK, vCentre);
	dAngle = M_PI_2 - dAngle;
	if( NULL != s_pLog )
	{
		fprintf(s_pLog, "\nAccepted origin = %.2f, %.2f\n", vCentre.x, vCentre.y);
	}
//	ft1CalcReal(vProf.begin(), vProf.size());
//	PowerSpectrum(vProf, vSpectr);
//	dDist = -1;
//	dPos = AnalyzeSpectrum(vProf, vSpectr);
//	if( dPos <= 0.0 )
//	{
//		return;
//	}
//	iPos = 2*(int)::floor(dPos + 0.5);
//	// the real spacing
//	dDist = vProf.size() / dPos;
//	dPhase = FastAcos(vProf[iPos+1] / FastSqrt(SQR(vProf[iPos]) + SQR(vProf[iPos+1])));
//	dOffset = (M_PI_2 - dPhase) * vProf.size() / (M_2PI * (iPos>>1));
//
//	// find min-max
//	FindMinMaxX(vPeaksIn, dMinX, dMaxX);
//	if( dMinX == dMaxX )
//	{
//		return;
//	}
//
//	dThresh = dDistH * 0.05;
//	if( dThresh < 1.0 )
//	{
//		dThresh = 1.0;
//	}
//
//	if( NULL != s_pLog )
//	{
//		fprintf(s_pLog, "------------------------\n");
//		fprintf(s_pLog, "Closest peaks to the direction = %.2fdeg and dist = %.2f\n\n",
//			RAD2DEG(dAngle), dDistH);
//	}
//	ax = ay = 0.0;
//	for(pit = vPeaksIn.begin(); pit != vPeaksIn.end(); ++pit)
//	{
//		dPos = pit->_x - dMinX;
//		dCnt = dPos / dDistH;
//		nCnt = (int)::floor(dCnt + 0.5);
//		if( FastAbs(dPos - nCnt*dDistH) < dThresh )
//		{
//			vPeaksOut.push_back(*pit);
//			if( NULL != s_pLog )
//			{
//				fprintf(s_pLog, "%10.2f\t%10.2f\n", pit->x, pit->y);
//			}
//			ax += pit->A;
//			ay += pit->A*nCnt*dDistH;
//		}
//	}
//	if( ax != 0 )
//	{
//		dMassCentre = ay / ax;
//		dCnt = dMassCentre / dDistH;
//		nOriginH = (int)::floor(dCnt + 0.5);
//		if( NULL != s_pLog )
//		{
//			fprintf(s_pLog, "\nAccepted H-origin = %d row or reflections\n", nOriginH);
//		}
//		for(pit = vPeaksOut.begin(); pit != vPeaksOut.end(); ++pit)
//		{
//			dPos = pit->_x - dMinX;
//			dCnt = dPos / dDistH;
//			nCnt = (int)::floor(dCnt + 0.5);
//			if( nOriginH == nCnt )
//			{
//				vRowPeaks.push_back(*pit);
//			}
//		}
//		FindRowOrigin(vRowPeaks, dAngle, dDistK, nOriginK);
//	}
//	else if( NULL != s_pLog )
//	{
//		fprintf(s_pLog, "\nNO ORIGIN WAS ACCEPTED!\n");
//		nOriginH = -1;
//		nOriginK = -1;
//	}
}
*/
