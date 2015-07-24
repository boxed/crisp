/****************************************************************************/
/* Copyright (c) 2006 Ananlitex *********************************************/
/****************************************************************************/
/* CRISP2.EDRD_background * by Peter Oleynikov ******************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"
#include "rgconfig.h"
#include "EDRD.h"

#pragma warning(disable:4305)

#define		RESAMPLING_SIZE		512

// uncommented by Peter
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

extern bool Fft1D_Calculate(double *data, size_t nSize, int iSign);

BOOL ScaleProfileNearest_1D(double *pProfile, int nSize, vector<double> &vOutProf, double &dScale)
{
	BOOL bRet = FALSE;

	try
	{
		int nScale, nCount, iPt, i, j, nNewSize;
		double dSum;

		dScale = nSize / double(RESAMPLING_SIZE);
		nScale = (int)floor(dScale);
		if( nScale <= 1 )
		{
			nNewSize = __max(nSize,RESAMPLING_SIZE);
			vOutProf.resize(nNewSize, 0);
			copy(pProfile, pProfile + nSize, vOutProf.begin());
		}
		else
		{
			// needs to scale down
			nNewSize = (int)floor((double)nSize / (double)nScale + 0.5) + 1;
			vOutProf.resize(nNewSize, 0);
			iPt = 0;
			for(i = 0; i < nSize; i += nScale, iPt++)
			{
				dSum = 0;
				nCount = __min(nScale, nSize-i);
				if( nCount > 0 )
				{
					for(j = 0; j < nCount; j++)
					{
						dSum += pProfile[i+j];
					}
					vOutProf[iPt] = dSum / double(nCount);
				}
			}
		}
		bRet = TRUE;
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("ScaleProfileNearest_1D() -> %s"), e.what()));
	}

	return bRet;
}

BOOL ScaleProfile_1D(double *pProfile, int nSize, vector<double> &vOutProf, double &dScale)
{
	BOOL bRet = FALSE;
	if( NULL == pProfile )
		return FALSE;
	try
	{
		vector<double> scaled;
		double _x;
		int i;

		if( FALSE == ScaleProfileNearest_1D(pProfile, nSize, scaled, dScale) )
		{
			return FALSE;
		}
		vOutProf.resize(RESAMPLING_SIZE, 0);
		if( dScale <= 1 )
		{
			copy(scaled.begin(), scaled.end(), vOutProf.begin());
		}
		else
		{
			_x = 0;
			dScale = scaled.size() / double(RESAMPLING_SIZE);
			for(i = 0; i < RESAMPLING_SIZE; i++, _x += dScale)
			{
				vOutProf[i] = Bilinear<double>(&*scaled.begin(), 0, 0, 0, _x, 0);
			}
		}
		bRet = TRUE;
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("ScaleProfile_1D() -> %s"), e.what()));
	}
	return bRet;
}

static BOOL MakeGauss1D(vector<double> &Gauss, size_t w, double dAmpl, double FWHM)
{
	BOOL bRet = FALSE;
	try
	{
		double *ptr, dInvFWHM;
		int x, nCentr = w >> 1;

		Gauss.resize(w<<1, 0);
		ptr = &*Gauss.begin();
		
		dInvFWHM = 1.0 / SQR(FWHM);
		for(x = 1; x < nCentr; x++)
		{
			ptr[x<<1] = ptr[(w-x)<<1] = dAmpl*FastExp(-dInvFWHM * SQR(x));
		}
		ptr[0] = dAmpl;
		bRet = TRUE;
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("MakeGauss1D() -> %s"), e.what()));
	}
	return bRet;
}

BOOL IterateBackground1D(vector<double> &Smooth, vector<double> &Data,
						 size_t w, size_t nIter)
{
	BOOL bRet = FALSE;
	try
	{
		vector<double> IFft(RESAMPLING_SIZE*2, 0.0);
		vector<double> Bkg(RESAMPLING_SIZE*2, 0.0);
		double dVal, *ptr, *ptr2, *dst, dInv, dMax;
		int i, x;

		// convert the background
		ptr = &*Bkg.begin();
		ptr2 = &*Data.begin();
		for(i = 0; i < w; i++)
		{
			*ptr = *ptr2;
			ptr += 2;
			ptr2++;
		}
		dInv = 1.0 / RESAMPLING_SIZE;
		Fft1D_Calculate(&*Smooth.begin(), RESAMPLING_SIZE, 1);
		dMax = 1.0 / *max_element(Smooth.begin(), Smooth.end());
		for(x = 0; x < RESAMPLING_SIZE<<1; x++)
		{
			Smooth[x] *= dMax;
		}
		// iterations
		for(i = 0; i < nIter; i++)
		{
			sprintf(TMP, "1D bkg iteration: %d\n", i+1);
			::OutputDebugString(TMP);
			Fft1D_Calculate(&*Bkg.begin(), RESAMPLING_SIZE, 1);
			for(x = 0; x < RESAMPLING_SIZE<<1; x++)
			{
				Bkg[x] *= dInv;
			}
			ptr  = &*Smooth.begin();
			ptr2 = &*Bkg.begin();
			dst  = &*IFft.begin();
			// Complex multiplication of FFTs
			for(x = 0; x < RESAMPLING_SIZE; x++)
			{
				dst[0] = ptr[0]*ptr2[0] - ptr[1]*ptr2[1];
				dst[1] = ptr[0]*ptr2[1] + ptr[1]*ptr2[0];
				ptr += 2;
				ptr2 += 2;
				dst += 2;
			}
			// IFFT
			Fft1D_Calculate(&*IFft.begin(), RESAMPLING_SIZE, -1);
			dst  = &*Bkg.begin();
			ptr2 = &*Data.begin();
			ptr  = &*IFft.begin();
			for(x = 0; x < w; x++)
			{
				dVal = (*ptr2) - (*ptr);
				if( dVal < 0.0 )
					dVal = 0.0;
				dst[0] = (*ptr2) - dVal;
				dst[1] = 0.0;
				ptr2++;
				ptr += 2;
				dst += 2;
			}
		}
		ptr = &*Bkg.begin();
		dst = &*Data.begin();
		// Complex multiplication of FFTs
		for(x = 0; x < RESAMPLING_SIZE; x++)
		{
			*dst = *ptr;
			dst++;
			ptr += 2;
		}
		bRet = TRUE;
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("IterateBackground1D() -> %s"), e.what()));
	}
	return bRet;
}

BOOL SubtractBackground1D(double *pProfile, double *pOutProfile, int nSize,
						  vector<double> &bkg, double dScale)
{
	BOOL bRet = FALSE;
	try
	{
		double _x, dInv;
		int x;

		dInv = 1.0;
		if( dScale > 1.0 )
		{
			dInv = 1.0 / dScale;
		}
		_x = 0;
		for(x = 0; x < nSize; x++, _x += dInv)
		{
			pOutProfile[x] = pProfile[x] - Bilinear(&*bkg.begin(), 0, 0, 0, _x, 0);
		}
		bRet = TRUE;
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("SubtractBackground1D() -> %s"), e.what()));
	}
	return bRet;
}

BOOL EstimateBackground_1D(double *pProfile, int nSize, double *pOutProfile)
{
	BOOL bRet = FALSE;
	try
	{
		vector<double> data, Gauss;
		double dScale;

		if( (NULL == pProfile) || (NULL == pOutProfile) )
			return FALSE;

		if( FALSE == ScaleProfile_1D(pProfile, nSize, data, dScale) )
			return FALSE;

		if( TRUE == MakeGauss1D(Gauss, RESAMPLING_SIZE, 1, (int)floor(0.02*nSize+0.5)) )
		{
			if( TRUE == IterateBackground1D(Gauss, data, RESAMPLING_SIZE, 20) )
			{
				bRet = SubtractBackground1D(pProfile, pOutProfile, nSize, data,
					nSize / double(RESAMPLING_SIZE));
			}
		}
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("EstimateBackground_1D() -> %s"), e.what()));
	}
	return bRet;
}
