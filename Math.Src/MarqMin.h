#pragma once

#include "MathKrnl.h"
#include "GRuntimeClass.h"

class NonlinearFitFunction
{
public:
	virtual GRuntimeClass* GetRuntimeClass() const;
	bool IsKindOf(const GRuntimeClass* pClass) const
	{
		GRuntimeClass* pClassThis = GetRuntimeClass();
		return pClassThis->IsDerivedFrom(pClass);
	}
	static const GRuntimeClass classNonlinearFitFunction;

protected:
	NonlinearFitFunction();

	// static members
public:
	static NonlinearFitFunction *GetFirstFitter();
	static NonlinearFitFunction *FindFitter(LPCTSTR pszName);

	NonlinearFitFunction *GetNextFitter() { return m_pNext; }

	enum
	{
		TYPE_UNKNOWN = 0,
		TYPE_1D = 1,
		TYPE_2D = 2,
		TYPE_ASYMMETRIC = 4,
		TYPE_MULTIPROFILE = 8,

	};
	// virtual members
public:
	virtual int GetNumParams(int nNumProfiles) = 0;
	virtual LPCTSTR GetName() = 0;
	virtual UINT GetType() = 0;

protected:
	NonlinearFitFunction *m_pNext;
};

#include "MarqMin1.h"
