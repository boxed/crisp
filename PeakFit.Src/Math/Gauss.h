// Gauss.h: interface for the CGauss class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

namespace GaussSolve
{
	bool Solve(CMatrix &a, int n, CMatrix &b, int m);
	bool Solve(CMatrix &a, CVector &b, int n);
	bool Solve(CMatrix &a, CVector &b);
	// extended solving - if some element(s) become ZERO
	bool SolveX(CMatrix &a, CVector &b, int n);
}; // namespace GaussSolve
