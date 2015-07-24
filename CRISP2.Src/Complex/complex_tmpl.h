#pragma once

template <typename _Tx>
class Complex
{
public:
	typedef _Tx				type;
	typedef Complex<type>	_Ctx;
	typedef Complex<int>	ComplexI;
	typedef Complex<float>	ComplexF;
	typedef Complex<double>	ComplexD;

	type i, j;

	Complex(const ComplexI &x) { i = x.i; j = x.j; }
	Complex(const ComplexF &x) { i = x.i; j = x.j; }
	Complex(const ComplexD &x) { i = x.i; j = x.j; }
	// 	Complex(int x) { i = x; j = 0; }
	// 	Complex(float x) { i = x; j = 0; }
	Complex(double x) { i = (_Tx)x; j = 0; }
	Complex(double x, double y) { i = (_Tx)x; j = (_Tx)y; }
	Complex(const POINT& p) { i = (_Tx)p.x; j = (_Tx)p.y; }
	Complex() {}

	_Ctx &operator += (const ComplexI& x)
	{
		i += (_Tx)x.i; j += (_Tx)x.j; return *this;
	}
	_Ctx &operator += (const ComplexF& x)
	{
		i += (_Tx)x.i; j += (_Tx)x.j; return *this;
	}
	_Ctx &operator += (const ComplexD& x)
	{
		i += (_Tx)x.i; j += (_Tx)x.j; return *this;
	}
	_Ctx &operator += (double x)	{ i += (_Tx)x;  return *this; }
	// 	friend	_Ctx	operator + (const _Ctx& x, double##X& y)	{return _Ctx( x.i + Z##R(y), x.j + Z##I(y) );}
#if _MSC_VER > 1200
	friend	_Ctx	operator + (const _Ctx& x, const ComplexI& y)
	{
		return _Ctx( x.i + (_Tx)y.i, x.j + (_Tx)y.j );
	}
	friend	_Ctx	operator + (const _Ctx& x, const ComplexF& y)
	{
		return _Ctx( x.i + (_Tx)y.i, x.j + (_Tx)y.j );
	}
	friend	_Ctx	operator + (const _Ctx& x, const ComplexD& y)
	{
		return _Ctx( x.i + (_Tx)y.i, x.j + (_Tx)y.j );
	}
	friend	_Ctx	operator + (type x, const _Ctx& y) { return _Ctx( x + y.i, y.j );}
	friend	_Ctx	operator + (const _Ctx& x, type y) { return _Ctx( x.i + y,  x.j );}
#endif
	_Ctx &operator + () { return *this; }

#if _MSC_VER > 1200
	// 	friend	_Ctx	operator - (const _Ctx& x, double##X& y)	{return _Ctx( x.i + Z##R(y), x.j + Z##I(y) );}
	friend	_Ctx	operator- (const _Ctx& x, const ComplexI& y)
	{
		return _Ctx( x.i - (_Tx)y.i, x.j - (_Tx)y.j );
	}
	friend	_Ctx	operator- (const _Ctx& x, const ComplexF& y)
	{
		return _Ctx( x.i - (_Tx)y.i, x.j - (_Tx)y.j );
	}
	friend	_Ctx	operator- (const _Ctx& x, const ComplexD& y)
	{
		return _Ctx( x.i - (_Tx)y.i, x.j - (_Tx)y.j );
	}
	friend	_Ctx	operator- (double x, const _Ctx& y)		{return _Ctx( (_Tx)x - y.i, -y.j );}
	friend	_Ctx	operator- (const _Ctx& x, double y)		{return _Ctx( x.i - (_Tx)y,  x.j );}
#endif
	_Ctx& operator-=(const ComplexI& x)
	{
		i -= (_Tx)x.i; j -= (_Tx)x.j; return *this;
	}
	_Ctx& operator-=(const ComplexF& x)
	{
		i -= (_Tx)x.i; j -= (_Tx)x.j; return *this;
	}
	_Ctx& operator-=(const ComplexD& x)
	{
		i -= (_Tx)x.i; j -= (_Tx)x.j; return *this;
	}
	_Ctx& operator-=(double x)				{ i -= (_Tx)x;  return *this; }
	_Ctx	operator - () { return _Ctx(-i, -j);}
	// OPERATOR *		*******************************************************************
	friend	_Ctx	operator* (const _Ctx& x, ComplexI& y)
	{
		return _Ctx( x.i*y.i - x.j*y.j, x.i*y.j + x.j*y.i );
	}
	friend	_Ctx	operator* (const _Ctx& x, ComplexF& y)
	{
		return _Ctx( x.i*y.i - x.j*y.j, x.i*y.j + x.j*y.i );
	}
	friend	_Ctx	operator* (const _Ctx& x, ComplexD& y)
	{
		return _Ctx( x.i*y.i - x.j*y.j, x.i*y.j + x.j*y.i );
	}
#if _MSC_VER > 1200
	friend	_Ctx	operator* (double x, const _Ctx& y)		{return _Ctx((_Tx)x*y.i,(_Tx)x*y.j );}
	friend	_Ctx	operator* (const _Ctx& x, double y)		{return _Ctx( x.i*(_Tx)y, x.j*(_Tx)y );}
#endif
	_Ctx& operator *= (const ComplexI& x)
	{
		_Tx r=i; i = i*x.i - j*x.j; j= r*x.j + j*x.i; return *this;
	}
	_Ctx& operator *= (const ComplexF& x)
	{
		_Tx r=i; i = i*x.i - j*x.j; j= r*x.j + j*x.i; return *this;
	}
	_Ctx& operator *= (const ComplexD& x)
	{
		_Tx r=i; i = i*x.i - j*x.j; j= r*x.j + j*x.i; return *this;
	}
	_Ctx& operator *= (double x)				{i *= (_Tx)x; j *=(_Tx)x;  return *this; }
	// OPERATOR /		*******************************************************************
#if _MSC_VER > 1200
	friend	_Ctx  operator/ (const _Ctx& x, ComplexI& y)
	{
		double t = (double)1.0 / (y.i*y.i + y.j*y.j);
		return _Ctx( t*(x.i*y.i + x.j*y.j), t*(x.j*y.i - x.i*y.j) );
	}
	friend	_Ctx  operator/ (const _Ctx& x, ComplexF& y)
	{
		double t = (double)1.0 / (y.i*y.i + y.j*y.j);
		return _Ctx( t*(x.i*y.i + x.j*y.j), t*(x.j*y.i - x.i*y.j) );
	}
	friend	_Ctx  operator/ (const _Ctx& x, ComplexD& y)
	{
		double t = (double)1.0 / (y.i*y.i + y.j*y.j);
		return _Ctx( t*(x.i*y.i + x.j*y.j), t*(x.j*y.i - x.i*y.j) );
	}
	friend	_Ctx  operator/ (double x,	const _Ctx& y)
	{
		double t = (double)x/(y.i*y.i - y.j*y.j); return _Ctx( t*y.i, -t*y.j );
	}
	friend	_Ctx  operator/ (const _Ctx& x, double y)
	{
		return _Ctx( x.i/(_Tx)y, x.j/(_Tx)y );
	}
#endif
	_Ctx& operator/=(const ComplexI& y)
	{
		_Tx r = i;
		double t = (double)1.0 / (y.i*y.i + y.j*y.j);
		i = t*(r*y.i + j*y.j);
		j = t*(j*y.i - r*y.j);
		return *this;
	}
	_Ctx& operator/=(const ComplexF& y)
	{
		_Tx r = i;
		double t = (double)1.0 / (y.i*y.i + y.j*y.j);
		i = t*(r*y.i + j*y.j);
		j = t*(j*y.i - r*y.j);
		return *this;
	}

	_Ctx& operator/=(const ComplexD& y)
	{
		_Tx r = i;
		double t = (double)1.0 / (y.i*y.i + y.j*y.j);
		i = t*(r*y.i + j*y.j);
		j = t*(j*y.i - r*y.j);
		return *this;
	}
	_Ctx& operator /= (double x)	{i /= (_Tx)x; j /=(_Tx)x;  return *this; }
	// OPERATOR ^		*******************************************************************
	_Ctx	 operator~() {return _Ctx(i,-j);}

#if _MSC_VER > 1200
	friend	_Tx	real(const _Ctx& c) { return c.i; }   // the real part
	friend	_Tx	imag(const _Ctx& c) { return c.j; }   // the imaginary part
	friend	_Tx	sqr	(const _Ctx& c) { return c.i*c.i + c.j*c.j;}
	// OVERLOADS for MATH func. ***********************************************************
	friend	double	aqrt	(_Tx x)			{return (double)sqrt((double)x);}
	friend	double	atan2	(const _Ctx& x)	{return atan2((double)x.j, (double)x.i);}
#endif


	// 	friend	_Tx	sin		(_Tx x)			{return (C)::sin ((double)x);}
	// 	friend	_Tx	cos		(_Tx x)			{return (C)::cos ((double)x);}
	friend	_Ctx	sincos	(_Tx x)			{return _Ctx( (_Tx)::cos((double)x),(_Tx)::sin((double)x) );}
	friend	_Ctx	pol		(const _Ctx& c)	{return _Ctx(sqrt((double)c.i*c.i + (double)c.j*c.j), atan2(c) );}
// 	friend	double	Gauss(double	x,	double s)
// 	{ 
// 		_asm{
// 			fld		[x]
// 			fdiv	[s]
// 			fmul	st,		st
// 				fchs
// 				fldl2e
// 				fmulp	st(1),	st
// 				fld		st
// 				frndint
// 				fsub	st(1),	st
// 				fxch	st(1)
// 				f2xm1
// 				fld1
// 				faddp	st(1),	st
// 				fscale
// 				fstp	[x]
// 			fstp	st
// 		}
// 		return	x;
// 	}
// 	friend	int	roundi(_Tx c)
// 	{
// 		_asm {
// 			fld		[c]
// 			fistp	[c]
// 		}
// 		return	*(int*)(&c);
// 	}
// 	friend	ComplexI	round(const ComplexF& c){	return	ComplexI( roundi(c.i), roundi(c.j) );}
// 	friend	ComplexI	round(const ComplexD& c){	return	ComplexI( roundi(c.i), roundi(c.j) );}

#if _MSC_VER > 1200
	friend	double	abs	(const _Ctx& c)		{ return sqrt((double)c.i*c.i + (double)c.j*c.j); }
	friend	_Tx	norm(const _Ctx& c)			{ return (_Tx)(c.i*c.i + c.j*c.j); }
#endif
};

typedef Complex<int>	ICOMPLEX;
typedef Complex<float>	FCOMPLEX;
typedef Complex<double>	DCOMPLEX;

#if _MSC_VER == 1200

template <typename _Tx, typename _Ty>
Complex<_Tx>	operator + (const Complex<_Tx>& x, const Complex<_Ty> &y)
{
	return Complex<_Tx>( x.i + (_Tx)y.i, x.j + (_Tx)y.j );
}
//////////////////////////////////////////////////////////////////////////////////////////
template <typename _Tx, typename _Ty>
Complex<_Tx>	operator - (const Complex<_Tx>& x, const Complex<_Ty>& y)
{
	return Complex<_Tx>( x.i - (_Tx)y.i, x.j - (_Tx)y.j );
}
//////////////////////////////////////////////////////////////////////////
template <typename _Tx, typename _Ty>
Complex<_Tx>  operator/ (const Complex<_Tx>& x, const Complex<_Ty>& y)
{
	double t = (double)1.0 / (y.i*y.i + y.j*y.j);
	return Complex<_Tx>( t*(x.i*y.i + x.j*y.j), t*(x.j*y.i - x.i*y.j) );
}

template <typename _Tx>
double  operator / (double x, const Complex<_Tx>& y)
{
	double t = (double)x/(y.i*y.i - y.j*y.j);
	return Complex<_Tx>( t*y.i, -t*y.j );
}

template <typename _Tx>
Complex<_Tx>  operator / (const Complex<_Tx>& x, double y)
{
	double t = 1.0 / (double)y;
	return Complex<_Tx>( x.i * t, x.j * t );
//	return Complex<_Tx>( x.i/(double)y, x.j/(double)y );
}


template <typename _Tx>
Complex<_Tx>	operator* (double x, const Complex<_Tx>& y)		{ return Complex<_Tx>((_Tx)x*y.i,(_Tx)x*y.j ); }

template <typename _Tx>
Complex<_Tx>	operator* (const Complex<_Tx>& x, double y)		{ return Complex<_Tx>( x.i*(_Tx)y, x.j*(_Tx)y ); }

template <typename _Tx>
_Tx	real(const Complex<_Tx>& c) { return c.i; }   // the real part

template <typename _Tx>
_Tx	imag(const Complex<_Tx>& c) { return c.j; }   // the imaginary part

template <typename _Tx>
_Tx	sqr	(const Complex<_Tx>& c) { return c.i*c.i + c.j*c.j;}

template <typename _Tx>
_Tx	norm(const Complex<_Tx>& c)	{ return (_Tx)(c.i*c.i + c.j*c.j); }

template <typename _Tx>
double	abs(const Complex<_Tx>& c)		{ return ::sqrt((double)c.i*c.i + (double)c.j*c.j); }

// OVERLOADS for MATH func. ***********************************************************
template <typename _Tx>
double aqrt(_Tx x)			{ return (double)::sqrt((double)x);}

template <typename _Tx>
double atan2(const Complex<_Tx>& x)	{return ::atan2((double)x.j, (double)x.i);}

#endif

__forceinline int	round(float c)
{
	_asm {
		fld		[c]
		fistp	[c]
	}
	return	*(int*)(&c);
}
__forceinline int	round(double c)
{
	_asm {
		fld		[c]
		fistp	[c]
	}
	return	*(int*)(&c);
}
__forceinline ICOMPLEX round(const FCOMPLEX& c) { return ICOMPLEX( round(c.i), round(c.j) ); }
__forceinline ICOMPLEX round(const DCOMPLEX& c) { return ICOMPLEX( round(c.i), round(c.j) ); }

__forceinline double Gauss(double x, double s)
{ 
	_asm{
		fld		[x]
		fdiv	[s]
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
