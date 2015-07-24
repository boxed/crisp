// MathKrnl.cpp: implementation of the CMathKernel class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <algorithm>
#include <functional>

#include "MathKrnl.h"
#include "../CRISP2.Src/trace.h"

typedef vector<float>::iterator VI;
typedef vector<POINT>::iterator PTI;

#define			THR_DELTA		1

double epsilon()
{
	double ret = 1.0;
	while (ret + 1.0 != 1.0)
		ret = ret * 0.5;
	return 2.0 * ret;
}

// the greatest common divisor
long FindGCD2(long ri, long rj)
{
	long rk;
	if( ri < 0 ) ri = -ri;
	
	if( rj )
	{
		for(;;)
		{
			rk = ri % rj; if (rk == 0) { ri = rj; break; }
			ri = rj % rk; if (ri == 0) { ri = rk; break; }
			rj = rk % ri; if (rj == 0) {          break; }
		}
		if (ri < 0) ri = -ri;
	}
	return ri;
}

//////////////////////////////////////////////////////////////////////
// rotate by Angle around axis L, passing through the point Org in direction Dir
//////////////////////////////////////////////////////////////////////

CMatrix4x4 RotateAroundAxis(double dAngle, const CVector3d &vOrg, const CVector3d &vDir)
{
	CMatrix4x4 res, tr, rx, ry, rz;
	double d, inv_d, dSin, dCos;

	tr.SetTranslation(-vOrg);

	rx.LoadIdentity();
	d = FastSqrt(SQR(vDir.y) + SQR(vDir.z));
	if( d > 1e-9 )
	{
		inv_d = 1.0 / d;
		rx.d[1][1] = rx.d[2][2] = vDir.z * inv_d;
		rx.d[1][2] =  vDir.y * inv_d;
		rx.d[2][1] = -vDir.y * inv_d;
	}

	ry.LoadIdentity();
	ry.d[0][0] = ry.d[2][2] = d;
	ry.d[0][2] =  vDir.x;
	ry.d[2][0] = -vDir.x;

	FastSinCos(dAngle, dSin, dCos);
	rz.LoadIdentity();
	rz.d[0][0] = rz.d[1][1] = dCos;
	rz.d[0][1] =  dSin;
	rz.d[1][0] = -dSin;

	res = rz*(ry*(rx*tr));
//	res = tr;
//	res *= rx;
//	res *= ry;
//	res *= rz;

	rx.d[1][2] = -rx.d[1][2];
	rx.d[2][1] = -rx.d[2][1];

	ry.d[0][2] = -ry.d[0][2];
	ry.d[2][0] = -ry.d[2][0];

	tr.SetTranslation(vOrg);

	res *= ry;
	res *= rx;
	res *= tr;
	res = (tr * (rx*(ry*res)));

	return res;
}

// Makes z-axis to coincide with some axis L, passing through the point Org
// in direction Dir. Finally, rotates by Angle around the axis L.
CMatrix4x4 CoincideAndRotateZAxis(double dAngle, const CVector3d &vOrg, const CVector3d &vDir)
{
	CMatrix4x4 res, tr, rx, ry, rz;
	double d, inv_d, dSin, dCos;

	tr.SetTranslation(-vOrg);

	rx.LoadIdentity();
	d = FastSqrt(SQR(vDir.y) + SQR(vDir.z));
	if( d > 1e-9 )
	{
		inv_d = 1.0 / d;
		rx.d[1][1] = rx.d[2][2] = vDir.z * inv_d;
		rx.d[1][2] = -vDir.y * inv_d;
		rx.d[2][1] =  vDir.y * inv_d;
//		rx.d[1][2] =  vDir.y * inv_d;
//		rx.d[2][1] = -vDir.y * inv_d;
	}

	ry.LoadIdentity();
	ry.d[0][0] = ry.d[2][2] = d;//vDir.x;
	ry.d[0][2] = -vDir.x;//d;
	ry.d[2][0] =  vDir.x;//d;
//	ry.d[0][2] =  vDir.x;//d;
//	ry.d[2][0] = -vDir.x;//d;

	FastSinCos(dAngle, dSin, dCos);
	rz.LoadIdentity();
	rz.d[0][0] = rz.d[1][1] = dCos;
	rz.d[0][1] = -dSin;
	rz.d[1][0] =  dSin;
//	rz.d[0][1] =  dSin;
//	rz.d[1][0] = -dSin;

	res = rz*(ry*(rx*tr));
//	res  = tr;
//	res *= rx;
//	res *= ry;
//	res *= rz;
/*
	rx.d[1][2] = -rx.d[1][2];
	rx.d[2][1] = -rx.d[2][1];

	ry.d[0][2] = -ry.d[0][2];
	ry.d[2][0] = -ry.d[2][0];
*/
	tr.SetTranslation(vOrg);

//	res *= ry;
//	res *= rx;
//	res *= tr;
	res = tr * res;

	return res;
}

//////////////////////////////////////////////////////////////////////
// CMatrix
//////////////////////////////////////////////////////////////////////

CMatrix::CMatrix()
{
	m_nRows = m_nCols = 0;
}

CMatrix::CMatrix(size_t nRows, size_t nCols)
{
	Create(nRows, nCols);
}

CMatrix::CMatrix(const CMatrix &m)
{
	m_nRows = m.rows();
	m_nCols = m.cols();
	m_Data.resize(m.rows());
	for(int i = 0; i < m_Data.size(); i++)
	{
		m_Data[i].resize(m.cols());
		m_Data[i] = m.m_Data[i];
	}
}

void CMatrix::Create(size_t nRows, size_t nCols)
{
	if( nCols < 0 || nRows < 0)
	{
		::OutputDebugString("Matrix creation: rows or(and) columns size(s) is(are) < 0");
		return;
	}
	m_nRows = nRows;
	m_nCols = nCols;
	m_Data.resize(nRows);
	for(int i = 0; i < m_Data.size(); i++)
	{
		m_Data[i].resize(nCols);
		fill( &m_Data[i][0], &m_Data[i][m_Data[i].size()], 0.0 );
	}
}

CVector CMatrix::col(size_t c) const
{
	CVector v(m_nRows);
	for(int j = 0; j < v.size(); j++)
		v[j] = m_Data[j][c];
	return v;
}


CMatrix& CMatrix::operator = ( const CMatrix &m )
{
	m_Data.resize(m.rows());
	for(int i = 0; i < m_Data.size(); i++)
	{
		m_Data[i].resize(m.cols());
		m_Data[i] = m.m_Data[i];
	}
	return *this;
}


CMatrix& CMatrix::operator += ( const CMatrix& m )
{
	if( (m_nRows != m.m_nRows) || (m_nCols != m.m_nCols) )
	{
		throw MathException("Matrix +=: matrices have different sizes");
	}
	for(int i = 0; i < m_Data.size(); i++)
	{
		m_Data[i] += m.m_Data[i];
	}
	return *this;
}


CMatrix& CMatrix::operator -= ( const CMatrix& m )
{
	if( (m_nRows != m.m_nRows) || (m_nCols != m.m_nCols) )
	{
		throw MathException("Matrix -=: matrices have different sizes");
	}
	for(int i = 0; i < m_Data.size(); i++)
		m_Data[i] -= m.m_Data[i];
	return *this;
}


CMatrix& CMatrix::operator *= ( double v )
{
	for(int j = 0; j < m_Data.size(); j++)
		m_Data[j] *= v;
	return *this;
}


CMatrix& CMatrix::operator /= ( double v )
{
	double inv_v = 1.0 / v;
	for(int j = 0; j < m_Data.size(); j++)
		m_Data[j] *= inv_v;
	return *this;
}
/*
CMatrix& CMatrix::operator *= ( const CMatrix& m )
{
	CVector s(m.rows());

	if( (m_nCols != m.m_nRows) || (m_nRows != m.m_nCols) )
	{
		throw MathException("Matrix *=: matrices have different sizes");
	}
	for(int j = 0; j < m_nRows; j++)
	{
		for(int i = 0; i < m.m_nRows; i++)
		{
			s = m.col(i);
			s *= m_Data[j];
			m_Data[j][i] = s.sum();
		}
	}
	return *this;
}
*/
#ifdef _DEBUG

void CMatrix::Print()
{
	for(int r = 0; r < m_nRows; r++)
	{
		for(int c = 0; c < m_nCols; c++)
		{
			printf("%20.6f\t", m_Data[r][c]);
		}
		::OutputDebugString("\n");
	}
	::OutputDebugString("\n");
}
#endif
////////////////////////////////////////////////////////////////////////////////////
//	friend functions


CMatrix &CMatrix::Inverse( const CMatrix& m )
{
	double c, *f, *begin;
	int i, j, t, s, Num;

	if(m.m_nCols != m.m_nRows)
	{
		throw MathException("Inverse: matrix isn't square");
	}

	Num = m.rows();

	CMatrix tmp(Num, Num * 2);
//	CMatrix out(Num, Num);

	for(j = 0; j < Num; j++)
	{
//		copy( &m.m_Data[j][0], &m.m_Data[j][Num], &tmp[j][0] );
//		fill( &tmp[j][Num], &tmp[j][Num*2-1], 0.0 );
		for(i = 0; i < Num; i++)
		{
			// left half of tmp - base matrix, right  half of tmp - diagonal with 1.0 on diagonal
			tmp[j][i] = m.m_Data[j][i];
			tmp[j][Num + i] = 0;
		}
		tmp[j][Num + j] = 1.0;
	}
//	tmp.Print();

	// do for all rows
	for(s = 0; s < Num; s++)
	{
		begin = &tmp[s][s];
		// find first non-zero element
		f = std::find_if( begin, &tmp[s][Num-1], bind2nd(not2(equal_to<double>()), 0) );
		if( !f )
		{
			throw MathException("Error while calculating inv. matrix : not single solv.");
		}

		if( f != begin )
		{
			if( t >= Num )
			{
				throw MathException("CMatrix::Inverse() -> cannot swap lines");
			}
			swap_ranges(&tmp[s][0], &tmp[s][Num-1], &tmp[t][0]);
		}

		c = 1. / (*begin);

		// make the element as 1.0
		tmp[s] *= c;

//		tmp.Print();

		for(t = 0; t < Num; t++)
		{
			if(t != s)
			{
				c = -tmp[t][s];
				tmp[t] += c * tmp[s];
			}
		}
//		tmp.Print();
	}

	m_nRows = m_nCols = Num;
	m_Data.resize( Num );
	for(j = 0; j < Num; j++)
	{
		m_Data[j].resize( Num );
//		copy( &tmp[j][Num], &tmp[j][Num<<1], &out[j][0] );
		for(i = 0; i < Num; i++)
			m_Data[j][i] = tmp[j][i + Num];
	}
//	out.Print();

	return *this;
}

CMatrix &CMatrix::Inverse( const CMatrix& m, size_t nSize )
{
	double c, *f, *begin;
	int i, j, t, s;

	if( (nSize >= m.m_nCols) || (nSize >= m.m_nRows) )
	{
		throw MathException("Inverse: matrix cannot be inversed");
	}
	// TODO: what about t
	t = 1;

	CMatrix tmp(nSize, nSize * 2);
//	CMatrix out(Num, Num);

	for(j = 0; j < nSize; j++)
	{
//		copy( &m.m_Data[j][0], &m.m_Data[j][nSize], &tmp[j][0] );
//		fill( &tmp[j][nSize], &tmp[j][nSize*2-1], 0.0 );
		for(i = 0; i < nSize; i++)
		{
			// left half of tmp - base matrix, right  half of tmp - diagonal with 1.0 on diagonal
			tmp[j][i] = m.m_Data[j][i];
			tmp[j][nSize + i] = 0;
		}
		tmp[j][nSize + j] = 1.0;
	}
//	tmp.Print();

	// do for all rows
	for(s = 0; s < nSize; s++)
	{
		begin = &tmp[s][s];
		// find first non-zero element
		f = find_if( begin, &tmp[s][nSize-1], bind2nd(not2(equal_to<double>()), 0) );
		if( 0.0 == *f )
		{
			throw MathException("Error while calculating inv. matrix : not single solv.");
		}

		if( f != begin )
		{
			if( t >= nSize )
			{
				throw MathException("CMatrix::Inverse() -> cannot swap lines");
			}
			swap_ranges(&tmp[s][0], &tmp[s][nSize-1], &tmp[t][0]);
		}

		c = 1. / (*begin);

		// make the element as 1.0
		tmp[s] *= c;

//		tmp.Print();

		for(t = 0; t < nSize; t++)
		{
			if(t != s)
			{
				c = -tmp[t][s];
				tmp[t] += c * tmp[s];
			}
		}
//		tmp.Print();
	}

//	m_nRows = m_nCols = nSize;
//	m_Data.resize( nSize );
	for(j = 0; j < nSize; j++)
	{
//		m_Data[j].resize( nSize );
//		copy( &tmp[j][nSize], &tmp[j][nSize<<1], &out[j][0] );
		for(i = 0; i < nSize; i++)
			m_Data[j][i] = tmp[j][i + nSize];
	}
//	out.Print();

	return *this;
}

CVector Gauss( const CMatrix& m, const CVector& b )
{
	CVector out(b);
	CMatrix tmp(m);

	double c, *f, *begin;
	int t, s, Num;

	if(m.m_nCols != m.m_nRows)
	{
		::OutputDebugString("Gauss() -> Matrix isn't square\n");
		return out;
	}

	Num = m.rows();

	// do for all rows
	for(s = 0; s < Num; s++)
	{
		begin = &tmp[s][s];
		// find first non-zero element
		f = find_if( begin, &tmp[s][Num-1], bind2nd(not2(equal_to<double>()), 0) );
		if( !f )
		{
			::OutputDebugString("Gauss() -> error while calculating : no single solution\n");
			return out;
		}

		if( f != begin )
		{
			if( t >= Num )
			{
				throw MathException("CMatrix::Inverse() -> cannot swap lines");
			}
			swap_ranges(&tmp[s][0], &tmp[s][Num-1], &tmp[t][0]);
			swap(out[s], out[t]);
		}

		c = 1. / (*begin);

		// make the element as 1.0
		tmp[s] *= c;
		out *= c;
		for(t = 0; t < Num; t++)
		{
			if(t != s)
			{
				c = -tmp[t][s];
				tmp[t] += c * tmp[s];
				out[t] += c * out[s];
			}
		}
	}

	return out;
}


CMatrix &CMatrix::Transpose( const CMatrix& m )
{
	m_nRows = m.m_nCols;
	m_nCols = m.m_nRows;
	m_Data.resize(m.m_nCols);
	for(int j = 0; j < m.m_nCols; j++)
	{
		m_Data[j].resize(m.m_nRows);
		m_Data[j] = m.col(j);
	}
	return *this;
}

CMatrix operator + ( const CMatrix& A, const CMatrix& B )
{
	CMatrix out(A);
	if( (A.cols() != B.cols()) || (A.rows() != B.rows()) )
	{
		throw MathException("Matrix add: matrices have different sizes");
	}
	out += B;
	return out;
}

CMatrix operator - ( const CMatrix& A, const CMatrix& B )
{
	CMatrix out(A);
	if( (A.cols() != B.cols()) || (A.rows() != B.rows()) )
	{
		throw MathException("Matrix sub: matrices have different sizes");
	}
	out -= B;
	return out;
}

CMatrix operator * ( const CMatrix& m, double d )
{
	CMatrix out(m);

	out *= d;
	return out;
}

CVector operator * ( const CMatrix& m, const CVector& v )
{
	CVector out(m.rows());

	if(m.cols() != v.size())
	{
		throw("Matrix mul: matrix and vector have different sizes\n");
	}
	for(int j = 0; j < m.rows(); j++)
	{
//#if _MSC_VER == 1200
//		out[j] = (m.row(j) * v).sum();
//#else if _MSC_VER > 1200
		out[j] = (m.m_Data[j] * v).sum();
//#endif
	}
	return out;
}

CMatrix operator * ( const CMatrix& A, const CMatrix& B)
{
	CVector s;
	CMatrix out(A.cols(), A.cols());

	if( A.cols() != B.rows() || A.rows() != B.cols() )
	{
		throw MathException("Matrix mul: matrices have different sizes");
	}

	for(int j = 0; j < A.rows(); j++)
	{
		for(int i = 0; i < B.cols(); i++)
		{
			s = B.col(i);
			s *= A.m_Data[j];
			out[j][i] = s.sum();
		}
	}
	return out;
}

////////////////////////////////////////////////////////////////////////////////////
//	solver function for A * X = B equation

BOOL Solver(const CMatrix &A, const CVector &V, CVector &X)
{
	BOOL bFlag = TRUE;
	try
	{
		if(X.size() != A.cols())
		{
			X.resize(A.cols());
		}
		CMatrix Ainv(A.cols(), A.cols());

		if( A.cols() == A.rows() )
		{
			Ainv.Inverse(A);	// [Ainv] = [A]^(-1)
			X = Ainv * V;		// [X] = [A]^(-1) * [B]
		}
		else
		{
			CMatrix At(A.cols(), A.rows()), m(A.cols(), A.cols());
			CVector v(A.cols());

			At.Transpose(A);	// [At] = [A]^T
			m = At * A;			// [m] = [At] * [A]
			Ainv.Inverse( m );	// [Ainv] = ([At] * [A])^(-1) == m^(-1)
			v = At * V;			// [v] = [At] * [V]
			X = Ainv * v;		// [X] = ([At] * [A])^(-1) * v
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("Solver -> %s"), e.what()));
		bFlag = FALSE;
	}
	return bFlag;
}

////////////////////////////////////////////////////////////////////////////////////
//	CPolynomFit class


CPolynomFit::CPolynomFit()
{
}

static void FillPolynimialA(CMatrix &A, double *X, double *Y, int N)
{
	// assume that this is initial matrix filled by 0.0
	for(int i = 0; i < A.rows(); i++)
	{
		for(int k = 0; k < N; k++)
		{
			for(int c = 0; c < A.cols(); c++)
			{
				A[i][c] += pow(X[k], c+i);
			}
		}
	}
}

static void FillPolynimialB(CVector &B, double *X, double *Y, int N)
{
	double x;

	fill( &B[0], &B[B.size()], 0.0 );
	for(int k = 0; k < N; k++)
	{
		x = 1.0;
		for(int i = 0; i < B.size(); i++)
		{
			B[i] += x * Y[k];
			x *= X[k];
		}
	}
}


BOOL CPolynomFit::Solve(double *pXdata, double *pYdata,
									 size_t nPoints, size_t nOrder, CVector& out)
{
	try
	{
		CMatrix A(nOrder, nOrder);
		CVector B(nOrder);

		FillPolynimialA(A, pXdata, pYdata, nPoints);
		FillPolynimialB(B, pXdata, pYdata, nPoints);
  
		Solver(A, B, out);
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("CPolynomFit::Solve() -> %s"), e.what()));
		return FALSE;
	}

	return TRUE;
}

double CPolynomFit::GetValue(const CVector& A, double x)
{
	double res = 0, xc = 1;

	for(int i = 0; i < A.size(); i++)
	{
		res += A[i] * xc;
		xc *= x;
	}

	return res;
}

////////////////////////////////////////////////////////////////////////////////////
//	CEllipseFit class


CEllipseFit::CEllipseFit()
{
}

// centre of the ellipse in the (0, 0) point
/*
         2 2    2 2
    A = s M  + r N

         2 2    2 2
    B = s N  + r M

    C = 0

    D = 0
                2   2
    E = 2 MN ( s - r )

    F = -1

	where M - cos(angle), N - sin(angle)
*/
static void FillEllipseC(CMatrix &A, CVector &B, double *X, double *Y, int N)
{
	if( A.rows() != N )
	{
//		A.ResizeRows(N);
	}
	if( B.size() != N )
	{
		B.resize(N);
	}
	for(int i = 0; i < A.rows(); i++)
	{
//		A.ResizeRow(i, 3);
		B[i] = 1;
		A[i][0] = X[i]*X[i];
		A[i][1] = Y[i]*Y[i];
		A[i][2] = X[i]*Y[i];
	}
}

// axis-aligned ellipse
/*
          2
    A =  s

          2
    B =  r

          2
    C = -s 2c

          2
    D = -r 2d


    E =  0

          2 2    2 2    2 2
    F =  r d  + s c  - r s

	where c, d - centre of ellipse, r, s - main axises
*/
static void FillEllipseAA(CMatrix &A, CVector &B, double *X, double *Y, int N)
{
	if( A.rows() != N )
	{
//		A.ResizeRows(N);
	}
	if( B.size() != N )
	{
		B.resize(N);
	}
	for(int i = 0; i < A.rows(); i++)
	{
//		A.ResizeRow(i, 3);
		B[i] = 1;
		A[i][0] = X[i]*X[i];
		A[i][1] = Y[i]*Y[i];
		A[i][2] = X[i]*Y[i];
	}
}

// arbitrary ellipse
/*
              2 2    2 2
    A =      s M  + r N

              2 2    2 2
    B =      s N  + r M

              2      2
    C = -2 ( s Mc + r Nd )

              2      2
    D = -2 ( s Nc + r Md )

              2      2
    E =  2 ( s MN - r MN )

              2 2    2 2    2 2
    F =      s c  + r d  - r s

	where M - cos(angle), N - sin(angle)
*/
static void FillEllipse(CMatrix &A, CVector &B, double *X, double *Y, int N)
{
	if( A.rows() != N )
	{
//		A.ResizeRows(N);
	}
	if( B.size() != N )
	{
		B.resize(N);
	}
	for(int i = 0; i < A.rows(); i++)
	{
		B[i] = -1.0;
		A[i][0] = SQR(X[i]);
		A[i][1] = 2.0*X[i]*Y[i];
		A[i][2] = SQR(Y[i]);
		A[i][3] = 2.0*X[i];
		A[i][4] = 2.0*Y[i];
/*
		B[i] = -Y[i]*Y[i];
		A[i][0] = X[i]*X[i] + B[i];
		A[i][1] = X[i]*Y[i];
		A[i][2] = X[i];
		A[i][3] = Y[i];
		A[i][4] = 1.0;
*/
	}
}

static void FillIn(CVector &out, const CVector &X, UINT nFlags)
{
	switch(nFlags)
	{
	case CEllipseFit::ellCentred:
		{
			out[0] = X[0];
			out[1] = X[1];
			out[4] = X[2];
			out[5] = -1.0;
		}
		break;
	case CEllipseFit::ellAxisAligned:
		{
			out[0] = X[0];
			out[1] = X[1];
			out[2] = X[2];
			out[3] = X[3];
			out[5] = X[4];
		}
		break;
	case CEllipseFit::ellArbitrary:
		{
			out[0] = X[0];
			out[1] = X[1];
			out[2] = X[2];
			out[3] = X[3];
			out[4] = X[4];
			out[5] = 1.0;
/*
			out[0] = X[0];
			out[1] = X[1];
			out[2] = 1.0 - X[0];
			out[3] = X[2];
			out[4] = X[3];
			out[5] = X[4];
*/
		}
		break;
	default: break;
	}
}


BOOL CEllipseFit::Solve(double *pXdata, double *pYdata, size_t nPoints, UINT nFlags, CVector &out)
{
	CVector B(0.0, nPoints);

	out.resize(6, 0.0);
	try
	{
		int size;

		switch(nFlags)
		{
		case ellCentred: size = 3; break;
		case ellAxisAligned:
		case ellArbitrary: size = 5; break;
		default: break;
		}

		if( nPoints < size )
		{
			::OutputDebugString("CEllipseFit::Solve() -> insufficient parameters for solving\n");
			return FALSE;
		}

		CMatrix A(nPoints, size);
		CVector X(0.0, size);

		switch(nFlags)
		{
		case ellCentred: FillEllipseC(A, B, pXdata, pYdata, nPoints); break;
		case ellAxisAligned: FillEllipseAA(A, B, pXdata, pYdata, nPoints); break;
		case ellArbitrary: FillEllipse(A, B, pXdata, pYdata, nPoints); break;
		default: break;
		}
		// solve ellipse equation
		Solver(A, B, X);
		FillIn(out, X, nFlags);
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("CEllipseFit::Solve() -> %s"), e.what()));
		return FALSE;
	}

	return TRUE;
}


BOOL CEllipseFit::GetValue(const CVector& X, double x, CVector& out)
{
	double res1 = 0.0, res2 = 0.0, D, A, B, C;

	out.resize(2, 0.0);
	A = 2 * X[1];					// == B
	B = X[4]*x + X[3];				// == Ex + D
	C = (X[0]*x + X[2])*x + X[5];	// == (Ax + C)x + F
	D = B * B - 2 * A * C;			// A is doubled!!!
	if( D < 0.0 )
	{
		::OutputDebugString("CEllipseFit::GetValue() -> D < 0\n");
		return FALSE;
	}
	else
	{
//		D = /*FastSqrt*/sqrt(D);
		D = sqrt(D);
		out[0] = (-B + D) / A;
		out[1] = (-B - D) / A;
	}

	return TRUE;
}

bool CEllipseFit::GetEllipseParams(CVector &X, UINT nFlags,
								   CVector2d &centre, double &a, double &b, double &tilt)
{
	switch(nFlags)
	{
	case CEllipseFit::ellCentred:
		{
		}
		break;
	case CEllipseFit::ellAxisAligned:
		{
		}
		break;
	case CEllipseFit::ellArbitrary:
		{
			double A_, B_, C_, D_, F_, G_, G_2, A_2, C_2, CosA, SinA;
			// get the values
			tilt = 0.5 * atan(2.0 * X[1] / (X[2] - X[0]));
			FastSinCos(tilt, SinA, CosA);
			A_ = X[0]*SQR(CosA) - 2.0*X[1]*SinA*CosA + X[2]*SQR(SinA);
			B_ = X[1]*(SQR(CosA) - SQR(SinA)) + (X[0] - X[2])*SinA*CosA;
			C_ = X[0]*SQR(SinA) + 2.0*X[1]*SinA*CosA + X[2]*SQR(CosA);
			D_ = X[3]*CosA - X[4]*SinA;
			F_ = X[3]*SinA + X[4]*CosA;
			G_ = 1.0;
			
			G_2 = -G_ + SQR(D_)/A_ + SQR(F_)/C_;
			A_2 = A_ / G_2;
			C_2 = C_ / G_2;
			a = 1 / FastSqrt(A_2);
			b = 1 / FastSqrt(C_2);
			centre.x = -(D_/A_*CosA + F_/C_*SinA);
			centre.y =  (D_/A_*SinA - F_/C_*CosA);
		}
		break;
	default: break;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////
//	circle refinement

CCircleFit::CCircleFit()
{
}

static void FillCircle(CMatrix &A, CVector &B, double *X, double *Y, int N)
{
	if( A.rows() != N )
	{
//		A.ResizeRows(N);
	}
	if( B.size() != N )
	{
		B.resize(N);
	}
	for(int i = 0; i < A.rows(); i++)
	{
//		A.ResizeRow(i, 3);
		B[i] = -(SQR(X[i]) + SQR(Y[i]));
		A[i][0] = - 2.0 * X[i];
		A[i][1] = - 2.0 * Y[i];
		A[i][2] = 1.0;
	}
}


BOOL CCircleFit::Solve(double *pXdata, double *pYdata, size_t nPoints, CVector &out)
{
	try
	{
		CVector B(0.0, nPoints);

		out.resize(3, 0.0);

		if( nPoints < 3 )
		{
			::OutputDebugString("CCircleFit::Solve() -> insufficient parameters for solving\n");
			return FALSE;
		}

		CMatrix A(nPoints, 3);
		CVector X(0.0, 3);

		FillCircle(A, B, pXdata, pYdata, nPoints);

		// solve ellipse equation
		Solver(A, B, X);
		out[0] = X[0];
		out[1] = X[1];
		out[2] = /*FastSqrt*/sqrt(SQR(X[0]) + SQR(X[1]) - X[2]);

		m_dChiSq = 0.0;
		// chi-sq
		for(int i = 0; i < nPoints; i++)
		{
			m_dChiSq += SQR( out[2] - /*FastSqrt*/sqrt( SQR(pXdata[i] - X[0]) + SQR(pYdata[i] - X[1]) ) );
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("CCircleFit::Solve() -> %s"), e.what()));
		return FALSE;
	}

	return TRUE;
}


BOOL CCircleFit::GetValue(const CVector& A, double x, CVector& out)
{
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
//	statistical functions

template<class T> T CStatistics<T>::Mean(T *pXdata, int nSize)
{
	return accumulate(pXdata, pXdata+nSize, (T)0.0) / nSize;
}


template<class T> T CStatistics<T>::Median(T *pXdata, int nSize)
{
	T res;
	int size = nSize >> 1;

	sort(pXdata, pXdata+nSize);
	res =  pXdata[size];
	if( !(nSize & 1) )
	{
		res += pXdata[size - 1];
		res *= 0.5;
	}
	return res;
}


template<class T> T CStatistics<T>::Variance(T *pXdata, int nSize)
{
	T mean = CStatistics::Mean(pXdata, nSize);
	T res = (T)0.0;
	for(T *_P = pXdata; _P != pXdata+nSize; ++_P)
		res += SQR((*_P) - mean);
	return res / (T)(nSize - 1);
}
