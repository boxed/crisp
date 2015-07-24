// FftIndex.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"

#include "commondef.h"
#include "objects.h"
#include "ft32util.h"
#include "eld.h"
#include "common.h"

#ifdef max
#undef max
#endif

using namespace std;

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

typedef std::vector<float>	VectorF;
typedef VectorF::iterator	VectorFit;

extern char TMP[65536];

static const int nLevels = 20;

typedef std::pair<double, LPEREFL> ReflPair;

struct sort_Chi2 : std::binary_function<ReflPair, ReflPair, bool>
{
	bool operator()(const ReflPair &p1, const ReflPair &p2) const
	{
		return p1.first < p2.first;
	}
};

struct sort_by_chi2 : std::binary_function<bool, EREFL, EREFL>
{
	bool operator () (const EREFL &p1, const EREFL &p2) const
	{
		return p1.dummy[0] < p2.dummy[0];
	}
};

struct sort_by_ampl : std::binary_function<bool, EREFL, EREFL>
{
	bool operator () (const EREFL &p1, const EREFL &p2) const
	{
		return p1.IntegrA < p2.IntegrA;
	}
};

struct sort_by_oversat : std::binary_function<bool, EREFL, EREFL>
{
	bool operator () (const EREFL &p1, const EREFL &p2) const
	{
		return p1.num_over < p2.num_over;
	}
};

/****************************************************************************/
// added by Peter on 20 maj 2003
//static int _debug = 0;
class SumCircle
{
public:
	typedef std::vector<CVector3d> Vec;

	SumCircle(LPVOID img, PIXEL_FORMAT nPix, int neg, size_t w, size_t h, double dRad,
		LPBYTE pMask, size_t mw, size_t mh, double nOversat)
	{
		_pImg = img;
		_w = w;
		_h = h;
		_rad = dRad;
		_pMask = pMask;
		_mw = mw;
		_mh = mh;
		_pix = nPix;
		_neg = neg;
		_Oversat = nOversat;

		// circle length around the reflection
		_clen = M_2PI * _rad;
		_nlen = (size_t)floor(_clen + 0.5);
		// angle step
		_step = M_2PI / (double)_nlen;
		// array of points
		_Pts.resize(_nlen);
		// peak minus background
		_peak.resize(_mw * _mh, 0.0);
	}
	virtual ~SumCircle()
	{
		_Pts.clear();
	}

	template <typename _Tx>
	double sum_along(_Tx *unused, double xc0, double yc0, double &noise)
	{
		double sum, angle, x, y, dSin, dCos;
		int i, nx, ny;
		_Tx /**ptr, */*img = (_Tx*)_pImg, _val;
		Vec::iterator it;

		sum = 0.0;
		noise = 0.0;
		// sum all pixels along the circle
		for(sum = angle = 0.0, i = 0, it = _Pts.begin(); i < _nlen; i++, angle += _step, ++it)
		{
			FastSinCos(angle, dSin, dCos);
			x = xc0 + _rad * dCos;
			y = yc0 + _rad * dSin;
			nx = (int)floor(x + 0.5);
			if( nx < 0 ) { nx = 0; }
			if( nx >= _w ) { nx = _w - 1; }
			ny = (int)floor(y + 0.5);
			if( ny < 0 ) { ny = 0; }
			if( ny >= _h ) { ny = _h - 1; }
			it->x = nx;
			it->y = ny;
//			ptr = img + nx + ny * _w;
			_val = *(img + nx + ny * _w);
			sum += (it->z = Negate((_Tx)_val, (int)_neg));
		}
		// get the noise level of the background
		if( _nlen > 1 )
		{
			// average
			sum /= (double)_nlen;
			for(i = 0, it = _Pts.begin(); i < _nlen/* - 1*/; i++, ++it)
			{
				noise += SQR(it->z - sum);
//				noise += FastAbs(it->z - (it+1)->z);
			}
//			noise /= (_nlen - 1);
			noise = FastSqrt(noise / (_nlen - 1));
		}
		return sum;
	}
	template <typename _Tx>
	double above_level(_Tx *unused, double xc0, double yc0, double dLevel, long &nOverSat/*, _Tx &unused*/)
	{
		int i, nx, ny, cnt, num;
		_Tx *ptr, *img = (_Tx*)_pImg, val;
		LPBYTE pMask = _pMask;
		nOverSat = 0;

		nx = floor(xc0 + 0.5) - _rad;
		ny = floor(yc0 + 0.5) - _rad;
		if( nx < 0 )
			nx = 0;
		if( ny < 0 )
			ny = 0;
		ptr = img + _w * ny + nx;
		num = 0;
		// sum all pixels
		for(i = 0; i < _mh; i++)
		{
			if( ny + i >= _h )
				break;
			for(cnt = 0; cnt < _mw; cnt++)
			{
				if( nx + cnt >= _w )
					break;
				if( 0 != pMask[cnt] )
				{
					val = Negate(ptr[cnt], _neg);
					if( val >= dLevel )	num++;
					if( val >= _Oversat )	nOverSat++;
				}
			}
			pMask += _mw;
			ptr += _w;
		}
		return num;
	}
	template <typename _Tx>
	double pixels_level(_Tx *unused, double xc0, double yc0, double dBkg, VectorF &vTable, int &num/*, _Tx &unused*/)
	{
		int i, nx, ny, cnt;
		_Tx *ptr, *img = (_Tx*)_pImg;
		float val;
		LPBYTE pMask = _pMask;

		nx = floor(xc0 + 0.5) - _rad;
		ny = floor(yc0 + 0.5) - _rad;
		if( nx < 0 )
			nx = 0;
		if( ny < 0 )
			ny = 0;
		ptr = img + _w * ny + nx;
		num = 0;
		// get all pixels
		for(i = 0; i < _mh; i++)
		{
			if( ny + i >= _h )
				break;
			for(cnt = 0; cnt < _mw; cnt++)
			{
				if( nx + cnt >= _w )
					break;
				if( pMask[cnt] != 0 )
				{
					val = float(Negate(ptr[cnt], _neg)) - dBkg;
					if( val > 0.0 )
					{
						vTable[num++] = val;
					}
					else
					{
						vTable[num++] = 0;
					}
				}
			}
			pMask += _mw;
			ptr += _w;
		}
		sort(vTable.begin(), vTable.begin() + num);
		return num;
	}
	template <typename _Tx>
	bool GetMaximum3x3(_Tx *unused, double xc0, double yc0, _Tx &max_val)
	{
		int i, nx, ny, cnt;
		_Tx val, *ptr, *img = (_Tx*)_pImg;
		LPBYTE pMask = _pMask;

		nx = (int)floor(xc0 + 0.5);
		ny = (int)floor(yc0 + 0.5);
		if( nx < 0 )
			nx = 0;
		if( ny < 0 )
			ny = 0;
		ptr = img + _w * ny + nx;
		// look for the max value
		max_val = 0.0;
		for(i = -1; i <= 1; i++)
		{
			if( ny+i >= _h )
				break;
			for(cnt = -1; cnt <= 1; cnt++)
			{
				if( nx+cnt >= _w )
					break;
				ptr = img + _w * (ny+i) + (nx+cnt);
				val = Negate(*ptr, _neg);
				if( max_val < val )
					max_val = val;
			}
			pMask += _mw;
		}
		return true;
	}
	template <typename _Tx>
	bool GetMaximum(_Tx *unused, double xc0, double yc0, _Tx &max_val)
	{
		double sum = 0.0;
		int i, nx, ny, cnt;
		_Tx val, *ptr, *img = (_Tx*)_pImg;
		LPBYTE pMask = _pMask;
		
		max_val = -numeric_limits<_Tx>::max();
		nx = floor(xc0 + 0.5) - _rad;
		ny = floor(yc0 + 0.5) - _rad;
		if( nx < 0 )
			nx = 0;
		if( ny < 0 )
			ny = 0;
		ptr = img + _w * ny + nx;
		// find the maximum inside the circle
		for(i = 0; i < _mh; i++)
		{
			if( ny + i >= _h )
				break;
			for(cnt = 0; cnt < _mw; cnt++)
			{
				if( nx + cnt >= _w )
					break;
				if( pMask[cnt] != 0 )
				{
					val = Negate(ptr[cnt], _neg);
					if( val > max_val) max_val = val;
				}
			}
			pMask += _mw;
			ptr += _w;
		}
		return true;
	}
	template <typename _Tx>
	double sum_inside(_Tx *unused, double xc0, double yc0, int &num, _Tx &nMax)
	{
		double sum = 0.0;
		int i, nx, ny, cnt;
		_Tx val, *ptr, *img = (_Tx*)_pImg;
		LPBYTE pMask = _pMask;

		nMax = -numeric_limits<_Tx>::max();
		nx = floor(xc0 + 0.5) - _rad;
		ny = floor(yc0 + 0.5) - _rad;
		if( nx < 0 )
			nx = 0;
		if( ny < 0 )
			ny = 0;
		ptr = img + _w * ny + nx;
		num = 0;
		// sum all pixels
		for(i = 0; i < _mh; i++)
		{
			if( ny + i >= _h )
				break;
			for(cnt = 0; cnt < _mw; cnt++)
			{
				if( nx + cnt >= _w )
					break;
				if( pMask[cnt] != 0 )
				{
					num++;
					val = Negate(ptr[cnt], _neg);
					sum += val;
					if( val > nMax) nMax = val;
				}
			}
			pMask += _mw;
			ptr += _w;
		}
		//	sum /= double(num);
		return sum;
	}
/*
	template <typename _Tx>
	bool CalculateBandSum(LPEREFL lpR, int nClip, double dBkg, double &dSum, _Tx &nMax)
	{
		double dVal, dStep;
		int i, nx, ny, cnt;
		_Tx *ptr, *img = (_Tx*)_pImg;
		LPBYTE pMask = _pMask;

		dStep = (nClip * lpR->Amp_bkg) / (double)nLevels;
		dSum = 0.0;
		nMax = -numeric_limits<_Tx>::max();
		nx = ::floor(lpR->xc0 + 0.5) - _rad;
		ny = ::floor(lpR->yc0 + 0.5) - _rad;
		if( nx < 0 )
			nx = 0;
		if( ny < 0 )
			ny = 0;
		ptr = img + _w * ny + nx;
//		num = 0;
		// sum all pixels
		for(i = 0; i < _mh; i++)
		{
			if( ny + i >= _h )
				break;
			for(cnt = 0; cnt < _mw; cnt++)
			{
				if( nx + cnt >= _w )
					break;
				dVal = (ptr[cnt] ^ _neg) - dBkg;
				if( (pMask[cnt] != 0) && (dVal <= dStep) && (dVal > 0.0) )
				{
//					num++;
					dSum += dVal;
				}
			}
			pMask += _mw;
			ptr += _w;
		}
		return true;
	}
	bool FitBkgPlane(CVector3d &norm, double &dist)
	{
		Vec::iterator it;
		CMatrix3x3 mS, mSinv;
		CVector3d v(0.0);

		for(it = _Pts.begin(); it != _Pts.end(); ++it)
		{
			mS.d[0][0] += SQR(it->x);
			mS.d[0][1] += it->x*it->y;
			mS.d[0][2] += it->x*it->z;

			mS.d[1][0] += it->x*it->y;
			mS.d[1][1] += SQR(it->y);
			mS.d[1][2] += it->y*it->z;

			mS.d[2][0] += it->x*it->z;
			mS.d[2][1] += it->y*it->z;
			mS.d[2][2] += SQR(it->z);

			v += *it;
		}
		if( true == mS.Invert(mSinv) )
		{
			norm = mSinv * v;
			_norm = (norm = Normalize(norm));
			_dist = (dist = -(norm & v) / (double)_Pts.size());
			return true;
		}
		return false;
	}
*/
	template <typename _Tx>
		double Get3x3Bkg(_Tx *unused, double xc0, double yc0, double &stdev/*, _Tx &unused*/)
	{
		_Tx *ptr, *img = (_Tx*)_pImg;
		double aVals[9], dAvg, Bkg;
		int h[4] = {1, 1, -1, -1};
		int k[4] = {1, -1, -1, 1};
		int nCnt, i, nx, ny;

		stdev = 0.0;
		Bkg = 0.0;
		nCnt = 0;
		for(i = 0; i < 4; i++)
		{
			nx = (int)floor(xc0 + 0.5*(h[i] * h_dir.x + k[i] * k_dir.x) + 0.5);
			if( (nx < 1) || (nx >= _w-1) )
				continue;
			ny = (int)floor(yc0 + 0.5*(h[i] * h_dir.y + k[i] * k_dir.y) + 0.5);
			if( (ny < 1) || (ny >= _h-1) )
				continue;
			ptr = img + _w * (ny - 1) + (nx - 1);
			aVals[0] = Negate(ptr[0], _neg);
			aVals[1] = Negate(ptr[1], _neg);
			aVals[2] = Negate(ptr[2], _neg);
//			dSum[i] = Negate(ptr[0], _neg) + Negate(ptr[1], _neg) + Negate(ptr[2], _neg);
			ptr += _w;
			aVals[3] = Negate(ptr[0], _neg);
			aVals[4] = Negate(ptr[1], _neg);
			aVals[5] = Negate(ptr[2], _neg);
//			dSum[i] += Negate(ptr[0], _neg) + Negate(ptr[1], _neg) + Negate(ptr[2], _neg);
			ptr += _w;
			aVals[6] = Negate(ptr[0], _neg);
			aVals[7] = Negate(ptr[1], _neg);
			aVals[8] = Negate(ptr[2], _neg);
//			dSum[i] += Negate(ptr[0], _neg) + Negate(ptr[1], _neg) + Negate(ptr[2], _neg);
//			dSum[i] *= 1.0 / 9.0;
			dAvg = (aVals[0] + aVals[1] + aVals[2] + aVals[3] + aVals[4] + aVals[5] +
				aVals[6] + aVals[7] + aVals[8]) / 9.0;
			// calculate the stdev
			stdev += SQR(dAvg - aVals[0]) + SQR(dAvg - aVals[1]) + SQR(dAvg - aVals[2]) +
				SQR(dAvg - aVals[3]) + SQR(dAvg - aVals[4]) + SQR(dAvg - aVals[5]) +
				SQR(dAvg - aVals[6]) + SQR(dAvg - aVals[7]) + SQR(dAvg - aVals[8]);
			nCnt++;
			Bkg += dAvg;
		}
		stdev = FastSqrt(stdev / (9 * nCnt - 1));
		if( nCnt > 0 )
		{
			Bkg /= nCnt;
		}
		return Bkg;
	}
/*
	template <typename _Tx>
	bool ProcessSingleRefl(double xc0, double yc0, double Bkg,
		ELD::Histo &histo, double &max_val, _Tx &unused)
	{
		double step, cur_lev;
		int i, nx, ny, cnt, iLev, num;
		_Tx val, *ptr, *img = (_Tx*)_pImg;
		LPBYTE pMask = _pMask;

		nx = (int)floor(xc0 + 0.5);
		ny = (int)floor(yc0 + 0.5);
		if( nx < 0 )
			nx = 0;
		if( ny < 0 )
			ny = 0;
		ptr = img + _w * ny + nx;
		// look for the max value
		max_val = 0.0;
		for(i = -1; i <= 1; i++)
		{
			if( ny+i >= _h )
				break;
			for(cnt = -1; cnt <= 1; cnt++)
			{
				if( nx+cnt >= _w )
					break;
				ptr = img + _w * (ny+i) + (nx+cnt);
				val = Negate(*ptr, _neg);
//				if( pMask[cnt] != 0 )
				{
					if( max_val < val )
						max_val = val;
				}
			}
			pMask += _mw;
//			ptr += w;
		}
		nx = (int)floor(xc0 + 0.5) - _rad;
		ny = (int)floor(yc0 + 0.5) - _rad;
		if( nx < 0 )
			nx = 0;
		if( ny < 0 )
			ny = 0;
		// split into N regions
		//	max_val = 255.0;
		if( max_val <= Bkg )
			return false;
		step = (max_val - Bkg) / nLevels;
		cur_lev = Bkg;
		histo.clear();
		histo.reserve(nLevels);
		for(iLev = 0; iLev < nLevels; iLev++, cur_lev += step)
		{
			num = 0;
			ptr = img + _w * ny + nx;
			pMask = _pMask;
			for(i = 0; i < _mh; i++)
			{
				if( ny+i >= _h )
					break;
				for(cnt = 0; cnt < _mw; cnt++)
				{
					if( nx+cnt >= _w )
						break;
					if( pMask[cnt] != 0 )
					{
						if( Negate(ptr[cnt], _neg) >= cur_lev )
							num++;
					}
				}
				pMask += _mw;
				ptr += _w;
			}
			histo.push_back(num);
		}
		return true;
	}
*/
public:
	// image
	LPVOID _pImg;
	size_t _w;
	size_t _h;
	PIXEL_FORMAT _pix;
	int _neg;
	// mask
	LPBYTE _pMask;
	size_t _mw;
	size_t _mh;

	double _Oversat;

	size_t _nlen;
	double _rad;
	double _clen;
	double _step;
	Vec _Pts;

	CVector2d h_dir;
	CVector2d k_dir;

	CVector3d _norm;
	double _dist;

	VectorD _peak;
};
/*
class NLExp : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na)
	{
		// a * exp(-b * x^d) + c
		return a[2] + a[0] * FastExp(-a[1] * pow(x, a[3]));
	}
	virtual int GetNumParams(int nNumProfiles) { return 4; }
	virtual LPCTSTR GetName() { return "NLExp"; }
	virtual UINT GetType() { return TYPE_1D | TYPE_MULTIPROFILE; }

} NL_Exp;

static double CalcHisto(double x, double dA, double dB, double dC, double dD)
{
	return dA * FastExp(-dB * pow(x, dD)) + dC;
}

static double CalcF(double *pdX, double *pdY, int nSize,
					double dA, double dB, double dC, double dD)
{
	double sum = 0.0;
	for(int n = 0; n < nSize; n++)
	{
		sum += SQR(pdY[n] - (dA * FastExp(-dB * pow(pdX[n], dD)) + dC));
	}
	return sum;
}

BOOL ELD::FitHisto(DWVector &vPlot, int nColLen, VectorD &vX, VectorD &vY, VectorD &vdParams)
{
	LPDWORD ptr = vPlot.begin();
	DWORD val;
//	double as, ax;
	int i, j;

	vX.resize(nLevels);
	vY.resize(nLevels);
	// find the mass-center of each column
	for(i = 0; i < nLevels; i++)
	{
		int as = 0;
		int ax = ptr[i];
		for(j = 0; j < nColLen; j++)
		{
			val = ptr[j*nLevels + i];
			if( val > ax )
			{
				ax = val;
				as = j;
			}
		}
		if( (as > 0) && (as < nLevels-1) )
		{
			double am;
			am  = ptr[(as-1)*nLevels + i]*(as-1);
			am += ptr[as*nLevels + i]*as;
			am += ptr[(as+1)*nLevels + i]*(as+1);
			as = am / (ptr[(as-1)*nLevels + i] + ptr[as*nLevels + i] + ptr[(as+1)*nLevels + i]);
		}
		vX[i] = i;
		vY[i] = as;
	}
	CNonlinearFit nlf;
	Vectorb vbParams(4, true);

	double dMin = *min_element(vY.begin(), vY.end());
	double dMax = *max_element(vY.begin(), vY.end());
	double dA = dMax - dMin, dB, dD;

	vdParams.resize(4, 0);
	dB = 1.0;
	dD = 0.5;

	// look for the minimum by gradient search
	int nSteps = 0, x_dir, y_dir;
	double dStep = 0.05;
	double sum, dMinSum;
	dMinSum = numeric_limits<double>::max();
	while( nSteps < 200 )
	{
		x_dir = y_dir = 0;
		for(j = -1; j <= 1; j++)
		{
			sum = CalcF(vX.begin(), vY.begin(), nLevels, dA, dB, dMin, dD+j*dStep);
			if( dMinSum > sum )
			{
				y_dir = j;
				dMinSum = sum;
			}
		}
		for(i = -1; i <= 1; i++)
		{
			sum = CalcF(vX.begin(), vY.begin(), nLevels, dA, dB + i*dStep, dMin, dD);
			if( dMinSum > sum )
			{
				x_dir = i;
				dMinSum = sum;
			}
		}
		if( (0 == x_dir) && (0 == y_dir) )
		{
			dStep *= 0.1;
			if( dStep < 1e-10 )
			{
				break;
			}
		}
		else
		{
			dB += x_dir * dStep;
			dD += y_dir * dStep;
		}
		nSteps++;
	}

	vdParams[0] = dMax - dMin;
	vdParams[1] = dB;
	vdParams[2] = dMin;
	vdParams[3] = dD;
	BOOL bRes = nlf.Fit(vY.begin(), vX.begin(), NULL, vX.size(),
		vdParams.begin(), vbParams.begin(), vdParams.size(), &NL_Exp, false);

	return bRes;
}

class NLHisto : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na)
	{
		// a * exp(-b * (SCALE*x)^d) + c
		return a[2] + a[0] * FastExp(-a[1] * pow(a[4] * x, a[3])) + a[5];
	}
	virtual int GetNumParams(int nNumProfiles) { return 6; }
	virtual LPCTSTR GetName() { return "NLHisto"; }
	virtual UINT GetType() { return TYPE_1D; }

};

BOOL ELD::FitHistogram(VectorD &vX, VectorD &vdParams, Histo &histo, double &dScaleX)
{
	BOOL bRes = FALSE;
	CNonlinearFit nlf;
	NLHisto nl_histo;
	Vectorb vbParams(nl_histo.GetNumParams(1), false);
	VectorD vdLocPrm(nl_histo.GetNumParams(1), 0.0);
	VectorD vY(vX.size(), 0.0);

	copy(histo.begin(), histo.end(), vY.begin());
	dScaleX = 1.0;

	vdLocPrm[0] = vdParams[0];
	vdLocPrm[1] = vdParams[1];
	vdLocPrm[2] = vdParams[2];
	vdLocPrm[3] = vdParams[3];
	vdLocPrm[4] = 1.0;
	vdLocPrm[5] = 0.0;

	vbParams[4] = vbParams[5] = true;

	bRes = nlf.Fit(vY.begin(), vX.begin(), NULL, vX.size(),
		vdLocPrm.begin(), vbParams.begin(), nl_histo.GetNumParams(1), &nl_histo, false);
	dScaleX = vdLocPrm[4];
	if( (dScaleX > 2.0) || (dScaleX <= 0.0) )
	{
		dScaleX = 1.0;
		bRes = FALSE;
	}
	return bRes;
}

BOOL ELD::IsCloseToHisto(const VectorD &vX, const VectorD &vdParams,
						 const Histo &histo, double &dChi2)
{
	dChi2 = 0.0;
	for(int n = 0; n < nLevels; n++)
	{
		dChi2 += SQR(histo[n] - CalcHisto(vX[n], vdParams[0], vdParams[1], vdParams[2], vdParams[3]));
	}
	return TRUE;
}
*/
//BOOL ELD::CalculateBandSum(LPEREFL lpR, int nClip, double &dSum)
//{
//	dSum = 0.0;
//	return TRUE;
//}

static BOOL GetAverageFwhm(EldReflVec &vRefls, int &nMax)
{
	LPEREFL	lpR;
	int i, iMax;
	VectorD vHisto;

	iMax = 0;
	// find the max Index
	for(i = 0, lpR = &*vRefls.begin(); i < vRefls.size(); i++, lpR++)
	{
		if( iMax < lpR->num_2 )
		{
			iMax = lpR->num_2;
		}
	}
	vHisto.resize(iMax + 1, 0);
	for(i = 0, lpR = &*vRefls.begin(); i < vRefls.size(); i++, lpR++)
	{
		if( (lpR->Amp_bkg > 0.0) && (lpR->num_2 >= 0) )
		{
			vHisto[lpR->num_2] += lpR->Amp_bkg;
		}
	}
	double *pMax = &*std::max_element(vHisto.begin(), vHisto.end());
	if( *pMax <= 0.0 )
	{
		nMax = -1;
		return FALSE;
	}
	nMax = pMax - &*vHisto.begin();
//	double dMax_2 = 0.25 * (*dMax);
//	while(  )
//	{
//	}
	return TRUE;
}

BOOL ELD::GetGoodRefls(EldReflVec &vGoodRefls,
					   EldReflVec &vWeakRefls,
					   EldReflVec &vStrNeighbRefls,
					   double &dSigma, double dMaxVal)
{
	LPEREFL	lpR;
	int i, nSigma;

	// get the average FWHM
	if( FALSE == GetAverageFwhm(m_pCurLZ->m_vRefls, nSigma) )
	{
		return FALSE;
	}
	// fill reflections close enough to the average
	for(i = 0, lpR = &*m_pCurLZ->m_vRefls.begin(); i < m_pCurLZ->m_vRefls.size(); i++, lpR++)
	{
		if( lpR->a0 <= dMaxVal )
		{
			if( lpR->Amp_bkg >= 4.0 * lpR->noise )
			{
				if( (lpR->noise * 5 > lpR->circ_noise) &&
					(FastAbs(nSigma - lpR->num_2) <= nSigma * 0.3) )
				{
					vGoodRefls.push_back(*lpR);
				}
				else
				{
					vStrNeighbRefls.push_back(*lpR);
				}
			}
			else
			{
				vWeakRefls.push_back(*lpR);
			}
		}
	}
	dSigma = FastSqrt(nSigma / M_PI);
	return TRUE;
}

static BOOL EstimateTables(EldReflVec &vGoodRefls, VectorF &vTables, int nSigma,
						   SumCircle &circle, IMAGE &image)
{
	LPEREFL	lpR;
	int i, num, max_num;
	float *ptr;
	VectorF vCurTbl(4*SQR(nSigma+1), 0);

	if( true == vTables.empty() )
	{
		return FALSE;
	}
	max_num = 0;
	ptr = &*vTables.begin();
	for(i = 0, lpR = &*vGoodRefls.begin(); i < vGoodRefls.size(); i++, lpR++)
	{
		switch( image.npix )
		{
		case PIX_BYTE:
			{
				BYTE cUnused;
				circle.pixels_level(&cUnused, lpR->xc0, lpR->yc0, lpR->bkg, vCurTbl, num);
				break;
			}
		case PIX_CHAR:
			{
				CHAR cUnused;
				circle.pixels_level(&cUnused, lpR->xc0, lpR->yc0, lpR->bkg, vCurTbl, num);
				break;
			}
		case PIX_WORD:
			{
				WORD wUnused;
				circle.pixels_level(&wUnused, lpR->xc0, lpR->yc0, lpR->bkg, vCurTbl, num);
				break;
			}
		case PIX_SHORT:
			{
				SHORT wUnused;
				circle.pixels_level(&wUnused, lpR->xc0, lpR->yc0, lpR->bkg, vCurTbl, num);
				break;
			}
		case PIX_DWORD:
			{
				DWORD dwUnused;
				circle.pixels_level(&dwUnused, lpR->xc0, lpR->yc0, lpR->bkg, vCurTbl, num);
				break;
			}
		case PIX_LONG:
			{
				LONG dwUnused;
				circle.pixels_level(&dwUnused, lpR->xc0, lpR->yc0, lpR->bkg, vCurTbl, num);
				break;
			}
		case PIX_FLOAT:
			{
				float fUnused;
				circle.pixels_level(&fUnused, lpR->xc0, lpR->yc0, lpR->bkg, vCurTbl, num);
				break;
			}
		case PIX_DOUBLE:
			{
				double dUnused;
				circle.pixels_level(&dUnused, lpR->xc0, lpR->yc0, lpR->bkg, vCurTbl, num);
				break;
			}
		}
		std::copy(vCurTbl.begin(), vCurTbl.begin() + num, ptr);
		lpR->pix_table = ptr;
		lpR->tbl_size = num;
		ptr += num;
		if( num > max_num )
		{
			max_num = num;
		}
	}
	return TRUE;
}

static void MakeRefTable(LPEREFL lpR)
{
	double denom;
	std::reverse(lpR->pix_table, lpR->pix_table + lpR->tbl_size);
	denom = lpR->pix_table[0];
	if( denom == 0.0 )
	{
		lpR->tbl_size = 0;
		lpR->integr_tbl = NULL;
		return;
	}
	// scale by the first pixel
	transform(lpR->pix_table, lpR->pix_table + lpR->tbl_size, lpR->integr_tbl,
		bind2nd(multiplies<float>(), 1.0 / denom));
}

// This function takes all so-called GOOD reflections (not saturated)
// and constructs the pixel reference tables from their pixel tables
// by sorting pixel tables from Max to Min and dividin the whole pixel
// table by Max. The average curve is calculated then.
// The reflections are sorted by Chi^2 (closest to the average comes first).
// Top 1/10 are taken to calculate the final REFERENCE CURVE
// Those 1/10 of reflections which take part in the REFERENCE CURVE calculation
// will be stored in vRefCurveRefls.
BOOL ELD::ProcessGoodRefls(EldReflVec &vGoodRefls, EldReflVec &vRefCurveRefls,
						   VectorF &vTables, VectorF &vIntegrTables,
						   VectorF &vRefCurve)
{
	LPEREFL	lpR;
	int i, j, nMaxSize;
//	double denom;
	float *ptr;

	nMaxSize = 0;
	if( true == vGoodRefls.empty() )
	{
		return FALSE;
	}
	ptr = &*vIntegrTables.begin();
	for(i = 0, lpR = &*vGoodRefls.begin(); i < vGoodRefls.size(); i++, lpR++)
	{
		lpR->integr_tbl = ptr;
		MakeRefTable(lpR);
//		denom = *max_element(lpR->pix_table, lpR->pix_table + lpR->tbl_size);
//		if( denom == 0.0 )
//		{
//			lpR->integr_tbl = NULL;
//			continue;
//		}
//		reverse(lpR->pix_table, lpR->pix_table + lpR->tbl_size);
//		denom = 1.0 / denom;
//		for(j = 0; j < lpR->tbl_size - 1; j++)
//		{
//			lpR->integr_tbl[j] = lpR->pix_table[j] * denom;
//		}
//		lpR->integr_tbl[lpR->tbl_size - 1] = 0.0f;
		ptr += lpR->tbl_size;
		nMaxSize = __max(nMaxSize, lpR->tbl_size);
	}
	// calculate the average curve
	VectorF vAvgCurve(nMaxSize, 0.0f);
	float inv_mul = 1.0 / vGoodRefls.size();
	ptr = &*vAvgCurve.begin();
	for(i = 0, lpR = &*vGoodRefls.begin(); i < vGoodRefls.size(); i++, lpR++)
	{
		for(j = 0; j < lpR->tbl_size; j++)
		{
			ptr[j] += lpR->integr_tbl[j] * inv_mul;
		}
	}
	// get the Chi^2 for each reflection
	std::vector<ReflPair> vChiTbl;
	double dChi2;
	ptr = &*vAvgCurve.begin();
	for(i = 0, lpR = &*vGoodRefls.begin(); i < vGoodRefls.size(); i++, lpR++)
	{
		dChi2 = 0.0;
		for(j = 0; j < lpR->tbl_size; j++)
		{
			dChi2 += SQR(lpR->integr_tbl[j] - ptr[j]);
		}
		dChi2 /= nMaxSize;
		vChiTbl.push_back(ReflPair(dChi2, lpR));
		lpR->dummy[0] = dChi2*1e6;
	}
	// sort by Chi^2
	if( false == vChiTbl.empty() )
	{
		sort(vChiTbl.begin(), vChiTbl.end(), sort_Chi2());
		// take first 1/5 of all reflections, but min 10
		int nTotal = vChiTbl.size() / 5;
		if( nTotal < 10 )
		{
			nTotal = (vChiTbl.size() < 10) ? vChiTbl.size() : 10;
		}
		// recalculate the average curve
		ptr = &*vAvgCurve.begin();
		fill_n(vAvgCurve.begin(), vAvgCurve.size(), 0.0f);
		inv_mul = 1.0 / nTotal;
		for(i = 0; i < nTotal; i++)
		{
			lpR = vChiTbl[i].second;
			// save it
			vRefCurveRefls.push_back(*lpR);
			for(j = 0; j < lpR->tbl_size; j++)
			{
				ptr[j] += lpR->integr_tbl[j] * inv_mul;
			}
		}
	}
	vRefCurve = vAvgCurve;
	return TRUE;
}

static void ProcessStrongNeighbours(EldReflVec &vStrNeighbRefls,
									IMAGE &image, VectorF &vRefCurve, double dSigma)
{
	LPEREFL	lpR;
	int i, j, nSize, nRad, nCount, xc, yc;
	VectorF vCurve;
	float val;

	try
	{
		nSize = (int)::floor(dSigma + 1.5);
		if( nSize < 2 ) nSize = 2;
		vCurve.resize(SQR(2*nSize + 1));
		for(i = 0, lpR = &*vStrNeighbRefls.begin(); i < vStrNeighbRefls.size(); i++, ++lpR)
		{
			// the reflections has bad shape
			if( lpR->noise * 5 > lpR->circ_noise )
			{
				continue;
			}
//			try
			{
				nCount = 0;
				xc = (int)::floor(lpR->xc0 + 0.5);
				if( (xc - nSize < 0) || (xc + nSize >= image.w) )
				{
					continue;
				}
				yc = (int)::floor(lpR->yc0 + 0.5) - nSize;
				if( (yc < 0) || (yc + 2*nSize + 1 >= image.h) )
				{
					continue;
				}
				// TODO: PIX_CHAR
				if( PIX_BYTE == image.npix )
				{
					LPBYTE pData = (LPBYTE)image.pData;
					pData = (LPBYTE)(((LPBYTE)pData) + image.stride * yc + xc);
					for(int y = -nSize; y <= nSize; y++)
					{
						for(int x = -nSize; x <= nSize; x++)
						{
							nRad = x*x + y*y;
							if( nRad > nSize*nSize ) continue;
							val = Negate(pData[x], image.neg);
							vCurve[nCount] = val;
							nCount++;
						}
						pData = (LPBYTE)(((LPBYTE)pData) + image.stride);
					}
				}
				else if( PIX_WORD == image.npix )
				{
					LPWORD pData = (LPWORD)image.pData;
					pData = (LPWORD)(((LPBYTE)pData) + image.stride * yc + xc * 2);
					for(int y = -nSize; y <= nSize; y++)
					{
						for(int x = -nSize; x <= nSize; x++)
						{
							nRad = x*x + y*y;
							if( nRad > nSize*nSize ) continue;
							val = Negate(pData[x], image.neg);
							vCurve[nCount] = val;
							nCount++;
						}
						pData = (LPWORD)(((LPBYTE)pData) + image.stride);
					}
				}
				else if( PIX_DWORD == image.npix )
				{
				}
				else if( PIX_FLOAT == image.npix )
				{
				}
				sort(&*vCurve.begin(), &*vCurve.begin() + nCount);
				reverse(vCurve.begin(), vCurve.begin() + nCount);
				double dBkg = 0.0, dSum = 0.0, dMax;
				dMax = vCurve[0];
				VectorFit zero = find_if(vCurve.begin(), vCurve.end(), bind2nd(equal_to<float>(), 0.0));
				int nZero = zero - vCurve.begin();
				if( nZero > 0 )
				{
					nCount = __min(nZero, nCount);
					for(j = 0; j < nCount; j++)
					{
						val = vRefCurve[j];
						dSum += SQR(1.0 - val);
						dBkg += (val * dMax - vCurve[j]) * (val - 1.0);
					}
					if( dSum > 0.0 ) dBkg /= dSum;
					else dBkg = 0.0;
					if( dBkg > 0.0 )
					{
						lpR->bkg = dBkg;
						lpR->Amp_bkg = lpR->a0 - dBkg;
					}
				}
			}
		}
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("ProcessStrongNeighbours() -> failed for h=%d, k=%d: %s"), lpR->h, lpR->k, e.what()));
	}
}

static LPVOID CreateMask(int arad, int &size, int nRemRad,
						 const CVector2d &h_dir, const CVector2d &k_dir,
						 bool bRemoveNeighbours = false);
static double ProcessMaskedRefls(IMAGE &image, EldReflVec &vRefls,
								 double &dvfrom, double &dvto,
								 double &infrom, double &into,
								 SumCircle &circle);

static void DumpMask(LPBYTE pMask, int nMaskSize)
{
// 	for(int j = 0; j < nMaskSize; j++)
// 	{
// 		for(int i = 0; i < nMaskSize; i++)
// 		{
// 			sprintf(TMP, "%4d", pMask[i + j*R4(nMaskSize)]);
// 			::OutputDebugString(TMP);
// 		}
// 		::OutputDebugString("\n");
// 		Sleep(25);
// 	}
}

BOOL ProcessStrongRefls(EldReflVec &vStrongRefls, EldReflVec &vGoodStrongRefls,
						EldReflVec &vBadStrongRefls,
						VectorF &vTables, VectorF &vIntegrTables, VectorF &vRefCurve,
						int irad, IMAGE &image,
						SumCircle &circle, VectorF &vNewRefCurve,
						bool bUseFull = false)
{
	LPEREFL	lpR;
	int i, j;
	double dvfrom, dvto, infrom, into;
	float *ptr;
	VectorF vCurve(vRefCurve.size()), vScales(vRefCurve.size());
	vector<int> vCntCurRef(vRefCurve.size());

	if( true == vRefCurve.empty() )
	{
		return FALSE;
	}
	vNewRefCurve.resize(vRefCurve.size(), 0);
	vGoodStrongRefls.clear();
	vBadStrongRefls.clear();
	try
	{
		ProcessMaskedRefls(image, vStrongRefls, dvfrom, dvto, infrom, into, circle);

		vTables.resize(vStrongRefls.size() * 2*SQR(2*irad+1), 0);
		vIntegrTables.resize(vStrongRefls.size() * 2*SQR(2*irad+1), 0);
		EstimateTables(vStrongRefls, vTables, irad, circle, image);

		ptr = &*vIntegrTables.begin();

		sort(vStrongRefls.begin(), vStrongRefls.end(), sort_by_oversat());

		size_t nLim0_1, nLim0_1_2nd, nScaled, nSize;
		if( true == bUseFull )
		{
			VectorFit lim = find_if(vRefCurve.begin(), vRefCurve.end(), bind2nd(less_equal<float>(), 1e-5));
			nLim0_1 = lim - vRefCurve.begin();
		}
		else
		{
			// find the pixel number where we have 0.1 level on the reference curve
			VectorFit lim = find_if(vRefCurve.begin(), vRefCurve.end(), bind2nd(less_equal<float>(), 0.1));
			nLim0_1 = lim - vRefCurve.begin();
		}

		// split all strong reflections in groups
		// upon the number of pixels which are above the OVERSAT level
		int nGroups = 10;
		int nGroupSize = 1, iGroup, iPeak, nPixels;
		nSize = vStrongRefls.size();
		// the approximate group size
		if( nSize > nGroups )
		{
			nGroupSize = nSize / nGroups;
		}
//		fill_n(vNewRefCurve.begin(), vNewRefCurve.size(), 0);
//		fill_n(vCntCurRef.begin(), vCntCurRef.size(), 0);
		// copy the beginning of the reference curve
		copy(vRefCurve.begin(), vRefCurve.begin() + nLim0_1, vNewRefCurve.begin());
		// process each group
		for(i = iGroup = 0, lpR = &*vStrongRefls.begin(); iGroup < nGroups; iGroup++)
		{
			nScaled = 0;
			// calculate the average ratio of the current peak profile with the reference curve
			// here we use all pixels above the number of oversaturated pixels but >= 0.1
			for(iPeak = 0; iPeak < nGroupSize && i < nSize; iPeak++, lpR++, i++)
			{
				lpR->integr_tbl = NULL;
				// we have too many oversaturated pixels now, so quit
				nPixels = nLim0_1 - lpR->num_over;
				if( lpR->num_over > nLim0_1 )
				{
					vBadStrongRefls.push_back(*lpR);
					continue;
				}
				lpR->integr_tbl = ptr;
				MakeRefTable(lpR);
				nLim0_1_2nd = find_if(lpR->integr_tbl, lpR->integr_tbl + lpR->tbl_size,
					bind2nd(less_equal<float>(), 0.1)) - lpR->integr_tbl;
				// divide the integrated table by the reference curve
				transform(lpR->integr_tbl + lpR->num_over, lpR->integr_tbl + nLim0_1,
					vNewRefCurve.begin() + lpR->num_over, vScales.begin(), divides<float>());
				// find the average
				float fSum = accumulate(vScales.begin(), vScales.begin() + nPixels, 0.0f);
				fSum /= nPixels;
				// set the estimated amplitude as the maximum pixel
				lpR->dEstA = lpR->pix_table[0];
				// if the new curve is longer then the current - extend the current
				if( /*(nLim0_1_2nd > nLim0_1) &&*/ (fSum > 1.0) )
				{
					lpR->dEstA = lpR->pix_table[0] * fSum;
					VectorFit src, dst;
					vector<int>::iterator it;
					fill_n(vScales.begin(), vScales.size(), 0.0f);
					transform(lpR->integr_tbl, lpR->integr_tbl + nLim0_1_2nd,
						vScales.begin(), bind2nd(multiplies<float>(), 1.0 / fSum));
					// how far is it from the reference curve?
					double dChi2 = 0.0;
					src = vNewRefCurve.begin() + lpR->num_over;
					dst = vScales.begin() + lpR->num_over;
					for(j = lpR->num_over; j < nLim0_1_2nd; j++, ++src, ++dst)
					{
						if( *src > 0.0 )
						{
							dChi2 += SQR(*dst / *src);
//							dChi2 += SQR(1.0 - *dst / *src);
						}
					}
					dChi2 /= j;
					sprintf(TMP, "%d\t%d\t%g\n", lpR->h, lpR->k, dChi2);
					OutputDebugString(TMP);
					if( ((false == bUseFull) && (dChi2 >= 0.8) && (dChi2 <= 1.2)) ||
						(true == bUseFull) )
					{
						src = vScales.begin() + lpR->num_over;
						dst = vNewRefCurve.begin() + lpR->num_over;
						it = vCntCurRef.begin() + lpR->num_over;
						for(j = lpR->num_over; j < nLim0_1_2nd; j++, ++src, ++dst, ++it)
						{
							*dst = ((*dst) * (*it) + *src) / (*it + 1);
							(*it)++;
						}
						nScaled++;
						// change the bottom limit
						if( nLim0_1_2nd > nLim0_1 )
						{
							nLim0_1 = nLim0_1_2nd;
						}
						vGoodStrongRefls.push_back(*lpR);
					}
					else
					{
						vBadStrongRefls.push_back(*lpR);
					}
				}
				else
				{
					// not so bad, but just doesn't pass to the reference curve
					vGoodStrongRefls.push_back(*lpR);
				}
				ptr += lpR->tbl_size;
			}
		}
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("ProcessStrongRefl() -> %s"), e.what()));
	}
	return TRUE;
}

void ProcessBadStrongRefls(EldReflVec &vGoodStrongRefls, VectorF &vTables,
						   EldReflVec &vRefCurveRefls,
						   EldReflVec &vBadStrongRefls,
						   VectorF &vRefCurve,
						   IMAGE &image, int irad, double dMaxVal, LPELD pEld)
{
	LPBYTE bits = NULL;
	LPEREFL	lpR;
	int size, new_irad;
	double dvfrom, dvto, infrom, into;
	VectorF vNewRefCurve, vIntegrTables, vTempRefCurve;
	EldReflVec vTempCurveRefls;

	if( true == vGoodStrongRefls.empty() )
	{
		return;
	}
	{
		new_irad = (int)::floor(irad * 1.5 + 0.5) + 1;
		if( NULL == (bits = (LPBYTE)CreateMask(new_irad, size, irad,
			pEld->m_pCurLZ->h_dir, pEld->m_pCurLZ->k_dir, true)) )
		{
			goto error_exit;
		}
		DumpMask(bits, size);
		SumCircle circle(image.pData, image.npix, image.neg, image.w, image.h,
			new_irad, bits, R4(size), size, dMaxVal);
		circle.h_dir = pEld->m_pCurLZ->h_dir;
		circle.k_dir = pEld->m_pCurLZ->k_dir;
		// first process all those reflections which form the REFERENCE CURVE
//		ProcessMaskedRefls(image, vRefCurveRefls, dvfrom, dvto, infrom, into, circle);
//		vTables.resize(vRefCurveRefls.size() * 2*SQR(2*new_irad + 1), 0);
//		vIntegrTables.resize(vRefCurveRefls.size() * 2*SQR(2*new_irad + 1), 0);
//		EstimateTables(vRefCurveRefls, vTables, new_irad, circle, image);
//		pEld->m_ProcessGoodRefls(vRefCurveRefls, vTempCurveRefls, vTables, vIntegrTables, vNewRefCurve);
		// now process all good strong reflections
		ProcessMaskedRefls(image, vGoodStrongRefls, dvfrom, dvto, infrom, into, circle);
		vTables.resize(vGoodStrongRefls.size() * 2*SQR(2*new_irad + 1), 0);
		vIntegrTables.resize(vGoodStrongRefls.size() * 2*SQR(2*new_irad + 1), 0);
		EstimateTables(vGoodStrongRefls, vTables, new_irad, circle, image);

//		ProcessStrongRefls(vGoodStrongRefls, vTempCurveRefls, vTempCurveRefls,
//			vTables, vIntegrTables, vNewRefCurve, new_irad, image, circle, vTempRefCurve);
		int nCurveSize = 0;
		LPBYTE ptr = bits;
		for(int y = 0; y < size; y++)
		{
			for(int x = 0; x < size; x++)
			{
				if( 0 != ptr[x] ) nCurveSize++;
			}
			ptr += R4(size);
		}
		vNewRefCurve.resize(nCurveSize, 0.0f);
//		copy(vRefCurve.begin(), vRefCurve.end(), vNewRefCurve.begin());
		// make an average curve from the good strong reflections
		vector<int> vCntCurRef(nCurveSize);
		VectorF vCurve(nCurveSize), vScales(nCurveSize);
		int i, nMinOver = nCurveSize, nCurOver;
		float *ptbl = &*vIntegrTables.begin();
//		lpR = vGoodStrongRefls.begin();
//		MakeRefTable(lpR);
//		ptbl += lpR->tbl_size;
//		copy(lpR->integr_tbl + lpR->num_over, lpR->integr_tbl + lpR->tbl_size, vNewRefCurve.begin());
//		fill_n(vCntCurRef.begin() + lpR->num_over, vCntCurRef.end(), 1);
		for(i = 0, lpR = &*vGoodStrongRefls.begin(); i < vGoodStrongRefls.size(); ++lpR, i++)
		{
			lpR->integr_tbl = ptbl;
//			MakeRefTable(lpR);
			ptbl += lpR->tbl_size;
			if( nMinOver > lpR->num_over ) nMinOver = lpR->num_over;
			// devide the integrated table by the reference curve
//			transform(lpR->integr_tbl + lpR->num_over, lpR->integr_tbl + lpR->tbl_size,
//				vNewRefCurve.begin() + lpR->num_over, vScales.begin(), divides<float>());
			reverse(lpR->pix_table, lpR->pix_table + lpR->tbl_size);
			transform(lpR->pix_table + lpR->num_over, lpR->pix_table + lpR->tbl_size,
				lpR->integr_tbl + lpR->num_over, bind2nd(divides<float>(), lpR->dEstA));
			transform(lpR->integr_tbl + lpR->num_over, lpR->integr_tbl + lpR->tbl_size,
				vNewRefCurve.begin() + lpR->num_over, vNewRefCurve.begin() + lpR->num_over, plus<float>());
			transform(vCntCurRef.begin() + lpR->num_over, vCntCurRef.end(),
				vCntCurRef.begin() + lpR->num_over, bind2nd(plus<int>(), 1));
		}
		vTables.resize(vBadStrongRefls.size() * 2*SQR(2*new_irad + 1), 0);
		vIntegrTables.resize(vBadStrongRefls.size() * 2*SQR(2*new_irad + 1), 0);
		EstimateTables(vBadStrongRefls, vTables, new_irad, circle, image);

		transform(vNewRefCurve.begin(), vNewRefCurve.end(),
			vCntCurRef.begin(), vNewRefCurve.begin(), divides<float>());
		VectorFit lim = find_if(vNewRefCurve.begin() + nMinOver, vNewRefCurve.end(), bind2nd(less_equal<float>(), 0.01));
		int nPixels, nHighLim = lim - vNewRefCurve.begin();
		ptbl = &*vIntegrTables.begin();
		for(i = 0, lpR = &*vBadStrongRefls.begin(); i < vBadStrongRefls.size(); ++lpR, i++)
		{
			lpR->integr_tbl = NULL;
			nCurOver = __max(nMinOver, lpR->num_over);
			nPixels = nHighLim - nCurOver;
			if( nPixels <= 0 )
			{
				continue;
			}
			lpR->integr_tbl = ptbl;
			ptbl += lpR->tbl_size;
			MakeRefTable(lpR);
			// devide the integrated table by the reference curve
			transform(lpR->integr_tbl + nCurOver, lpR->integr_tbl + nHighLim,
				vNewRefCurve.begin() + nCurOver, vScales.begin(), divides<float>());
			// find the average
			float fSum = accumulate(vScales.begin(), vScales.begin() + nPixels, 0.0f);
			fSum /= nPixels;
			// set the estimated amplitude as the maximum pixel
			if( fSum > 1.0 )
			{
				lpR->dEstA = lpR->pix_table[0] * fSum;
			}
		}
//		ProcessStrongRefls(vBadStrongRefls, vTempCurveRefls, vTempCurveRefls,
//			vTables, vIntegrTables, vNewRefCurve, new_irad, image, circle, vTempRefCurve, true);
//		ProcessStrongRefls(vGoodStrongRefls, vTempCurveRefls, vTempCurveRefls,
//			vTables, vIntegrTables, vNewRefCurve, new_irad, image, circle, vTempRefCurve);
		// copy the first reflection to the reference curve
//		lpR = vGoodStrongRefls.begin();
//		vNewRefCurve.resize(lpR->tbl_size);
	}
error_exit:
	if( NULL != bits )
	{
		delete[] bits;
	}
}

template <typename _Tx>
void ProcessMaskedRefls_t(_Tx *unused, LPEREFL lpR, SumCircle &circle, const double &xc0, const double &yc0, double &area, double &dAmpl, int &num)
{
	_Tx cMaxVal;
	lpR->bkg = circle.Get3x3Bkg(unused, xc0, yc0, lpR->noise);
	lpR->circ_bkg = circle.sum_along(unused, xc0, yc0, lpR->circ_noise);
	area = circle.sum_inside(unused, xc0, yc0, num, cMaxVal);
	// The maximum within 3x3 square depends on the precision of the lattice refinement
	//circle.GetMaximum3x3(unused, xc0, yc0, cMaxVal);
	circle.GetMaximum(unused, xc0, yc0, cMaxVal);
	dAmpl = cMaxVal;
	if( num > 0 )
	{
		//					circle.ProcessSingleRefl(xc0, yc0, bkg, histo, dAmpl, cMaxVal);
		lpR->num_2 = circle.above_level(unused, xc0, yc0, 0.5*(dAmpl + lpR->bkg), lpR->num_over);
		if( lpR->num_2 < 0 )
		{
			lpR->num_2 = 0;
		}
	}
}

static double ProcessMaskedRefls(IMAGE &image, EldReflVec &vRefls,
								 double &dvfrom, double &dvto,
								 double &infrom, double &into,
								 SumCircle &circle)
{
	int		num, i;
	LPEREFL	lpR;
//	ELD::Histo	histo;
	double	meanampl, dAmpl;
	double	xc0, yc0, area;//, bkg, circ_bkg;

	meanampl = 0.0;
	try
	{
		for(i = 0, lpR = &*vRefls.begin(); i < vRefls.size(); i++, lpR++)
		{
			// reflection center
			xc0 = lpR->xc0;
			yc0 = lpR->yc0;
			lpR->tbl_size = 0;
			lpR->pix_table = NULL;
			dAmpl = 0.0;
			lpR->IntegrA = 0.0;
			// sum all points on that circle - it will be a background
			switch( image.npix )
			{
			case PIX_BYTE: { BYTE unused; ProcessMaskedRefls_t<BYTE>(&unused, lpR, circle, xc0, yc0, area, dAmpl, num); break; }
			case PIX_CHAR: { CHAR unused; ProcessMaskedRefls_t<CHAR>(&unused, lpR, circle, xc0, yc0, area, dAmpl, num); break; }
			case PIX_WORD: { WORD unused; ProcessMaskedRefls_t<WORD>(&unused, lpR, circle, xc0, yc0, area, dAmpl, num); break; }
			case PIX_SHORT: { SHORT unused; ProcessMaskedRefls_t<SHORT>(&unused, lpR, circle, xc0, yc0, area, dAmpl, num); break; }
			case PIX_DWORD: { DWORD unused; ProcessMaskedRefls_t<DWORD>(&unused, lpR, circle, xc0, yc0, area, dAmpl, num); break; }
			case PIX_LONG: { LONG unused; ProcessMaskedRefls_t<LONG>(&unused, lpR, circle, xc0, yc0, area, dAmpl, num); break; }
			case PIX_FLOAT: { float unused; ProcessMaskedRefls_t<float>(&unused, lpR, circle, xc0, yc0, area, dAmpl, num); break; }
			case PIX_DOUBLE: { double unused; ProcessMaskedRefls_t<double>(&unused, lpR, circle, xc0, yc0, area, dAmpl, num); break; }
			}
			// memorize all the calculated values
			if( num > 0 )
			{
				lpR->IntegrA = area - lpR->bkg * num;
// 				lpR->IntegrA = area / num - lpR->bkg;
			}
			if( lpR->IntegrA < 0 )
			{
				lpR->IntegrA = 0;
			}
//			lpR->bkg = bkg;
			lpR->dEstA = lpR->Amp_bkg = dAmpl - lpR->bkg;
			lpR->a0 = dAmpl;
			lpR->ae = dAmpl = dAmpl - lpR->bkg;//(area - num*bkg < 0.0) ? 0.0 : area/(float)num - bkg;
			if( dvfrom > lpR->dval )
				dvfrom = lpR->dval;
			if( dvto   < lpR->dval )
				dvto   = lpR->dval;
			if( infrom > lpR->a0 )
				infrom = lpR->a0;
			if( into   < lpR->a0 )
				into   = lpR->a0;
			// 256 is some scale by unknown & strange reason
			if( infrom > dAmpl/256.0)	infrom = dAmpl / 256.0;
			if( into   < dAmpl/256.0)	into   = dAmpl / 256.0;
			meanampl +=  dAmpl;
		}
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("ProcessMaskedRefl() -> %s"), e.what()));
	}
	return meanampl;
}

static LPVOID CreateMask(int arad, int &size, int nRemRad,
						 const CVector2d &h_dir, const CVector2d &k_dir,
						 bool bRemoveNeighbours/* = false*/)
{
	BITMAPINFO	*bb;
	LPBYTE	bipal;
	int		xc, c, orop, length;
	HDC		hDC;
	HBITMAP hBMP, hOldBMP;
	LPVOID	bits = NULL, pvData = NULL;
	HPEN	hp, hp1, op, hp0;
	HBRUSH	hb, ob, hb0;

	bb = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFO) + sizeof(RGBQUAD)*256];

//	size = (int)::floor(arad + arad + 1);
	size = 2*arad + 1;

	length = R4(size) * size;
	bb->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bb->bmiHeader.biPlanes = 1;
	bb->bmiHeader.biBitCount = 8;
	bb->bmiHeader.biCompression = BI_RGB;
	bb->bmiHeader.biClrUsed  = COLORS;
	bb->bmiHeader.biClrImportant = COLORS;
	bb->bmiHeader.biSizeImage = length;
	bb->bmiHeader.biWidth  = size;
	bb->bmiHeader.biHeight = size;
	bipal = (LPBYTE)(bb->bmiColors);
	for(c = 0; c < 256; c++)
	{
		*bipal++ = c;
		*bipal++ = c;
		*bipal++ = c;
		*bipal++ = 0;
	}

	// create the mask for the reflection
	if( NULL == (hDC = CreateCompatibleDC(NULL)) )
		goto errexit;

	if( NULL == (hBMP = CreateDIBSection(hDC, bb, DIB_RGB_COLORS, &bits, NULL, 0)) )
		goto errexit;

	hOldBMP = SelectBitmap( hDC, hBMP );

	hp  = CreatePen(PS_SOLID, 1, RGB(255,255,255));
	hp1 = CreatePen(PS_SOLID, 1, RGB(200,200,200));
	hb  = CreateSolidBrush(RGB(200,200,200));
	hp0 = CreatePen(PS_SOLID, 1, RGB(0,0,0));
	hb0 = CreateSolidBrush(RGB(0,0,0));

	orop = SetROP2( hDC, R2_COPYPEN );
	ob = SelectBrush( hDC, hb );
	op = SelectPen( hDC, hp );

	memset(bits, 0, length);

	Ellipse( hDC, 0, 0, size, size );

	xc = size >> 1;
	if( true == bRemoveNeighbours )
	{
		int _x, _y;
		SelectBrush( hDC, hb0 );
		SelectPen( hDC, hp0 );
		for(int k = -1; k <= 1; k++)
		{
			for(int h = -1; h <= 1; h++)
			{
				if( (0 == h) && (0 == k) ) continue;
				_x = xc + (h_dir.x * h + k_dir.x * k);
				_y = xc - (h_dir.y * h + k_dir.y * k);
				Ellipse( hDC, _x - nRemRad, _y - nRemRad, _x + nRemRad, _y + nRemRad );
			}
		}
	}
	SelectBrush( hDC, hb );
	SelectPen( hDC, hp1 );
	Ellipse( hDC, xc - nRemRad, xc - nRemRad, xc + nRemRad, xc + nRemRad );

	SetROP2( hDC, orop );
	SelectObject( hDC, op );
	SelectObject( hDC, ob );
	DeleteObject( hb );
	DeleteObject( hp );
	DeleteObject( hb0 );
	DeleteObject( hp0 );

	// copy the mask
	if( NULL == (pvData = new BYTE[length]) )
	{
		goto errexit;
	}
	memcpy(pvData, bits, length);

errexit:
	// clean up
	SelectBitmap( hDC, hOldBMP );
	DeleteDC( hDC );
	DeleteObject( hBMP );
	delete[] bb;

	return pvData;
}

BOOL ELD::IntensitySummation()
{
	BOOL	bRet = FALSE;
	double	mind, rad;
	int		nPass, size, irad, nrad;
	LPVOID	bits = NULL;
	double dMaxVal;

	if( NULL == m_pObj )
	{
		return FALSE;
	}
	// The current Laue Zone pointer
	if( NULL == m_pCurLZ )
	{
		return FALSE;
	}
	mind = __min(m_pCurLZ->a_length, m_pCurLZ->b_length);
	rad = 0.5 * mind;
	nrad = (int)::floor(rad + 0.5);
	for(nPass = 1; nPass < 2; nPass++)
	{
		try
		{
			if( 1 == nPass )
			{
				irad = nrad + 1;
				// create a mask
				if( NULL == (bits = CreateMask(irad, size, 0, m_pCurLZ->h_dir, m_pCurLZ->k_dir, true)) )
				{
					goto errexit;
				}
			}
			else
			{
				irad = (int)::floor(1.5 * nrad + 0.5) + 1;
				// create a mask
				if( NULL == (bits = CreateMask(irad, size, nrad + 1, m_pCurLZ->h_dir, m_pCurLZ->k_dir, true)) )
				{
					goto errexit;
				}
			}
			DumpMask((LPBYTE)bits, size);
			// put the value of maximum value for a good reflection
			dMaxVal = 0.6 * m_n10Histo;//m_pObj->imaxa;
			SumCircle circle(m_image.pData, m_image.npix, m_image.neg, m_image.w, m_image.h, irad, (LPBYTE)bits, R4(size), size, dMaxVal);
			circle.h_dir = m_pCurLZ->h_dir;
			circle.k_dir = m_pCurLZ->k_dir;
			// some statistics
			m_bCentOnly = FALSE;
			m_into = -(m_infrom =  numeric_limits<double>::max());
			m_dvto = -(m_dvfrom =  numeric_limits<double>::max());
			ProcessMaskedRefls(m_image, m_pCurLZ->m_vRefls, m_dvfrom, m_dvto, m_infrom, m_into, circle);
		}
		catch( std::exception& e )
		{
			Trace(_FMT(_T("IntensitySummation() -> %s"), e.what()));
		}
errexit:
		if( NULL != bits )
		{
			delete[] bits;
		}
		bRet = TRUE;
	}
	return bRet;
}

BOOL ELD::InternalCalibration()
{
	double	mind, rad;
	int		i, nPass, size, irad, nrad;//, nPlotL;
	LPEREFL	lpR;
	LPVOID	bits = NULL;
	BOOL	bRet = FALSE;
	FILE	*pOutF = NULL;
	int nSigma;
	double dSigma;
	double dMaxVal;

	if( NULL == m_pObj )
	{
		return FALSE;
	}
	// The current Laue Zone pointer
	if( NULL == m_pCurLZ )
	{
		return FALSE;
	}
	// the integration area
//	nPlotL = SQR(rma)*M_PI;
//	vPlot.resize(nLevels * nPlotL, 0);
//	vPlot2.resize(nLevels * nPlotL, 0);
	// allocate the space for the histograms
//	vHistos.resize(nLevels * m_pCurLZ->m_vRefls.size(), 0);
	// this vector shows how good the reflections are:
	// true if we used itfor the Average Histo
	// false if it was oversaturated
//	vbPeaks.resize(m_pCurLZ->m_vRefls.size(), false);

	mind = __min(m_pCurLZ->a_length, m_pCurLZ->b_length);
	rad = 0.5 * mind;
	nrad = (int)::floor(rad + 0.5);
	for(nPass = 1; nPass < 2; nPass++)
	{
		try
		{
			EldReflVec vGoodRefls, vWeakRefls, vStrongRefls, vStrNeighbRefls;
			EldReflVec vRefCurveRefls, vGoodStrongRefls, vBadStrongRefls;
			VectorF vTables, vIntegrTables, vRefCurve, vNewRefCurve;
			VectorF vTablesStrong, vIntegrTablesStrong, vGoodTablesStrong;

			if( 1 == nPass )
			{
				irad = nrad + 1;
				// create a mask
				if( NULL == (bits = CreateMask(irad, size, 0, m_pCurLZ->h_dir, m_pCurLZ->k_dir, true)) )
				{
					goto errexit;
				}
			}
			else
			{
				irad = (int)::floor(1.5 * nrad + 0.5) + 1;
				// create a mask
				if( NULL == (bits = CreateMask(irad, size, nrad + 1, m_pCurLZ->h_dir, m_pCurLZ->k_dir, true)) )
				{
					goto errexit;
				}
			}
			DumpMask((LPBYTE)bits, size);
			// put the value of maximum value for a good reflection
			dMaxVal = 0.6 * m_n10Histo;//m_pObj->imaxa;
			SumCircle circle(m_image.pData, m_image.npix, m_image.neg, m_image.w, m_image.h, irad, (LPBYTE)bits, R4(size), size, dMaxVal);
			circle.h_dir = m_pCurLZ->h_dir;
			circle.k_dir = m_pCurLZ->k_dir;
			// some statistics
			m_bCentOnly = FALSE;
			m_into = -(m_infrom =  numeric_limits<double>::max());
			m_dvto = -(m_dvfrom =  numeric_limits<double>::max());
			ProcessMaskedRefls(m_image, m_pCurLZ->m_vRefls, m_dvfrom, m_dvto, m_infrom, m_into, circle);
			// find all reflections which are close to the common Histo
			GetGoodRefls(vGoodRefls, vWeakRefls, vStrNeighbRefls, dSigma, dMaxVal);
			nSigma = (int)::floor(dSigma*5 + 0.5);
			if( nSigma > irad )
			{
				nSigma = irad;
			}
			vTables.resize(vGoodRefls.size() * 2*SQR(2*irad+1), 0);
			vIntegrTables.resize(vGoodRefls.size() * 2*SQR(2*irad+1), 0);
			EstimateTables(vGoodRefls, vTables, irad, circle, m_image);
			if( FALSE == ProcessGoodRefls(vGoodRefls, vRefCurveRefls, vTables, vIntegrTables, vRefCurve) )
			{
				::OutputDebugString("ELD::Integration() -> No good reflections were found.\n");
				goto errexit;
			}
			ProcessStrongNeighbours(vStrNeighbRefls, m_image, vRefCurve, dSigma);
			// figure out strong reflections
			for(i = 0, lpR = &*m_pCurLZ->m_vRefls.begin(); i < m_pCurLZ->m_vRefls.size(); i++, lpR++)
			{
				if( lpR->a0 >= dMaxVal )
				{
					vStrongRefls.push_back(*lpR);
				}
			}
			// sort the strong reflections
//			sort(vStrongRefls.begin(), vStrongRefls.end(), sort_by_oversat());
			::ProcessStrongRefls(vStrongRefls, vGoodStrongRefls, vBadStrongRefls,
				vTablesStrong, vIntegrTablesStrong, vRefCurve,
				irad, m_image, circle, vNewRefCurve);
			::ProcessBadStrongRefls(vGoodStrongRefls, vGoodTablesStrong, vRefCurveRefls,
				vBadStrongRefls, vNewRefCurve, m_image, irad, dMaxVal, this);
			m_pCurLZ->m_vRefls.clear();
			m_pCurLZ->m_vRefls.insert(m_pCurLZ->m_vRefls.end(),
				vWeakRefls.begin(), vWeakRefls.end());
			m_pCurLZ->m_vRefls.insert(m_pCurLZ->m_vRefls.end(),
				vGoodRefls.begin(), vGoodRefls.end());
			m_pCurLZ->m_vRefls.insert(m_pCurLZ->m_vRefls.end(),
				vStrNeighbRefls.begin(), vStrNeighbRefls.end());
			m_pCurLZ->m_vRefls.insert(m_pCurLZ->m_vRefls.end(),
				vGoodStrongRefls.begin(), vGoodStrongRefls.end());
			m_pCurLZ->m_vRefls.insert(m_pCurLZ->m_vRefls.end(),
				vBadStrongRefls.begin(), vBadStrongRefls.end());
			// output
			char drv[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME];
#ifdef _DEBUG
			_splitpath(m_pPar->fname, drv, dir, fname, NULL);
			strcat(fname, "_int_cal");
			sprintf(TMP, "_pass%d", nPass);
			strcat(fname, TMP);
			_makepath(TMP, drv, dir, fname, ".txt");
			if( NULL != (pOutF = fopen(TMP, "wt")) )
			{
				// the reference curve
				fprintf(pOutF, "---- the reference curve ----\n");
				for(int j = 0; j < vRefCurve.size(); j++)
				{
					fprintf(pOutF, "%d\t%g\n", j + 1, vRefCurve[j]);
				}
				// all reflections
				fprintf(pOutF, "---- all relfs, pixel tables ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tMaxVal\tAmpl\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = &*m_pCurLZ->m_vRefls.begin(), i = 0; i < m_pCurLZ->m_vRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t\t",
						lpR->h, lpR->k, lpR->bkg, lpR->noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
// 					if( NULL != lpR->pix_table )
// 					{
// 						for(int j = 0; j < lpR->tbl_size; j++)
// 						{
// 							fprintf(pOutF, "%.1f\t", lpR->pix_table[j]);
// 						}
// 					}
					fprintf(pOutF, "\n");
				}
				// good reflections pixel tables
				fprintf(pOutF, "---- good relfs, pixel tables ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tMaxVal\tAmpl\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = &*vGoodRefls.begin(), i = 0; i < vGoodRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t\t",
						lpR->h, lpR->k, lpR->bkg, lpR->noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
//					if( NULL != lpR->pix_table )
//					{
//						for(int j = 0; j < lpR->tbl_size; j++)
//						{
//							fprintf(pOutF, "%.1f\t", lpR->pix_table[j]);
//						}
//					}
					fprintf(pOutF, "\n");
				}
				// good reflections integrated tables
				fprintf(pOutF, "---- good relfs, integrated tables ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tCirBkg\tCirStd\tMaxVal\tAmpl\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = &*vGoodRefls.begin(), i = 0; i < vGoodRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t\t",
						lpR->h, lpR->k, lpR->bkg, lpR->noise, lpR->circ_bkg, lpR->circ_noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
//					if( NULL != lpR->integr_tbl )
//					{
//						for(int j = 0; j < lpR->tbl_size; j++)
//						{
//							fprintf(pOutF, "%.3f\t", lpR->integr_tbl[j]);
//						}
//					}
					fprintf(pOutF, "\n");
				}
				// reflections with strong neighbors
				fprintf(pOutF, "---- relfs with strong neighbours ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tCirBkg\tCirStd\tMaxVal\tAmpl\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = &*vStrNeighbRefls.begin(), i = 0; i < vStrNeighbRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\n",
						lpR->h, lpR->k, lpR->bkg, lpR->noise, lpR->circ_bkg, lpR->circ_noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
				}
				// strong reflections pixel tables
				fprintf(pOutF, "---- strong relfs, pixel tables ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tCirBkg\tCirStd\tMaxVal\tAmpl\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = &*vStrongRefls.begin(), i = 0; i < vStrongRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t\t",
						lpR->h, lpR->k, lpR->bkg, lpR->noise, lpR->circ_bkg, lpR->circ_noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
//					if( NULL != lpR->pix_table )
//					{
//						for(int j = 0; j < lpR->tbl_size; j++)
//						{
//							fprintf(pOutF, "%.1f\t", lpR->pix_table[j]);
//						}
//					}
					fprintf(pOutF, "\n");
				}
				// strong reflections integrated tables
				fprintf(pOutF, "---- strong relfs, integrated tables ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tCirBkg\tCirStd\tMaxVal\tAmpl\tEstA\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = &*vStrongRefls.begin(), i = 0; i < vStrongRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\t",
						lpR->h, lpR->k, lpR->bkg, lpR->noise, lpR->circ_bkg, lpR->circ_noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->dEstA, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
//					if( NULL != lpR->integr_tbl )
//					{
//						for(int j = 0; j < lpR->tbl_size; j++)
//						{
//							fprintf(pOutF, "%.3f\t", lpR->integr_tbl[j]);
//						}
//					}
					fprintf(pOutF, "\n");
				}
/*				// reference curve reflections
				fprintf(pOutF, "---- reference relfs, pixel tables ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tCirBkg\tCirStd\tMaxVal\tAmpl\tEstA\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = vRefCurveRefls.begin(), i = 0; i < vRefCurveRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\t",
						lpR->h, lpR->k, lpR->bkg, lpR->noise, lpR->circ_bkg, lpR->circ_noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->dEstA, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
					for(int j = 0; j < lpR->tbl_size; j++)
					{
						fprintf(pOutF, "%.3f\t", lpR->pix_table[j]);
					}
					fprintf(pOutF, "\n");
				}
*/
				// strong reflections integrated tables
				fprintf(pOutF, "---- 'good' strong relfs, integrated tables ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tCirBkg\tCirStd\tMaxVal\tAmpl\tEstA\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = &*vGoodStrongRefls.begin(), i = 0; i < vGoodStrongRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\t",
						lpR->h, lpR->k, lpR->bkg, lpR->noise, lpR->circ_bkg, lpR->circ_noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->dEstA, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
//					if( NULL != lpR->pix_table )
//					{
//						reverse(lpR->pix_table, lpR->pix_table + lpR->tbl_size);
//						for(int j = 0; j < lpR->tbl_size; j++)
//						{
//							fprintf(pOutF, "%.3f\t", lpR->pix_table[j]);
//						}
//					}
					fprintf(pOutF, "\n");
				}
				// strong reflections integrated tables
				fprintf(pOutF, "---- 'bad' strong relfs, integrated tables ----\n");
				fprintf(pOutF, "!h\tk\tBkg\tNoise\tCirBkg\tCirStd\tMaxVal\tAmpl\tEstA\tIntegr\tChi^2\tNum/2\tNumOvrS\n");
				for(lpR = &*vBadStrongRefls.begin(), i = 0; i < vBadStrongRefls.size(); i++, lpR++)
				{
					fprintf(pOutF, "%d\t%d\t%.1f\t%.2f\t%.1f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\t",
						lpR->h, lpR->k, lpR->bkg, lpR->noise, lpR->circ_bkg, lpR->circ_noise,
						(int)lpR->a0, (int)lpR->Amp_bkg, (int)lpR->dEstA, (int)lpR->IntegrA,
						(int)lpR->dummy[0], lpR->num_2, lpR->num_over);
//					if( NULL != lpR->pix_table )
//					{
//						reverse(lpR->pix_table, lpR->pix_table + lpR->tbl_size);
//						for(int j = 0; j < lpR->tbl_size; j++)
//						{
//							fprintf(pOutF, "%.3f\t", lpR->pix_table[j]);
//						}
//					}
					fprintf(pOutF, "\n");
				}
			}
#endif
			FILE *pFile;
			_makepath(TMP, drv, dir, fname, ".hke");
			if( NULL != (pFile = fopen(TMP, "wt")) )
			{
				fprintf(pFile, ";\n; File : %s, internal integration\n;\n", m_pPar->fname);
				fprintf(pFile, "; a=%.3fÅ, b=%.3fÅ, gamma=%.3f\n;\n", m_pCurLZ->a_length, m_pCurLZ->b_length,
					RAD2DEG(m_pCurLZ->gamma));
				fprintf(pFile, "Format: h k s a\n;\n; h\tk\tIobs\tIest\n");
				fprintf(pFile, ";------------------------------------------------\n");
				for(lpR = &*vGoodRefls.begin(), i = 0; i < vGoodRefls.size(); i++, lpR++)
				{
					fprintf(pFile, "%d\t%d\t%.2f\t%.2f\n", lpR->h, lpR->k, lpR->Amp_bkg, lpR->Amp_bkg);
				}
				fprintf(pFile, "; strong reflections\n");
				for(lpR = &*vStrongRefls.begin(), i = 0; i < vStrongRefls.size(); i++, lpR++)
				{
					fprintf(pFile, "%d\t%d\t%.2f\t%.2f\n", lpR->h, lpR->k, lpR->Amp_bkg, lpR->dEstA);
				}
				fclose(pFile);
			}
			bRet = TRUE;
		}
		catch (std::exception& e)
		{
			Trace(_FMT(_T("Internal calibration calculations failed on %d-th reflection: %s"), i, e.what()));
		}
errexit:
		if( NULL != bits )
		{
			delete[] bits;
		}
#ifdef _DEBUG
		if( NULL != pOutF )
		{
			fclose(pOutF);
		}
#endif
	}
	return bRet;
}
