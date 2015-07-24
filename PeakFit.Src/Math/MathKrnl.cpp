// MathKrnl.cpp: implementation of the CMathKernel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MathKrnl.h"

#include <math.h>

typedef vector<float>::iterator VI;
typedef vector<POINT>::iterator PTI;

#define			THR_DELTA		1

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
	for(size_t i = 0; i < m_Data.size(); i++)
	{
		m_Data[i].resize(m.cols());
		m_Data[i] = m.m_Data[i];
	}
}

void CMatrix::Create(size_t nRows, size_t nCols)
{
	if( nCols < 0 || nRows < 0)
	{
		TRACE0("Matrix creation: rows or(and) columns size(s) is(are) < 0");
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
		throw "Matrix +=: matrices have different sizes";
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
		throw "Matrix -=: matrices have different sizes";
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
		throw "Matrix *=: matrices have different sizes\n";
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
			TRACE1("%20.6f\t", m_Data[r][c]);
		}
		TRACE0("\n");
	}
	TRACE0("\n");
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
		throw "Inverse: matrix isn't square";
	}
	// TODO: what about t
	t = 1;

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
		f = find_if( begin, &tmp[s][Num-1], bind2nd(not2(equal_to<double>()), 0) );
		if( 0.0 == *f )
		{
			throw "Error while calculating inv. matrix : not single solv.";
		}

		if( f != begin )
		{
			if( t >= Num )
			{
				throw "CMatrix::Inverse() -> cannot swap lines";
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

CVector Gauss( const CMatrix& m, const CVector& b )
{
	CVector out(b);
	CMatrix tmp(m);

	double c, *f, *begin;
	int t, s, Num;

	if(m.m_nCols != m.m_nRows)
	{
		TRACE0("Gauss() -> Matrix isn't square\n");
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
			TRACE0("Gauss() -> error while calculating : no single solution\n");
			return out;
		}

		if( f != begin )
		{
			if( t >= Num )
			{
				throw "CMatrix::Inverse() -> cannot swap lines";
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
		throw "Matrix add: matrices have different sizes";
	}
	out += B;
	return out;
}

CMatrix operator - ( const CMatrix& A, const CMatrix& B )
{
	CMatrix out(A);
	if( (A.cols() != B.cols()) || (A.rows() != B.rows()) )
	{
		throw "Matrix sub: matrices have different sizes";
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

CVector operator * ( CMatrix& m, const CVector& v )
{
	CVector out(m.rows());

	if(m.cols() != v.size())
	{
		throw("Matrix mul: matrix and vector have different sizes\n");
	}
	for(int j = 0; j < m.rows(); j++)
	{
		out[j] = (m.row(j) * v).sum();
	}
	return out;
}

CMatrix operator * ( CMatrix& A, const CMatrix& B)
{
	CVector s;
	CMatrix out(A.cols(), A.cols());

	if( (A.cols() != B.rows()) || (A.rows() != B.cols()) )
	{
		throw "Matrix mul: matrices have different sizes\n";
	}

	for(int j = 0; j < A.rows(); j++)
	{
		for(int i = 0; i < B.cols(); i++)
		{
			s = B.col(i);
			s *= A.row(j);
			out[j][i] = s.sum();
		}
	}
	return out;
}

////////////////////////////////////////////////////////////////////////////////////
//	solver function for A * X = B equation

bool Solver(const CMatrix &A, const CVector &V, CVector &X)
{
	bool bFlag = true;
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
	catch(char *pMsg)
	{
		TRACE1("Solver -> %s.\n", pMsg);
		bFlag = false;
	}
	catch(...)
	{
		bFlag = false;
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

bool CPolynomFit::Solve(double *pXdata, double *pYdata,
									 size_t nPoints, size_t nOrder, CVector& out)
{
	CMatrix A(nOrder, nOrder);
	CVector B(nOrder);

	try
	{
		FillPolynimialA(A, pXdata, pYdata, nPoints);
		FillPolynimialB(B, pXdata, pYdata, nPoints);
  
		Solver(A, B, out);
	}
	catch(char *pString)
	{
		TRACE1("CPolynomFit::Solve() -> %s\n", pString);
	}
	catch(...)
	{
		TRACE0("CPolynomFit::Solve() -> Unknown error\n");
		return false;
	}

	return true;
}

double CPolynomFit::GetValue(const double *pdCoeffs, int nCoeffs, double x)
{
	double res = 0, xc = 1;

	for(int i = 0; i < nCoeffs; i++)
	{
		res += pdCoeffs[i] * xc;
		xc *= x;
	}

	return res;
}

double CPolynomFit::GetValue(CVector& A, double x)
{
	return GetValue(A.data(), A.size(), x);
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
//		A.ResizeRow(i, 5);
		A[i][0] = SQR(X[i]);
		A[i][1] = 2.0*X[i]*Y[i];
		A[i][2] = SQR(Y[i]);
		A[i][3] = 2.0*X[i];
		A[i][4] = 2.0*Y[i];
		B[i] = -1;
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
			out[5] = 1;
		}
		break;
	default: break;
	}
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

bool CEllipseFit::Solve(double *pXdata, double *pYdata, size_t nPoints,
						UINT nFlags, CVector &out)
{
	CVector B(0.0, nPoints);

	out.resize(6, 0.0);
	try
	{
		int size;

		switch(nFlags)
		{
		case ellCentred: size = 3; break;
		case ellAxisAligned: size = 5; break;
		case ellArbitrary: size = 5; break;
		default: break;
		}

		if( nPoints < size )
		{
			TRACE0("CEllipseFit::Solve() -> insufficient parameters for solving\n");
			return false;
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
		// fill the solution vector
		FillIn(out, X, nFlags);
	}
	catch(char *pString)
	{
		TRACE1("CEllipseFit::Solve() -> %s\n", pString);
		return false;
	}
	catch(...)
	{
		TRACE0("CEllipseFit::Solve() -> Unknown error\n");
		return false;
	}

	return true;
}

bool CEllipseFit::GetValue(const CVector& X, double x, CVector& out)
{
	double res1 = 0.0, res2 = 0.0, D, A, B, C;

	out.resize(2, 0.0);
	A = 2 * X[1];					// == B
	B = X[4]*x + X[3];				// == Ex + D
	C = (X[0]*x + X[2])*x + X[5];	// == (Ax + C)x + F
	D = B * B - 2 * A * C;	// A is doubled!!!
	if( D < 0.0 )
	{
		TRACE0("CEllipseFit::GetValue() -> D < 0\n");
		return false;
	}
	else
	{
//		D = /*FastSqrt*/sqrt(D);
		D = sqrt(D);
		out[0] = (-B + D) / A;
		out[1] = (-B - D) / A;
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

bool CCircleFit::Solve(double *pXdata, double *pYdata, size_t nPoints, CVector &out)
{
	try
	{
		CVector B(0.0, nPoints);

		out.resize(3, 0.0);

		if( nPoints < 3 )
		{
			TRACE0("CCircleFit::Solve() -> insufficient parameters for solving\n");
			return false;
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
	catch(char *pString)
	{
		Trace(Format("CCircleFit::Solve() -> %1%") % pString);
		return false;
	}
	catch(...)
	{
		Trace(_T("CCircleFit::Solve() -> Unknown error"));
		return false;
	}

	return true;
}

bool CCircleFit::GetValue(const CVector& A, double x, CVector& out)
{
	return true;
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

////////////////////////////////////////////////////////////////////////////////////
//	Error function

// returns the value of ln|G(xx)| for xx > 0.
static double _gammln(double xx)
{
	double x, y, tmp, ser;

	static double cof[] = {76.18009172947146, -86.50532032941677,
		24.01409824083091, -1.231739572450155,
		0.1208650973866179e-2, -0.5395239384953e-5};
	int j;

	y = x = xx;
	tmp = x + 5.5;
	tmp -= (x + 0.5)*log(tmp);
	ser = 1.000000000190015;
	for(j = 0; j <= 5; j++)
		ser += cof[j]/++y;
	return -tmp+log(2.5066282746310005*ser/x);
}

#define ITMAX 100
#define EPS 3.0e-7
#define FPMIN 1.0e-30

// Returns incomplete gamma function Q(a,x) evaluated by its continued fraction representation
// as gammcf. Also returns ln G(a) as gln.
void _gcf(double &gammcf, double a, double x, double &gln)
{
	int i;
	double an,b,c,d,del,h;

	gln = _gammln(a);
	b = x + 1.0 - a;
	c = 1.0/FPMIN;
	d = 1.0/b;
	h = d;
	for(i = 1; i <= ITMAX; i++)
	{
		an = -i*(i-a);
		b += 2.0;
		d = an*d+b;
		if( FastAbs(d) < FPMIN)
			d = FPMIN;
		c = b + an/c;
		if( FastAbs(c) < FPMIN)
			c = FPMIN;
		d = 1.0/d;
		del = d*c;
		h *= del;
		if( FastAbs(del-1.0) < EPS)
			break;
	}
	if(i > ITMAX)
		throw("a too large, ITMAX too small in gcf");
	gammcf = exp(-x + a*log(x) - gln)*h;
}

static void _gser(double &gamser, double a, double x, double &gln)
{
	int n;
	double sum,del,ap;

	gln = _gammln(a);
	if (x <= 0.0)
	{
		if(x < 0.0)
			throw("x less than 0 in routine gser");
		gamser = 0.0;
	}
	else
	{
		ap = a;
		del = sum = 1.0 / a;
		for (n = 1; n <= ITMAX; n++)
		{
			++ap;
			del *= x/ap;
			sum += del;
			if( FastAbs(del) < FastAbs(sum)*EPS)
			{
				gamser = sum*exp(-x + a*log(x) - gln);
				return;
			}
		}
		throw "a too large, ITMAX too small in routine gser";
	}
}
#undef ITMAX
#undef EPS

static double _gammp(double a, double x)
{
	double gamser, gammcf, gln;

	if( (x < 0.0) || (a <= 0.0) )
		throw "Gamma function -> invalid argument(s)";

	if( x < (a+1.0))
	{
		_gser(gamser, a, x, gln);
		return gamser;
	}
	else
	{
		_gcf(gammcf, a, x, gln);
		return 1.0 - gammcf;
	}
}

double ErrFunc(double x)
{
	return CSIGN(x) * _gammp(0.5, x*x);
}

////////////////////////////////////////////////////////////////////////////////////
//	Linear fitting

void LinearFit(double *x, double *y, int ndata, double *sig, double *out)
{
	try
	{
		double ss, sx = 0.0, sy = 0.0, sxoss, t, wt, st2 = 0.0, sigdat;
		int i;

		out[0] = out[1] = out[2] = out[3] = out[4] = 0.0;
		ss = 0.0;
		for(i = 0; i < ndata; i++)
		{
			wt = 1.0;
			if(sig)
				wt = 1.0 / SQR(sig[i]);
			ss += wt;
			sx += x[i]*wt;
			sy += y[i]*wt;
		}
		sxoss = sx / ss;

		for(i = 0; i < ndata; i++)
		{
			wt = 1.0;
			if(sig)
				wt = 1.0 / sig[i];
			t = (x[i] - sxoss) * wt;
			st2 += t*t;
			out[1] += t*y[i] * wt;
		}
		out[1] /= st2;
		out[0] = (sy - sx*out[1]) / ss;
		out[2] = sqrt((1.0 + sx*sx / (ss*st2)) / ss);
		out[3] = sqrt(1.0 / st2);
		if( !sig )
		{
			for(i = 0; i < ndata; i++)
			{
				out[4] += SQR(y[i] - out[0] - out[1]*x[i]);
				sigdat = sqrt(out[4]/(ndata-2));
				out[2] *= sigdat;
				out[3] *= sigdat;
			}
		}
		else
		{
			for(i = 0; i < ndata; i++)
				out[4] += SQR( (y[i] - out[0] - out[1]*x[i]) / sig[i] );
		}
	}
	catch(char *pString)
	{
		printf("LinearFit() -> %s\n", pString);
	}
	catch(...)
	{
		printf("LinearFit() -> Unknown error\n");
	}
}

void LinearFit3(double *x, double *y, double *z, int ndata, double *sig, double *out)
{
	try
	{
		double sx = 0.0, sy = 0.0, sz = 0.0, sxy = 0.0, sx2 = 0.0, sy2 = 0.0, sxz = 0.0, syz = 0.0;
		double denom, wt, st2 = 0.0;//, sigdat, val;
		int i, N = ndata;

		out[0] = out[1] = out[2] = out[3] = 0.0;
		for(i = 0; i < ndata; i++)
		{
			wt = 1.0;
			if(sig)
				wt = 1.0 / SQR(sig[i]);
			sx  += x[i]*wt;
			sy  += y[i]*wt;
			sz  += z[i]*wt;
			sxy += x[i]*y[i]*SQR(wt);
			sx2 += SQR(x[i])*SQR(wt);
			sy2 += SQR(y[i])*SQR(wt);
			sxz += x[i]*z[i]*SQR(wt);
			syz += y[i]*z[i]*SQR(wt);
		}
		denom = -sy*sy*sx2 + 2*sy*sx*sxy - sxy*sxy*N - sy2*sx*sx + sy2*sx2*N;
		out[0] = -(-sx2*sz*sy2 + sx2*sy*syz - sy*sxy*sxz - sxy*sx*syz + sz*sxy*sxy + sxz*sx*sy2) / denom;
		out[1] = (-sxy*syz*N + sxz*sy2*N - sy*sy*sxz + sy*sxy*sz + sy*syz*sx - sz*sy2*sx) / denom;
		out[2] = (-sy*sx2*sz + sy*sx*sxz - sxy*N*sxz + sxy*sz*sx - syz*sx*sx + syz*sx2*N) / denom;
/*
		if( !sig )
		{
			for(i = 0; i < ndata; i++)
			{
				val = z[i] - out[0] - out[1]*x[i] - out[2]*y[i];
				out[3] += SQR(val);
//				sigdat = FastSqrt(out[3]/(ndata-2));
			}
		}
		else
		{
			for(i = 0; i < ndata; i++)
			{
				val = z[i] - out[0] - out[1]*x[i] - out[2]*y[i];
				out[3] += SQR( val / sig[i] );
			}
		}
*/	}
	catch(char *pString)
	{
		printf("LinearFit() -> %s\n", pString);
	}
	catch(...)
	{
		printf("LinearFit() -> Unknown error\n");
	}
}
