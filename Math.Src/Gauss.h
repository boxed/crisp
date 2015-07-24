// Gauss.h: interface for the CGauss class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAUSS_H__A7C18EA0_9550_11D5_95F7_00010296D95E__INCLUDED_)
#define AFX_GAUSS_H__A7C18EA0_9550_11D5_95F7_00010296D95E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace GaussSolve
{
	bool Solve(CMatrix &a, int n, CMatrix &b, int m);
	bool Solve(CMatrix &a, CVector &b, int n);
	bool Solve(CMatrix &a, CVector &b);
}; // namespace GaussSolve

#endif // !defined(AFX_GAUSS_H__A7C18EA0_9550_11D5_95F7_00010296D95E__INCLUDED_)