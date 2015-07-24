#pragma once

#include "LinAlg.h"

//#define		NON_INTEGER_HKL

#ifdef NON_INTEGER_HKL
typedef		double	HKLindex;
#else
typedef		short	HKLindex;
#endif

// this is a basic hkl-Miller indices
typedef		CVector3<HKLindex>	HKLMiller;

typedef struct REFLECTION_
{
	HKLMiller hkl;
	double dIobs;
	double dIfit;

} REFLECTION;

typedef std::vector<REFLECTION>	REFLECTIONvec;
typedef std::vector<REFLECTION>::iterator	REFLECTIONvecIt;
typedef std::vector<REFLECTION>::const_iterator	REFLECTIONvecCit;
