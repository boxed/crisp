// Gauss.cpp: implementation of the CGauss class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "MathKrnl.h"
#include "Gauss.h"
#include <algorithm>

using namespace std;
/*
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace GaussSolve
{

bool Solve(CMatrix &a, int n, CMatrix &b, int m)
{
	valarray<int>	indxc(n), indxr(n), ipiv(0, n);
	int i,icol,irow,j,k,l,ll;
	double big, pivinv, z;

	if( a.cols() != a.rows() )
	{
		printf("CGauss::Solve() -> a matrix is not square!\n");
		return false;
	}
	
	irow = icol = 0;
	big = 1.0;
	for (i = 0; i < n; i++)
	{
		big = 0.0;
		for (j = 0; j < n; j++)
		{
			if (ipiv[j] != 1)
			{
				for (k = 0; k < n; k++)
				{
					if (ipiv[k] == 0)
					{
						z = fabs(a[j][k]);
						if (z >= big)
						{
							big  = z;
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1)
					{
						printf("CGauss::Solve() -> singular matrix (1)\n");
						return false;
					}
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol)
		{
			swap_ranges(&a[irow][0], &a[irow][n], &a[icol][0]);
			swap_ranges(&b[irow][0], &b[irow][m], &b[icol][0]);
		}
		indxr[i] = irow;
		indxc[i] = icol;
		z = a[icol][icol];
		if (z == 0.0)
		{
			printf("CGauss::Solve() -> singular matrix (2)\n");
			return false;
		}
		pivinv = 1.0 / z;
		a[icol][icol] = 1.0;
		a[icol] *= pivinv;
		b[icol] *= pivinv;
		for (ll = 0; ll < n; ll++)
		{
			if (ll != icol)
			{
				z = a[ll][icol];
				a[ll][icol] = 0.0;
				a[ll] -= a[icol] * z;
				b[ll] -= b[icol] * z;
			}
		}
	}
	for (l = n-1; l >= 0; l--)
	{
		if (indxr[l] != indxc[l])
		{
			for (k = 0; k < n; k++)
				swap(a[k][indxr[l]],a[k][indxc[l]]);
		}
	}
	return true;
}

bool Solve(CMatrix &a, CVector &b, int n)
{
	valarray<int>	indxc(n), indxr(n), ipiv(0, n);
	int i,icol,irow,j,k,l,ll;
	double big, pivinv, z;

	if( (a.cols() < n) || (a.rows() < n) )
	{
		::OutputDebugString("CGauss::Solve() -> one of the sizes of the A matrix is less then n!\n");
		return false;
	}
	
	irow = icol = 0;
	big = 1.0;
	for (i = 0; i < n; i++)
	{
		big = 0.0;
		for (j = 0; j < n; j++)
		{
			if (ipiv[j] != 1)
			{
				for (k = 0; k < n; k++)
				{
					if (ipiv[k] == 0)
					{
						z = fabs(a[j][k]);
						if (z >= big)
						{
							big  = z;
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1)
					{
						::OutputDebugString("CGauss::Solve() -> singular matrix (1)\n");
						return false;
					}
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol)
		{
			swap_ranges(&a[irow][0], &a[irow][n], &a[icol][0]);
			swap(b[irow], b[icol]);
		}
		indxr[i] = irow;
		indxc[i] = icol;
		z = a[icol][icol];
		if (z == 0.0)
		{
			::OutputDebugString("CGauss::Solve() -> singular matrix (2)\n");
			z = 1e-6;
//			return false;
		}
		pivinv = 1.0 / z;
		a[icol][icol] = 1.0;
		a[icol] *= pivinv;
		b[icol] *= pivinv;
		for (ll = 0; ll < n; ll++)
		{
			if (ll != icol)
			{
				z = a[ll][icol];
				a[ll][icol] = 0.0;
				a[ll] -= a[icol] * z;
				b[ll] -= b[icol] * z;
			}
		}
	}
	for (l = n-1; l >= 0; l--)
	{
		if (indxr[l] != indxc[l])
		{
			for (k = 0; k < n; k++)
				swap(a[k][indxr[l]],a[k][indxc[l]]);
		}
	}
	return true;
}

bool Solve(CMatrix &a, CVector &b)
{
	int n,i,icol,irow,j,k,l,ll;
	double big, pivinv, z;

	if( a.cols() != a.rows() )
	{
		::OutputDebugString("CGauss::Solve() -> a matrix is not square!\n");
		return false;
	}

	n = a.cols();
	
	valarray<int>	indxc(n), indxr(n), ipiv(0, n);

	irow = icol = 0;
	big = 1.0;
	for (i = 0; i < n; i++)
	{
		big = 0.0;
		for (j = 0; j < n; j++)
		{
			if (ipiv[j] != 1)
			{
				for (k = 0; k < n; k++)
				{
					if (ipiv[k] == 0)
					{
						z = fabs(a[j][k]);
						if (z >= big)
						{
							big  = z;
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1)
					{
						::OutputDebugString("CGauss::Solve() -> singular matrix (1)\n");
						return false;
					}
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol)
		{
			swap_ranges(&a[irow][0], &a[irow][n], &a[icol][0]);
			swap(b[irow], b[icol]);
		}
		indxr[i] = irow;
		indxc[i] = icol;
		z = a[icol][icol];
		if (z == 0.0)
		{
			::OutputDebugString("CGauss::Solve() -> singular matrix (2)\n");
			pivinv = 1.0;
//			return false;
		}
		else
			pivinv = 1.0 / z;
		a[icol][icol] = 1.0;
		a[icol] *= pivinv;
		b[icol] *= pivinv;
		for (ll = 0; ll < n; ll++)
		{
			if (ll != icol)
			{
				z = a[ll][icol];
				a[ll][icol] = 0.0;
				a[ll] -= a[icol] * z;
				b[ll] -= b[icol] * z;
			}
		}
	}
	for (l = n-1; l >= 0; l--)
	{
		if (indxr[l] != indxc[l])
		{
			for (k = 0; k < n; k++)
				swap(a[k][indxr[l]],a[k][indxc[l]]);
		}
	}
	return true;
}

}; // namespace GaussSolve

