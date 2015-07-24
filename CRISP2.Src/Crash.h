#pragma once

#include <stdio.h>

//
// guard/unguardf/unguard macros.
// For showing calling stack when errors occur in major functions.
// Meant to be enabled in release builds.
//
#define		appUnwindf		::OutputDebugString
#define		DO_GUARD		1

struct classPrintLog
{
public:
	classPrintLog(FILE *pFile, LPCTSTR pszFuncName)
	{
		m_pFile = pFile;
		m_pszFuncName = pszFuncName;
		if( NULL != pFile )
		{
			fprintf(pFile, "enter: %s\n", pszFuncName);
			fflush(pFile);
		}
	}
	~classPrintLog()
	{
		if( NULL != m_pFile )
		{
			fprintf(m_pFile, "exit:  %s\n", m_pszFuncName);
			fflush(m_pFile);
		}
	}
	FILE *m_pFile;
	LPCTSTR m_pszFuncName;
};

long __stdcall CrashHandler(struct _EXCEPTION_POINTERS *ExceptionInfo);
void CrashHandlerTranslator(unsigned int, EXCEPTION_POINTERS* pExc);

class UnhandledException : public std::exception
{
	typedef std::exception super;
public:
	UnhandledException()
		: super(_T("System exception"))
	{
	}
};