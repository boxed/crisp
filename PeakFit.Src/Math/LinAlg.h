#ifndef __C_VECTOR_H__
#define __C_VECTOR_H__

#include <math.h>

#include "MathDef.h"
#include "Random.h"

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
// 2-component vector
//
template<class _Tx>
class CVector2
{
public:
	_Tx	x, y;

	CVector2 (_Tx f = (_Tx)0) : x(f), y(f) {}
	CVector2 (const CVector2& w) : x(w.x), y(w.y) {}
	CVector2 (_Tx vx, _Tx vy) : x(vx), y(vy) {}
	CVector2 (POINT pt) : x(_Tx(pt.x)), y(_Tx(pt.y)) {}

	operator _Tx* () { return &x; }
	template<class _Ty> CVector2& operator = (const CVector2<_Ty>& v) { x = (_Tx)v.x; y = (_Tx)v.y; return *this;}
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
		register _Tx ooz = _Tx(1.0 / f); x *= ooz; y *= ooz; return *this;
	}
	friend CVector2 operator * (const CVector2& v, const CVector2& u)
	{
		return CVector2(v.x*u.x, v.y*u.y);
	}
	friend CVector2 operator / (const CVector2& v, _Tx f)
	{
		register _Tx ooz = _Tx(1.0 / f); return CVector2((_Tx)(v.x*ooz), (_Tx)(v.y*ooz));
	}
	friend CVector2 operator / (const CVector2& v, const CVector2& u)
	{
		return CVector2(v.x/u.x, v.y/u.y);
	}
	friend CVector2 operator ^ (const CVector2&, const CVector2&);
	friend _Tx operator & (const CVector2& v, const CVector2& u) {return u.x*v.x + u.y*v.y;}
	friend _Tx operator % (const CVector2& v, const CVector2& u) {return u.x*v.x - u.y*v.y;}
	friend CVector2 Normalize (const CVector2& v) { return v / _Tx(sqrt(v.x*v.x + v.y*v.y)); }
	friend CVector2 abs (const CVector2& v) { return CVector2(abs(v.x), abs(v.y)); }

	_Tx operator ! () {return (_Tx) sqrt(x*x + y*y); }
	_Tx& operator [] (int n) {return * (&x + n); }
	_Tx& operator () (int n) {return * (&x + n); }
	int operator < (_Tx v) { return x < v && y < v; }
	int operator > (_Tx v) { return x > v && y > v; }
	int operator == (CVector2 &v) { return x == v.x && y == v.y; }

	_Tx Sqr() { return (x*x + y*y); }
	BOOL	Equals(CVector2 &v, _Tx tolerance)
	{
		return ( fabs(x - v.x) < fabs( tolerance )
				&& fabs(y - v.y) < fabs( tolerance ));
	}
	void Normalize() 
	{
		*this /= !(*this);
	}
};

template<typename _Tx>
CVector2<_Tx> operator * (_Tx f, const CVector2<_Tx>& v)
	{ return CVector2<_Tx>(v.x*f, v.y*f); }

template<typename _Tx>
CVector2<_Tx> operator * (const CVector2<_Tx>& v, _Tx f)
	{ return CVector2<_Tx>(v.x*f, v.y*f); }

template<typename _Tx>
CVector2<_Tx> operator + (const CVector2<_Tx>& v, const CVector2<_Tx>& u)
	{ return CVector2<_Tx>(v.x+u.x, v.y+u.y); }

template<typename _Tx>
CVector2<_Tx> operator - (const CVector2<_Tx>& v, const CVector2<_Tx>& u)
	{ return CVector2<_Tx>(v.x-u.x, v.y-u.y); }

typedef CVector2<short>		CVector2s;
typedef CVector2<long>		CVector2l;
typedef CVector2<float>		CVector2f;
typedef CVector2<double>	CVector2d;
//
// 3-component vector
//
template <class _Tx>
class CVector3
{
public:
	_Tx	x, y, z;

	CVector3 (_Tx f = (_Tx)0) : x(f), y(f), z(f) {}
	CVector3 (const CVector3& w) : x(w.x), y(w.y), z(w.z) {}
	CVector3 (_Tx vx, _Tx vy, _Tx vz) : x(vx), y(vy), z(vz) {}

	operator _Tx*() { return &x; }
	CVector3& operator = (const CVector3& v) { x=v.x; y=v.y; z=v.z; return *this;}
	CVector3& operator = (_Tx f) { x=y=z=f; return *this;}
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
	CVector3& operator *= (_Tx f)
	{
		x *= f; y *= f; z *= f; return *this;
	}
	CVector3& operator /= (_Tx f)
	{
		register double ooz = 1.0/f; x *= ooz; y *= ooz; z *= ooz; return *this;
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
	friend CVector3 operator * (_Tx f, const CVector3& u)
	{
		return CVector3(u.x*f, u.y*f, u.z*f);
	}
	friend CVector3 operator * (const CVector3& u, _Tx f)
	{
		return CVector3(u.x*f, u.y*f, u.z*f);
	}
	friend CVector3 operator / (const CVector3& u, _Tx f)
	{
		register double ooz = 1.0/f; return CVector3(_Tx(u.x*ooz), _Tx(u.y*ooz), _Tx(u.z*ooz));
	}
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
	friend CVector3 Normalize (const CVector3& v) { return v / (!v); }
	friend CVector3 abs (const CVector3& v) { return CVector3(abs(v.x), abs(v.y), abs(v.z)); }

	_Tx operator ! () { return (_Tx) sqrt(x*x + y*y + z*z); }
	_Tx& operator [] (int n) { return * (&x + n); }
	_Tx& operator () (int n) { return * (&x + n); }
	int operator < (_Tx v) { return x < v && y < v && z < v; }
	int operator > (_Tx v) { return x > v && y > v && z > v; }
	int operator == (CVector3 &v) { return x == v.x && y == v.y && z == v.z; }

	_Tx Sqr() { return (x*x + y*y + z*z); }
	BOOL	Equals(CVector3 &v, _Tx tolerance)
	{
		return ( fabs(x - v.x) < fabs( tolerance )
				&& fabs(y - v.y) < fabs( tolerance )
				&& fabs(z - v.z) < fabs( tolerance ));
	}
	void Normalize() 
	{
		*this /= !(*this);
	}
	void RndVector()
	{
		*this = CVector3(CFloatRND() - 0.5f*RAND_MAX, CFloatRND() - 0.5f*RAND_MAX, CFloatRND() - 0.5f*RAND_MAX);
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

typedef CVector3<short>	 CVector3s;
typedef CVector3<long>	 CVector3l;
typedef CVector3<double> CVector3d;

//
// 3 float component vector
//
class CVector3f
{
public:
	float	x, y, z;

	CVector3f (float f = 0.0f) : x(f), y(f), z(f) {}
	CVector3f (const CVector3f& w) : x(w.x), y(w.y), z(w.z) {}
	CVector3f (float vx, float vy, float vz) : x(vx), y(vy), z(vz) {}

	operator float*() { return &x; }
	CVector3f& operator = (const CVector3f& v) { x=v.x; y=v.y; z=v.z; return *this;}
	CVector3f& operator = (float f) { x=y=z=f; return *this;}
	CVector3f	operator - () const;
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

	BOOL	Equals(CVector3f &v, float tolerance)
	{
		return ( fabs(x - v.x) < fabs( tolerance )
				&& fabs(y - v.y) < fabs( tolerance )
				&& fabs(z - v.z) < fabs( tolerance ));
	}
	void Normalize() { *this /= !(*this); }
	friend CVector3f Normalize(CVector3f & v) { return v / (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);}
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
};

inline CVector3f CVector3f::operator - () const
{
	return CVector3f(-x, -y, -z);
}

inline CVector3f operator + (const CVector3f& v, const CVector3f& u)
{
	return CVector3f(v.x+u.x, v.y+u.y, v.z+u.z);
}

inline CVector3f operator - (const CVector3f& v, const CVector3f& u)
{
	return CVector3f(v.x-u.x, v.y-u.y, v.z-u.z);
}

inline CVector3f operator * (const CVector3f& v, const CVector3f& u)
{
	return CVector3f(v.x*u.x, v.y*u.y, v.z*u.z);
}

inline CVector3f operator * (const CVector3f& v, float f)
{
	return CVector3f(v.x*f, v.y*f, v.z*f);
}

inline CVector3f operator * (float f, const CVector3f& v)
{
	return CVector3f(v.x*f, v.y*f, v.z*f);
}

inline CVector3f operator / (const CVector3f& v, const CVector3f& u)
{
	return CVector3f(v.x/u.x, v.y/u.y, v.z/u.z);
}

inline CVector3f operator / (const CVector3f& v, float f)
{
	register float ooz = 1.0f/f;
	return CVector3f(v.x*ooz, v.y*ooz, v.z*ooz);
}

inline CVector3f& CVector3f::operator += (const CVector3f &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

inline CVector3f& CVector3f::operator -= (const CVector3f &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

inline CVector3f& CVector3f::operator *= (float v)
{
	x *= v;
	y *= v;
	z *= v;
	return *this;
}

inline CVector3f& CVector3f::operator *= (const CVector3f &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

inline CVector3f& CVector3f::operator /= (float v)
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
	CVector4f (float f = 0.0f) : x(f), y(f), z(f), w(1) {}
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

	BOOL	Equals(CVector4f &v, float tolerance)
	{
		return ( fabs(x - v.x) < fabs( tolerance )
				&& fabs(y - v.y) < fabs( tolerance )
				&& fabs(z - v.z) < fabs( tolerance )
				&& fabs(w - v.w) < fabs( tolerance ));
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

inline CVector4f CVector4f::operator - () const
{
	return CVector4f(-x, -y, -z, -w);
}

inline CVector4f operator + (const CVector4f& v, const CVector4f& u)
{
	return CVector4f(v.x+u.x, v.y+u.y, v.z+u.z, v.w+u.w);
}

inline CVector4f operator - (const CVector4f& v, const CVector4f& u)
{
	return CVector4f(v.x-u.x, v.y-u.y, v.z-u.z, v.w-u.w);
}

inline CVector4f operator * (const CVector4f& v, const CVector4f& u)
{
	return CVector4f(v.x*u.x, v.y*u.y, v.z*u.z, v.w*u.w);
}

inline CVector4f operator * (const CVector4f& v, float f)
{
	return CVector4f(v.x*f, v.y*f, v.z*f, v.w*f);
}

inline CVector4f operator * (float f, const CVector4f& v)
{
	return CVector4f(v.x*f, v.y*f, v.z*f, v.w*f);
}

inline CVector4f operator / (const CVector4f& v, const CVector4f& u)
{
	return CVector4f(v.x/u.x, v.y/u.y, v.z/u.z, v.w/u.w);
}

inline CVector4f operator / (const CVector4f& v, float f)
{
	register float ooz = 1.0f/f;
	return CVector4f(v.x*ooz, v.y*ooz, v.z*ooz, v.w*ooz);
}

inline CVector4f& CVector4f::operator += (const CVector4f &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

inline CVector4f& CVector4f::operator -= (const CVector4f &v)
{
	x -= v.x; 
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

inline CVector4f& CVector4f::operator *= (float v)
{
	x *= v;
	y *= v;
	z *= v;
	w *= v;
	return *this;
}

inline CVector4f& CVector4f::operator *= (const CVector4f &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}

inline CVector4f& CVector4f::operator /= (float v)
{
	register float ooz = 1.0f/v;
	x *= ooz;
	y *= ooz;
	z *= ooz;
	w *= ooz;
	return *this;
}
//
// class CRay3
//
template<class _Tx>
class CRay3
{
// Variables
public:
	CVector3<_Tx>	m_vOrg;
	CVector3<_Tx>	m_vDir;

public:
	CRay3 () {}
	CRay3 (CVector3<_Tx> &o, CVector3<_Tx> &d) { m_vOrg = o; m_vDir = d; }

public:
	CVector3d Point(double t) { return m_vOrg + m_vDir*t; }

	// returns the distance from the point to this line
//	double PointDistance(CVector3<_Tx> &p) { return ((p - m_vOrg) & m_vDir) / m_vDir.Sqr();}
	double PointDistance(CVector3<_Tx> &p)
	{
		double t = ((p - m_vOrg) & m_vDir) / m_vDir.Sqr();
		return !(p - (m_vOrg + (t * m_vDir)));
	}
};

typedef CRay3<double>	CRay3d;
typedef CRay3<float>	CRay3f;

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
template<class _Tx>
class CPlane
{
// Variables
public:
	CVector3<_Tx>	m_vN;
	_Tx				m_fDist;

public:
	CPlane	() { m_vN = CVector3<_Tx>(0, 0, 1); m_fDist = 0; }
	CPlane	(_Tx a, _Tx b, _Tx c, _Tx dist);
	CPlane	(const CPlane<_Tx>& p) { m_vN = p.m_vN; m_fDist = p.m_fDist;}
	CPlane	(const CVector3<_Tx>& norm, _Tx dist)
	{
		Create(norm, dist);
	}
	CPlane	(const CVector3<_Tx>& norm, const CVector3<_Tx>& point);
	CPlane	(const CVector3<_Tx>& v1, const CVector3<_Tx>& v2, const CVector3<_Tx>& v3)
	{
		Create(v1, v2, v3);
	}

	void		Create(const CVector3<_Tx>& v1, const CVector3<_Tx>& v2, const CVector3<_Tx>& v3);
	void		Create(const CVector3<_Tx>& norm, const _Tx& dist) { m_vN = norm; m_fDist = dist; }

	_Tx			ClassifyPoint(CVector3<_Tx>& v) { return (m_vN & v) + m_fDist; }
	BOOL		Intersect(CRay3<_Tx> &, double &);
	CPlane&	operator = (const CPlane<_Tx>& p) { m_vN = p.m_vN; m_fDist = p.m_fDist; return *this;}
};
//
// CMatrix4x4
//
class CMatrix4x4
{
// Constructor/destructor
public:
  	CMatrix4x4();
	CMatrix4x4(float fVal);
	~CMatrix4x4();

// Variables
public:
  	float d[4][4];

public:
	void	LoadMatrix(float* pfData);
  	void	LoadIdentity();
  	void	ZeroMatrix();
  	void	Invert();
  	void	Transpose();
	BOOL	IsOrthonormal();

public:
	CMatrix4x4& operator += (const CMatrix4x4&);
	CMatrix4x4& operator -= (const CMatrix4x4&);
	CMatrix4x4& operator *= (float v);
	CMatrix4x4& operator *= (const CMatrix4x4&);
	CMatrix4x4& operator /= (const CMatrix4x4&);
	operator float*() { return &d[0][0]; }
	float* operator []( int i ) { return &d[i][0]; }

  	friend CMatrix4x4 operator + (const CMatrix4x4 &, const CMatrix4x4 &);
  	friend CMatrix4x4 operator - (const CMatrix4x4 &, const CMatrix4x4 &);
  	friend CMatrix4x4 operator * (const CMatrix4x4 &, float);
  	friend CVector3f operator * (const CMatrix4x4 &, const CVector3f &);
  	friend CVector3f operator * (const CVector3f &, const CMatrix4x4 &);
  	friend CMatrix4x4 operator * (const CMatrix4x4 &, const CMatrix4x4 &);

	CVector3f GetTranslate(void);

	void Translate(const CVector3f &);
	void Scale    (const CVector3f &);
	void RotateX  (float);
	void RotateY  (float);
	void RotateZ  (float);
	void MirrorX  ();
	void MirrorY  ();
	void MirrorZ  ();
};

CMatrix4x4 Translate(const CVector3f &);
CMatrix4x4 Scale    (const CVector3f &);
CMatrix4x4 RotateX  (float);
CMatrix4x4 RotateY  (float);
CMatrix4x4 RotateZ  (float);
CMatrix4x4 MirrorX  ();
CMatrix4x4 MirrorY  ();
CMatrix4x4 MirrorZ  ();
/*
//
// CViewVolume
//
class CViewVolume
{
// Constructor/destructor
public:
   CViewVolume();
   ~CViewVolume();

// Methods
public:
	enum ProjectionType {
		C_ORTHOGRAPHIC,
		C_PERSPECTIVE
	};
	
	void			getMatrices(CMatrix4x4 &affine, CMatrix4x4 &proj) const;
	CMatrix4x4		getMatrix() const;
	CMatrix4x4		getCameraSpaceMatrix() const;

	void			projectPointToLine( const CVector2f & pt, CRay & line ) const;
	void			projectPointToLine( const CVector2f & pt, CVector3f & line0, CVector3f & line1 ) const;
	CVector3f		projectToScreen(const CVector3f &src) const;

	CPlane		getPlane( float distFromEye ) const;
	CVector3f		getSightPoint( float distFromEye ) const;
	CVector3f		getPlanePoint( float distFromEye, const CVector2f & normPoint ) const;
//	AbRotation	getAlignRotation( AbBool rightAngleOnly = FALSE ) const;
	float		getWorldToScreenScale( const CVector3f & worldCenter, float normRadius ) const;

//	CVector2f		projectBox( const AbBox3f & box ) const;

	CViewVolume narrow( float left, float bottom, float right, float top ) const;
//	CViewVolume narrow( const AbBox3f & box ) const;

	void		ortho( float left, float right, float bottom, float top, float znear, float zfar );
	void		perspective( float fovy, float aspect, float znear, float zfar );
//	void		rotateCamera( const AbRotation & q );
//	void		translateCamera( const CVector3f & v );

	CVector3f		zVector() const;
	CViewVolume	zNarrow( float znear, float zfar ) const;

	void			scale( float factor );
	void			scaleWidth( float ratio );
	void			scaleHeight( float ratio );

	ProjectionType		getProjectionType() const;
	const CVector3f	&getProjectionPoint() const;
	const CVector3f	&getProjectionDirection() const;

	float		getNearDist() const;
	float		getWidth() const;
	float		getHeight() const;
	float		getDepth() const;
   
// Variables
protected:
};
*/
/***************************************************************************

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

********************************************/
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

	BOOL	Equals(CVector4f &v, float tolerance)
	{
		return ( fabs(x - v.x) < fabs( tolerance )
				&& fabs(y - v.y) < fabs( tolerance )
				&& fabs(z - v.z) < fabs( tolerance )
				&& fabs(w - v.w) < fabs( tolerance ));
	}
	void Normalize() 
	{
		*this *= 1.0f/(!(*this));
	}
	void			SetFromAxisAngle(CVector3f Axis, float Theta);
	BOOL			GetAxisAngle(CVector3f& Axis, float& Theta);
	void			GetVector(CVector3f& V, float& w);
	BOOL			IsUnit();
	void			FromMatrix(CMatrix4x4& M);
	void			ToMatrix(CMatrix4x4& M);
	CQuaternionf&	Slerp(CQuaternionf& Q0, CQuaternionf& Q1, float T);
	CQuaternionf&	SlerpNotShortest(CQuaternionf& Q0, CQuaternionf& Q1, float T);
	CVector3f		Rotate(const CVector3f& V);
};

inline CQuaternionf CQuaternionf::operator - () const
{
	return CQuaternionf(-x, -y, -z, w);
}

inline CQuaternionf operator + (const CQuaternionf& v, const CQuaternionf& u)
{
	return CQuaternionf(v.x+u.x, v.y+u.y, v.z+u.z, v.w+u.w);
}

inline CQuaternionf operator - (const CQuaternionf& v, const CQuaternionf& u)
{
	return CQuaternionf(v.x-u.x, v.y-u.y, v.z-u.z, v.w-u.w);
}

inline CQuaternionf operator * (const CQuaternionf& v, const CQuaternionf& u)
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

inline CQuaternionf operator * (const CQuaternionf& v, float f)
{
	return CQuaternionf(v.x*f, v.y*f, v.z*f, v.w*f);
}

inline CQuaternionf operator * (float f, const CQuaternionf& v)
{
	return CQuaternionf(v.x*f, v.y*f, v.z*f, v.w*f);
}

inline CQuaternionf& CQuaternionf::operator += (const CQuaternionf& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

inline CQuaternionf& CQuaternionf::operator -= (const CQuaternionf& v)
{
	x -= v.x; 
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

inline CQuaternionf& CQuaternionf::operator *= (const CQuaternionf& q)
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

inline CQuaternionf& CQuaternionf::operator *= (float v)
{
	x *= v;
	y *= v;
	z *= v;
	w *= v;
	return *this;
}

#endif // __C_VECTOR_H__