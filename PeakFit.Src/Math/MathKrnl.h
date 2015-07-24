// MathKrnl.h: interface for the CMathKernel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATHKRNL_H__D5F0EB03_EC58_11D4_95F7_00010296D95E__INCLUDED_)
#define AFX_MATHKRNL_H__D5F0EB03_EC58_11D4_95F7_00010296D95E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace std;
#include "../Crisp2.src/trace.h"

#include "MathDef.h"
#include "Vector.h"
#include "LinAlg.h"

//-------------------------------------------------------------

// CMatrix class
class CMatrix
{
private:
	valarray<CVector>	m_Data;
	size_t				m_nRows, m_nCols;

public:
	CMatrix();
	CMatrix(size_t nRows, size_t nCols);	// rows, columns
	CMatrix(const CMatrix &);
	CMatrix& operator = (const CMatrix &);
	~CMatrix() { m_Data.free(); }

	void Create(size_t nRows, size_t nCols);

	size_t size() const { return m_nRows*m_nCols; }
	size_t rows() const { return m_nRows; }
	size_t cols() const { return m_nCols; }

	CVector &row(size_t r) { return m_Data[r]; }
	CVector col(size_t c) const;

	CVector &operator[](size_t i) { return row(i); }	// C-style subscript

	CMatrix& operator += ( const CMatrix& );
	CMatrix& operator -= ( const CMatrix& );
	CMatrix& operator *= ( double );
	CMatrix& operator /= ( double );

	friend CMatrix operator + ( const CMatrix&, const CMatrix& );
	friend CMatrix operator - ( const CMatrix&, const CMatrix& );
	friend CMatrix operator * ( CMatrix&, const CMatrix& );
	friend CVector operator * ( CMatrix&, const CVector& );
	friend CMatrix operator * ( const CMatrix&, double );

	friend CVector Gauss( const CMatrix&, const CVector& );
	CMatrix &Transpose( const CMatrix& );
	CMatrix &Inverse( const CMatrix& );

#ifdef _DEBUG
	void Print();
#endif
};

bool Solver(const CMatrix &A, const CVector &V, CVector &X);

double ErrFunc(double x);

#pragma warning(disable:4035)               // re-enable below

static float __sqrt_half = 0.5F;
static float __sqrt_oneandhalf = 1.5F;

static inline float __cdecl qsqrt (float x)
{
	__asm
	{
		fld	[x]					// x
		mov	eax,0be6f0000h
		sub	eax,[x]
		shr	eax,1
		mov	[x],eax
		fld	[__sqrt_half]		// x 0.5
		fmul	st,st(1)		// x h
		fld	[__sqrt_oneandhalf]	// x h 1.5
		fld	[x]					// x h 1.5 a
		fld	st					// x h 1.5 a a
		fmul	st,st			// x h 1.5 a a*a
		fmul	st,st(3)		// x h 1.5 a a*a*h
		fsubr	st,st(2)		// x h 1.5 a 1.5-a*a*h
		fmulp	st(1),st		// x h 1.5 a
		fld	st					// x h 1.5 a a
		fmul	st,st			// x h 1.5 a a*a
		fmulp	st(3),st		// x a*a*h 1.5 a
		fxch					// x a*a*h a 1.5
		fsubrp	st(2),st		// x 1.5-a*a*h a
		fmulp	st(1),st		// x a
		fmulp	st(1),st		// a
	}
}

static inline float __cdecl qisqrt (float x)
{
	__asm
	{
		fld	[x]					// x
		mov	eax,0be6f0000h
		sub	eax,[x]
		shr	eax,1
		mov	[x],eax
		fld	[__sqrt_half]		// x 0.5
		fmulp	st(1),st		// h
		fld	[__sqrt_oneandhalf]	// h 1.5
		fld	[x]					// h 1.5 a
		fld	st					// h 1.5 a a
		fmul	st,st			// h 1.5 a a*a
		fmul	st,st(3)		// h 1.5 a a*a*h
		fsubr	st,st(2)		// h 1.5 a 1.5-a*a*h
		fmulp	st(1),st		// h 1.5 a
		fld	st					// h 1.5 a a
		fmul	st,st			// h 1.5 a a*a
		fmulp	st(3),st		// a*a*h 1.5 a
		fxch					// a*a*h a 1.5
		fsubrp	st(2),st		// 1.5-a*a*h a
		fmulp	st(1),st		// a
	}
}

#pragma warning(default:4035)

//-------------------------------------------------------------

// fit in 2D
void LinearFit(double *x, double *y, int ndata, double *sig, double *out);
// fit in 3D
void LinearFit3(double *x, double *y, double *z, int ndata, double *sig, double *out);

// polynom is A(0) + A(1)x + A(2)x^2 + A(3)x^3 + A(4)x^4 + ... + A(i)x^n
class CPolynomFit
{
public:
	CPolynomFit();

	bool Solve(double *pXdata, double *pYdata, size_t nPoints, size_t nOrder, CVector& out);
	double GetValue(const double *pdCoeffs, int nCoeffs, double x);
	double GetValue(CVector& A, double x);
};

// ellipse is Ax^2 + By^2 + Cx + Dy + Exy + F = 0
class CEllipseFit
{
public:
	CEllipseFit();

	enum
	{
		ellAxisAligned,
		ellCentred,
		ellArbitrary
	};
	bool Solve(double *pXdata, double *pYdata, size_t nPoints,
		UINT nFlags, CVector &out);
	// tilt - in angle Rad
	bool GetEllipseParams(CVector &X, UINT nFlags,
		CVector2d &centre, double &a, double &b, double &tilt);
	bool GetValue(const CVector& A, double x, CVector& out);
};

// cricle is Ax^2 + By^2 + Cx + Dy + Exy + F = 0
// where A = B = 1, C = -2c, D = -2d, E = 0 and F = c^2 + d^2 - r^2
// (c, d) - circle centre, r - circle radius
class CCircleFit
{
public:
	CCircleFit();

	bool Solve(double *pXdata, double *pYdata, size_t nPoints, CVector &out);
	bool GetValue(const CVector& A, double x, CVector& out);
	double GetChiSq() { return m_dChiSq; }

private:
	double m_dChiSq;
};

// statistical functions
template<class T>
class CStatistics
{
public:
	static T Mean(T *pXdata, int nSize);
	static T Median(T *pXdata, int nSize);
	static T Variance(T *pXdata, int nSize);
};

#endif // !defined(AFX_MATHKRNL_H__D5F0EB03_EC58_11D4_95F7_00010296D95E__INCLUDED_)