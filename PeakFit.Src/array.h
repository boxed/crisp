// array.h: interface for the array class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARRAY_H__8AC3A7AC_12D1_484A_A9DE_5F6B9024F28F__INCLUDED_)
#define AFX_ARRAY_H__8AC3A7AC_12D1_484A_A9DE_5F6B9024F28F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <cstddef>
#include <stdexcept>
#include <iterator>
#include <algorithm>

/*
The class array<..> is aggregatable since it is a class
(as well as array or structure) which: 
- Has no constructors.
- Has no nonpublic members.
- Has no base classes .
- Has no virtual functions.

  so it can be initialized as array<int, 3> = {1,1,1};
*/
template<class _Tx, size_t N>
class array
{
public:
	_Tx _a[N];	// fixed-size (N) array of elements of type _Tx

public:
	// type definitions	- according to the std::vector
	typedef array<_Tx, N>	_Mxt;
	typedef size_t			size_type;
	typedef ptrdiff_t		difference_type;
	typedef _Tx&			reference;
	typedef const _Tx&		const_reference;
	typedef _Tx				value_type;
	typedef _Tx*			iterator;
	typedef const _Tx*		const_iterator;
#if _MSC_VER >= 1300 // MSC .NET and higher
	typedef std::reverse_iterator<_Tx> reverse_iterator;
	typedef std::reverse_iterator<_Tx> const_reverse_iterator;
#else // MSC 6.0
	typedef std::reverse_iterator<iterator,_Tx> reverse_iterator;
	typedef std::reverse_iterator<const_iterator,_Tx> const_reverse_iterator;
#endif

	// assignment with type conversion
	template <typename _Ty>
	_Mxt& operator = (const array<_Ty, N>& _X)
	{
		if( this == &_X )
			;
		else
			std::copy(_X.begin(), _X.end(), begin());
		return *this;
	}

	iterator begin()
		{ return _a; }
	const_iterator begin() const
		{ return _a; }
	iterator end()
		{ return _a + N; }
	const_iterator end() const
		{ return _a + N; }
	reverse_iterator rbegin()
		{ return (reverse_iterator(end())); }
	const_reverse_iterator rbegin() const
		{ return (const_reverse_iterator(end())); }
	reverse_iterator rend()
		{ return (reverse_iterator(begin())); }
	const_reverse_iterator rend() const
		{ return (const_reverse_iterator(begin())); }
	size_type size() const
		{ return N; }
	size_type max_size()
		{ return N; }
	bool empty()
		{ return false; }	// connot be empty
	const_reference at(size_type i) const
		{ rangecheck(i); return _a[i]; }
	reference at(size_type i)
		{ rangecheck(i); return _a[i]; }
	const_reference operator [] (size_type i) const
		{ return _a[i]; }
	reference operator [] (size_type i)
		{ return _a[i]; }
	reference front()
		{ return _a[0]; }
	const_reference front() const
		{ return _a[0]; }
	reference back()
		{ return _a[N-1]; }
	const_reference back() const
		{ return _a[N-1]; }
	void assign (const _Tx& value)	// assign one value to all elements
		{ std::fill_n(begin(),size(),value); }
	void swap (_Mxt& y)
		{ std::swap_ranges(begin(),end(),y.begin()); }
	const _Tx* data() const
		{ return _a; }
	// size is constant
	friend void swap (_Mxt& _X, _Mxt& _Y)
		{ _X.swap(_Y); }
	
	// check range (may be private because it is static)
	static void rangecheck (size_type i)
		{ if (i >= size()) { throw std::range_error("array"); } }
//	enum { static_size = N };
//	static size_type size() { return N; }
//	static size_type max_size() { return N; }
//	static bool empty() { return false; }
};

// comparisons
template<class T, size_t N>
bool operator == (const array<T,N>& _X, const array<T,N>& _Y)
{
	return std::equal(_X.begin(), _X.end(), _Y.begin());
}

template<class T, size_t N>
bool operator < (const array<T,N>& _X, const array<T,N>& _Y)
{
	return std::lexicographical_compare(_X.begin(),_X.end(),_Y.begin(),_Y.end());
}

template<class T, size_t N>
bool operator != (const array<T,N>& _X, const array<T,N>& _Y)
{
	return !(_X == _Y);
}

template<class T, size_t N>
bool operator > (const array<T,N>& _X, const array<T,N>& _Y)
{
	return _Y < _X;
}

template<class T, size_t N>
bool operator <= (const array<T,N>& _X, const array<T,N>& _Y)
{
	return !(_Y < _X);
}

template<class T, size_t N>
bool operator >= (const array<T, N>& _X, const array<T, N>& _Y)
{
	return !(_X < _Y);
}
/*
	// global swap()
	template<class T, size_t N>
	inline void swap (array<T,N>& x, array<T,N>& y) {
	x.swap(y);
}
*/

#define ARRAY_LOOP(x)		for(size_t i=0;i<x.size();i++)

template<class _Tx, size_t N>
bool operator == (const array<_Tx,N>& _X, const _Tx& value)
{
//	for (size_t i = 0; i < x.size(); i++)
	ARRAY_LOOP(_X) { if (_X[i] != (_Tx)value) return false; }
	return true;
}

template<class _Tx, size_t N>
bool operator != (const array<_Tx,N>& _X, const _Tx& value)
{
//	for (size_t i = 0; i < x.size(); i++)
	ARRAY_LOOP(_X) { if (_X[i] != value) return true; }
	return false;
}

template<class _Tx, size_t N>
_Tx operator * (const array<_Tx,N>& _X, const array<_Tx,N>& _Y)
{
	_Tx result = 0;
//	for (size_t i = 0; i < _X.size(); i++) result += _X[i] * _Y[i];
	ARRAY_LOOP(_X) result += _X[i] * _Y[i];
	return result;
}

template<class _Tx, size_t N>
const array<_Tx,N> operator / (const array<_Tx,N>& _X, const _Tx& _Y)
{
	array<_Tx,N> result;
//	for (size_t i = 0; i < _X.size(); i++) result[i] = _X[i] / _Y;
	ARRAY_LOOP(_X) result[i] = _X[i] / _Y;
	return result;
}

template<class _Tx, size_t N>
const array<_Tx,N> operator * (const _Tx& _C, const array<_Tx,N>& _X)
{
	array<_Tx,N> result;
//	for (size_t i = 0; i < _X.size(); i++) result[i] = _C * _X[i];
	ARRAY_LOOP(_X) result[i] = _C * _X[i];
	return result;
}

template<class _Tx, size_t N>
array<_Tx,N> operator + (const array<_Tx,N>& _X, const array<_Tx,N>& _Y)
{
	array<_Tx,N> result;
//	for (size_t i = 0; i < _X.size(); i++) result[i] = _X[i] + _Y[i];
	ARRAY_LOOP(_X) result[i] = _X[i] + _Y[i];
	return result;
}

template<class _Tx, size_t N>
array<_Tx,N> operator - (const array<_Tx,N>& _X, const array<_Tx,N>& _Y)
{
	array<_Tx,N> result;
//	for (size_t i = 0; i < _X.size(); i++) result[i] = _X[i] - _Y[i];
	ARRAY_LOOP(_X) result[i] = _X[i] - _Y[i];
	return result;
}

template<class _Tx, size_t N>
array<_Tx,N> operator - (const array<_Tx,N>& _X)
{
	array<_Tx,N> result;
//	for (size_t i = 0; i < _X.size(); i++) result[i] = -_X[i];
	ARRAY_LOOP(_X) result[i] = -_X[i];
	return result;
}

template<class _Tx, size_t N>
std::ostream& operator << (std::ostream& os, const array<_Tx,N>& _X)
{
	os << "[";
//	for (size_t i = 0; ; )
	ARRAY_LOOP(_X)
	{
		os << _X[i];
		if(i+1 != _X.size())
			os << ",";
	}
	os << "]";
	return os;
}

template <class _Tx, size_t N>
size_t array_min_index(const array<_Tx, N>& _X)
{
	return std::min_element(_X.begin(), _X.end()) - _X.begin();
}

template <class _Tx, size_t N>
size_t array_max_index(const array<_Tx, N>& _X)
{
	return std::max_element(_X.begin(), _X.end()) - _X.begin();
}

template <class _Tx, size_t N>
array<_Tx, N> array_abs(const array<_Tx, N>& _X)
{
	array<_Tx, N> result;
//	for (size_t i = 0; i < N; i++)
	ARRAY_LOOP(_X) result[i] = (_X[i] < 0) ? -_X[i] : _X[i];
	return result;
}

#undef ARRAY_LOOP

#endif // !defined(AFX_ARRAY_H__8AC3A7AC_12D1_484A_A9DE_5F6B9024F28F__INCLUDED_)
