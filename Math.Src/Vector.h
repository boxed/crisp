// Vector.h: interface for the CVector class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <valarray>
using namespace std;

//
// general vector
//
class CVector : public std::valarray<double>
{
//	friend CVector operator* ( const CMatrix&, const CVector& );
public:
	CVector() : valarray<double>() { }
	CVector(size_t _S) : valarray<double>(_S) { }
	CVector(double _V, size_t _S) : valarray<double>(_V, _S) { }
	CVector(double *_V, size_t _S) : valarray<double>(_V, _S) { }
	~CVector() { }

	double *data() { return &operator[](0); }
//	double *data() { return &((*(valarray<double>*)this)[0]); }
//	operator double *() { return &((*(valarray<double>*)this)[0]); }
//	double operator [] (size_t _I) const { return *((double*)&((valarray<double>*)this)[_I]); }
//	double &operator [] (size_t _I) { return *((double*)&((valarray<double>*)this)[_I]); }
#ifdef _DEBUG
//	void Print() { for(int i=0; i < size(); i++) TRACE1("%9.4f", this->operator[](i)); TRACE0("\n"); }
#endif
};
