// MathDef.h: interface for the MathDef class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATHDEF_H__5854B6CE_9AEB_4C19_A66F_3E1B6FF72609__INCLUDED_)
#define AFX_MATHDEF_H__5854B6CE_9AEB_4C19_A66F_3E1B6FF72609__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define		SQR(x)		((x)*(x))

#define		M_PI		(double)3.14159265358979323846264338328
#define		M_2PI		(double)6.28318530717958647692528676656
#define		M_PI_2		(double)1.57079632679489661923132169164
#define		M_PI_4		(double)0.78539816339744830961566084582

#define		M_PI_3		(double)1.04719755119659774615421446109
#define		M_PI_6		(double)0.52359877559829887307710723055
#define		M_SQRT_PI	(double)1.77245385090551602729816748334

#define		M_E			(double)2.71828182845904523536028747135
#define		M_LOG_E		(double)0.43429448190325182765112891892

#define		RAD2DEG(x)	((x)*(double)57.2957795130823208767981548141)
#define		DEG2RAD(x)	((x)*(double)0.017453292519943295769236907685)

//
// Fast 32-bit float evaluations. 
// Warning: likely not portable, and useful on Pentium class processors only.
//
#define CFLOAT2LONG(x)			(*(LPLONG)&(x))
#define CFLOATSIGN(x)			(CFLOAT2LONG(x)&0x80000000)
#define CFLOATZERO(x)			(CFLOAT2LONG(x)+CFLOAT2LONG(x))
#define CFLOATCHANGESIGN(x)		(CFLOAT2LONG(x)=CFLOAT2LONG(x)^0x80000000)

// avlid for any number: float, int, double etc.
#define CSIGN(x)				(((x)<0)?-1:1)

inline BOOL IsSmallerPositiveFloat(float F1, float F2)
{
	return ( (*(DWORD*)&F1) < (*(DWORD*)&F2));
}

inline FLOAT MinPositiveFloat(float F1, float F2)
{
	if ( (*(DWORD*)&F1) < (*(DWORD*)&F2)) return F1; else return F2;
}
inline FLOAT MaxPositiveFloat(float F1, float F2)
{
	if ( (*(DWORD*)&F1) < (*(DWORD*)&F2)) return F2; else return F1;
}
//
// Warning: 0 and -0 have different binary representations.
//
inline BOOL EqualPositiveFloat(float F1, float F2)
{
	return ( *(DWORD*)&F1 == *(DWORD*)&F2 );
}

// bilinear interpolation - returns an interpolated value from the array of data with
// width nWidth if the rational coordinates (x,y) of point are known
// bilinear interpolation - returns an interpolated value from the array of data with
// width nWidth if the rational coordinates (x,y) of point are known
template<class _Tx>
double Bilinear(_Tx *pData, int nStride, double x, double y)
{
	double res = 0.0f;
	int ix = 0, iy = 0;
	int	c11 = 0, X1 = 0, X2 = 0, X3 = 0;
	double ux, ux_1, uy, uy_1;

	ix = (int)x;
	iy = (int)y;
	ux = x - ix;
	ux_1 = 1.0f - ux;
	uy = y - iy;
	uy_1 = 1.0f - uy;
	pData = (_Tx*)(((LPBYTE)pData) + iy*nStride + ix*sizeof(_Tx));

	res =  (double)pData[0]*ux_1*uy_1 + (double)pData[1]*ux*uy_1;
	pData = (_Tx*)(((LPBYTE)pData) + nStride);
	res += (double)pData[0]*ux_1*uy + (double)pData[1]*ux*uy;

	return res;
}
/*
template<class _Tx>
double Bilinear(_Tx *pData, int nWidth, double x, double y)
{
	double res = 0.0f;
	int ix = 0, iy = 0;
	int	c11 = 0, X1 = 0, X2 = 0, X3 = 0;
	double ux, ux_1, uy, uy_1;
	int off;

	ix = (int)x;
	iy = (int)y;
	ux = x - ix;
	ux_1 = 1.0f - ux;
	uy = y - iy;
	uy_1 = 1.0f - uy;
	off = iy*nWidth+ix;
	res = (double)pData[off]*ux_1*uy_1 + (double)pData[off+1]*ux*uy_1 +
		(double)pData[off+nWidth]*ux_1*uy + (double)pData[off+nWidth+1]*ux*uy;

	return res;
}
*/
//
// fast Floating Point Math
//
inline double FastSqrt(double a)
{
	__asm
	{
		fld		[a] 
		fsqrt
		fstp	[a]
	}
	return a;
}
inline double FastAbs(double a)
{
	__asm
	{
		fld		[a] 
		fabs
		fstp	[a]
	}
	return a;
}
inline double FastSin(double a)
{
	__asm
	{
		fld		[a] 
		fsin
		fstp	[a]
	}
	return a;
}
inline double FastCos(double a)
{
	__asm
	{
		fld		[a] 
		fcos
		fstp	[a]
	}
	return a;
}
inline void FastSinCos(double a, double &s, double &c)
{
	__asm
	{
		fld		[a]
		fsincos			// st(1) -> sin, st(0) -> cos
		mov		eax, dword ptr [s]
		mov		ecx, dword ptr [c]
		fstp	qword ptr [ecx]
		fstp	qword ptr [eax]
	}
}
inline double FastAtan2(double x, double y)
{
	__asm
	{
		fld		[y]
		fld		[x]
		fpatan
		fstp	[x]
	}
	return x;
}
inline double FastExp(double x)
{
	__asm
	{
		fld		[x]
		fldl2e
		fmulp	st(1),	st
		fld		st
		frndint
		fsub	st(1),	st
		fxch	st(1)
		f2xm1
		fld1
		faddp	st(1),	st
		fscale
		fstp	[x]
		fstp	st
	}
	return x;
}
inline double FastGauss(double x, double c, double w)
{
	__asm {
		fld		[x]
		fsub	[c]
		fdiv	[w]
		fmul	st,		st
		fchs
		fldl2e
		fmulp	st(1),	st
		fld		st
		frndint
		fsub	st(1),	st
		fxch	st(1)
		f2xm1
		fld1
		faddp	st(1),	st
		fscale
		fstp	[x]
		fstp	st
	}
	return	x;
}
//
// Round a floating point number to an integer.
// Note that (int+.5) is rounded to (int+1).
//
#define DEFINED_gRound 1
inline int gRound( double F )
{
	int I;
	__asm fld [F]
	__asm fistp [I]
	return I;
}
//
// Converts to integer equal to or less than.
//
#define DEFINED_gFloor 1
inline int gFloor( double F )
{
	static float Half=0.5;
	int I;
	__asm fld [F]
	__asm fsub [Half]
	__asm fistp [I]
	return I;
}

#endif // !defined(AFX_MATHDEF_H__5854B6CE_9AEB_4C19_A66F_3E1B6FF72609__INCLUDED_)
