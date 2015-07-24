#pragma once

#include "Profile.h"

class NonlinearFit1Function;

//
// CProfileFitter class is in use by background subtraction procedure
//
class CProfileFitter
{
public:
	// interval array-mask definition
	typedef std::vector<bool>	Interval;
	typedef Interval::iterator	IntervalIt;

public:
	CProfileFitter(CProfile *pProf);
	virtual ~CProfileFitter();

	enum { MAX_REF_PARAMS = 32 };	// a bit more than needed
	enum {
		FIT_NONE = 0,
		FIT_POLY = 1,
		FIT_NON_LIN = 2,
	};

	NonlinearFit1Function *m_pfnFuncs;		// pointer to the fitting function (Gauss, etc.)
	int			m_nPolyOrder;	// when fitting by polinom - the order

	bool		m_bFitted;		// true if the profile has been fitted
	Interval	m_vInterval;	// the mask map (intervals) over the profile
	CProfile	*m_pProfile;	// the profile itself
	CProfile	m_vProfile;		// the profile of fitting curve
	int			m_nProfileID;	// numeric ID of the profile (0-...)
	int			m_nMA;			// number of fitting parameters
	int			m_nFitType;		// FIT_POLY etc.
	double		m_adParams[MAX_REF_PARAMS];	// maximum number of parameters
	bool		m_abParams[MAX_REF_PARAMS];	// mask for those which must be fitted

	double m_dMinVal;			// max & min values on the profile
	double m_dMaxVal;
};
