// LUDecomp.h: interface for the CLUDecomp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

namespace LUDecomp
{
	bool LUDecomp(CMatrix &a, vector<int> &indx, double &d);
	bool LUBacksub(CMatrix &a, vector<int> &indx, CVector &b);
}; // namespace LUDecomp
