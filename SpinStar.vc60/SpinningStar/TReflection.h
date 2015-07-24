// TReflection.h: interface for the TReflection class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "SpinningStar/UnitCell.h"
#include "SpinningStar/Spacegroups.h"

class TReflection  
{
public:
	TReflection();
	virtual ~TReflection() {}

	HKLMiller	hkl;	// real hkl data
//	CVector3d	HKL;	// HKL * 2PI
	double		d;		// hkl-plane distance
	double		Theta;	// Theta, in RADIANS?
	double		Fhkl;	// amplitude
	double		Pha;	// phase, in RADIANS
	double		A;		// Fhkl*cos(Pha)
	double		B;		// Fhkl*sin(Pha)
	double		LP;		// Lorentz polarization
	double		Int;	// intensity = Fhkl^2 * mult * LP
	int			mult;	// multiplicity
	size_t		p;		// p-factor for the calculation of |Ehkl|
	size_t		eps;	// systematically enhanced factor
	double		Sg;		// excitation error

	bool operator == (const HKLMiller &_hkl) { hkl.Equals(_hkl, 1e-6); }

	typedef std::less<double> ASC;
	typedef std::greater<double> DESC;

	// sort functions
	template <class OP>
	struct sort_d : std::binary_function<TReflection, TReflection, bool>
	{
		bool operator () (const TReflection &r1, const TReflection &r2) const
		{
//			return r1.d < r2.d;
			return OP()(r1.d, r2.d);
		}
	};
	// Fhkl
	template <class OP>
	struct sort_Fhkl : std::binary_function<TReflection, TReflection, bool>
	{
		bool operator () (const TReflection &r1, const TReflection &r2) const
		{
			return OP()(r1.Fhkl, r2.Fhkl);
		}
	};

	struct sort_hkl : std::binary_function<TReflection, TReflection, bool>
	{
	//	static TCHAR tmp[256];
		bool operator() (const TReflection &r1, const TReflection &r2) const
		{
	//		_stprintf(tmp, _T("%.4f (%d, %d, %d) - %.4f (%d, %d, %d)\n"),
	//			r1.d, r1.h, r1.k, r1.l, r2.d, r2.h, r2.k, r2.l);
	//		::OutputDebugString(tmp);
			if( r1.d < r2.d )
				return false;
			else if( r1.d > r2.d )
				return true;
			else
			{
				if( r1.hkl.x < r2.hkl.x )
					return true;
				else if( r1.hkl.x > r2.hkl.x )
					return false;
				else
				{
					if( r1.hkl.y < r2.hkl.y )
						return true;
					else if( r1.hkl.y > r2.hkl.y )
						return false;
					else
					{
						if( r1.hkl.z < r2.hkl.z )
							return true;
					}
				}
			}
			return false;
		}
	};

};

class TReflsVec
{
public:
	typedef TReflection _Ty;
	typedef std::vector<_Ty> _Myt;

	typedef _Myt::difference_type difference_type;
//	typedef _Myt::pointer _Tptr;
//	typedef _Myt::const_pointer _Ctptr;
	typedef _Myt::reference reference;
	typedef _Myt::const_reference const_reference;
	typedef _Myt::size_type size_type;
	typedef _Myt::value_type value_type;
	typedef _Myt::iterator iterator;
	typedef _Myt::const_iterator const_iterator;
	typedef _Myt::const_reverse_iterator const_reverse_iterator;
	typedef _Myt::reverse_iterator reverse_iterator;
	typedef const_iterator _Cit;
public:
	TReflsVec() : _refls()
		{ _AngleType = UnitCell::DEGREES; }
	explicit TReflsVec(size_type _N, const _Ty& _V = _Ty())
		: _refls(_N, _V)
		{ _AngleType = UnitCell::DEGREES; }
	TReflsVec(const _Myt& _X)
		: _refls(_X)
		{ _AngleType = UnitCell::DEGREES; }
	TReflsVec(_Cit _F, _Cit _L)
		: _refls(_F, _L)
		{ _AngleType = UnitCell::DEGREES; }
	~TReflsVec()
	{}
	TReflsVec& operator = (const TReflsVec& _X)
		{ _refls = _X._refls; _AngleType = _X._AngleType; return *this; }
	_Myt& operator = (const _Myt& _X)
		{ _refls = _X; _AngleType = UnitCell::DEGREES; return _refls; }
	void reserve(size_type _N)
		{ _refls.reserve(_N); }
	size_type capacity() const
		{ return _refls.capacity(); }
	iterator begin()
		{ return _refls.begin(); }
	const_iterator begin() const
		{ return _refls.begin(); }
	iterator end()
		{ return _refls.end(); }
	const_iterator end() const
		{ return _refls.end(); }
	reverse_iterator rbegin()
		{ return _refls.rbegin(); }
	const_reverse_iterator rbegin() const
		{ return _refls.rbegin(); }
	reverse_iterator rend()
		{ return _refls.rend(); }
	const_reverse_iterator rend() const
		{ return _refls.rend(); }
	void resize(size_type _N, const _Ty& _X = _Ty())
		{ _refls.resize(_N, _X);	}
	size_type size() const
		{ return _refls.size(); }
	size_type max_size() const
		{ return _refls.max_size(); }
	bool empty() const
		{ return _refls.empty(); }
	const_reference at(size_type _P) const
		{ return _refls.at(_P); }
	reference at(size_type _P)
		{ return _refls.at(_P); }
	const_reference operator[](size_type _P) const
		{ return _refls.operator[](_P); }
	reference operator[](size_type _P)
		{ return _refls.operator[](_P); }
	reference front()
		{ return _refls.front(); }
	const_reference front() const
		{ return _refls.front(); }
	reference back()
		{ return _refls.back(); }
	const_reference back() const
		{ return _refls.back(); }
	void push_back(const _Ty& _X)
		{ _refls.push_back(_X); }
	void pop_back()
		{ _refls.pop_back(); }
	void assign(_Cit _F, _Cit _L)
		{ _refls.assign(_F, _L); }
	void assign(size_type _N, const _Ty& _X = _Ty())
		{ _refls.assign(_N, _X); }
	iterator insert(iterator _P, const _Ty& _X = _Ty())
		{ return _refls.insert(_P, _X); }
	void insert(iterator _P, size_type _M, const _Ty& _X)
		{ _refls.insert(_P, _M, _X); }
	void insert(iterator _P, _Cit _F, _Cit _L)
		{ _refls.insert(_P, _F, _L); }
	iterator erase(iterator _P)
		{ return _refls.erase(_P); }
	iterator erase(iterator _F, iterator _L)
		{ return _refls.erase(_F, _L); }
	void clear()
		{ _refls.clear(); }
	void swap(_Myt& _X)
		{ _refls.swap(_X); }

public:
	// checks the list of reflections to belong to a specific zone (2D reflections list)
	// or to be 3D reflections list
//	bool CheckReflsType(CVector3d &zone, int &nZoneLayer);
//	bool GetZoneRefls(const CVector3d &zone, TReflsVec &vRefls,
//						  bool bUseZoneLayer, int nIndexNum, int nZoneLayer);
//	bool ExpandReflections(const SpaceGroup &SG, TReflsVec &vReflList);
	bool MergeReflections(const SpaceGroup &SG, TReflsVec &vReflList, bool bMergeFriedel = true);
//	bool MergeNoFriedelReflections(const SpaceGroup &SG, TReflsVec &vReflList);

	UnitCell::ANGLES AnleType() { return _AngleType; }
	void AnleType(UnitCell::ANGLES Angles) { _AngleType = Angles; }

protected:
	_Myt _refls;		// the list of reflections
	UnitCell::ANGLES _AngleType;		// RADIANS or DEGREES (DEG by default)
};

//typedef std::vector<TReflection>					TReflsVec;
//typedef std::vector<TReflection>::iterator			TReflsVecIt;
//typedef std::vector<TReflection>::const_Citerator	TReflsVecCit;
typedef TReflsVec::iterator							TReflsVecIt;
typedef TReflsVec::const_iterator					TReflsVecCit;
typedef std::list<TReflection*>						TReflsList;
typedef std::list<TReflection*>::iterator			TReflsListIt;
typedef std::list<TReflection*>::const_iterator		TReflsListCit;
typedef std::multimap<long, TReflection*>			TReflsMap;
typedef std::multimap<long, TReflection*>::iterator TReflsMapIt;
typedef std::multimap<long, TReflection*>::const_iterator TReflsMapCit;

