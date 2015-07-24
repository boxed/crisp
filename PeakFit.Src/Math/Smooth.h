// Smooth.h: interface for the CSmooth class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMOOTH_H__F2CB0CC1_07EB_11D5_95F7_00010296D95E__INCLUDED_)
#define AFX_SMOOTH_H__F2CB0CC1_07EB_11D5_95F7_00010296D95E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace Smooth
{
	bool SavitzkyGolay(double *pYdata, int nPoints, double *pOutYdata,
		int nPtsLeft, int nPtsRight, int nDeriv, bool bWrap);

	bool SavitzkyGolay(std::vector<double> &Ydata, std::vector<double> &OutYdata,
		int nPtsLeft, int nPtsRight, int nDeriv, bool bWrap);
}; // namespace Smooth

#endif // !defined(AFX_SMOOTH_H__F2CB0CC1_07EB_11D5_95F7_00010296D95E__INCLUDED_)
