// Profile.h: interface for the CProfile class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CProfile : public std::vector<double>  
{
public:
	typedef std::vector<CVector2f>	Vector;
	typedef std::vector<CVector2f>::iterator	VectorIt;

	explicit CProfile()
		: std::vector<double>(), m_bInitialized(false) {}
	explicit CProfile(size_type n, const double& v = double())
		: std::vector<double>(n, v), m_bInitialized(false) {}
	CProfile(const CProfile& x)
		: std::vector<double>(x), m_nWidth(x.m_nWidth), m_bInitialized(false) {}

	virtual ~CProfile() {}

	CProfile &operator -= (const double &x)
	{
		std::transform(begin(), end(), begin(), std::bind2nd(std::minus<double>(), x));
		return *this;
	}

	bool GetBaseLine(int nLevel);
	bool GetBackground(double dGrad, double dAvg, double &dBkg, int &nBkgSize);
	bool Smooth();

	virtual bool Initialize();
	virtual void Reset();
	virtual void GetPoints(Vector &pts);
	virtual void SingleStep();

public:
	int m_nWidth;		// the width of the profile

protected:
	bool m_bInitialized;
};
