#include "StdAfx.h"

#include "ProfileFitter.h"

// CProfileFitter class is in use by CBackgroundFitterDlg

CProfileFitter::CProfileFitter(CProfile *pProf)
: m_pProfile(pProf)
{
	m_nPolyOrder = 0;
	m_nFitType = FIT_NONE;
	m_bFitted = false;
	m_pfnFuncs = NULL;
	m_nProfileID = 0;
	m_nMA = 0;
	memset(m_adParams, 0, sizeof(double)*MAX_REF_PARAMS);
	fill(m_abParams, m_abParams+MAX_REF_PARAMS, false);

	m_dMinVal = 0.0;
	m_dMaxVal = 0.0;
}

CProfileFitter::~CProfileFitter()
{
}
