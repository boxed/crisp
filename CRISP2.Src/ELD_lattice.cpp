/****************************************************************************/
/* Copyright (c) 2006 Calidris **********************************************/
/****************************************************************************/
/* CRISP2.ELD ***************************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "objects.h"
#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"
#include "rgconfig.h"
#include "eld.h"
#include "EDRD.h"

#include "Crash.h"
#include "HelperFn.h"

#define MAX_LIMIT		1024

extern int s_nTab;

//bool DoAutoHOLZrefinement(Pt2Vector &vPtObs, Pt2Vector &vPtCalc, LPELD lpEld,
//						  VectorD &vParamsOut);
//BOOL ELD_EstimateAB(LPELD pEld, EldAritemVec &vDists,
//					const LatCellVec &vRedCells, LatCellVec &vNewRedCells);

typedef struct
{
	const LATTICE_CELL *pCell;
	int nMissed;
	int nAccepted;
	double dSumAmpl;
	double dRatio1;
	double dRatio2;
	CVector2d h_dir;
	CVector2d k_dir;
	CVector2d center;
	CVector2d mass_center;
	double a_axis;
	double b_axis;
	BOOL bValid;

} RED_CELL_RATE;

typedef std::vector<RED_CELL_RATE> RedCellRateVec;
typedef std::vector<RED_CELL_RATE>::iterator RedCellRateVecIt;

struct sort_rate_Ratio1 : std::binary_function<bool, RED_CELL_RATE, RED_CELL_RATE>
{
	bool operator ()(const RED_CELL_RATE &p1, const RED_CELL_RATE &p2) const
	{
		if( FALSE == p1.bValid )
		{
			return false;
		}
		return (p1.dRatio1 > p2.dRatio1) ? true : false;
	}
};

struct sort_rate_Ratio2 : std::binary_function<bool, RED_CELL_RATE, RED_CELL_RATE>
{
	bool operator ()(const RED_CELL_RATE &p1, const RED_CELL_RATE &p2) const
	{
		if( FALSE == p1.bValid )
		{
			return false;
		}
		return (p1.dRatio2 > p2.dRatio2) ? true : false;
	}
};

struct sort_aritem_ampl : std::binary_function<bool, ARITEM, ARITEM>
{
	bool operator ()(const ARITEM &p1, const ARITEM &p2) const
	{
		return (p1.as > p2.as) ? true : false;
	}
};

BOOL ELD_TryToIndexLatt(OBJ* pObj, LPELD pEld, LATTICE_CELL *pCell,
						RED_CELL_RATE &rate, double dRadius, bool bChooseAxes = true)
{

	CVector2d h_dir, k_dir, center;
	double diff_a_ra, diff_a_rb, diff_b_ra, diff_b_rb, min_diff;
	double x, y, red_a, red_b, gamma, dSin, dCos, rad, rad_2, dLimitR2;
	int h, k, nStartH, nFinishH, nStartK, nFinishK, imgw, imgh;
	float xx, yy;
	double ampl;

	// fast access
	red_a = pCell->cell[0];
	red_b = pCell->cell[1];
	gamma  = pCell->cell[5];
	FastSinCos(gamma, dSin, dCos);
	dLimitR2 = SQR(dRadius);
	imgw = pEld->m_image.w;
	imgh = pEld->m_image.h;
	rate.bValid = FALSE;
	rate.pCell = pCell;
	rate.a_axis = red_a;
	rate.b_axis = red_b;
	rate.dSumAmpl = 0.0;
	rate.nMissed = 0;
	rate.nAccepted = 0;
	diff_a_ra = FastAbs(pCell->a_axis - red_a);
	diff_a_rb = FastAbs(pCell->a_axis - red_b);
	diff_b_ra = FastAbs(pCell->b_axis - red_a);
	diff_b_rb = FastAbs(pCell->b_axis - red_b);
	min_diff = __min(__min(diff_a_ra,diff_a_rb), __min(diff_b_ra,diff_b_rb));
	if( min_diff > 1.0 )
	{
		return FALSE;
	}

	int i;
	int nCycles = 4;
	int anMissed[4] = {0, 0, 0, 0}, anAccepted[4] = {0, 0, 0, 0};
	double adSumAmpl[4] = {0, 0, 0, 0}, adRatio1[4] = {0, 0, 0, 0}, adRatio2[4] = {0, 0, 0, 0};
	double adSumX[4] = {0, 0, 0, 0}, adSumY[4] = {0, 0, 0, 0};
	CVector2d vHaxis[4], vKaxis[4], vCenter[4];
	if( true == bChooseAxes )
	{
		vHaxis[0] = CVector2d(pCell->h_dir.y, -pCell->h_dir.x);
		vHaxis[1] = CVector2d(pCell->h_dir.y, -pCell->h_dir.x);
		vHaxis[2] = CVector2d(pCell->k_dir.y, -pCell->k_dir.x);
		vHaxis[3] = CVector2d(pCell->k_dir.y, -pCell->k_dir.x);

		vKaxis[0].x =  vHaxis[0].x * dCos - vHaxis[0].y * dSin;
		vKaxis[0].y =  vHaxis[0].x * dSin + vHaxis[0].y * dCos;
		// -
		vKaxis[1].x =  vHaxis[1].x * dCos + vHaxis[1].y * dSin;
		vKaxis[1].y = -vHaxis[1].x * dSin + vHaxis[1].y * dCos;
		// -
		vKaxis[2].x =  vHaxis[2].x * dCos - vHaxis[2].y * dSin;
		vKaxis[2].y =  vHaxis[2].x * dSin + vHaxis[2].y * dCos;
		// -
		vKaxis[3].x =  vHaxis[3].x * dCos + vHaxis[3].y * dSin;
		vKaxis[3].y = -vHaxis[3].x * dSin + vHaxis[3].y * dCos;
		nCycles = 4;
	}
	else
	{
		vHaxis[0] = pCell->h_dir;
		vKaxis[0] = pCell->k_dir;
		nCycles = 1;
	}
	// initial values for cycles
	nStartH  = (int)::floor(dRadius / red_a + 0.5);
	nFinishH = (int)::floor(dRadius / red_a + 0.5);
	nStartK  = (int)::floor(dRadius / red_b + 0.5);
	nFinishK = (int)::floor(dRadius / red_b + 0.5);
	rad = (int)::floor(0.5*__min(red_a, red_b));
	rad_2 = SQR(0.5 * rad);

	// try to see which lattice directions match
	for(i = 0; i < nCycles; i++)
	{
		double accepted_missed_by = 0;

		h_dir = red_a * vHaxis[i];
		k_dir = red_b * vKaxis[i];

		//Trace(_FN("ELD_TryToIndexLatt h: %.1f\tk: %.1f") % h_dir.Length() % k_dir.Length());

		// do some center adjustments
		center = pCell->center;
// 		if( false == ELD_FindClosestCentre(pEld->m_mass_center, h_dir, k_dir, center) )
		if( false == pEld->ELD_FindClosestCenter(pEld->m_mass_center, h_dir, k_dir, center) )
		{
			center = pCell->center;
		}
		vCenter[i] = center;
		for(k = -nFinishK-1; k <= nFinishK+1; k += 1)
		{
			for(h = -nFinishH-1; h <= nFinishH+1; h += 1)
			{
				// current point in the coordinate system of EDP
				x = h * h_dir.x + k * k_dir.x;
				y = h * h_dir.y + k * k_dir.y;
				if( x*x + y*y > dLimitR2 )
				{
					continue;
				}
				xx = (x += center.x);
				yy = (y += center.y);
				if( (xx < 0) || (xx >= imgw) || (yy < 0) || (yy >= imgh) )
				{
					continue;
				}

				double as = 0;
				if( !pEld->PeakCenter(rad, xx, yy, rad, 0.5, TRUE, ampl, as) )
				{
					anMissed[i]++;
					continue;
				}
				if( SQR(xx - x) + SQR(yy - y) > rad_2 )
				{
					anMissed[i]++;
					continue;
				}
				accepted_missed_by += SQR(xx - x) + SQR(yy - y);
				anAccepted[i]++;
				adSumAmpl[i] += ampl;
				adSumX[i] += ampl * xx;
				adSumY[i] += ampl * yy;
			}
		}
		if( 0 != anMissed[i] + anAccepted[i] )
		{
			adRatio1[i] = adSumAmpl[i] / (anMissed[i] + anAccepted[i]);
			adRatio2[i] = (anAccepted[i] * adRatio1[i])/accepted_missed_by;
		}
	}
	int nBest = 0;
	double dBest = adRatio2[0];
	for(i = 0; i < nCycles; i++)
	{
		if( adRatio2[i] > dBest )
		{
			nBest = i;
			dBest = adRatio2[i];
		}
	}

	rate.mass_center = CVector2d(adSumX[nBest] / adSumAmpl[nBest], adSumY[nBest] / adSumAmpl[nBest]);
	rate.nMissed = anMissed[nBest];
	rate.nAccepted = anAccepted[nBest];
	rate.dSumAmpl = adSumAmpl[nBest];
	rate.dRatio1 = adRatio1[nBest];
	rate.dRatio2 = adRatio2[nBest];
	rate.h_dir = Normalize(vHaxis[nBest]);
	rate.k_dir = Normalize(vKaxis[nBest]);
	rate.center = vCenter[nBest];
	pCell->center = vCenter[nBest];

	Trace(Format("TryToIndex: accepted:%1% missed:%2% ratio1:%3% ratio2:%4% center:%5%,%6%") % rate.nAccepted % rate.nMissed % rate.dRatio1 % rate.dRatio2 % rate.center.x % rate.center.y); 
	if( 0 != rate.nMissed + rate.nAccepted )
	{
		rate.bValid = TRUE;
	}
	return TRUE;

}

static BOOL ELD_TryToIndex4(OBJ* pObj, LPELD pEld,
							LatCellVec &vRedCells,
							RedCellRateVec &vRedCellRates,
							double dLimitR, BOOL bCheckR)
{

	LatCellVecIt cit;
	RED_CELL_RATE rate;

	// clear just in case
	vRedCellRates.clear();
	// no cells???
	if( true == vRedCells.empty() )
	{
		return FALSE;
	}
	// the limiting radius
	if( false == bCheckR )
	{
		dLimitR = __max(pEld->m_image.w, pEld->m_image.h);
	}
	Trace(_T("trying all reduced cells..."));

	for(cit = vRedCells.begin(); cit != vRedCells.end(); ++cit)
	{
		if( TRUE == ELD_TryToIndexLatt(pObj, pEld, &*cit, rate, dLimitR) )
		{
			if( rate.nAccepted > 3 )
			{
				vRedCellRates.push_back(rate);
			}
		}
	}

	// sort cell rates
	std::sort(vRedCellRates.begin(), vRedCellRates.end(), sort_rate_Ratio2());
	Trace(_T("------- Rates -------"));
	for(RedCellRateVecIt rit = vRedCellRates.begin(); rit != vRedCellRates.end(); ++rit)
	{
		Trace(_FMT(_T("red. cell %g\t%g\t%g\t"
			"mis.=%d, acc.=%d, A_sum=%g, A/peak=%g, A*acc=%g")
			, rit->pCell->cell[1]
			, rit->pCell->cell[0]
			, RAD2DEG(rit->pCell->cell[5])
			, rit->nMissed
			, rit->nAccepted
			, rit->dSumAmpl
			, rit->dRatio1
			, rit->dRatio2));
	}
	return TRUE;

}

static BOOL ELD_SetBestAxes(CVector2d &h_dir, CVector2d &k_dir)
{

	double a_len, b_len, a_slope, b_slope, gamma;

	a_len = (!h_dir);
	b_len = (!k_dir);
	// equal axes length? 10% marginal
	if( FastAbs(a_len - b_len) < 0.1*__min(a_len, b_len) )
	{
		// choose as A* that axis which is closer to +X
		a_slope = FastAbs(FastAcos(FastAbs(h_dir.x) / a_len));
		b_slope = FastAbs(FastAcos(FastAbs(k_dir.x) / b_len));
		if( a_slope > M_PI ) a_slope = M_2PI - a_slope;
		if( b_slope > M_PI ) b_slope = M_2PI - b_slope;
		// swap them if b_slope is smaller
		if( b_slope <= a_slope )
		{
			std::swap(h_dir, k_dir);
		}
		// make it positive
		if( h_dir.x < 0.0 )
		{
			h_dir = -h_dir;
		}
		// the angle between two directions
		gamma = FastAcos( (h_dir & k_dir) / (a_len * b_len));
		if( FastAbs(RAD2DEG(FastAbs(gamma)) - 120.0) < 2.0 )
		{
		}
		else if( FastAbs(RAD2DEG(FastAbs(gamma)) -  60.0) < 2.0 )
		{
			// set the k-dir so it will be counter-clockwise
		}
		else if( FastAbs(RAD2DEG(FastAbs(gamma)) -  90.0) < 2.0 )
		{
		}
		else
		{
		}
	}
	else
	{
		// make a-axis to be the smallest
		if( a_len > b_len )
		{
			std::swap(h_dir, k_dir);
		}
		if( h_dir.x < 0.0 )
		{
			h_dir = -h_dir;
		}
		gamma = FastAcos( (h_dir & k_dir) / (a_len * b_len));
		if( FastAbs(RAD2DEG(FastAbs(gamma)) -  90.0) < 2.0 )
		{
			if( k_dir.y > 0.0 )
			{
				k_dir = -k_dir;
			}
		}
		else if( gamma > 0.0 )
		{
			k_dir = -k_dir;
		}
	}
	return TRUE;

}

// BOOL ELD_AnalyzePeaks(OBJ* pObj, LPELD pEld, const EldAritemVec &vPeaks,
// 					  double dLimitR, BOOL bCheckR)
BOOL ELD::ELD_AnalyzePeaks(const EldAritemVec &vPeaks, double dLimitR, BOOL bCheckR)
{
	//Trace(_FN("ELD_AnalyzePeaks: %1% peaks") % vPeaks.size());
	LatCellVec vRedCells, vNewRedCells;
	RedCellRateVec vRedCellRates;
	const int N_RATES = 3;
	RED_CELL_RATE *pRate, rates[N_RATES];
	LATTICE_CELL lattice;
	int iCell, iRates, iCurRate;
	CVector2d center, mass_center;
	BOOL bRes;

	iRates = 0;
	if( false == bCheckR )
	{
		dLimitR = __max(this->m_image.w, this->m_image.h);
	}
	memset(rates, 0, sizeof(rates));
	if( false == ELD_FftIndex(vPeaks, vRedCells, 180) )
	{
		return FALSE;
	}
	if( true == vRedCells.empty() )
	{
		return FALSE;
	}

	if( TRUE == ELD_TryToIndex4(m_pObj, this, vRedCells, vRedCellRates, dLimitR, bCheckR) )
	{
		// take N_RATES top-most cells and try to refine them
		iRates = 0;
		if( true == vRedCellRates.empty() )
		{
			return FALSE;
		}
		pRate = &*vRedCellRates.begin();
		for(iCell = 0; iCell < vRedCellRates.size(); iCell++, pRate++)
		{
			CVector2d h_dir = pRate->h_dir * pRate->a_axis;
			CVector2d k_dir = pRate->k_dir * pRate->b_axis;
			CVector2d center = pRate->center;
			for(int iPass = 0; iPass < ELD_MAX_PASSES_LATREF; iPass++)
			{
				if( FALSE == (bRes = ELD_RefineLattice(iPass, 0.5, dLimitR, bCheckR)) )
				{
					break;
				}
			}
			// if all passes succeeded
			if( TRUE == bRes )
			{
				lattice.center = center;
				lattice.h_dir = Normalize(h_dir);
				lattice.k_dir = Normalize(k_dir);
				lattice.cell[0] = !h_dir;
				lattice.cell[1] = !k_dir;
				lattice.cell[5] = FastAcos(lattice.h_dir & lattice.k_dir);
				lattice.a_axis = lattice.cell[0];
				lattice.b_axis = lattice.cell[1];
				if( TRUE == (bRes = ELD_TryToIndexLatt(m_pObj, this, &lattice, rates[iRates], dLimitR, false)) )
				{
					rates[iRates].pCell = pRate->pCell;
					// enough with rates?
					if( N_RATES == ++iRates )
					{
						break;
					}
				}
			}
		}
	}

	// nothing good was found
	if( 0 == iRates )
	{
		return FALSE;
	}
	// sort to get the highest scores on the top
	std::sort(&rates[0], &rates[iRates], sort_rate_Ratio2());
	Trace(_T("------- Rates after the refinement -------"));
	for(pRate = &rates[0]; pRate != &rates[iRates]; ++pRate)
	{
		if( TRUE == pRate->bValid )
		{
			Trace(_FMT(_T("red. cell %g\t%g\t%g\t"
				"mis.=%d, acc.=%d, A_sum=%g, A/peak=%g, A*acc=%g")
				, pRate->pCell->cell[1]
				, pRate->pCell->cell[0]
				, RAD2DEG(pRate->pCell->cell[5])
				, pRate->nMissed
				, pRate->nAccepted
				, pRate->dSumAmpl
				, pRate->dRatio1
				, pRate->dRatio2));
		}
	}
	// pick up the best cell
	pRate = rates;

	try
	{
		m_center = GetImageCenter(m_image);
	}
	catch (std::exception& e)
	{
		Trace(Format(_T("Failed to get image center %1%")) % e.what());
	}

	/*for(iCurRate = 0; iCurRate < iRates; iCurRate++, pRate++)
	{
		Trace(_FN("center: %1% %2%") % pRate->center.x % pRate->center.y);
	}*/
	pRate = rates;
	for(iCurRate = 0; iCurRate < iRates; iCurRate++, pRate++)
	{
		if( TRUE == pRate->bValid )
		{
			m_h_dir = pRate->h_dir * pRate->a_axis;
			m_k_dir = pRate->k_dir * pRate->b_axis;
			ELD_SetBestAxes(m_h_dir, m_k_dir);
			pRate->a_axis = m_pCurLZ->a_length = !m_h_dir;
			pRate->b_axis = m_pCurLZ->b_length = !m_k_dir;
			if( (!(pRate->mass_center - m_mass_center) > __max(pRate->a_axis, pRate->b_axis)) )
			{
				ELD_FindClosestCenter(mass_center, m_h_dir, m_k_dir, center);
			}
			break;
		}
	}

	// no good were found?
	if( N_RATES == iCurRate )
	{
		return FALSE;
	}
	return TRUE;

}

// BOOL ELD_RefineLattice(OBJ* pObj, LPELD pEld, int nPass, double dErrPrec,
// 					   double dLimitR, BOOL bCheckR, EldAritemVec *pvPeaks/* = NULL*/)
BOOL ELD::ELD_RefineLattice(int nPass, double dErrPrec, double dLimitR,
							BOOL bCheckR, EldAritemVec *pvPeaks/* = NULL*/)
{

	Pt2Vector vPtObs, vPtCalc;
	CVector2d _center, point;
	double axis_a, axis_b, rad, rad_2, CurRad2, det, X, Y, dLimitR2, dRad, as = 0;
	int nStartH, nFinishH, nStartK, nFinishK, imgw, imgh;
	VectorD vParams;
	float xx, yy;
	bool bRes;
	ARITEM refl;
	EldAritemVec vPeaks;
// 	DWORD ampl;
	double ampl;

	imgw = m_image.w;
	imgh = m_image.h;
	if( false == bCheckR )
	{
		dLimitR = __max(imgw, imgh);
	}
	dLimitR2 = SQR(dLimitR);
	_center = m_center;
	axis_a = !m_h_dir;
	axis_b = !m_k_dir;

	memset(&refl, 0, sizeof(refl));
	nStartH  = (int)::floor(dLimitR / axis_a + 0.5);
	nFinishH = (int)::floor(dLimitR / axis_a + 0.5);
	nStartK  = (int)::floor(dLimitR / axis_b + 0.5);
	nFinishK = (int)::floor(dLimitR / axis_b + 0.5);
	// the radius for the peak search
// 	rad = (int)::floor(0.5*__min(axis_a, axis_b));
// 	rad_2 = SQR(dErrPrec*rad);
// 	Rad = m_nPeakWindow;
// 	Rad_2 = SQR(Rad);
	//if (!m_pCurLZ->bAuto)
	{
		m_nPeakWindow = (int)::floor(0.5*__min(m_pCurLZ->a_length, m_pCurLZ->b_length) + 0.5);
	}

	rad = m_nPeakWindow;
	rad_2 = SQR(rad);

	Trace(_T("------------------------"));
	Trace(_FMT(_T("refinement cycle %d"), nPass));
	Trace(_FMT(_T("h=[-%d, %d], k=[-%d, %d]"), nStartH, nFinishH, nStartK, nFinishK));
	Trace(_FMT(_T("rad=%g"), rad));

	m_hkmax = __max(nFinishH, nFinishK);
	for(Y = -nFinishK-1; Y <= nFinishK+1; Y += 1.0)
	{
		for(X = -nFinishH-1; X <= nFinishH+1; X += 1.0)
		{
			// current point in the coordinate system of EDP
			point = m_pCurLZ->GetPeakRelXY(X, Y);
			if( point.Sqr() > dLimitR2 )
				continue;

			xx = (point.x += _center.x);
			yy = (point.y += _center.y);
			if( (xx < 0) || (xx >= imgw) || (yy < 0) || (yy >= imgh) )
				continue;

			m_hkmaxe = __max(m_hkmaxe, __max(abs(X), abs(Y)));
			CurRad2 = rad_2;
			if( !PeakCenter(rad, xx, yy, rad, 0.1f, TRUE, ampl, as) )
				continue;

			if( SQR(xx - point.x) + SQR(yy - point.y) > CurRad2 )
				continue;

			DrawMark(xx, yy, m_pCurLZ->hPen, -3);

			vPtCalc.push_back(CVector2d(X, Y));
			vPtObs.push_back(CVector2d(xx, yy));
			m_hkmaxe = __max(m_hkmaxe, __max(abs(X), abs(Y)));
			refl.as = as;
			refl.x = xx;
			refl.y = yy;
			vPeaks.push_back(refl);
			if( NULL != pvPeaks )
			{
				pvPeaks->push_back(refl);
			}
		}
	}
	m_pCurLZ->nLRefCnt = vPtObs.size();

	//Trace(_FMT(_T("points for refinement: %d"), m_pLZ->nLRefCnt));
	for(int i = 0; i < vPtObs.size(); i++)
	{
		point = m_pCurLZ->GetPeakRelXY(vPtCalc[i].x, vPtCalc[i].y);
		point += _center;

		/*Trace(Format("h:%1% k:%2% x:%3% y:%4% center x:%5% center y:%6% \t%7%")
			% (int)vPtCalc[i].x
			% (int)vPtCalc[i].y
			% vPtObs[i].x
			% vPtObs[i].y %	point.x% point.y
			% FastSqrt(SQR(vPtObs[i].x-point.x) + SQR(vPtObs[i].y-point.y)));
		*/
		//DrawMark(vPtObs[i].x, vPtObs[i].y, 2, 4);
	}

	// do the refinement
	m_pCurLZ->shift = 0;
	m_pCurLZ->a_length = axis_a;
	m_pCurLZ->b_length = axis_b;
	if( true == (bRes = DoAutoHOLZrefinement(vPtObs, vPtCalc, vParams)) )
	{
		X = vParams[6];
		Y = vParams[7];
		double gamma = FastAcos((m_h_dir & m_k_dir)/(m_h_dir.Length()*m_k_dir.Length()));
		m_pCurLZ->a_length = vParams[0];
		m_pCurLZ->b_length = vParams[1];
		dRad = vParams[2];
		m_h_dir = m_pCurLZ->a_length * CVector2d(FastCos(dRad), FastSin(dRad));
		dRad = vParams[3];
		m_k_dir = m_pCurLZ->b_length * CVector2d(FastCos(dRad), FastSin(dRad));
		dRad = (m_h_dir / (!m_h_dir)) & (m_k_dir / (!m_k_dir));
		if( dRad < -0.2 )
		{
			m_k_dir = -m_k_dir;
		}
		double new_gamma = FastAcos((m_h_dir & m_k_dir)/((!m_h_dir)*(!m_k_dir)));

		Trace(_T("refinement is done with:"));
		Trace(_FMT(_T("centreX old:%g new:%g"), vParams[4], (vParams[4]+X)));
		Trace(_FMT(_T("centreY old:%g new:%g"), vParams[5], (vParams[5]+Y)));
		//Trace(_FMT(_T("a old:%g new:%g"), pLZ->m_al, vParams[0]));
		//Trace(_FMT(_T("b old:%g new:%g"), pLZ->m_bl, vParams[1]));
		Trace(_FMT(_T("gamma old:%g new:%g"), RAD2DEG(gamma), RAD2DEG(new_gamma)));
		Trace(Format(_T("h-dir x:%1% y:%2%  length:%3%")) % m_h_dir.x % m_h_dir.y % sqrt(SQR(m_h_dir.x) + SQR(m_h_dir.y)));
		Trace(Format(_T("k-dir x:%1% y:%2%  length:%3%")) % m_k_dir.x % m_k_dir.y % sqrt(SQR(m_k_dir.x) + SQR(m_k_dir.y)));
		
		det = m_h_dir.y * m_k_dir.x - m_h_dir.x * m_k_dir.y;
		if( FastAbs(det) > 1e-4 )
		{
			_center = CVector2d(vParams[4] + X, vParams[5] + Y);
			m_center = _center;
			protocolLatAxies(vPeaks.size(), nPass);
			// store the nPass data
			m_aPasses[nPass].vCenter = _center;
			m_aPasses[nPass].vAxisH = m_h_dir;
			m_aPasses[nPass].vAxisK = m_k_dir;
			m_aPasses[nPass].nRefls = vPeaks.size();
		}
		SetELDparamsFromHOLZRef(vParams);
	}
	else
	{
			// store the nPass data
			m_aPasses[nPass].vCenter = _center;
			m_aPasses[nPass].vAxisH = m_h_dir;
			m_aPasses[nPass].vAxisK = m_k_dir;
			m_aPasses[nPass].nRefls = 3;
			bRes = TRUE;
	}
	Trace(_T("------------------------"));
	return (true == bRes) ? TRUE : FALSE;

}

// BOOL ELD_CalculateUserLattice(OBJ* pObj, LPELD pEld)
BOOL ELD::ELD_CalculateUserLattice()
{

	double det, sum_h2, sum_k2, sum_hk, sum_h, sum_k;
	CMatrix3x3 mtx, mtxi, mtx2;
	BOOL bRes;
	int j;

	try
	{
		// the determinant
		sum_h2 = SQR(m_pCurLZ->h[0]) + SQR(m_pCurLZ->h[1]) + SQR(m_pCurLZ->h[2]);
		sum_k2 = SQR(m_pCurLZ->k[0]) + SQR(m_pCurLZ->k[1]) + SQR(m_pCurLZ->k[2]);
		sum_hk = m_pCurLZ->h[0]*m_pCurLZ->k[0] + m_pCurLZ->h[1]*m_pCurLZ->k[1] + m_pCurLZ->h[2]*m_pCurLZ->k[2];
		sum_h  = m_pCurLZ->h[0] + m_pCurLZ->h[1] + m_pCurLZ->h[2];
		sum_k  = m_pCurLZ->k[0] + m_pCurLZ->k[1] + m_pCurLZ->k[2];
		mtx.d[0][0] = sum_h2;	mtx.d[0][1] = sum_hk;	mtx.d[0][2] = sum_h;
		mtx.d[1][0] = sum_hk;	mtx.d[1][1] = sum_k2;	mtx.d[1][2] = sum_k;
		mtx.d[2][0] = sum_h;	mtx.d[2][1] = sum_k;	mtx.d[2][2] = 3.0;

		Trace((boost::format("matrix %1% %2% %3% %4% %5% %6% %7% %8% %9%") 
			% mtx.d[0][0] % mtx.d[0][1] % mtx.d[0][2] 
			% mtx.d[1][0] % mtx.d[1][1] % mtx.d[1][2] 
			% mtx.d[2][0] % mtx.d[2][1] % mtx.d[2][2] 
		).str());

		det = mtx.Determinant();
		if( FastAbs(det) < 1e-3 )
		{
			MessageBox(m_pObj->hWnd, "Specified 3 indices are linearly dependent.", "ELD ERROR!", MB_OK);
			return FALSE;
		}
		for(j = 0; j < 3; j++)
		{
			mtx2.d[0][0] += m_pCurLZ->h[j]*m_pCurLZ->x[j];
			mtx2.d[0][1] += m_pCurLZ->k[j]*m_pCurLZ->x[j];
			mtx2.d[0][2] += m_pCurLZ->x[j];
			// ---
			mtx2.d[1][0] += m_pCurLZ->h[j]*m_pCurLZ->y[j];
			mtx2.d[1][1] += m_pCurLZ->k[j]*m_pCurLZ->y[j];
			mtx2.d[1][2] += m_pCurLZ->y[j];
		}
		mtx.Invert(mtxi);
		mtx = mtx2 * mtxi;
		m_pCurLZ->h_dir.x  = mtx.d[0][0];
		m_pCurLZ->k_dir.x  = mtx.d[0][1];
		m_pCurLZ->center.x = mtx.d[0][2];
		m_pCurLZ->h_dir.y  = mtx.d[1][0];
		m_pCurLZ->k_dir.y  = mtx.d[1][1];
		m_pCurLZ->center.y = mtx.d[1][2];
		// if the user has Zero Order Laue Zone circle - set the limiting radius
		double dLimitRad = 0.0;
		BOOL bCheckR = FALSE;
		// TODO: MinRad and MaxRad!!!
		if( (TRUE == m_bUseLaueCircles) && (0 == m_nCurLaueZone) )
		{
			if( true == m_vbLaueCircles[0] )
			{
				dLimitRad = m_vLaueZones[0]->m_dRadius;
				bCheckR = TRUE;
			}
		}
		double tolerance[4] = {0.6, 0.6, 0.5, 0.25};
		for(m_nCurPass = 0; m_nCurPass < ELD_MAX_PASSES_LATREF; m_nCurPass++)
		{
			Trace(_FMT(_T("\tpass %d..."), (m_nCurPass+1)));
			// on the last step fill the successfully detected peaks which lie close
			// to the lattice points. This will be used during the search of nearest 3 reflections.
			if( FALSE == (bRes = ELD_RefineLattice(m_nCurPass, tolerance[m_nCurPass], dLimitRad, bCheckR)) )
			{
				bRes = FALSE;
				Trace(_T("Failed to refine the lattice from user reflections!"));
				MessageBox(m_pObj->hWnd, "Failed to refine the lattice from user reflections!", "ELD ERROR!", MB_OK);
				break;
			}
		}
		if( TRUE == bRes )
		{
			// Finally refined successfully
			if( m_pCurLZ == m_pZOLZ )
			{
				m_bRefinedLattice = TRUE;
				EnableWindow(GetDlgItem(m_pObj->hWnd, IDC_ELD_DEFINE_HOLZ), m_bRefinedLattice);
			}
			m_pCurLZ->m_nFlags = LaueZone::ZONE_AUTO_INDEXED;
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("ELD_CalculateUserLattice() -> %s"), e.what()));
	}
	return bRes;
}
