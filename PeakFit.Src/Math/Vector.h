// Vector.h: interface for the CVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VECTOR_H__32E233C6_C071_444E_B162_0ADB0FA7EC2F__INCLUDED_)
#define AFX_VECTOR_H__32E233C6_C071_444E_B162_0ADB0FA7EC2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <valarray>
using namespace std;

//
// general vector
//
class CVector : public valarray<double>
{
//	friend CVector operator* ( const CMatrix&, const CVector& );
public:
	CVector() : valarray<double>() { }
	CVector(size_t _S) : valarray<double>(_S) { }
	CVector(double _V, size_t _S) : valarray<double>(_V, _S) { }
	CVector(double *_V, size_t _S) : valarray<double>(_V, _S) { }
	~CVector() { free(); }

	double *data() { return &operator[](0); }
//	double *data() { return &((*(valarray<double>*)this)[0]); }
//	operator double *() { return &((*(valarray<double>*)this)[0]); }
//	double operator [] (size_t _I) const { return *((double*)&((valarray<double>*)this)[_I]); }
//	double &operator [] (size_t _I) { return *((double*)&((valarray<double>*)this)[_I]); }
#ifdef _DEBUG
	void Print() { for(size_t i=0; i < size(); i++) TRACE1("%9.4f", this->operator[](i)); TRACE0("\n"); }
#endif
};

#endif // !defined(AFX_VECTOR_H__32E233C6_C071_444E_B162_0ADB0FA7EC2F__INCLUDED_)
