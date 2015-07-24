#pragma once

//using namespace std;

#undef min
#undef max

#include "MathDef.h"
#include "Vector.h"
#include "LinAlg.h"

class MathException : public std::exception
{
	typedef std::exception super;
public:
	MathException(char* inError)
		: super(inError)
	{
	}
};

long FindGCD2(long ri, long rj);

// bilinear interpolation - returns an interpolated value from the array of data with
// width nWidth if the rational coordinates (x,y) of point are known
// bilinear interpolation - returns an interpolated value from the array of data with
// width nWidth if the rational coordinates (x,y) of point are known
template<class _Tx>
double Bilinear(_Tx *pData, int nStrideX, int nSizeX, int nSizeY, double x, double y)
{
	int ix = 0, iy = 0, nOffX = 0, nOffY = 0;
	int	c11 = 0, X1 = 0, X2 = 0, X3 = 0;
	double res = 0.0, ux = 0.0, ux_1 = 1.0, uy = 0.0, uy_1 = 1.0;
	
	ix = (int)x;
	iy = (int)y;
	if( ix < nSizeX-1 )
	{
		ux = x - ix;
		ux_1 = 1.0f - ux;
		nOffX = 1;
	}
	if( iy < nSizeY )
	{
		uy = y - iy;
		uy_1 = 1.0f - uy;
		nOffY = 1;
	}
	pData = (_Tx*)(((LPBYTE)pData) + iy*nStrideX + ix*sizeof(_Tx));
	res =  (double)pData[0]*ux_1*uy_1 + (double)pData[nOffX]*ux*uy_1;
	
	if( 1 == nOffY )
	{
		pData = (_Tx*)(((LPBYTE)pData) + nStrideX);
		res += (double)pData[0]*ux_1*uy + (double)pData[nOffX]*ux*uy;
	}
	
	return res;
}

//-------------------------------------------------------------

// rotate by Angle around axis L, passing through the point Org
// in direction Dir
CMatrix4x4 RotateAroundAxis(double dAngle, const CVector3d &vOrg, const CVector3d &vDir);

// Makes z-axis to coincide with some axis L, passing through the point Org
// in direction Dir. Finally, rotates by Angle around the axis L.
CMatrix4x4 CoincideAndRotateZAxis(double dAngle, const CVector3d &vOrg, const CVector3d &vDir);

//-------------------------------------------------------------

// CMatrix class
class CMatrix
{
public:
	valarray<CVector>	m_Data;
	size_t				m_nRows, m_nCols;

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
	friend CMatrix operator * ( const CMatrix&, const CMatrix& );
	friend CVector operator * ( const CMatrix&, const CVector& );
	friend CMatrix operator * ( const CMatrix&, double );

	friend CVector Gauss( const CMatrix&, const CVector& );
	CMatrix &Transpose( const CMatrix& );
	CMatrix &Inverse( const CMatrix& m, size_t nSize );
	CMatrix &Inverse( const CMatrix& );

#ifdef _DEBUG
	void Print();
#endif
};

BOOL Solver(const CMatrix &A, const CVector &V, CVector &X);

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

// polynom is A(0) + A(1)x + A(2)x^2 + A(3)x^3 + A(4)x^4 + ... + A(i)x^n
class /*CRISP_API*/ CPolynomFit
{
public:
	CPolynomFit();

	BOOL Solve(double *pXdata, double *pYdata, size_t nPoints, size_t nOrder, CVector& out);
	double GetValue(const CVector& A, double x);
};

// ellipse is Ax^2 + By^2 + Cx + Dy + Exy + F = 0
class /*CRISP_API*/ CEllipseFit
{
public:
	CEllipseFit();

	enum
	{
		ellAxisAligned,
		ellCentred,
		ellArbitrary
	};
	BOOL Solve(double *pXdata, double *pYdata, size_t nPoints, UINT nFlags, CVector &out);
	BOOL GetValue(const CVector& A, double x, CVector& out);
	// tilt - in angle Rad
	bool GetEllipseParams(CVector &X, UINT nFlags,
		CVector2d &centre, double &a, double &b, double &tilt);
};

// cricle is Ax^2 + By^2 + Cx + Dy + Exy + F = 0
// where A = B = 1, C = -2c, D = -2d, E = 0 and F = c^2 + d^2 - r^2
// (c, d) - circle centre, r - circle radius
class /*CRISP_API*/ CCircleFit
{
public:
	CCircleFit();

	BOOL Solve(double *pXdata, double *pYdata, size_t nPoints, CVector &out);
	BOOL GetValue(const CVector& A, double x, CVector& out);
	double GetChiSq() { return m_dChiSq; }

private:
	double m_dChiSq;
};

// statistical functions
template<class T>
class /*CRISP_API*/ CStatistics
{
public:
	static T Mean(T *pXdata, int nSize);
	static T Median(T *pXdata, int nSize);
	static T Variance(T *pXdata, int nSize);
};
