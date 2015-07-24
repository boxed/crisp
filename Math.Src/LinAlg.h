#pragma once

#include <math.h>

#include "Random.h"
#include "MathDef.h"

#include <vector>

//
// if C_WIN32_VECTOR_ASM defined then asm code will be used
//
#ifdef _WINDOWS
//#define C_WIN32_VECTOR_ASM
#endif

enum
{
	_X,_Y,_Z,_W
};
//
// 2 float component vector
//
template<class _Tx>
class CVector2
{
public:
	_Tx	x, y;

	CVector2 () {}
	CVector2 (_Tx f) : x(f), y(f) {}
	CVector2 (const CVector2& w) : x(w.x), y(w.y) {}
	CVector2 (_Tx vx, _Tx vy) : x(vx), y(vy) {}

	operator _Tx* () { return &x; }
	CVector2& operator = (const CVector2& v) { x = v.x; y = v.y; return *this;}
	CVector2& operator = (_Tx f) { x = y = f; return *this;}
	CVector2  operator - () const
	{
		return CVector2(-x, -y);
	}
	CVector2& operator += (const CVector2& v)
	{
		x += v.x; y += v.y; return *this;
	}
	CVector2& operator -= (const CVector2& v)
	{
		x -= v.x; y -= v.y; return *this;
	}
	CVector2& operator *= (const CVector2& v)
	{
		x *= v.x; y *= v.y; return *this;
	}
	CVector2& operator *= (_Tx f)
	{
		x *= f; y *= f; return *this;
	}
	CVector2& operator /= (_Tx f)
	{
		register double ooz = 1.0 / f; x *= ooz; y *= ooz; return *this;
	}

	friend CVector2 operator + (const CVector2& v, const CVector2& u)
	{
		return CVector2(v.x+u.x, v.y+u.y);
	}
	friend CVector2 operator - (const CVector2& v, const CVector2& u)
	{
		return CVector2(v.x-u.x, v.y-u.y);
	}
	friend CVector2 operator * (const CVector2& v, const CVector2& u)
	{
		return CVector2(v.x*u.x, v.y*u.y);
	}
	friend CVector2 operator * (_Tx f, const CVector2& v)
	{
		return CVector2(v.x*f, v.y*f);
	}
	friend CVector2 operator * (const CVector2& v, _Tx f)
	{
		return CVector2(v.x*f, v.y*f);
	}
#if _MSC_VER > 1200
	friend CVector2 operator / (const CVector2& v, _Tx f)
	{
		register double ooz = 1.0/f;
		return CVector2((_Tx)(v.x*ooz), (_Tx)(v.y*ooz));
	}
#endif
	friend CVector2 operator / (const CVector2& v, const CVector2& u)
	{
		return CVector2(v.x/u.x, v.y/u.y);
	}
	friend CVector2 operator ^ (const CVector2&, const CVector2&);
	friend _Tx operator & (const CVector2& v, const CVector2& u) {return u.x*v.x + u.y*v.y;}
	friend _Tx operator % (const CVector2& v, const CVector2& u) {return u.x*v.x - u.y*v.y;}

	_Tx Length() const
	{
		return (_Tx) sqrt(x*x + y*y); 
	}

	_Tx operator ! () const {return Length(); }
	_Tx& operator [] (int n) {return * (&x + n); }
	_Tx& operator () (int n) {return * (&x + n); }
	int operator < (_Tx v) { return x < v && y < v; }
	int operator > (_Tx v) { return x > v && y > v; }
	int operator == (CVector2 &v) { return x == v.x && y == v.y; }

	_Tx Sqr() const { return (x*x + y*y); }
	bool Equals(const CVector2 &v, _Tx tolerance)
	{
		return ( FastAbs(x - v.x) < FastAbs( tolerance )
				&& FastAbs(y - v.y) < FastAbs( tolerance ));
	}
	void Normalize() 
	{
		*this /= !(*this);
	}
/*
	void RndVector()
	{
		*this = CVector2(CFloatRND() - 0.5f*RAND_MAX, CFloatRND() - 0.5f*RAND_MAX);
		Normalize();
	}
	void Clip()
	{
		if(x < 0.0) x = 0.0;
		else if(x > 1.0) x = 1.0;
		if(y < 0.0) y = 0.0;
		else if(y > 1.0) y = 1.0;
	}
*/
};

typedef CVector2<short>		CVector2s;
typedef CVector2<long>		CVector2l;
typedef CVector2<float>		CVector2f;
typedef CVector2<double>	CVector2d;

#if _MSC_VER == 1200
template <typename _Tx>
CVector2<_Tx> operator / (const CVector2<_Tx>& v, _Tx f)
{
	register double ooz = 1.0/f;
	return CVector2<_Tx>((_Tx)(v.x*ooz), (_Tx)(v.y*ooz));
}
#endif

//
// 3 float component vector
//
template <typename _Tx>
class CVector3
{
public:
	_Tx	x, y, z;

	CVector3 () {}
	CVector3 (double f) : x((_Tx)f), y((_Tx)f), z((_Tx)f) {}
//	CVector3 (const CVector3& w) : x(w.x), y(w.y), z(w.z) {}

	CVector3 (double vx, double vy, double vz) : x((_Tx)vx), y((_Tx)vy), z((_Tx)vz) {}
	template <typename _Ty>
		CVector3<_Tx>(const CVector3<_Ty> &w) : x((_Tx)w.x), y((_Tx)w.y), z((_Tx)w.z) {}

	operator _Tx*() { return &x; }
	template <typename _Ty>
		CVector3<_Tx>& operator = (const CVector3<_Ty>& v)
	{ x=(_Tx)v.x; y=(_Tx)v.y; z=(_Tx)v.z; return *this; }

	CVector3& operator = (double f) { x = y = z = f; return *this;}

	CVector3  operator - () const
	{
		return CVector3(-x, -y, -z);
	}
	CVector3& operator += (const CVector3& u)
	{
		x += u.x; y += u.y; z += u.z; return *this;
	}
	CVector3& operator -= (const CVector3& u)
	{
		x -= u.x; y -= u.y; z -= u.z; return *this;
	}
	CVector3& operator *= (const CVector3& u)
	{
		x *= u.x; y *= u.y; z *= u.z; return *this;
	}
	CVector3& operator *= (double f)
	{
		x *= f; y *= f; z *= f; return *this;
	}
	CVector3& operator /= (double f)
	{
		register double ooz = 1.0/f;
		x *= ooz; y *= ooz; z *= ooz; return *this;
	}

	friend CVector3 operator + (const CVector3& u, const CVector3& v)
	{
		return CVector3(u.x+v.x, u.y+v.y, u.z+v.z);
	}
	friend CVector3 operator - (const CVector3& u, const CVector3& v)
	{
		return CVector3(u.x-v.x, u.y-v.y, u.z-v.z);
	}
	friend CVector3 operator * (const CVector3& u, const CVector3& v)
	{
		return CVector3(u.x*v.x, u.y*v.y, u.z*v.z);
	}
	friend CVector3 operator * (double f, const CVector3& u)
	{
		return CVector3(u.x*f, u.y*f, u.z*f);
	}
	friend CVector3 operator * (const CVector3& u, double f)
	{
		return CVector3(u.x*f, u.y*f, u.z*f);
	}
#if _MSC_VER > 1200
	friend CVector3 operator / (const CVector3& u, double f)
	{
		register double ooz = 1.0/f;
		return CVector3(_Tx(u.x*ooz), _Tx(u.y*ooz), _Tx(u.z*ooz));
	}
#endif
	friend CVector3 operator / (const CVector3& u, const CVector3& v)
	{
		return CVector3(u.x/v.x, u.y/v.y, u.z/v.z);
	}
	friend CVector3 operator ^ (const CVector3& u, const CVector3& v)
	{
		return CVector3(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
	}

	friend _Tx operator & (const CVector3& u, const CVector3& v)
	{
		return u.x*v.x + u.y*v.y + u.z*v.z;
	}

	double operator ! () const { return FastSqrt(Sqr()); }

	_Tx& operator [] (int n) { return * (&x + n); }
	_Tx& operator () (int n) { return * (&x + n); }
	int operator < (_Tx v) { return x < v && y < v && z < v; }
	int operator > (_Tx v) { return x > v && y > v && z > v; }
	int operator == (CVector3 &v) { return x == v.x && y == v.y && z == v.z; }
	_Tx Sqr() const { return (SQR(x) + SQR(y) + SQR(z)); }

	bool Equals(const CVector3 &v, double tolerance)
	{
		return ( FastAbs(x - v.x) < FastAbs( tolerance )
				&& FastAbs(y - v.y) < FastAbs( tolerance )
				&& FastAbs(z - v.z) < FastAbs( tolerance ));
	}

	void Normalize() 
	{
		*this /= !(*this);
	}

	void RndVector()
	{
		*this = CVector3(CFloatRND() - 0.5f*RAND_MAX,
			CFloatRND() - 0.5f*RAND_MAX,
			CFloatRND() - 0.5f*RAND_MAX);
		Normalize();
	}

	// clips each component to [0;1] interval
	void Clip()
	{
		if(x < 0.0) x = 0.0;
		else if(x > 1.0) x = 1.0;
		if(y < 0.0) y = 0.0;
		else if(y > 1.0) y = 1.0;
		if(z < 0.0) z = 0.0;
		else if(z > 1.0) z = 1.0;
	}
	// clips each component to [dMin;dMax] interval
	void Clip(double dMin, double dMax)
	{
		if(x < dMin) x = dMin;
		else if(x > dMax) x = dMax;
		if(y < dMin) y = dMin;
		else if(y > dMax) y = dMax;
		if(z < dMin) z = dMin;
		else if(z > dMax) z = dMax;
	}
};

typedef CVector3<short>	 CVector3s;
typedef CVector3<long>	 CVector3l;
typedef CVector3<double> CVector3d;

#if _MSC_VER == 1200
template <typename _Tx>
CVector3<_Tx> operator / (const CVector3<_Tx>& u, double f)
{
	register double ooz = 1.0/f;
	return CVector3<_Tx>(_Tx(u.x*ooz), _Tx(u.y*ooz), _Tx(u.z*ooz));
}
#endif


//
// 3 float component vector (optimized)
//
class CVector3f
{
public:
	float	x, y, z;

	CVector3f () {}
	CVector3f (float f) : x(f), y(f), z(f) {}
	CVector3f (const CVector3f& w) : x(w.x), y(w.y), z(w.z) {}
	CVector3f (float vx, float vy, float vz) : x(vx), y(vy), z(vz) {}

	operator float*() { return &x; }

	CVector3f& operator = (const CVector3f& v) { x=v.x; y=v.y; z=v.z; return *this;}
	CVector3f& operator = (float f) { x=y=z=f; return *this;}
	CVector3f  operator - () const;
	CVector3f& operator += (const CVector3f& );
	CVector3f& operator -= (const CVector3f& );
	CVector3f& operator *= (const CVector3f& );
	CVector3f& operator *= (float );
	CVector3f& operator /= (float );

	friend CVector3f operator + (const CVector3f&, const CVector3f& );
	friend CVector3f operator - (const CVector3f&, const CVector3f& );
	friend CVector3f operator * (const CVector3f&, const CVector3f& );
	friend CVector3f operator * (float, const CVector3f&);
	friend CVector3f operator * (const CVector3f& , float);
	friend CVector3f operator / (const CVector3f& , float);
	friend CVector3f operator / (const CVector3f& , const CVector3f&);
	friend CVector3f operator ^ (const CVector3f& u, const CVector3f& v)
	{
#ifdef C_WIN32_VECTOR_ASM
		CVector3f cross;
	_asm
		{
			mov 		eax, cross.x
			mov 		ecx, u
			mov 		edx, v

			// optimized cross product; 22 cycles
			fld dword ptr 	[ecx+4]		 	// starts & ends on cycle 0
			fmul dword ptr	[edx+8]		 	// starts on cycle 1 			
			fld dword ptr 	[ecx+8]		 	// starts & ends on cycle 2
			fmul dword ptr	[edx+0]		 	// starts on cycle 3 			
			fld dword ptr 	[ecx+0]		 	// starts & ends on cycle 4
			fmul dword ptr	[edx+4]		 	// starts on cycle 5 			
			fld dword ptr 	[ecx+8]		 	// starts & ends on cycle 6
			fmul dword ptr	[edx+4]		 	// starts on cycle 7 			
			fld dword ptr 	[ecx+0]		 	// starts & ends on cycle 8
			fmul dword ptr	[edx+8]		 	// starts on cycle 9 				
			fld dword ptr 	[ecx+4]		 	// starts & ends on cycle 10 
			fmul dword ptr	[edx+0]		 	// starts on cycle 11 			 
			fxch 			st(2)			// no cost 									
			fsubrp 			st(5),st(0) 	// starts on cycle 12 			 
			fsubrp 			st(3),st(0) 	// starts on cycle 13 			 
			fsubrp 			st(1),st(0) 	// starts on cycle 14 			 
			fxch 			st(2)			// no cost, stalls for cycle 15
			fstp dword ptr	[cross.x]		// starts on cycle 16, ends on cycle 17	
			fstp dword ptr	[cross.y]		// starts on cycle 18, ends on cycle 19	
			fstp dword ptr	[cross.z]		// starts on cycle 20, ends on cycle 21	
		}
		return cross;
#else
		return CVector3f(	u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z,
					u.x*v.y - u.y*v.x);
#endif
	}

	friend float operator & (const CVector3f& u, const CVector3f& v)
	{
#ifdef C_WIN32_VECTOR_ASM
		float dot;
		_asm
		{
			mov 		ecx, u
			mov 		eax, v
			// optimized dot product - 15 cycles
			fld dword ptr 	[eax+0]		// starts & ends on cycle 0
			fmul dword ptr	[ecx+0]		// starts on cycle 1
			fld dword ptr 	[eax+4]		// starts & ends on cycle 2
			fmul dword ptr	[ecx+4]		// starts on cycle 3
			fld dword ptr 	[eax+8]		// starts & ends on cycle 4
			fmul dword ptr	[ecx+8]		// starts on cycle 5
			fxch 					 st(1)			// no cost
			faddp 					st(2),st(0)	// starts on cycle 6, stalls for cycles 7-8
			faddp 					st(1),st(0)	// starts on cycle 9, stalls for cycles 10-12
			fstp dword ptr	[dot]			// starts on cycle 13, ends on cycle 14
		}
		return dot;
#else
		return u.x*v.x + u.y*v.y + u.z*v.z;
#endif
	}

	float operator ! () { return (float) sqrt(x*x + y*y + z*z); }
	float& operator [] (int n) { return * (&x + n); }
	float& operator () (int n) { return * (&x + n); }
	int operator < (float v) { return x < v && y < v && z < v; }
	int operator > (float v) { return x > v && y > v && z > v; }
	int operator == (CVector3f &v) { return x == v.x && y == v.y && z == v.z; }

	bool Equals(const CVector3f &v, float tolerance)
	{
		return ( FastAbs(x - v.x) < FastAbs( tolerance )
				&& FastAbs(y - v.y) < FastAbs( tolerance )
				&& FastAbs(z - v.z) < FastAbs( tolerance ));
	}

	void Normalize() 
	{
		*this /= !(*this);
	}

	void RndVector()
	{
		*this = CVector3f(CFloatRND() - 0.5f*RAND_MAX, CFloatRND() - 0.5f*RAND_MAX, CFloatRND() - 0.5f*RAND_MAX);
		Normalize();
	}

	void Clip()
	{
		if(x < 0.0) x = 0.0;
		else if(x > 1.0) x = 1.0;
		if(y < 0.0) y = 0.0;
		else if(y > 1.0) y = 1.0;
		if(z < 0.0) z = 0.0;
		else if(z > 1.0) z = 1.0;
	}
	// clips each component to [dMin;dMax] interval
	void Clip(double dMin, double dMax)
	{
		if(x < (float)dMin) x = (float)dMin;
		else if(x > (float)dMax) x = (float)dMax;
		if(y < (float)dMin) y = (float)dMin;
		else if(y > (float)dMax) y = (float)dMax;
		if(z < (float)dMin) z = (float)dMin;
		else if(z > (float)dMax) z = (float)dMax;
	}
};

__forceinline CVector3f CVector3f::operator - () const
{
	return CVector3f(-x, -y, -z);
}

__forceinline CVector3f operator + (const CVector3f& v, const CVector3f& u)
{
	return CVector3f(v.x+u.x, v.y+u.y, v.z+u.z);
}

__forceinline CVector3f operator - (const CVector3f& v, const CVector3f& u)
{
	return CVector3f(v.x-u.x, v.y-u.y, v.z-u.z);
}

__forceinline CVector3f operator * (const CVector3f& v, const CVector3f& u)
{
	return CVector3f(v.x*u.x, v.y*u.y, v.z*u.z);
}

__forceinline CVector3f operator * (const CVector3f& v, float f)
{
	return CVector3f(v.x*f, v.y*f, v.z*f);
}

__forceinline CVector3f operator * (float f, const CVector3f& v)
{
	return CVector3f(v.x*f, v.y*f, v.z*f);
}

__forceinline CVector3f operator / (const CVector3f& v, const CVector3f& u)
{
	return CVector3f(v.x/u.x, v.y/u.y, v.z/u.z);
}

__forceinline CVector3f operator / (const CVector3f& v, float f)
{
	register float ooz = 1.0f/f;
	return CVector3f(v.x*ooz, v.y*ooz, v.z*ooz);
}

__forceinline CVector3f& CVector3f::operator += (const CVector3f &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

__forceinline CVector3f& CVector3f::operator -= (const CVector3f &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

__forceinline CVector3f& CVector3f::operator *= (float v)
{
	x *= v;
	y *= v;
	z *= v;
	return *this;
}

__forceinline CVector3f& CVector3f::operator *= (const CVector3f &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

__forceinline CVector3f& CVector3f::operator /= (float v)
{
	register float ooz = 1.0f/v;
	x *= ooz;
	y *= ooz;
	z *= ooz;
	return *this;
}
//
// 4 float component vector
//

class CVector4f
{
public:
	float	x, y, z, w;

public:

	CVector4f () {}
	CVector4f (float f) : x(f), y(f), z(f), w(1) {}
	CVector4f (const CVector4f& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	CVector4f (float vx, float vy, float vz, float vw) : x(vx), y(vy), z(vz), w(vw) {}

	operator float*() { return &x; }

	CVector4f& operator = (const CVector4f& v) { x=v.x; y=v.y; z=v.z; w=v.w; return *this; }
	CVector4f& operator = (float f) { x=y=z=f; w=1; return *this; }
	CVector4f	operator - () const;
	CVector4f& operator += (const CVector4f& );
	CVector4f& operator -= (const CVector4f& );
	CVector4f& operator *= (const CVector4f& );
	CVector4f& operator *= (float );
	CVector4f& operator /= (float );

	friend CVector4f operator + (const CVector4f&, const CVector4f& );
	friend CVector4f operator - (const CVector4f&, const CVector4f& );
	friend CVector4f operator * (const CVector4f&, const CVector4f& );
	friend CVector4f operator * (float, const CVector4f&);
	friend CVector4f operator * (const CVector4f& , float);
	friend CVector4f operator / (const CVector4f& , float);
	friend CVector4f operator / (const CVector4f& , const CVector4f&);
	friend CVector4f operator ^ (const CVector4f& u, const CVector4f& v)
	{
#ifdef C_WIN32_VECTOR_ASM
		CVector4f cross;
	_asm
		{
			mov 		eax, cross.x
			mov 		ecx, u
			mov 		edx, v

			// optimized cross product; 22 cycles
			fld dword ptr 	[ecx+4]		 	// starts & ends on cycle 0
			fmul dword ptr	[edx+8]		 	// starts on cycle 1 			
			fld dword ptr 	[ecx+8]		 	// starts & ends on cycle 2
			fmul dword ptr	[edx+0]		 	// starts on cycle 3 			
			fld dword ptr 	[ecx+0]		 	// starts & ends on cycle 4
			fmul dword ptr	[edx+4]		 	// starts on cycle 5 			
			fld dword ptr 	[ecx+8]		 	// starts & ends on cycle 6
			fmul dword ptr	[edx+4]		 	// starts on cycle 7 			
			fld dword ptr 	[ecx+0]		 	// starts & ends on cycle 8
			fmul dword ptr	[edx+8]		 	// starts on cycle 9 				
			fld dword ptr 	[ecx+4]		 	// starts & ends on cycle 10 
			fmul dword ptr	[edx+0]		 	// starts on cycle 11 			 
			fxch 			st(2)			// no cost 									
			fsubrp 			st(5),st(0) 	// starts on cycle 12 			 
			fsubrp 			st(3),st(0) 	// starts on cycle 13 			 
			fsubrp 			st(1),st(0) 	// starts on cycle 14 			 
			fxch 			st(2)			// no cost, stalls for cycle 15
			fstp dword ptr	[cross.x]		// starts on cycle 16, ends on cycle 17	
			fstp dword ptr	[cross.y]		// starts on cycle 18, ends on cycle 19	
			fstp dword ptr	[cross.z]		// starts on cycle 20, ends on cycle 21	
		}
		cross.w = 1.0f;
		return cross;
#else
		return CVector4f(	v.y*u.z - v.z*u.y, v.z*u.x - v.x*u.z,
					v.x*u.y - v.y*u.x, 1.0f);
#endif
	}

	friend float operator & (const CVector4f& v, const CVector4f& u)
	{
#ifdef C_WIN32_VECTOR_ASM
		float dot;
		_asm
		{
			mov 		ecx, u
			mov 		eax, v
			// optimized dot product - 15 cycles
			fld dword ptr 	[eax+0]		// starts & ends on cycle 0
			fmul dword ptr	[ecx+0]		// starts on cycle 1
			fld dword ptr 	[eax+4]		// starts & ends on cycle 2
			fmul dword ptr	[ecx+4]		// starts on cycle 3
			fld dword ptr 	[eax+8]		// starts & ends on cycle 4
			fmul dword ptr	[ecx+8]		// starts on cycle 5
			fxch 			st(1)		// no cost
			faddp 			st(2),st(0)	// starts on cycle 6, stalls for cycles 7-8
			faddp 			st(1),st(0)	// starts on cycle 9, stalls for cycles 10-12
			fstp dword ptr	[dot]		// starts on cycle 13, ends on cycle 14
		}
		return dot;
#else
		return u.x*v.x + u.y*v.y + u.z*v.z;
#endif
	}

	float operator ! () {return (float) sqrt(x*x + y*y + z*z + w*w); }
	float& operator [] (int n) {return * (&x + n); }
	float& operator () (int n) {return * (&x + n); }
	int operator < (float v) { return x < v && y < v && z < v && w < v; }
	int operator > (float v) { return x > v && y > v && z > v && w > v; }
	int operator == (CVector4f &v) { return x == v.x && y == v.y && z == v.z && w == v.w; }

	bool Equals(const CVector4f &v, float tolerance)
	{
		return ( FastAbs(x - v.x) < FastAbs( tolerance )
				&& FastAbs(y - v.y) < FastAbs( tolerance )
				&& FastAbs(z - v.z) < FastAbs( tolerance )
				&& FastAbs(w - v.w) < FastAbs( tolerance ));
	}

	void Normalize() 
	{
		*this /= !(*this);
	}

	void RndVector()
	{
		*this = CVector4f(CFloatRND() - 0.5f*RAND_MAX, CFloatRND() - 0.5f*RAND_MAX, CFloatRND() - 0.5f*RAND_MAX, 1.0f);
		Normalize();
	}

	void Clip()
	{
		if(x < 0.0) x = 0.0;
		else if(x > 1.0) x = 1.0;
		if(y < 0.0) y = 0.0;
		else if(y > 1.0) y = 1.0;
		if(z < 0.0) z = 0.0;
		else if(z > 1.0) z = 1.0;
	}
};

__forceinline CVector4f CVector4f::operator - () const
{
	return CVector4f(-x, -y, -z, -w);
}

__forceinline CVector4f operator + (const CVector4f& v, const CVector4f& u)
{
	return CVector4f(v.x+u.x, v.y+u.y, v.z+u.z, v.w+u.w);
}

__forceinline CVector4f operator - (const CVector4f& v, const CVector4f& u)
{
	return CVector4f(v.x-u.x, v.y-u.y, v.z-u.z, v.w-u.w);
}

__forceinline CVector4f operator * (const CVector4f& v, const CVector4f& u)
{
	return CVector4f(v.x*u.x, v.y*u.y, v.z*u.z, v.w*u.w);
}

__forceinline CVector4f operator * (const CVector4f& v, float f)
{
	return CVector4f(v.x*f, v.y*f, v.z*f, v.w*f);
}

__forceinline CVector4f operator * (float f, const CVector4f& v)
{
	return CVector4f(v.x*f, v.y*f, v.z*f, v.w*f);
}

__forceinline CVector4f operator / (const CVector4f& v, const CVector4f& u)
{
	return CVector4f(v.x/u.x, v.y/u.y, v.z/u.z, v.w/u.w);
}

__forceinline CVector4f operator / (const CVector4f& v, float f)
{
	register float ooz = 1.0f/f;
	return CVector4f(v.x*ooz, v.y*ooz, v.z*ooz, v.w*ooz);
}

__forceinline CVector4f& CVector4f::operator += (const CVector4f &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

__forceinline CVector4f& CVector4f::operator -= (const CVector4f &v)
{
	x -= v.x; 
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

__forceinline CVector4f& CVector4f::operator *= (float v)
{
	x *= v;
	y *= v;
	z *= v;
	w *= v;
	return *this;
}

__forceinline CVector4f& CVector4f::operator *= (const CVector4f &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}

__forceinline CVector4f& CVector4f::operator /= (float v)
{
	register float ooz = 1.0f/v;
	x *= ooz;
	y *= ooz;
	z *= ooz;
	w *= ooz;
	return *this;
}
//
// class CRay
//
template<class _Tx>
class CRay
{
// Variables
public:
	CVector3<_Tx>	m_vOrg;
	CVector3<_Tx>	m_vDir;

public:
	CRay () {}
	CRay (CVector3<_Tx> &o, CVector3<_Tx> &d) { m_vOrg = o; m_vDir = d; }

public:

	CVector3<_Tx> Point(_Tx t) { return m_vOrg + m_vDir*t; }
};
typedef CRay<double> CRayd;
typedef CRay<float> CRayf;
//
// class CRay2
//
template<class _Tx>
class CRay2
{
// Variables
public:
	CVector2<_Tx>	m_vOrg;
	CVector2<_Tx>	m_vDir;

public:
	CRay2 () {}
	CRay2 (CVector2<_Tx> &o, CVector2<_Tx> &d) { m_vOrg = o; m_vDir = d; }

public:
	CVector2<_Tx> Point(double t) { return m_vOrg + m_vDir*(float)t; }

	// returns the distance from the point to this line
	double PointDistance(CVector2<_Tx> &p)
	{
		double t = ((p - m_vOrg) & m_vDir) / m_vDir.Sqr();
		CVector2<_Tx> AP = p - (m_vOrg + (t * m_vDir));
		double dSign = AP & CVector2<_Tx>(-m_vDir.y, m_vDir.x);//m_vDir;
		return (!(AP)) * CSIGN(dSign);
	}
};

typedef	CRay2<double>	CRay2d;
typedef	CRay2<float>	CRay2f;

//
// class CPlane
//
template <typename _Tx>
class CPlane
{
	typedef CVector3<_Tx> vec_type;
// Variables
public:

	vec_type	m_vN;
	_Tx			m_Dist;

public:

	CPlane() { m_vN = vec_type(0, 0, 1); m_Dist = 0; }
	CPlane(_Tx a, _Tx b, _Tx c, _Tx dist) // Not normalized, the user must do that
	{
		m_vN = vec_type(a, b, c);
//		m_vN.Normalize();
		m_Dist = dist;
	}
	CPlane(const CPlane& p) { m_vN = p.m_vN; m_Dist = p.m_Dist; }

	CPlane(const vec_type& norm, _Tx dist)
	{
		Create(norm, dist);
	}
	CPlane(const vec_type& norm, const vec_type& point)
	{
		m_vN = norm;
		m_vN.Normalize();
		m_Dist = 0;
		m_Dist = ClassifyPoint(point);
	}
	CPlane(const vec_type& v1, const vec_type& v2, const vec_type& v3)
	{
		Create(v1, v2, v3);
	}
	void Create(const vec_type& v1, const vec_type& v2, const vec_type& v3);
	void Create(const vec_type& norm, const _Tx& dist) { m_vN = norm; m_Dist = dist; }
	_Tx	 ClassifyPoint(CVector3<_Tx>& v) const { return (m_vN & v) + m_Dist; }
	bool Intersect(CRay<_Tx> &r, _Tx &d) const
	{
		double denom;
		denom = r.m_vDir & m_vN;
		if( FastAbs(denom) < 1e-6 )
			return false;
		d = - (m_Dist + (r.m_vOrg & m_vN)) / denom;
		return true;
	}
	CPlane&	operator = (const CPlane& p) { m_vN = p.m_vN; m_Dist = p.m_Dist; return *this; }
	void ClipByPlane(const std::vector<vec_type> &vIn, std::vector<vec_type> &vOut) const;
};

typedef CPlane<double>	CPlaned;
typedef CPlane<float>	CPlanef;

//
// CMatrix3x3
//
class CMatrix3x3
{
// Constructor/destructor
public:
  	CMatrix3x3();
	CMatrix3x3(double fVal);
	CMatrix3x3(const double *pMatrix);
	CMatrix3x3(const CMatrix3x3 &m);
	CMatrix3x3(double d11, double d12, double d13,
		double d21, double d22, double d23,
		double d31, double d32, double d33)
	{
		d[0][0] = d11, d[0][1] = d12, d[0][2] = d13;
		d[1][0] = d21, d[1][1] = d22, d[1][2] = d23;
		d[2][0] = d31, d[2][1] = d32, d[2][2] = d33;
	}
	CMatrix3x3(const CVector3d &v1, const CVector3d &v2, const CVector3d &v3, bool bCols = true)
	{
		if( true == bCols )		// vectors are columns
		{
			d[0][0] = v1.x, d[0][1] = v2.x, d[0][2] = v3.x;
			d[1][0] = v1.y, d[1][1] = v2.y, d[1][2] = v3.y;
			d[2][0] = v1.z, d[2][1] = v2.z, d[2][2] = v3.z;
		}
		else					// vectors are rows
		{
			d[0][0] = v1.x, d[0][1] = v1.y, d[0][2] = v1.z;
			d[1][0] = v2.x, d[1][1] = v2.y, d[1][2] = v2.z;
			d[2][0] = v3.x, d[2][1] = v3.y, d[2][2] = v3.z;
		}
	}
	~CMatrix3x3();

// Variables
public:
  	double d[3][3];

public:
	void	LoadMatrix(double* pfData);
  	void	LoadIdentity();
  	void	ZeroMatrix();
  	bool	Invert(CMatrix3x3 &out);
  	void	Transpose();
	bool	IsOrthonormal();

public:
	CMatrix3x3& operator += (const CMatrix3x3&);
	CMatrix3x3& operator -= (const CMatrix3x3&);
	CMatrix3x3& operator *= (double v);

	CMatrix3x3& operator *= (const CMatrix3x3&);
	CMatrix3x3& operator /= (const CMatrix3x3&);

	operator double*() { return &d[0][0]; }
	double* operator []( int i ) { return &d[i][0]; }
	double& operator ()( size_t x, size_t y ) { return d[y][x]; }

	CVector3d Row(int iRow) const;
	CVector3d Col(int iCol) const;
	void Row(int iRow, const CVector3d &row);
	void Col(int iCol, const CVector3d &col);

	double Determinant();

  	friend CMatrix3x3 operator + (const CMatrix3x3 &, const CMatrix3x3 &);
  	friend CMatrix3x3 operator - (const CMatrix3x3 &, const CMatrix3x3 &);
  	friend CMatrix3x3 operator * (const CMatrix3x3 &, double);
  	friend CVector3d operator * (const CMatrix3x3 &, const CVector3d &);
  	friend CVector3f operator * (const CMatrix3x3 &, const CVector3f &);
  	friend CVector3d operator * (const CVector3d &, const CMatrix3x3 &);
  	friend CVector3f operator * (const CVector3f &, const CMatrix3x3 &);
  	friend CMatrix3x3 operator * (const CMatrix3x3 &, const CMatrix3x3 &);

	// extract the translational part of the matrix
	CVector3d GetTranslation();

	void Translate(const CVector3d &);
	void Scale    (const CVector3d &);
	void RotateX  (double);
	void RotateY  (double);
	void RotateZ  (double);
	void MirrorX  ();
	void MirrorY  ();
	void MirrorZ  ();
};
/*
CMatrix3x3 Translate(const CVector3d &);
CMatrix3x3 Scale    (const CVector3d &);
CMatrix3x3 RotateX  (double);
CMatrix3x3 RotateY  (double);
CMatrix3x3 RotateZ  (double);
CMatrix3x3 MirrorX  ();
CMatrix3x3 MirrorY  ();
CMatrix3x3 MirrorZ  ();
/*
CMatrix3x3 operator + (const CMatrix3x3 &, const CMatrix3x3 &)
{
	CMatrix3x3 m;
}
CMatrix3x3 operator - (const CMatrix3x3 &, const CMatrix3x3 &);
CMatrix3x3 operator * (const CMatrix3x3 &, double);
*/
__forceinline CVector3d operator * (const CMatrix3x3 &m, const CVector3d &v)
{
	CVector3d out;
	out.x = m.d[0][0] * v.x + m.d[0][1] * v.y + m.d[0][2] * v.z;
	out.y = m.d[1][0] * v.x + m.d[1][1] * v.y + m.d[1][2] * v.z;
	out.z = m.d[2][0] * v.x + m.d[2][1] * v.y + m.d[2][2] * v.z;
	return out;
}
__forceinline CVector3f operator * (const CMatrix3x3 &m, const CVector3f &v)
{
	CVector3f out;
	out.x = (float)(m.d[0][0] * v.x + m.d[0][1] * v.y + m.d[0][2] * v.z);
	out.y = (float)(m.d[1][0] * v.x + m.d[1][1] * v.y + m.d[1][2] * v.z);
	out.z = (float)(m.d[2][0] * v.x + m.d[2][1] * v.y + m.d[2][2] * v.z);
	return out;
}
__forceinline CVector3d operator * (const CVector3d &v, const CMatrix3x3 &m)
{
	CVector3d out;
	out.x = m.d[0][0] * v.x + m.d[1][0] * v.y + m.d[2][0] * v.z;
	out.y = m.d[0][1] * v.x + m.d[1][1] * v.y + m.d[2][1] * v.z;
	out.z = m.d[0][2] * v.x + m.d[1][2] * v.y + m.d[2][2] * v.z;
	return out;
}
__forceinline CVector3f operator * (const CVector3f &v, const CMatrix3x3 &m)
{
	CVector3f out;
	out.x = (float)(m.d[0][0] * v.x + m.d[1][0] * v.y + m.d[2][0] * v.z);
	out.y = (float)(m.d[0][1] * v.x + m.d[1][1] * v.y + m.d[2][1] * v.z);
	out.z = (float)(m.d[0][2] * v.x + m.d[1][2] * v.y + m.d[2][2] * v.z);
	return out;
}
//CMatrix3x3 operator * (const CMatrix3x3 &, const CMatrix3x3 &);
//
// CMatrix4x4
//
class CMatrix4x4
{
// Constructor/destructor
public:
  	CMatrix4x4();
	CMatrix4x4(double fVal);
	~CMatrix4x4();

// Variables
public:
  	double d[4][4];

public:
	void	LoadMatrix(double* pfData);
  	void	LoadIdentity();
  	void	ZeroMatrix();
  	void	Invert();
  	void	Transpose();
	bool	IsOrthonormal();

public:
	CMatrix4x4& operator += (const CMatrix4x4&);
	CMatrix4x4& operator -= (const CMatrix4x4&);
	CMatrix4x4& operator *= (double v);

	CMatrix4x4& operator *= (const CMatrix4x4&);
	CMatrix4x4& operator /= (const CMatrix4x4&);

	operator double*() { return &d[0][0]; }
	double* operator []( int i ) { return &d[i][0]; }

  	friend CMatrix4x4 operator + (const CMatrix4x4 &, const CMatrix4x4 &);
  	friend CMatrix4x4 operator - (const CMatrix4x4 &, const CMatrix4x4 &);
  	friend CMatrix4x4 operator * (const CMatrix4x4 &, double);
  	friend CVector3d operator * (const CMatrix4x4 &, const CVector3d &);
  	friend CVector3d operator * (const CVector3d &, const CMatrix4x4 &);
//	friend CMatrix4x4 operator * (const CMatrix4x4 &, const CMatrix4x4 &);

	void SetView( const CVector3d &dvFrom, const CVector3d &dvTo, CVector3d &dvUp );
	
	// extract the translational part of the matrix
	CVector3d GetTranslation();
	// extract the rotational part of the matrix
	CMatrix3x3 GetRotation();

	void SetTranslation(const CVector3d &);
	void SetRotation(const CMatrix3x3 &);
	void SetScale(const CVector3d &);
	void RotateX (double);
	void RotateY (double);
	void RotateZ (double);
	void MirrorX ();
	void MirrorY ();
	void MirrorZ ();
};

CMatrix4x4 operator * (const CMatrix4x4 &, const CMatrix4x4 &);

CMatrix4x4 Translate(const CVector3d &);
CMatrix4x4 Scale    (const CVector3d &);
CMatrix4x4 RotateX  (double);
CMatrix4x4 RotateY  (double);
CMatrix4x4 RotateZ  (double);
CMatrix4x4 MirrorX  ();
CMatrix4x4 MirrorY  ();
CMatrix4x4 MirrorZ  ();


__forceinline CVector3d Normalize(CVector3d & v) { return v / (!v);}
__forceinline CVector3f Normalize(CVector3f & v) { return v / (!v);}
__forceinline CVector2d Normalize(CVector2d & v) { return v / (!v);}
__forceinline CVector2f Normalize(CVector2f & v) { return v / (!v);}
//
// CViewVolume
//
/*
class CViewVolume
{
// Constructor/destructor
public:
   CViewVolume();
   ~CViewVolume();

// Methods
public:

	enum ProjectionType
	{
		C_ORTHOGRAPHIC,
		C_PERSPECTIVE
	};
	
	void			getMatrices(CMatrix4x4 &affine, CMatrix4x4 &proj) const;
	CMatrix4x4		getMatrix() const;
	CMatrix4x4		getCameraSpaceMatrix() const;
	void			projectPointToLine( const CVector2f & pt, CRayf & line ) const;
	void			projectPointToLine( const CVector2f & pt, CVector3f & line0, CVector3f & line1 ) const;
	CVector3f		projectToScreen(const CVector3f &src) const;
	CPlanef			getPlane( float distFromEye ) const;
	CVector3f		getSightPoint( float distFromEye ) const;
	CVector3f		getPlanePoint( float distFromEye, const CVector2f & normPoint ) const;
//	AbRotation	getAlignRotation( AbBool rightAngleOnly = false ) const;

	float		getWorldToScreenScale( const CVector3f & worldCenter, float normRadius ) const;

//	CVector2f		projectBox( const AbBox3f & box ) const;

	CViewVolume narrow( float left, float bottom, float right, float top ) const;
//	CViewVolume narrow( const AbBox3f & box ) const;

	void		ortho( float left, float right, float bottom, float top, float znear, float zfar );

	void		perspective( float fovy, float aspect, float znear, float zfar );
//	void		rotateCamera( const AbRotation & q );
//	void		translateCamera( const CVector3f & v );

	CVector3f		zVector() const;
	CViewVolume		zNarrow( float znear, float zfar ) const;

	void			scale( float factor );
	void			scaleWidth( float ratio );
	void			scaleHeight( float ratio );

	ProjectionType	getProjectionType() const;

	const CVector3f	&getProjectionPoint() const;
	const CVector3f	&getProjectionDirection() const;

	float		getNearDist() const;
	float		getWidth() const;
	float		getHeight() const;
	float		getDepth() const;
   
// Variables
protected:
};
/////////////////////////////////////////////////////////////////////

	Next class contains basic support for a quaternion object.

	quaternions are an extension of complex numbers that allows an
	expression for rotation that can be easily interpolated.  geQuaternion_s are also 
	more numericaly stable for repeated rotations than matrices.

	
	A quaternion is a 4 element 'vector'  [w,x,y,z] where:

	q = w + xi + yj + zk
	i*i = -1
	j*j = -1
	k*k = -1
	i*j = -j*i = k
	j*k = -k*j = i
	k*i = -i*k = j
	q' (conjugate) = w - xi - yj - zk
	||q|| (magnitude) = sqrt(q*q') = sqrt(w*w + x*x + y*y + z*z)
	unit quaternion ||q|| == 1; this implies  q' == qinverse 
	quaternions are associative (q1*q2)*q3 == q1*(q2*q3)
	quaternions are not commutative  q1*q2 != q2*q1
	qinverse (inverse (1/q) ) = q'/(q*q')
	
	q can be expressed by w + xi + yj + zk or [w,x,y,z] 
	or as in this implementation (s,v) where s=w, and v=[x,y,z]

	quaternions can represent a rotation.  The rotation is an angle t, around a 
	unit vector u.   q=(s,v);  s= cos(t/2);   v= u*sin(t/2).

	quaternions can apply the rotation to a point.  let the point be p [px,py,pz],
	and let P be a quaternion(0,p).  Protated = q*P*qinverse 
	( Protated = q*P*q' if q is a unit quaternion)

	concatenation rotations is similar to matrix concatenation.  given two rotations
	q1 and q2,  to rotate by q1, then q2:  let qc = (q2*q1), then the combined 
	rotation is given by qc*P*qcinverse (= qc*P*qc' if q is a unit quaternion)

	multiplication: 
	q1 = w1 + x1i + y1j + z1k
	q2 = w2 + x2i + y2j + z2k
	q1*q2 = q3 =
			(w1*w2 - x1*x2 - y1*y2 - z1*z2)     {w3}
	        (w1*x2 + x1*w2 + y1*z2 - z1*y2)i	{x3}
			(w1*y2 - x1*z2 + y1*w2 + z1*x2)j    {y3}
			(w1*z2 + x1*y2 + y1*x2 + z1*w2)k	{z3}

	also, 
	q1 = (s1,v1) = [s1,(x1,y1,z1)]
	q2 = (s2,v2) = [s2,(x2,y2,z2)]
	q1*q2 = q3	=	(s1*s2 - dot_product(v1,v2),			{s3}
					(s1*v2 + s2*v1 + cross_product(v1,v2))	{v3}

	interpolation - it is possible (and sometimes reasonable) to interpolate between
	two quaternions by interpolating each component.  This does not quarantee a 
	resulting unit quaternion, and will result in an animation with non-linear 
	rotational velocity.

	spherical interpolation: (slerp) treat the quaternions as vectors 
	find the angle between them (w = arccos(q1 dot q2) ).
	given 0<=t<=1,  q(t) = q1*(sin((1-t)*w)/sin(w) + q2 * sin(t*w)/sin(w).
	since q == -q, care must be taken to rotate the proper way.  

	this implementation uses the notation quaternion q = (quatS,quatV) 
	  where quatS is a scalar, and quatV is a 3 element vector.
/////////////////////////////////////////////////////////////////////
//
// float quaternion
//

class CQuaternionf
{
public:
	float	x, y, z, w;

public:

	CQuaternionf () { w = 1.0f; x = y = z = 0.0f; }

	CQuaternionf (const CQuaternionf& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

	CQuaternionf (float vx, float vy, float vz, float vw) : x(vx), y(vy), z(vz), w(vw) {}

	CQuaternionf (const CVector3f& v, float vw) : x(v.x), y(v.y), z(v.z), w(vw) {}

	CQuaternionf (float vw, const CVector3f& v) : x(v.x), y(v.y), z(v.z), w(vw) {}

	CQuaternionf (const CMatrix4x4& M);

public:

	operator float*() { return &x; }

	CQuaternionf& operator = (const CQuaternionf& v) { x=v.x; y=v.y; z=v.z; w=v.w; return *this; }

	CQuaternionf  operator - () const;

	CQuaternionf& operator += (const CQuaternionf& );

	CQuaternionf& operator -= (const CQuaternionf& );

	CQuaternionf& operator *= (const CQuaternionf& );

	CQuaternionf& operator *= (float );

	friend CQuaternionf operator + (const CQuaternionf&, const CQuaternionf& );

	friend CQuaternionf operator - (const CQuaternionf&, const CQuaternionf& );

	friend CQuaternionf operator * (const CQuaternionf&, const CQuaternionf& );

	friend CQuaternionf operator * (float, const CQuaternionf& );

	friend CQuaternionf operator * (const CQuaternionf&, float );

	float& operator [] (int n) {return * (&x + n); }

	float& operator () (int n) {return * (&x + n); }

	float operator ! () {return (float)sqrt(x*x + y*y + z*z + w*w); }

	int operator < (float v) { return x < v && y < v && z < v && w < v; }

	int operator > (float v) { return x > v && y > v && z > v && w > v; }

	int operator == (CQuaternionf &v) { return x == v.x && y == v.y && z == v.z && w == v.w; }

	bool Equals(const CVector4f &v, float tolerance)
	{
		return ( FastAbs(x - v.x) < FastAbs( tolerance )
				&& FastAbs(y - v.y) < FastAbs( tolerance )
				&& FastAbs(z - v.z) < FastAbs( tolerance )
				&& FastAbs(w - v.w) < FastAbs( tolerance ));
	}

	void Normalize() 
	{
		*this *= 1.0f/(!(*this));
	}

	void			SetFromAxisAngle(CVector3f Axis, float Theta);

	bool			GetAxisAngle(CVector3f& Axis, float& Theta);

	void			GetVector(CVector3f& V, float& w);

	bool			IsUnit();

	void			FromMatrix(CMatrix4x4& M);

	void			ToMatrix(CMatrix4x4& M);

	CQuaternionf&	Slerp(CQuaternionf& Q0, CQuaternionf& Q1, float T);

	CQuaternionf&	SlerpNotShortest(CQuaternionf& Q0, CQuaternionf& Q1, float T);

	CVector3f		Rotate(const CVector3f& V);
};

__forceinline CQuaternionf CQuaternionf::operator - () const
{
	return CQuaternionf(-x, -y, -z, w);
}

__forceinline CQuaternionf operator + (const CQuaternionf& v, const CQuaternionf& u)
{
	return CQuaternionf(v.x+u.x, v.y+u.y, v.z+u.z, v.w+u.w);
}

__forceinline CQuaternionf operator - (const CQuaternionf& v, const CQuaternionf& u)
{
	return CQuaternionf(v.x-u.x, v.y-u.y, v.z-u.z, v.w-u.w);
}

__forceinline CQuaternionf operator * (const CQuaternionf& v, const CQuaternionf& u)
{
	CQuaternionf	res;
	res.x =	(  (v.w*u.x) + (v.x*u.w) 
			 + (v.y*u.z) - (v.z*u.y) );
	res.y =	(  (v.w*u.y) - (v.x*u.z) 
			 + (v.y*u.w) + (v.z*u.x) );
	res.z = (  (v.w*u.z) + (v.x*u.y) 
			 - (v.y*u.x) + (v.z*u.w) );
	res.w =	(  (v.w*u.w) - (v.x*u.x) 
			 - (v.y*u.y) - (v.z*u.z) );
	return res;
}

__forceinline CQuaternionf operator * (const CQuaternionf& v, float f)
{
	return CQuaternionf(v.x*f, v.y*f, v.z*f, v.w*f);
}

__forceinline CQuaternionf operator * (float f, const CQuaternionf& v)
{
	return CQuaternionf(v.x*f, v.y*f, v.z*f, v.w*f);
}

__forceinline CQuaternionf& CQuaternionf::operator += (const CQuaternionf& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

__forceinline CQuaternionf& CQuaternionf::operator -= (const CQuaternionf& v)
{
	x -= v.x; 
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

__forceinline CQuaternionf& CQuaternionf::operator *= (const CQuaternionf& q)
{
	x =	(  (w*q.x) + (x*q.w) 
		 + (y*q.z) - (z*q.y) );
	y =	(  (w*q.y) - (x*q.z) 
		 + (y*q.w) + (z*q.x) );
	z = (  (w*q.z) + (x*q.y) 
		 - (y*q.x) + (z*q.w) );
	w =	(  (w*q.w) - (x*q.x) 
		 - (y*q.y) - (z*q.z) );
	return *this;
}

__forceinline CQuaternionf& CQuaternionf::operator *= (float v)
{
	x *= v;
	y *= v;
	z *= v;
	w *= v;
	return *this;
}
*/
