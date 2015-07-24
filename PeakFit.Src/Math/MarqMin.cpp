#include "StdAfx.h"

#include "MathKrnl.h"
#include "MarqMin.h"
#include "Gauss.h"

#include <math.h>

static NonlinearFitFunction *g_pFirstFitter = NULL;

// special runtime-class structure for NonlinearFitFunction (no base class)
const struct GRuntimeClass NonlinearFitFunction::classNonlinearFitFunction =
{ "NonlinearFitFunction", sizeof(NonlinearFitFunction), NULL, NULL };

GRuntimeClass* NonlinearFitFunction::GetRuntimeClass() const
{
	return ((GRuntimeClass*)(&NonlinearFitFunction::classNonlinearFitFunction));
}

NonlinearFitFunction::NonlinearFitFunction()
{
	m_pNext = g_pFirstFitter;
	g_pFirstFitter = this;
}

NonlinearFitFunction *NonlinearFitFunction::GetFirstFitter()
{
	return g_pFirstFitter;
}

NonlinearFitFunction *NonlinearFitFunction::FindFitter(LPCTSTR pszName)
{
	NonlinearFitFunction *pFitter = g_pFirstFitter;
	while( pFitter )
	{
		if( 0 == _tcscmp(pszName, pFitter->GetName()) )
			return pFitter;
		pFitter = pFitter->GetNextFitter();
	}
	return NULL;
}
