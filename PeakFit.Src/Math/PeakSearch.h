// PeakSearch.h: interface for the PeakSearch class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MathKrnl.h"
#include "..\Profile.h"

typedef struct PEAKSEARCHtag
{
	double	piPos;		// position of the peak along the profile
	int		piStart;	// index in the data array of start of the peak along the profile
	int		piEnd;		// index in the data array of end of the peak along the profile

} PEAKSEARCH, *LPPEAKSEARCH;

namespace PeakSearch
{
	bool ScanForPeaks(double *begin, double *end, vector<PEAKSEARCH> &vPeaks, int iStart);
	bool GradScanForPeaks(double *begin, double *end, vector<PEAKSEARCH> &vPeaks, int iStart);
	bool LokalMaxPeaksScan(double *begin, double *end, vector<PEAKSEARCH> &vPeaks, int iStart);

	bool FindPeaksSmoothDeriv(CProfile &profile, CProfile &peaks,
						  double min_rad, bool bSmooth);
}; // namespace PeakSearch
