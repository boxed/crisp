/****************************************************************************/
/* Copyright (c) 2006 Calidris **********************************************/
/* Written by Peter Oleynikov  **********************************************/
/****************************************************************************/
/* CRISP2.ELD ***************************************************************/
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
#include "eld.h"

#include "Crash.h"
#include "HelperFn.h"

extern int GetAXYX(const IMAGE &Img, int x, int y, int rad, double thr, double &axs, double &ays, double &as);

BOOL ELD::ELD_FindCenter(double &x0, double &y0)
{
	int		i, j, xc, yc, r, nNumPixels;
	double	s, ax, ay, as;

	// the m_image center
	xc = m_image.w >> 1;
	yc = m_image.h >> 1;
	// the radius
	r = __min(xc, yc);
	// the threshold value
	double dMinMaxD = m_image.m_dMax - m_image.m_dMin;
	double dThreshold = 0.95 * dMinMaxD + m_image.m_dMin;
// 	double dThreshold = _max - 15;

	// find the mass center of pixels with values over the threshold
	// if none found
	while( (nNumPixels = GetAXYX(m_image, xc, yc, r-8, dThreshold, ax, ay, as)) < 100 && (dThreshold > m_image.m_dMin) )
	{
// 		dThreshold -= 3;
		dThreshold -= 0.01 * dMinMaxD;	// lower the threshold by 1% of the m_image maximum
	}
// 	if( dThreshold <= 0)
	if( dThreshold <= m_image.m_dMin)
	{
		return FALSE;
	}
	if( as < 1e-3 )
	{
		return FALSE;
	}

	// the mass center
	x0 = ax / as;
	y0 = ay / as;
	//	i = (IMGW(pEld) - x0); if(i>x0) i = x0; j = (IMGH(pEld) - yc); if(j>yc) j = yc;
	i = round(__min(x0, m_image.w - x0));
	j = round(__min(y0, m_image.h - y0));
	r = __min(r >> 1, __min(i,j));
	if( r > 15 )
		r -= 8;
	while( (nNumPixels = (float)GetAXYX(m_image, round(x0), round(y0), r, dThreshold, ax, ay, as)) < 70 && (dThreshold > m_image.m_dMin) )
	{
		dThreshold -= 0.02 * dMinMaxD;	// lower the threshold by 2% of the m_image maximum
// 		dThreshold -= 5;
	}
	if( as < 1e-3 )
	{
		return FALSE;
	}
	// the mass center
	x0 = ax / as;
	y0 = ay / as;
	// the mass center
	r = round(FastSqrt(nNumPixels * 0.5)) + 1;
	i = round(__min(x0, m_image.w - x0));
	j = round(__min(y0, m_image.h - y0));
	r = __min(r, __min(i,j));
	while( (s = (float)GetAXYX(m_image, round(x0), round(y0), r, dThreshold, ax, ay, as)) < 50 && (dThreshold > m_image.m_dMin) )
	{
		dThreshold -= 0.03 * dMinMaxD;	// lower the threshold by 3% of the m_image maximum
// 		dThreshold -= 10;
	}
	if( as < 1e-3 )
	{
		return FALSE;
	}
	// the mass center
	x0 = ax / as;
	y0 = ay / as;
	return TRUE;
}

BOOL ELD::ELD_FindClosestCenter(const CVector2d &mass_center, const CVector2d &h_dir,
						   const CVector2d &k_dir, CVector2d &center)
{
	int hi, ki;
	// the difference between the mass center and currently estimated center
	double deltax = mass_center.x - center.x;
	double deltay = mass_center.y - center.y;
	// the determinant
	double det = h_dir.y * k_dir.x - h_dir.x * k_dir.y;
	if( FastAbs(det) > 1e-3 )
	{
		double h = (deltay * k_dir.x - deltax * k_dir.y) / det;
		double k = (deltax * h_dir.y - deltay * h_dir.x) / det;
		if( h < 0 ) hi = (int)::ceil(h - 0.5);
		else hi = (int)::floor(h + 0.5);
		if( k < 0 ) ki = (int)::ceil(k - 0.5);
		else ki = (int)::floor(k + 0.5);
		center += hi * h_dir + ki * k_dir;
		return TRUE;
	}
	return FALSE;
}

int ELD::ELD_DoPeakSearch(EldAritemVec &vRefls, int nWindowR, double dLimitRad, BOOL bCheckR)
{
	int n0, n1, x, y, a, i, j, l, nCurSz, nClosestDist;
	float x0, y0, xx, yy;//, as;
	double d, dLimitRad2, fClosestDist2;
// 	DWORD amp;
	ARITEM ait;
	LPARITEM pRefl, pStart;
	double amp, as;

	m_image.CalcMinMax();

	int N_SIZE = vRefls.size();
	// the ELD pointer
	x0 = m_center.x;
	y0 = m_center.y;
	// the limiting radius
	dLimitRad2 = SQR(dLimitRad);
	// the maximum allowed distance (if less - the reflection is same)
	nClosestDist = nWindowR >> 1;
	fClosestDist2 = SQR(nClosestDist);
	// the index of the last reflection in the list vRefls
	nCurSz = 0;
	// the start pointer
	pStart = &*vRefls.begin();
	// move the search window around the m_image
	for(y = nWindowR; y <= m_image.h - nWindowR; y += nWindowR)
	{
		for(x = nWindowR; x <= m_image.w - nWindowR; x += nWindowR)
		{
			// check the limiting LZ radius - if required
			if( TRUE == bCheckR )
			{
				if( (SQR(x - x0) + SQR(y - y0)) > dLimitRad2 )
				{
					continue;
				}
			}
			xx = x;
			yy = y;
			// find the peak position -> returns refined coordinates in (xx, yy)
			if( a = PeakCenter2(nWindowR, xx, yy, nWindowR, 0.1f, TRUE, amp, as) )
			{
				continue;
			}
			if( TRUE == bCheckR )
			{
				if( (SQR(xx - x0) + SQR(yy - y0)) > dLimitRad2 )
				{
					continue;
				}
			}
			// mark the reflection
			DrawMark(xx, yy, 1, -3);
			// initialize the distance
			d = 0;
			// try to find any reflection which is closer than nClosestDist
			pRefl = pStart;
			for(l = 0; l < nCurSz; l++, pRefl++)
			{
				// squared distance from the actual to the refined peak position
				d = SQR(pRefl->x - xx) + SQR(pRefl->y - yy);
				// if found - stop
				if( d < fClosestDist2)
					break;
			}
			// not found?
			if( (l == nCurSz) || (d < 1) )
			{
				// do we have some space for a new reflection?
				if( l < nCurSz )
				{
					n0 = pRefl->n; // vTemp[l]
					pRefl->n = (n1 = n0 + 1);
					pRefl->x = (pRefl->x * n0 + xx ) / n1;
					pRefl->y = (pRefl->y * n0 + yy ) / n1;
					pRefl->a = (pRefl->a * n0 + amp) / n1;
					pRefl->as = (pRefl->as * n0 + as) / n1;
					pRefl->nOthers = 0;
				}
				else
				{
					// no space - try to insert the reflection
					n0 = 0;
					ait.n = n1 = 1;
					ait.x = xx;
					ait.y = yy;
					ait.a = amp;
					ait.as = as;
					ait.nOthers = 0;
					// a new reflection, try to insert it according to its' amplitude
					for(i = 0; i < nCurSz; i++)
					{
//						if( pStart[i].a < amp )
						if( pStart[i].as < as )
						{
							break;
						}
					}
					// put at the end of the list
					if( (i == nCurSz) && (nCurSz < N_SIZE-1) )
					{
						pStart[nCurSz] = ait;
						if( l == nCurSz )
						{
							nCurSz++;
						}
					}
					else
					{
						for(j = nCurSz-1; j >= i; j--)
						{
							pStart[j+1] = pStart[j];
						}
						pStart[i] = ait;
						if( (l == nCurSz) && (nCurSz < N_SIZE-1) )
						{
							nCurSz++;
						}
					}
				}
			}
		}
	}
#ifdef _DEBUG
	::OutputDebugString("--------\nreflections found:\n");
	for(x = 0; x < nCurSz; x++)
	{
		sprintf(TMP, "%g\t%g\t%g\n", vRefls[x].x, vRefls[x].y, vRefls[x].a);
		::OutputDebugString(TMP);
	}
	::OutputDebugString("--------\n");
#endif
	// return the total number of found reflections (not more than the initial N_SIZE)
	return nCurSz;
}

template <typename _Tx>
void ELD_GetWindowStat(_Tx *pSrc, int nStride, int X, int Y, int M, int N, double &dAvg, double &dNoise)
{
//	pSrc = (_Tx*)(((LPBYTE)pSrc)+nStride*Y);
//	pSrc += X;
	double diff_x = 0.0;
	double diff_y = 0.0;
	_Tx val;

	dAvg = 0.0;
	for(int y = 0; y < N; y++)
	{
		for(int x = 0; x < M; x++)
		{
			val = pSrc[x];
			diff_x += FastAbs(pSrc[x+1] - val);
			diff_y += FastAbs(*(_Tx*)(((LPBYTE)&pSrc[x]) + nStride) - val);
			dAvg += val;
		}
		pSrc += nStride;
	}
	dAvg   /= M*N;
	diff_x /= M*N;
	diff_y /= M*N;
	dNoise = 0.5*(diff_x + diff_y);
}

template <typename _Tx>
void ELD_WindowPixels(_Tx *pSrc, int nStride, int X, int Y, int M, int N, std::vector<CVector3d> &vPts)
{
	double dAvg, dNoise;
	_Tx val;

	pSrc = (_Tx*)(((LPBYTE)pSrc) + nStride*Y);
	pSrc += X;

	ELD_GetWindowStat(pSrc, nStride, X, Y, M, N, dAvg, dNoise);
	for(int y = 0; y < N; y++)
	{
		for(int x = 0; x < M; x++)
		{
			val = pSrc[x];
			if( val < dAvg + 4.0*dNoise )
			{
				continue;
			}
			vPts.push_back(CVector3d(x+X, y+Y, val));
		}
		pSrc += nStride;
	}
}

int ELD_DoPeakSearch2(OBJ* pObj, EldAritemVec &vRefls, int nWindowR,
					  double dLimitRad, BOOL bCheckR)
{
	LPELD pEld;
	int N_SIZE, x, y;//, n0, n1, i, j, l, nCurSz, a, nClosestDist;
//	float x0, y0, as, xx, yy;
//	double dLimitRad2, d, fClosestDist2;
//	WORD amp;
//	ARITEM ait;
	LPARITEM pRefl;//, pStart;

	N_SIZE = vRefls.size();
	// the ELD pointer
	pEld = (LPELD)&pObj->dummy;

//	double dAvg, dNoise;
//	ELD_GetWindowStat((LPBYTE)pEld->img, pEld->imgw, 140, 24, 10, 10, dAvg, dNoise);
	std::vector<CVector3d> vPts;
	int M = 10, N = 10;
	int WinX = pEld->m_image.w / M;
	int WinY = pEld->m_image.h / N;
	int nStride;
	nStride = pEld->m_image.w * IMAGE::PixFmt2Bpp(pEld->m_image.npix);
//	if( PIX_BYTE == pEld->nPix ) nStride = pEld->m_image.w;
//	else if( PIX_WORD == pEld->nPix ) nStride = pEld->m_image.w * 2;
	nStride = R4(nStride);
	switch( pEld->m_image.npix )
	{
	case PIX_BYTE:
		{
			for(y = 0; y < WinY; y++)
			{
				for(int x = 0; x < WinX; x++)
				{
					ELD_WindowPixels((LPBYTE)pEld->m_image.pData, nStride, x*M, y*N, M, N, vPts);
				}
			}
			break;
		}
	case PIX_CHAR:
		{
			for(y = 0; y < WinY; y++)
			{
				for(int x = 0; x < WinX; x++)
				{
					ELD_WindowPixels((LPSTR)pEld->m_image.pData, nStride, x*M, y*N, M, N, vPts);
				}
			}
			break;
		}
	case PIX_WORD:
		{
			for(y = 0; y < WinY; y++)
			{
				for(int x = 0; x < WinX; x++)
				{
					ELD_WindowPixels((LPWORD)pEld->m_image.pData, nStride, x*M, y*N, M, N, vPts);
				}
			}
			break;
		}
	case PIX_SHORT:
		{
			for(y = 0; y < WinY; y++)
			{
				for(int x = 0; x < WinX; x++)
				{
					ELD_WindowPixels((SHORT*)pEld->m_image.pData, nStride, x*M, y*N, M, N, vPts);
				}
			}
			break;
		}
	case PIX_DWORD:
		{
			for(y = 0; y < WinY; y++)
			{
				for(int x = 0; x < WinX; x++)
				{
					ELD_WindowPixels((LPDWORD)pEld->m_image.pData, nStride, x*M, y*N, M, N, vPts);
				}
			}
			break;
		}
	case PIX_LONG:
		{
			for(y = 0; y < WinY; y++)
			{
				for(int x = 0; x < WinX; x++)
				{
					ELD_WindowPixels((LPLONG)pEld->m_image.pData, nStride, x*M, y*N, M, N, vPts);
				}
			}
			break;
		}
	case PIX_FLOAT:
		{
			for(y = 0; y < WinY; y++)
			{
				for(int x = 0; x < WinX; x++)
				{
					ELD_WindowPixels((float*)pEld->m_image.pData, nStride, x*M, y*N, M, N, vPts);
				}
			}
			break;
		}
	case PIX_DOUBLE:
		{
			for(y = 0; y < WinY; y++)
			{
				for(int x = 0; x < WinX; x++)
				{
					ELD_WindowPixels((double*)pEld->m_image.pData, nStride, x*M, y*N, M, N, vPts);
				}
			}
			break;
		}
	}
	vRefls.resize(vPts.size());
	CVector3d *ptr = &vPts[0];
	pRefl = &*vRefls.begin();
	for(x = 0; x < vPts.size(); x++, ptr++, pRefl++)
	{
		pRefl->a = pRefl->as = ptr->z;
		pRefl->x = ptr->x;
		pRefl->y = ptr->y;
		pRefl->n = 1;
	}
	return vRefls.size();
/*
	x0 = pEld->center.x;
	y0 = pEld->center.y;
	// the limiting radius
	dLimitRad2 = SQR(dLimitRad);
	// the maximum allowed distance (if less - the reflection is same)
	nClosestDist = nWindowR >> 1;
	fClosestDist2 = SQR(nClosestDist);
	// the index of the last reflection in the list vRefls
	nCurSz = 0;
	// the start pointer
	pStart = vRefls.begin();
	// move the search window around the m_image
	for(y = nWindowR; y <= pEld->imgh - nWindowR; y += nWindowR)
	{
		for(x = nWindowR; x <= pEld->imgw - nWindowR; x += nWindowR)
		{
			// check the limiting LZ radius - if required
			if( TRUE == bCheckR )
			{
				if( (SQR(x - x0) + SQR(y - y0)) > SQR(dLimitRad2) )
				{
					continue;
				}
			}
			xx = x;
			yy = y;
			// find the peak position -> returns refined coordinates in (xx, yy)
			if( a = PeakCenter(pObj, nWindowR, &xx, &yy, nWindowR, 0.1f, TRUE, &amp, as) )
			{
				continue;
			}
			// mark the reflection
			Mark(pObj, xx, yy, 1, -3);
			// initialize the distance
			d = 0;
			// try to find any reflection which is closer than nClosestDist
			pRefl = pStart;
			for(l = 0; l < nCurSz; l++, pRefl++)
			{
				// squared distance from the actual to the refined peak position
				d = SQR(pRefl->x - xx) + SQR(pRefl->y - yy);
				// if found - stop
				if( d < fClosestDist2)
					break;
			}
			// not found?
			if( (l == nCurSz) || (d < 1) )
			{
				// do we have some space for a new reflection?
				if( l < nCurSz )
				{
					n0 = pRefl->n; // vTemp[l]
					pRefl->n = (n1 = n0 + 1);
					pRefl->x = (pRefl->x * n0 + xx ) / n1;
					pRefl->y = (pRefl->y * n0 + yy ) / n1;
					pRefl->a = (pRefl->a * n0 + amp) / n1;
					pRefl->as = (pRefl->as * n0 + as) / n1;
					pRefl->nOthers = 0;
				}
				else
				{
					// no space - try to insert the reflection
					n0 = 0;
					ait.n = n1 = 1;
					ait.x = xx;
					ait.y = yy;
					ait.a = amp;
					ait.as = as;
					ait.nOthers = 0;
					// a new reflection, try to insert it according to its' amplitude
					for(i = 0; i < nCurSz; i++)
					{
//						if( pStart[i].a < amp )
						if( pStart[i].as < as )
						{
							break;
						}
					}
					// put at the end of the list
					if( (i == nCurSz) && (nCurSz < N_SIZE-1) )
					{
						pStart[nCurSz] = ait;
						if( l == nCurSz )
						{
							nCurSz++;
						}
					}
					else
					{
						for(j = nCurSz-1; j >= i; j--)
						{
							pStart[j+1] = pStart[j];
						}
						pStart[i] = ait;
						if( (l == nCurSz) && (nCurSz < N_SIZE-1) )
						{
							nCurSz++;
						}
					}
				}
			}
		}
	}
	// return the total number of found reflections (not more than the initial N_SIZE)
	return nCurSz;
*/
}
