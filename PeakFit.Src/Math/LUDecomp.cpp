// LUDecomp.cpp: implementation of the CLUDecomp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#include "Math\MathKrnl.h"
#include "LUDecomp.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace LUDecomp
{

#define TINY 1.0e-20			// A small number.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
Given a matrix a[1..n][1..n], this routine replaces it by the LU decomposition of a row wise
permutation of itself. a and n are input. a is output, arranged as in equation (2.3.14) above;
indx[1..n] is an output vector that records the row permutation effected by the partial
pivoting; d is output as +/-1 depending on whether the number of row interchanges was even
or odd, respectively. This routine is used in combination with lubksb to solve linear equations
or invert a matrix.
*/
bool LUDecomp(CMatrix &a, vector<int> &indx, double &d)
{
	int n,i,imax,j,k;
	double big,dum,sum,temp;
	CVector vv;

	n = a.rows();
	if(n != a.cols())
	{
		printf("CLUDecomp::LUDecomp() -> non-square matrix!\n");
		return false;
	}
	
	indx.resize(n);
	vv.resize(n);
	d = 1.0;						// No row interchanges yet.

	for (i = 0; i < n; i++)			// Loop over rows to get the implicit scaling information.
	{
		big = 0.0;
		for (j = 0; j < n; j++)
		{
			// TODO;
			if ((temp = fabs(a[i][j])) > big)
				big=temp;
		}
		if (fabs(big) <= (double) TINY) 
		{
			printf("CLUDecomp::LUDecomp() -> Singular matrix!\n");
			return false;
		}
		// No nonzero largest element.
		vv[i] = 1.0 / big;			// Save the scaling.
	}
	for (j = 0; j < n; j++)			// This is the loop over columns of Crout's method.
	{
		for (i = 0; i < j; i++)		// This is equation (2.3.12) except for i = j.
		{
			sum = a[i][j];
			for (k = 0; k < i; k++)
				sum -= a[i][k]*a[k][j];
			a[i][j] = sum;
		}
		big = 0.0;					// Initialize for the search for largest pivot element.
		for (i = j; i < n; i++)		// This is i = j of equation (2.3.12) and i = j +1: ::N
		{
			sum=a[i][j];			// of equation (2.3.13).
			for (k = 0; k < j; k++)
				sum -= a[i][k]*a[k][j];
			a[i][j] = sum;
			// TODO:
			if ( (dum = vv[i]*fabs(sum)) >= big)
			{						// Is the figure of merit for the pivot better than the best so far?
				big=dum;
				imax=i;
			}
		}
		if (j != imax)				// Do we need to interchange rows?
		{
			for (k = 0; k < n; k++)	// Yes, do so...
			{
				dum = a[imax][k];
				a[imax][k] = a[j][k];
				a[j][k] = dum;
			}
			d = -d;					// ...and change the parity of d.
			vv[imax] = vv[j];		// Also interchange the scale factor.
		}
		indx[j] = imax;
		if (fabs(a[j][j]) <= (double) TINY)
			a[j][j]=TINY;
		// If the pivot element is zero the matrix is singular (at least to the precision of the
		// algorithm). For some applications on singular matrices, it is desirable to substitute
		// TINY for zero.
		if (j != (n-1))					// Now, finally, divide by the pivot element.
		{
			dum = 1.0/(a[j][j]);
			for (i = j+1; i < n; i++)
				a[i][j] *= dum;
		}
	}

	return true;
}
/*
Solves the set of n linear equations A . X = B. Herea[1..n][1..n] is input, not as the matrix
A but rather as its LU decomposition, determined by the routine ludcmp. indx[1..n] is input
as the permutation vector returned by ludcmp. b[1..n] is input as the right-hand side vector
B, and returns with the solution vector X. a, n, andindx are not modified by this routine
and can be left in place for successive calls with different right-hand sides b. This routine takes
into account the possibility that b will begin with many zero elements, so it is e.cient for use
in matrix inversion.
*/
bool LUBacksub(CMatrix &a, vector<int> &indx, CVector &b)
{
	int i,ii = -1,ip,j,n;
	double sum;

	n = a.rows();
	if(n != a.cols() || b.size() != n || indx.size() != n)
	{
		printf("CLUDecomp::LUDecomp() -> non-square matrix!\n");
		return false;
	}
	
	for (i = 0; i < n; i++)				// When ii is set to a positive value, it will become the
	{
		ip = indx[i];					// index of the first nonvanishing element of b. We now
		sum = b[ip];					// do the forward substitution, equation (2.3.6). The
		b[ip] = b[i];					// only new wrinkle is to unscramble the permutation
		if (ii >= 0)					// as we go.
		{
			for (j = ii; j <= (i-1); j++)
				sum -= a[i][j]*b[j];
		}
		else
		{
			if (sum)
				ii = i;					// A nonzero element was encountered, so from now on we
		}								// will have to do the sums in the loop above.
		b[i] = sum;
	}
	for (i = n-1; i >= 0; i--)			// Now we do the backsubstitution, equation (2.3.7).
	{
		sum = b[i];
		for (j = i+1; j < n; j++)
			sum -= a[i][j]*b[j];
		b[i] = sum/a[i][i];				// Store a component of the solution vector X.
	}									// All done!

	return true;
}

}; // namespace LUDecomp

