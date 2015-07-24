#pragma once

template <typename _Tx>
/*__forceinline*/inline _Tx SQR(const _Tx &x) { return x*x;}

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

// valid for any number: float, int, double etc.
#define CSIGN(x)				(((x)<0)?-1:1)

/*__forceinline*/inline bool IsSmallerPositiveFloat(float F1, float F2)
{
	return ( (*(DWORD*)&F1) < (*(DWORD*)&F2));
}

/*__forceinline*/inline FLOAT MinPositiveFloat(float F1, float F2)
{
	if ( (*(DWORD*)&F1) < (*(DWORD*)&F2)) return F1; else return F2;
}
/*__forceinline*/inline FLOAT MaxPositiveFloat(float F1, float F2)
{
	if ( (*(DWORD*)&F1) < (*(DWORD*)&F2)) return F2; else return F1;
}
//
// Warning: 0 and -0 have different binary representations.
//
/*__forceinline*/inline BOOL EqualPositiveFloat(float F1, float F2)
{
	return ( *(DWORD*)&F1 == *(DWORD*)&F2 );
}

//
// Floating Point Math
//
#pragma warning( disable: 4035 )		// complains about the missing return value

/*__forceinline*/inline double FastSqrt(double a)
{
	__asm
	{
		fld		[a] 
		fsqrt
		fstp	[a]
	} // return st(0)
	return a;
}
/*__forceinline*/inline double FastAbs(double a)
{
	__asm
	{
		fld		[a] 
		fabs
		fstp	[a]
	} // return st(0)
	return a;
}
/*__forceinline*/inline double FastSin(double a)
{
	__asm
	{
		fld		[a] 
		fsin
		fstp	[a]
	} // return st(0)
	return a;
}
/*__forceinline*/inline double FastCos(double a)
{
	__asm
	{
		fld		[a] 
		fcos
		fstp	[a]
	} // return st(0)
	return a;
}
/*__forceinline*/inline double FastTan(double a)
{
    // return sin( r ) / cos( r );
    __asm {
        fld a // r0 = r
		fsin // r0 = sinf( r0 )
		fld a // r1 = r0, r0 = r
		fcos // r0 = cosf( r0 )
		fdiv // r0 = r1 / r0
    } // returns st(0)
}
/*__forceinline*/inline void FastSinCos(double a, double &s, double &c)
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
/*__forceinline*/inline double FastAtan2(double x, double y)
{
	__asm
	{
		fld		[y]
		fld		[x]
		fpatan
		fstp	[x]
	} // return st(0)
	return x;
}
/*__forceinline*/inline double FastAtan(double x)
{
    const float one = 1.f;
    __asm
	{
        fld x		// r0 = r
        fld1		// {{ r0 = atan( r0 )
        fpatan		//
	} // return st(0)
}
/*__forceinline*/inline double FastExp(double x)
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
//		fstp	st(1)
	} // return st(0)
	return x;
}
/*__forceinline*/inline double FastAcos(double x)
{
	// return M_PI_2 + atan( x / -SQRT( 1.0 - x * x ) );
	const double pi_2 = M_PI_2;
	const double one = 1.0;
	__asm
	{
		fld x		// r0 = r
		fld st
		fmul x		// r0 = r0 * r
		fsubr one	// r0 = r0 - 1.f
		fsqrt		// r0 = sqrtf( r0 )
		fchs		// r0 = - r0
		fdiv		// r0 = r1 / r0
		fld1		// {{ r0 = atan( r0 )
		fpatan		// }}
		fadd pi_2	// r0 = r0 + pi / 2
		fstp [x]
	} // returns st(0)
	return x;
}
/*__forceinline*/inline double FastAsin(double x)
{
	// return atan( x / SQRT( 1.f - x * x ) );
    const float one = 1.0;
    __asm
	{
		fld x		// r0 = r
		fld st
		fmul x		// r0 = r0 * r
		fsubr one	// r0 = r0 - 1.f
		fsqrt		// r0 = sqrtf( r0 )
		fdiv		// r0 = r1 / r0
		fld1		// r0 = atan( r0 )
		fpatan
		fstp [x]
    } // returns st(0)
	return x;
}
//
// Round a floating point number to an integer.
// Note that (int+.5) is rounded to (int+1).
//
#define DEFINED_gRound 1
/*__forceinline*/inline int gRound( double F )
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
/*__forceinline*/inline int gFloor( double F )
{
	static float Half=0.5;
	int I;
	__asm fld [F]
	__asm fsub [Half]
	__asm fistp [I]
	return I;
}

double epsilon();

// EPSILON STAFF
const double eps = epsilon();

const double eps0 = FastSqrt(eps);
const double eps1 = 100. * eps0;
const double eps2 = 5000. * eps0;
const double eps3 = 10000. * eps0;
const double eps4 = 15000. * eps0;
const double eps5 = 20000. * eps0;
const double eps6 = 25000. * eps0;

