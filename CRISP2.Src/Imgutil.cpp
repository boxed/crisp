/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Image utilities * by ML **************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "objects.h"
#include "commondef.h"
#include "shtools.h"
#include "const.h"
#include "globals.h"
#include "common.h"
#include "ft32util.h"

// Added by Peter on 24 February 2006
#include "Crash.h"

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif


template <typename _Tx>
bool CalcHisto_t(_Tx *pSrc, int nHistoSz, double &dMin, double &dMax,
				 IMAGE *pImg, std::vector<long> &vHisto, bool bZoom = false)
{
	double dScale, val;
	int index;
	long *pHisto;
	int width, height, stride, x, y;
	_Tx *ptr;

	stride = pImg->stride;
	vHisto.resize(nHistoSz + 16, 0);
	// some checks
	if( (NULL == pImg) || (true == vHisto.empty()) )
	{
		return false;
	}
	// initialization
	pHisto = &*vHisto.begin();
	width = pImg->w, height = pImg->h;
	dMax = -(dMin = numeric_limits<double>::max());
	// find min-max values
	ptr = pSrc;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			val = ptr[x];
			if( val > dMax ) dMax = val;
			if( val < dMin ) dMin = val;
		}
		ptr = (_Tx*)(((LPBYTE)ptr) + stride);
	}
	pImg->m_dMin = dMin;
	pImg->m_dMax = dMax;
	if( dMin != dMax )
	{
		ptr = pSrc;
		if( true == bZoom )
		{
			dScale = (double)(nHistoSz - 1) / (double)(dMax - dMin);
			for(y = 0; y < height; y++)
			{
				for(x = 0; x < width; x++)
				{
					index = (int)((ptr[x] - dMin) * dScale);
					pHisto[index]++;
				}
				ptr = (_Tx*)(((LPBYTE)ptr) + stride);
			}
		}
		else
		{
			for(y = 0; y < height; y++)
			{
				for(x = 0; x < width; x++)
				{
					pHisto[(int)ptr[x]]++;
				}
				ptr = (_Tx*)(((LPBYTE)ptr) + stride);
			}
		}
		return true;
	}
	return false;
}

template <typename _Tx>
bool CalcMinMax_t(_Tx *pSrc, int nHistoSz, IMAGE *pImg, bool bZoom = false)
{
	long wMin, wMax, aMin, aMax;
	double dwMin, dwMax, daMin, daMax;
	double dScale, fMin, fMax;
	_Tx *ptr = pSrc;
	int width, height;
	std::vector<long> histo;
	long *pHisto;

	if( false == CalcHisto_t<_Tx>(pSrc, nHistoSz, fMin, fMax, pImg, histo, bZoom) )
	{
		return false;
	}
	// some checks
	if( (NULL == pImg) || (true == histo.empty()) )
	{
		return false;
	}
	pHisto = &*histo.begin();
	width = pImg->w, height = pImg->h;
	// initialization
//	fMax = -(fMin = numeric_limits<_Tx>::max());
//	// find min-max values
//	for(total = 0; total < width*height; total++, pSrc++)
//	{
//		if( fMin > *pSrc ) fMin = *pSrc;
//		if( fMax < *pSrc ) fMax = *pSrc;
//	}
//	pImg->m_dMin = fMin;
//	pImg->m_dMax = fMax;
	if( fMin != fMax )
	{
		dScale = (double)(nHistoSz - 1) / (double)(fMax - fMin);
		// Minimum in the histogram
		//long _Min = *std::min_element(histo.begin(), histo.end());
		// Maximum in the histogram
		std::vector<long>::iterator _Max = std::max_element(histo.begin(), histo.end());
		// analyze the left part
		std::vector<long>::iterator it;
		double dSum = 0.0;
		for(it = histo.begin(); it != _Max; ++it)
		{
			dSum += *it;
		}
		dSum += *_Max;
		double dCurSum = 0.0;
		for(it = _Max; it != histo.begin(); --it)
		{
			dCurSum += *it;
			if( dCurSum >= 0.9 * dSum )
			{
				break;
			}
		}
		wMin = it - histo.begin();
		// analyze the right part
		dSum = 0.0;
		for(it = _Max; it != histo.end(); ++it)
		{
			dSum += *it;
		}
		dCurSum = 0.0;
		for(it = _Max; it != histo.end(); ++it)
		{
			dCurSum += *it;
			if( dCurSum >= 0.996 * dSum )
			{
				break;
			}
		}
		wMax = it - histo.begin() + 1;
		for(aMin = 0;      pHisto[aMin]==0; aMin++) {}
		for(aMax = nHistoSz-1; pHisto[aMax]==0; aMax--) {}

		// default values
		dwMin = wMin;
		dwMax = wMax;
		daMin = aMin;
		daMax = aMax;

		// rescale if in Stretching mode
		if( true == bZoom )
		{
			dwMin = wMin / dScale + fMin;
			dwMax = wMax / dScale + fMin;
			daMin = aMin / dScale + fMin;
			daMax = aMax / dScale + fMin;
		}
	}
	if( dwMin >= dwMax )
	{
		dwMin = 0;
		dwMax = nHistoSz - 1;
	}
	pImg->m_dIMin  = dwMin;
	pImg->m_dIMax  = dwMax;
	pImg->m_dIMinA = daMin;
	pImg->m_dIMaxA = daMax;
	return true;
}

void IMAGE::CalcMinMax()
{
	switch( npix )
	{
	case PIX_BYTE:
		CalcMinMax_t<BYTE>((LPBYTE)pData, 256, this);
		break;
	case PIX_CHAR:
		CalcMinMax_t<CHAR>((LPSTR)pData, 256, this, true);
		break;
	case PIX_WORD:
		CalcMinMax_t<WORD>((LPWORD)pData, 65536/*256*/, this, true);
		break;
	case PIX_SHORT:
		CalcMinMax_t<SHORT>((SHORT*)pData, 65536/*256*/, this, true);
		break;
	case PIX_DWORD:
		CalcMinMax_t<DWORD>((LPDWORD)pData, 65536/*256*/, this, true);
		break;
	case PIX_LONG:
		CalcMinMax_t<LONG>((LPLONG)pData, 65536/*256*/, this, true);
		break;
	case PIX_FLOAT:
		CalcMinMax_t<FLOAT>((LPFLOAT)pData, 65536/*256*/, this, true);
		break;
	case PIX_DOUBLE:
		CalcMinMax_t<DOUBLE>((LPDOUBLE)pData, 65536/*256*/, this, true);
		break;
	default:
		break;
	}
}

bool IMAGE::CalculateHisto(std::vector<long> &vHisto)
{
	bool bRet = false;
	double dMin, dMax;

	switch( npix )
	{
	case PIX_BYTE:
		bRet = CalcHisto_t<BYTE>((LPBYTE)pData, 256, dMin, dMax, this, vHisto);
		break;
	case PIX_CHAR:
		bRet = CalcHisto_t<CHAR>((LPSTR)pData, 256, dMin, dMax, this, vHisto, true);
		break;
	case PIX_WORD:
		bRet = CalcHisto_t<WORD>((LPWORD)pData, 65536, dMin, dMax, this, vHisto);
		break;
	case PIX_SHORT:
		bRet = CalcHisto_t<SHORT>((SHORT*)pData, 65536, dMin, dMax, this, vHisto, true);
		break;
	case PIX_DWORD:
		bRet = CalcHisto_t<DWORD>((LPDWORD)pData, 65536, dMin, dMax, this, vHisto, true);
		break;
	case PIX_LONG:
		bRet = CalcHisto_t<LONG>((LPLONG)pData, 65536, dMin, dMax, this, vHisto, true);
		break;
	case PIX_FLOAT:
		bRet = CalcHisto_t<FLOAT>((LPFLOAT)pData, 65536, dMin, dMax, this, vHisto, true);
		break;
	case PIX_DOUBLE:
		bRet = CalcHisto_t<double>((LPDOUBLE)pData, 65536, dMin, dMax, this, vHisto, true);
		break;
	default:
		break;
	}
	return bRet;
}

bool IMAGE::IsNegative()
{
	vector<long> vHisto;
	double as, an, an0, mean;
	int		min, max, k, l, nSize;
	bool	ret = false;
	LPLONG	his;

	if( false == CalculateHisto(vHisto) )
	{
		return false;
	}
	if( true == vHisto.empty() )
		return false;

	his = &*vHisto.begin();
	an = 0;
	as = 0;
	an0 = round(0.005*(double)w * (double)h);
	if( an0 < 1 )
		an0 = 1;
	switch( npix )
	{
	case PIX_BYTE:
	case PIX_CHAR:
		nSize = 256;
		break;
	case PIX_WORD:
	case PIX_SHORT:
	case PIX_DWORD:
	case PIX_LONG:
	case PIX_FLOAT:
	case PIX_DOUBLE:
		nSize = 65536;
		break;
	default:
		MessageBox(NULL, "IsNegative - unsupported pixel format", "EDRD ERROR!", MB_OK);
		return false;
		break;
	}
	for(l = 0; l < nSize; l++)
	{
		if( an < an0 ) min = (WORD)l;
		as += (double)l*(double)his[l];
		an += (double)his[l];
	}
	mean = round(as / an);
	an = 0;
	for(l = nSize - 1; l > 0; l--)
	{
		if( an < an0 ) max = l;
		an += (double)his[l];
	}
	k = (min/2 + max/2);
	for(an = 0, l = min; l < k; l++)
		an += (double)his[l];
	for(as = 0, l = k; l < max; l++)
		as += (double)his[l];
	ret = ( as < an ) ? false : true;
	neg = 0;
	if( true == ret )
	{
		switch( npix )
		{
		case PIX_BYTE:
		case PIX_CHAR:
			neg = 0xFF;
			break;
		case PIX_WORD:
		case PIX_SHORT:
			neg = 0xFFFF;
			break;
		case PIX_DWORD:
		case PIX_LONG:
		case PIX_FLOAT:
		case PIX_DOUBLE:
			neg = 0xFFFFFF;
			break;
		default:
//			MessageBox(NULL, "SetNegative - unsupported pixel format", "EDRD ERROR!", MB_OK);
			break;
		}
	}
	return ret;

}

__declspec(dllexport) void imgCalcMinMax( OBJ* pObj )
{
	IMAGE image;

	image.w = pObj->x;
	image.h = pObj->y;
	image.stride = pObj->stride;
	image.npix = pObj->npix;
	image.pData = pObj->dp;
	image.CalcMinMax();
	pObj->imin = image.m_dIMin;
	pObj->imax = image.m_dIMax;
	pObj->imina = image.m_dIMinA;
	pObj->imaxa = image.m_dIMaxA;
}
