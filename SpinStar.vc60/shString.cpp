// String.cpp: implementation of the String class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#include <algorithm>
//#include <stdarg.h>
#include <malloc.h>
#include "shString.h"

LPTSTR SkipWhite(LPTSTR pszString)
{
	while( 1 )
	{
		if( ((*pszString == TCHAR(' ')) || (*pszString == TCHAR('\t'))) &&
			(TCHAR('\0') != *pszString) )
		{
			pszString++;
		}
		else
		{
			break;
		}
	}
	return pszString;
}

LPCTSTR SkipWhite(LPCTSTR pszString)
{
	while( 1 )
	{
		if( ((*pszString == TCHAR(' ')) || (*pszString == TCHAR('\t'))) &&
			(TCHAR('\0') != *pszString) )
		{
			pszString++;
		}
		else
		{
			break;
		}
	}
	return pszString;
}

LPTSTR FindWhite(LPTSTR pszString)
{
	while( 1 )
	{
		if( ((*pszString != TCHAR(' ')) && (*pszString != TCHAR('\t'))) &&
			(TCHAR('\0') != *pszString) )
//		if( ((TCHAR(' ') == *pszString) || (TCHAR('\t') == *pszString)) &&
//			(TCHAR('\0') != *pszString) )
		{
			pszString++;
		}
		else
		{
			break;
		}
	}
	return pszString;
}

LPCTSTR FindWhite(LPCTSTR pszString)
{
	while( 1 )
	{
		if( ((*pszString != TCHAR(' ')) && (*pszString != TCHAR('\t'))) &&
			(TCHAR('\0') != *pszString) )
//		if( ((TCHAR(' ') == *pszString) || (TCHAR('\t') == *pszString)) &&
//			(TCHAR('\0') != *pszString) )
		{
			pszString++;
		}
		else
		{
			break;
		}
	}
	return pszString;
}
/*
const String String::BLANK = String(_T(""));

//-----------------------------------------------------------------------
void String::trim(bool left, bool right)
{
	size_t lspaces, rspaces, len = length(), i;
	
	lspaces = rspaces = 0;
	
	if( left )
	{
		// Find spaces / tabs on the left
		for( i = 0;
		i < len && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
		++lspaces, ++i );
	}
	
	if( right && lspaces < len )
	{
		// Find spaces / tabs on the right
		for( i = len - 1;
		i >= 0 && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
		rspaces++, i-- );
	}
	
	*this = substr(lspaces, len-lspaces-rspaces);
}

//-----------------------------------------------------------------------
std::vector<String> String::split( const String& delims, unsigned int maxSplits) const
{
	// static unsigned dl;
	std::vector<String> ret;
	unsigned int numSplits = 0;
	
	// Use STL methods 
	size_t start, pos;
	start = 0;
	do 
	{
		pos = find_first_of(delims, start);
		if (pos == start)
		{
			// Do nothing
			start = pos + 1;
		}
		else if (pos == npos || (maxSplits && numSplits == maxSplits))
		{
			// Copy the rest of the string
			ret.push_back( substr(start) );
			break;
		}
		else
		{
			// Copy up to delimiter
			ret.push_back( substr(start, pos - start) );
			start = pos + 1;
		}
		// parse up to next real data
		start = find_first_not_of(delims, start);
		++numSplits;
		
	} while (pos != npos);
	
	
	
	return ret;
}

//-----------------------------------------------------------------------
String String::toLowerCase(void) 
{
	std::transform(
		begin(),
		end(),
		begin(),
		static_cast<int(*)(int)>(::tolower) );
	
	return *this;
}
//-----------------------------------------------------------------------
String String::toUpperCase()
{
	std::transform(
		begin(),
		end(),
		begin(),
		static_cast<int(*)(int)>(::toupper) );
	
	return *this;
}
//-----------------------------------------------------------------------
double String::toReal() const
{
	double dValue = 0;
	_stscanf(this->c_str(), _T("%lf"), &dValue);
	return dValue;
}
//-----------------------------------------------------------------------
bool String::startsWith(const String& pattern, bool lowerCase) const
{
	size_t thisLen = this->length();
	size_t patternLen = pattern.length();
	if (thisLen < patternLen || patternLen == 0)
		return false;
	
	String startOfThis = substr(0, patternLen);
	if (lowerCase)
		startOfThis.toLowerCase();
	
	return (startOfThis == pattern);
}
//-----------------------------------------------------------------------
bool String::endsWith(const String& pattern, bool lowerCase) const
{
	size_t thisLen = this->length();
	size_t patternLen = pattern.length();
	if (thisLen < patternLen || patternLen == 0)
		return false;
	
	String endOfThis = substr(thisLen - patternLen, patternLen);
	if (lowerCase)
		endOfThis.toLowerCase();
	
	return (endOfThis == pattern);
}
//-----------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
// Very simple sub-string extraction

// return nCount characters starting at zero-based nFirst
String String::mid(int nFirst, int nCount) const
{
	// out-of-bounds requests return sensible things
	if( nFirst < 0 )
		nFirst = 0;
	if( nCount < 0 )
		nCount = 0;

	if( nFirst + nCount > length() )
		nCount = length() - nFirst;
	if( nFirst > length() )
		nCount = 0;

//	ASSERT(nFirst >= 0);
//	ASSERT(nFirst + nCount <= GetData()->nDataLength);

	// optimize case of returning entire string
	if( (0 == nFirst) && (nFirst + nCount == length()) )
		return *this;

	String dest;
	dest.append(*this, nFirst, nCount);
	return dest;
}
// return all characters starting at zero-based nFirst
String String::mid(int nFirst) const
{
	return mid(nFirst, length() - nFirst);
}
// return first nCount characters in string
String String::left(int nCount) const
{
	if( nCount < 0 )
		nCount = 0;
	if( nCount >= length() )
		return *this;

	String dest;
	dest.append(*this, 0, nCount);
	return dest;
}
// return nCount characters from end of string
String String::right(int nCount) const
{
	if( nCount < 0 )
		nCount = 0;
	if( nCount >= length() )
		return *this;

	String dest;
	dest.append(*this, length()-nCount, nCount);
//	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

// printf-like formatting using passed string
void String::format(LPCTSTR lpszFormat, ...)
{
//	ASSERT(AfxIsValidString(lpszFormat));

	va_list argList;
	va_start(argList, lpszFormat);
	formatv(lpszFormat, argList);
	va_end(argList);
}
*/

#define TCHAR_ARG   TCHAR
#define WCHAR_ARG   WCHAR
#define CHAR_ARG    char

#ifdef _X86_
	#define DOUBLE_ARG  double
#else
	#define DOUBLE_ARG  double
#endif

// printf-like formatting using variable arguments parameter
#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000
/*
void StringFormat(LPCTSTR lpszFormat, va_list argList, String &str);

void StringFormatV(String &str, LPCTSTR lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	StringFormatV(lpszFormat, argList, str);
	va_end(argList);
}
*/
/*FRAMEWORK_API*/ void StringFormatV(LPCTSTR lpszFormat, va_list argList, String &str)
{
//	ASSERT(AfxIsValidString(lpszFormat));

	va_list argListSave = argList;

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	for(LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(lpsz = _tcsinc(lpsz)) == '%')
		{
			nMaxLen += _tclen(lpsz);
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = _ttoi(lpsz);
			for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
				;
		}
//		ASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz = _tcsinc(lpsz);

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz = _tcsinc(lpsz);
			}
			else
			{
				nPrecision = _ttoi(lpsz);
				for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
					;
			}
//			ASSERT(nPrecision >= 0);
		}

		// should be on type modifier or specifier
		int nModifier = 0;
		if (_tcsncmp(lpsz, _T("I64"), 3) == 0)
		{
			lpsz += 3;
			nModifier = FORCE_INT64;
#if !defined(_X86_) && !defined(_ALPHA_)
			// __int64 is only available on X86 and ALPHA platforms
//			ASSERT(FALSE);
#endif
		}
		else
		{
			switch (*lpsz)
			{
			// modifiers that affect size
			case 'h':
				nModifier = FORCE_ANSI;
				lpsz = _tcsinc(lpsz);
				break;
			case 'l':
				nModifier = FORCE_UNICODE;
				lpsz = _tcsinc(lpsz);
				break;

			// modifiers that do not affect size
			case 'F':
			case 'N':
			case 'L':
				lpsz = _tcsinc(lpsz);
				break;
			}
		}

		// now should be on specifier
		switch (*lpsz | nModifier)
		{
		// single characters
		case 'c':
		case 'C':
			nItemLen = 2;
			va_arg(argList, TCHAR_ARG);
			break;
		case 'c'|FORCE_ANSI:
		case 'C'|FORCE_ANSI:
			nItemLen = 2;
			va_arg(argList, CHAR_ARG);
			break;
		case 'c'|FORCE_UNICODE:
		case 'C'|FORCE_UNICODE:
			nItemLen = 2;
			va_arg(argList, WCHAR_ARG);
			break;

		// strings
		case 's':
			{
				LPCTSTR pstrNextArg = va_arg(argList, LPCTSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;  // "(null)"
				else
				{
				   nItemLen = _tcslen(pstrNextArg);
				   nItemLen = __max(1, nItemLen);
				}
			}
			break;

		case 'S':
			{
#ifndef _UNICODE
				LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;  // "(null)"
				else
				{
				   nItemLen = wcslen(pstrNextArg);
				   nItemLen = __max(1, nItemLen);
				}
#else
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = strlen(pstrNextArg);
				   nItemLen = __max(1, nItemLen);
				}
#endif
			}
			break;

		case 's'|FORCE_ANSI:
		case 'S'|FORCE_ANSI:
			{
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = strlen(pstrNextArg);
				   nItemLen = __max(1, nItemLen);
				}
			}
			break;

		case 's'|FORCE_UNICODE:
		case 'S'|FORCE_UNICODE:
			{
				LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = wcslen(pstrNextArg);
				   nItemLen = __max(1, nItemLen);
				}
			}
			break;
		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			if (nPrecision != 0)
				nItemLen = __min(nItemLen, nPrecision);
			nItemLen = __max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				if (nModifier & FORCE_INT64)
					va_arg(argList, __int64);
				else
					va_arg(argList, int);
				nItemLen = 32;
				nItemLen = __max(nItemLen, nWidth+nPrecision);
				break;

			case 'e':
			case 'g':
			case 'G':
				va_arg(argList, DOUBLE_ARG);
				nItemLen = 128;
				nItemLen = __max(nItemLen, nWidth+nPrecision);
				break;

			case 'f':
				{
					double f;
					LPTSTR pszTemp;

					// 312 == strlen("-1+(309 zeroes).")
					// 309 zeroes == max precision of a double
					// 6 == adjustment in case precision is not specified,
					//   which means that the precision defaults to 6
					pszTemp = (LPTSTR)_alloca(__max(nWidth, 312+nPrecision+6));

					f = va_arg(argList, double);
					_stprintf( pszTemp, _T( "%*.*f" ), nWidth, nPrecision+6, f );
					nItemLen = _tcslen(pszTemp);
				}
				break;

			case 'p':
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = __max(nItemLen, nWidth+nPrecision);
				break;

			// no output
			case 'n':
				va_arg(argList, int*);
				break;

			default:
//				ASSERT(FALSE);  // unknown formatting option
				;
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	// new code
	String tmp;
/*
	LPTSTR pszText = tmp.GetBuffer(nMaxLen + 1);
	if( NULL != pszText )
	{
		_vstprintf(pszText, lpszFormat, argListSave);
		LPTSTR pszText2 = str.GetBuffer(_tcslen(pszText) + 1);
		if( NULL != pszText2 )
		{
			_tcscpy(pszText2, pszText);
		}
	}
/*/
	tmp.resize(nMaxLen + 1);
	_vstprintf(&*tmp.begin(), lpszFormat, argListSave);
	str.resize(_tcslen(tmp.c_str()) + 1);
	_tcscpy((LPTSTR)&str.begin()[0], tmp.c_str());
//*/
	// end of new code
	// old code
//	resize(nMaxLen+1);
//	_vstprintf(&*begin(), lpszFormat, argListSave);
	// end of old code

//	GetBuffer(nMaxLen);
//	VERIFY(_vstprintf(m_pchData, lpszFormat, argListSave) <= GetAllocLength());
//	ReleaseBuffer();

	va_end(argListSave);
}
