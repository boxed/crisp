#include "StdAfx.h"

#include "globals.h"
#include "trace.h"
#include "HelperFn.h"

// printf-like formatting using variable arguments parameter
#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000

#define TCHAR_ARG   TCHAR
#define WCHAR_ARG   WCHAR
#define CHAR_ARG    char

#ifdef _X86_
	#define DOUBLE_ARG  double
//	#define DOUBLE_ARG  double_AFX_DOUBLE
#else
	#define DOUBLE_ARG  double
#endif

#ifndef _UNICODE
#	define _wcsinc(ptr) (((wchar_t*)ptr)+1)
#endif

//////////////////////////////////////////////////////////////////////////

void PrintWCharText(LPTSTR pszBuffer, LPCTSTR pszFormat, ...)
{
	try
	{
		va_list args;
		va_start(args, pszFormat);
		{
#ifndef _UNICODE
//			size_t len = strlen(pszFormat);
//			char *buffer = new char[len + 2];
//			memset(buffer, 0, (len + 2) * sizeof(char));
			vsprintf(TMP, pszFormat, args);
			size_t len = strlen(TMP);
			char *buffer = new char[len + 2];
			memset(buffer, 0, (len + 2) * sizeof(char));
			strcpy(buffer, TMP);
			mbstowcs((wchar_t*)pszBuffer, buffer, len+1);
			delete buffer;
//			wchar_t *wszFormat = new wchar_t[len + 2];
//			LPWSTR buffer = (LPWSTR)pszBuffer;
//			memset(wszFormat, 0, (len + 2) * sizeof(wchar_t));
//			mbstowcs(wszFormat, pszFormat, len);
//			len = vswprintf(buffer, wszFormat, args);
//			if( len > 0 )
//			{
//				buffer[len] = 0;
//				buffer[len+1] = 0;
//			}
//			delete[] wszFormat;
#else
			size_t len;
			len = vswprintf((LPWSTR)pszBuffer, pszFormat, args);
			if( len > 0 )
			{
				pszBuffer[len] = 0;
			}
#endif
		}
		va_end(args);
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("PrintWCharText() -> %s"), e.what()));
//		AfxMessageBox(_T("Failed to print debug string message."));
	}
}

void TextToDlgItemA(HWND hDlg, UINT nID, const char *pszFormat, ...)
{
	va_list argList;

	va_start(argList, pszFormat);
//	PrintWCharText(TMP, lpszFormat, argList);
#ifndef _UNICODE
	{
		size_t len = strlen(pszFormat);
		wchar_t *wszFormat = new wchar_t[len + 2];
		LPWSTR buffer = (LPWSTR)TMP;
		memset(wszFormat, 0, (len + 2) * sizeof(wchar_t));
		mbstowcs(wszFormat, pszFormat, len);
		len = wvsprintfW(buffer, wszFormat, argList);
		if( len > 0 )
		{
			buffer[len] = 0;
			buffer[len+1] = 0;
		}
		delete[] wszFormat;
	}
#else
	{
		size_t len;
		len = wvsprintfW((LPWSTR)TMP, pszFormat, argList);
		if( len > 0 )
		{
			pszBuffer[len] = 0;
		}
	}
#endif
	va_end(argList);

	// set Dialog item text
	SetDlgItemTextW(hDlg, nID, (wchar_t*)TMP);
}

void TextToDlgItemW(HWND hDlg, UINT nID, LPCSTR lpszFormat, ...)
{
	va_list argList;

	va_start(argList, lpszFormat);

	vsprintf(TMP, lpszFormat, argList);

	va_end(argList);

	// set Dlg item text
	SetDlgItemTextA(hDlg, nID, TMP);
}

#if 0
void TextToDlgItemW(HWND hDlg, UINT nID, const wchar_t *lpszFormat, ...)
{
	va_list argList;

	va_start(argList, lpszFormat);
	size_t len;
	len = wvsprintfW((LPWSTR)TMP, lpszFormat, argList);
	if( len > 0 )
	{
		TMP[len] = 0;
	}
//	PrintWCharText((LPTSTR)TMP, lpszFormat, argList);
	va_end(argList);

	// set Dialog item text
	SetDlgItemTextW(hDlg, nID, (wchar_t*)TMP);
}
#endif
