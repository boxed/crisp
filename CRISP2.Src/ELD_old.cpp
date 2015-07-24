/****************************************************************************/
/* Copyright (c) 2006 Calidris **********************************************/
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

#define MAX_LIMIT		1024

BOOL ELD_AnalyzePeaks_old(OBJ* pObj, LPELD pEld, const EldAritemVec &vPeaks, int nPeaks,
						  EldAritemVec &vOutDist, int nClosestDist)
{
	int i, j, l, n0, n1, nCurSz, nClosestDist2;
	const ARITEM *pPeaks = &*vPeaks.begin();
	LPARITEM pRefls;
	ARITEM ait;
	float x, y, x_i, y_i, d, h;

	// the last valid index (cannot be larger than MAX_LIMIT)
	nCurSz = 0;
	// the square min distance
	nClosestDist2 = SQR(nClosestDist);
	// basic pointer, NULL at the start because vOutDist is empty right now
	pRefls = &*vOutDist.begin();
#ifdef _DEBUG
//	FILE *pFile = fopen("D:\\working\\eld\\peaks.dat", "wt");
//	::OutputDebugString("----- peaks -----\n");
//	for(i = 0; i < nPeaks; i++)
//	{
//		sprintf(TMP, "%.4f\t%.4f\t%.4f\t%d\n", pPeaks[i].x, pPeaks[i].y,
//			pPeaks[i].as, (int)pPeaks[i].n);
//		if( pFile ) fprintf(pFile, TMP);
//		::OutputDebugString(TMP);
//	}
//	if( pFile ) fclose(pFile);
//	::OutputDebugString("-----\n");
#endif
	// ANALYZE the reflections list
	for(i = 0; i < nPeaks-1; i++)
	{
		x_i = pPeaks[i].x;
		y_i = pPeaks[i].y;
		for(j = i+1; j < nPeaks; j++)
		{
			x = x_i - pPeaks[j].x;
			y = y_i - pPeaks[j].y;
			// Commented by Peter
			h = FastSqrt(x*x + y*y);
			if( x <= 0 )
			{
				x = -x;
				y = -y;
			}
//			sprintf(TMP, "a=%d, l=%d, s=%d\n", a, l, vOutDist.size());
//			::OutputDebugString(TMP);
			// initialize the distance
			d = 0;
			// find the closest
			for(l = 0; l < nCurSz; l++)
			{
				// calculate the distance
				d = SQR(pRefls[l].x - x) + SQR(pRefls[l].y - y);
				// compare it
				if( d < nClosestDist2 )
					break;
			}
			// not found?
			if( (l == nCurSz) || (d < 1) )
			{
				if( l < vOutDist.size() )
				{
					n0 = pRefls[l].n;
					pRefls[l].n = n1 = n0+1;
					pRefls[l].x = (pRefls[l].x * n0 + x) / n1;
					pRefls[l].y = (pRefls[l].y * n0 + y) / n1;
					pRefls[l].l = (pRefls[l].l * n0 + h) / n1;
					pRefls[l].nOthers = 0;
				}
				// if we need to add one more
				else if( nCurSz < MAX_LIMIT )
				{
					n0 = 0;
					ait.n = n1 = 1;
					ait.x = x;
					ait.y = y;
					ait.l = h;
					ait.nOthers = 0;
					vOutDist.push_back(ait);
					// re-initialize the start pointer
					pRefls = &*vOutDist.begin();
					nCurSz++;
				}
			}
		}
	}
	return TRUE;
}

//------------------------------------------------------------------
static int __cdecl f_cmp(const void* x1, const void* x2)
{
	LPARITEM	R1 = (LPARITEM)x1, R2 = (LPARITEM)x2;
	if( R1->l  > R2->l)  return  1;
	if( R1->l  < R2->l)  return -1;
	return 0;
}
//------------------------------------------------------------------
static int __cdecl f_cmp_n(const void* x1, const void* x2)
{
	LPARITEM	R1 = (LPARITEM)x1, R2 = (LPARITEM)x2;
	if(R1->n  > R2->n)  return(-1);
	if(R1->n  < R2->n)  return(1);
	return(0);
}
//------------------------------------------------------------------
static int __cdecl Others_cmp(const void* x1, const void* x2)
{
	LPARITEM	R1 = (LPARITEM)x1, R2 = (LPARITEM)x2;
	if( R1->nOthers < R2->nOthers )  return  1;
	if( R1->nOthers > R2->nOthers )  return -1;
	return 0;
}

BOOL ELD_EstimateAB_old(LPELD pEld, EldAritemVec &vDists)
{
//	return TRUE;
	LPARITEM pDist;
	int i, j, pp, pp2, nLimit, nSize;
	double r0, x0, y0, s, x, y, xx, yy;
	static const int hi[4] = {0, 1, 1, -1}, ki[4] = {1, 0, 1, 1};

	nSize = vDists.size();
	pDist = &*vDists.begin();
	qsort(pDist, nSize, sizeof(ARITEM), f_cmp_n);
#ifdef _DEBUG
//	::OutputDebugString("----- vectors -----\n");
//	for(i = 0; i < nSize; i++)
//	{
//		sprintf(TMP, "%.4f\t%.4f\t%.4f\t%d\n", pDist[i].x, pDist[i].y, pDist[i].l, (int)pDist[i].n);
//		::OutputDebugString(TMP);
//	}
//	::OutputDebugString("-----\n");
#endif
	// 10% is the lower limit for a distance
	nLimit = pDist[0].n * 0.1;
	if( nLimit < 2 )
	{
		nLimit = 2;
	}
	// End of addition by Peter on 03 Apr 2006
	for(i = 0; i < nSize; i++)
	{
		if( pDist[i].n < nLimit )
			break;
	}
	nSize = i;
	qsort(pDist, nSize, sizeof(ARITEM), f_cmp);
	for(i = 0; i < nSize; i++)
	{
		pDist[i].a = R2D(ATAN2(pDist[i].y, pDist[i].x));
		pDist[i].l = FastSqrt(SQR(pDist[i].x) + SQR(pDist[i].y));
	}
//	ELD_AnalyzeCells(pDist, nSize, vRedCells, vNewRedCells);
//
//	return TRUE;
 	for(i = 0; i < nSize; i++)
	{
		if( pDist[i].n > 4 )
		{
			pEld->m_h_dir.x = pDist[i].x;
			pEld->m_h_dir.y = pDist[i++].y;
			break;
		}
	}
	for( ; i < nSize; i++)
	{
		if( pDist[i].n > 4 )
		{
			pp = R2P(ATAN2(pEld->m_h_dir.x, pEld->m_h_dir.y))*2;
			pp2 = R2P(ATAN2(pDist[i].x, pDist[i].y))*2;
			if( FastAbs(P2D(pp - pp2)) < 30 )
				continue;
			pEld->m_k_dir.x = pDist[i].x;
			pEld->m_k_dir.y = pDist[i].y;
			break;
		}
	}
	if( i == nSize )
		return FALSE;

	for(r0 = 1000, i = 0; i < 4; i++)
	{
		x0 = hi[i]*pEld->m_h_dir.x + ki[i]*pEld->m_k_dir.x;
		y0 = hi[i]*pEld->m_h_dir.y + ki[i]*pEld->m_k_dir.y;
		if( (s = FastSqrt(x0*x0 + y0*y0)) < r0 )
		{
			r0 = s;
			x = x0;
			y = y0;
			j = i;
		}
	}
	for(r0 = 1000, i = 0; i < 4; i++)
	{
		if( i == j )
			continue;
		x0 = hi[i]*pEld->m_h_dir.x + ki[i]*pEld->m_k_dir.x;
		y0 = hi[i]*pEld->m_h_dir.y + ki[i]*pEld->m_k_dir.y;
		if( (s = FastSqrt(x0*x0 + y0*y0)) < r0 )
		{
			r0 = s;
			xx = x0;
			yy = y0;
		}
	}
	pEld->m_h_dir.x = x;
	pEld->m_h_dir.y = y;
	pEld->m_k_dir.x = xx;
	pEld->m_k_dir.y = yy;
	return TRUE;
}
