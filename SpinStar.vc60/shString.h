#pragma once

#include <tchar.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdarg.h>
#if _MSC_VER >= 1300
#include <hash_map>
#endif

#ifndef FRAMEWORK_API
#	define	FRAMEWORK_API	extern
#endif

#ifndef _CHAR_DEFINED
#define _CHAR_DEFINED
	typedef char CHAR;
#endif // _CHAR_DEFINED

#ifndef _WCHAR_DEFINED
#define _WCHAR_DEFINED
	typedef wchar_t WCHAR;
#endif // _WCHAR_DEFINED

#ifdef _UNICODE
	typedef WCHAR TCHAR;
#else
	typedef CHAR TCHAR;
#endif // _UNICODE

typedef WCHAR *LPWSTR;
typedef const WCHAR *LPCWSTR;
typedef CHAR *LPSTR;
typedef const CHAR *LPCSTR;
typedef TCHAR *LPTSTR;
typedef const TCHAR *LPCTSTR;

// Note - not in the original STL, but exists in SGI STL and STLport
#ifdef EXT_HASH
#   include <ext/hash_map>
#   include <ext/hash_set>
#else
//#   include <hash_set>
//#   include <hash_map>
#endif

//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
//disable warning about non-dll...
#pragma warning (disable : 4275)

#if _UNICODE
    typedef std::wstring StringBase;
//	STRINGLIB_EXPIMP_TEMPLATE template class STRINGLIB_API std::basic_string<wchar_t>;
#else
    typedef std::string StringBase;
//	extern template class __declspec(dllimport) std::basic_string<char>;
//	STRINGLIB_EXPIMP_TEMPLATE template class STRINGLIB_API std::basic_string<char>;//string;
#endif

#ifdef GCC_3_1
#   define HashMap ::__gnu_cxx::hash_map
#else
//#   if OGRE_COMPILER == COMPILER_MSVC
#       if _MSC_VER > 1300 && !defined(_STLP_MSVC)
#           define HashMap ::stdext::hash_map
#       else
#           define HashMap ::std::hash_map
#       endif
#		undef _DEFINE_DEPRECATED_HASH_CLASSES
#		if _MSC_VER > 1300
#			define _DEFINE_DEPRECATED_HASH_CLASSES 0
#		else
#			define _DEFINE_DEPRECATED_HASH_CLASSES 1
#		endif
//#   else
//#       define HashMap ::std::hash_map
//#   endif
#endif

#if !defined( _STLP_HASH_FUN_H )
#if _MSC_VER > 1300
#if _DEFINE_DEPRECATED_HASH_CLASSES
namespace std 
#else
namespace stdext
#endif
{
    template<>
	size_t hash_compare<StringBase, std::less<StringBase> >::operator ()
		( const StringBase& str ) const
    {
		// This is the PRO-STL way, but it seems to cause problems with VC7.1
		// and in some other cases (although I can't recreate it)
		// hash<const char*> H;
		// return H(_stringBase.c_str());
        register size_t ret = 0;
		StringBase::const_iterator it;
        for(it = str.begin(); it != str.end(); ++it )
            ret = 31 * ret + *it;
		
        return ret;
    }
}
#endif // _MSC_VER
#endif // _STLP_HASH_FUN_H
/*
#ifndef STRINGLIB_API
#error STRINGLIB_API must be defined as DLL export in oder to use Framework.DLL
#endif
*/

class String;

typedef std::vector<String> StringVec;
typedef std::vector<String>::iterator StringVecIt;
typedef std::vector<String>::const_iterator StringVecCit;

FRAMEWORK_API void StringFormatV(LPCTSTR lpszFormat, va_list argList, String &str);
FRAMEWORK_API LPTSTR SkipWhite(LPTSTR pszString);
FRAMEWORK_API LPCTSTR SkipWhite(LPCTSTR pszString);
FRAMEWORK_API LPTSTR FindWhite(LPTSTR pszString);
FRAMEWORK_API LPCTSTR FindWhite(LPCTSTR pszString);

class String : public StringBase
{
public:
//	typedef std::stringstream StrStreamType;
	typedef StringBase::value_type value_type;

public:
	// Default constructor.
	String() : StringBase() {}
	~String() {}
	
	String(const String& rhs) : StringBase( static_cast< const StringBase& >( rhs ) ) {}
	
	// Copy constructor for std::string's.
	String( const StringBase& rhs ) : StringBase( rhs ) {}
	
	// Copy-constructor for C-style strings.
	String( LPCTSTR rhs ) : StringBase( rhs ) {}

	void clear() { *this = _T(""); }
	
	// Used for interaction with functions that require the old C-style strings.
	// TODO: convert if in UNICODE
	operator LPCTSTR () const { return (LPCTSTR)c_str(); }
	operator LPTSTR () { return (LPTSTR)c_str(); }
	
	// Removes any whitespace characters, be it standard space or TABs and so on.
	//  --- Remarks
	// The user may specify whether they want to trim only the
	// beginning or the end of the String ( the default action is to trim both).
	void trim(bool left = true, bool right = true)
	{
		size_t i, lspaces, rspaces, len = length();
		lspaces = rspaces = 0;
		LPCTSTR p;
		if( true == left )
		{
			p = c_str();
			// Find spaces / tabs on the left
			for( i = 0;
//				i < len && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
				i < len && ( *p == ' ' || *p == '\t' || *p == '\r');
				++lspaces, ++i, ++p );
		}
		if( (true == right) && (lspaces < len) )
		{
			p = c_str() + (len - 1);
			// Find spaces / tabs on the right
			for( i = len - 1;
//				i >= 0 && ( at(i) == ' ' || at(i) == '\t' || at(i) == '\r');
				i >= 0 && ( *p == ' ' || *p == '\t' || *p == '\r');
				rspaces++, i--, p++ );
		}
		*this = substr(lspaces, len - lspaces - rspaces);
	}
	// Returns a StringVector that contains all the substrings delimited
	// by the characters in the passed <code>delims</code> argument.
	//  @param 
	//      delims A list of delimiter characters to split by
	//  @param 
	//      maxSplits The maximum number of splits to perform (0 for unlimited splits). If this
	//      parameters is > 0, the splitting process will stop after this many splits, left to right.
	bool split(std::vector<String> &ret, const String& delims = _T("\t\n "),
		size_t maxSplits = 0) const
	{
		// static unsigned dl;
//		std::vector<String> ret;
		size_t numSplits = 0;
		size_t start, pos;
		
		// Use STL methods 
		start = 0;
		ret.clear();
		do 
		{
			if( start == (pos = find_first_of(delims, start)) )
			{
				// Do nothing
				start = pos + 1;
			}
			else if( (pos == npos) || (maxSplits && numSplits == maxSplits) )
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
		} while( pos != npos );

		return true;
	}
	// Upper-cases all the characters in the string.
	String &toLowerCase()
	{
		std::transform(begin(), end(), begin(), static_cast<int(*)(int)>(::tolower) );
		return *this;
	}
	// Lower-cases all the characters in the string.
	String &toUpperCase()
	{
		std::transform(begin(), end(), begin(), static_cast<int(*)(int)>(::toupper) );
		return *this;
	}
	// Converts the contents of the string to a Real.
	//@remarks
	//    Assumes the only contents of the string are a valid parsable Real. Defaults to  a
	//    value of 0.0 if conversion is not possible.
	double toReal(LPTSTR *ppszEndChar) const
	{
		return _tcstod(this->c_str(), ppszEndChar);
/*
		double dValue = 0;
		_stscanf(this->c_str(), _T("%lf"), &dValue);
		return dValue;
*/
	}
	
	// Returns whether the string begins with the pattern passed in.
	// @param pattern The pattern to compare with.
	// @param lowerCase If true, the end of the string will be lower cased before 
	//    comparison, pattern should also be in lower case.
	bool startsWith(const String& pattern, bool lowerCase = true) const
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
	// Returns whether the string ends with the pattern passed in.
	// @param pattern The pattern to compare with.
	// @param lowerCase If true, the end of the string will be lower cased before 
	//     comparison, pattern should also be in lower case.
	bool endsWith(const String& pattern, bool lowerCase = true) const
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
	//  Template operator for appending another type into the string. 
	//  @remarks
	//      Because this operator is templated, you can append any value into a string as
	//      long as there is an operator<<(std::basic_iostream, type) or similar method defined somewhere.
	//      All the primitive types have this already, and many of the Ogre types do too (see Vector3
	//      for an example).
	template<typename _Tx>
		String& operator << (_Tx value)
	{
		// Create stringstream to convert
		StrStreamType sstr;
		// Write value to string
		sstr << value;
		// Append
		*this += sstr.str();
		
		return *this;
	}
	// simple sub-string extraction

	// return nCount characters starting at zero-based nFirst
	String Mid(size_t nFirst, size_t nCount) const
	{
		String dest;
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

		dest.append(*this, nFirst, nCount);
		return dest;
	}
	// return all characters starting at zero-based nFirst
	String Mid(size_t nFirst) const
	{
		return Mid(nFirst, length() - nFirst);
	}
	// return first nCount characters in string
	String Left(size_t nCount) const
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
	String Right(size_t nCount) const
	{
		if( nCount < 0 )
			nCount = 0;
		if( nCount >= length() )
			return *this;

		String dest;
		dest.append(*this, length()-nCount, nCount);
		return dest;
	}

	// simple formatting

	// printf-like formatting using passed string
	void Format(LPCTSTR lpszFormat, ...)
	{
		va_list argList;
		va_start(argList, lpszFormat);
		Formatv(lpszFormat, argList);
//		StringFormatV(lpszFormat, argList, *this);
		va_end(argList);
	}
	// printf-like formatting using referenced string resource
//	void /*AFX_CDECL*/ format(size_t nFormatID, ...);
	// printf-like formatting using variable arguments parameter
	void Formatv(LPCTSTR lpszFormat, va_list argList)
	{
		StringFormatV(lpszFormat, argList, *this);
	}

	// Constant blank string, useful for returning by ref where local does not exist
//	static const String BLANK;
	static const String &BLANK() { static const String _blank = _T(""); return _blank; }
	// scans all tokens into String vector
	bool ScanMultiple(StringVec &vFields)
	{
		return ScanMultiple(c_str(), vFields);
	}
	static bool ScanMultiple(LPCTSTR pszString, StringVec &vFields)
	{
//		String token;
		std::vector<TCHAR> token;
		LPCTSTR ptr = pszString;
		LPTSTR pszTok;
		token.resize(_tcslen(pszString) + 1);
		pszTok = (LPTSTR)&*token.begin();
		// clear the vector
		vFields.clear();
		// iterate
		while( 1 )
		{
			// skip white symbols
//			ptr = SkipWhite(ptr);
			// read a token if possible
			if( 1 != _stscanf(ptr, _T("%s"), pszTok) )
			{
				break;
			}
			// add token to the list
			vFields.push_back(&*token.begin());//.c_str());
			// find the token
			if( NULL == (ptr = _tcsstr(ptr, pszTok)) )
			{
				break;
			}
			// shift within the string
			ptr += _tcslen(pszTok);//.c_str());
		}
		return !vFields.empty();
	}
};

//#endif

/*
typedef std::vector<String> StringVec;
typedef std::vector<String>::iterator StringVecIt;
typedef std::vector<String>::const_iterator StringVecCit;
*/

#if _MSC_VER > 1300
#if !defined( _STLP_HASH_FUN_H )
#if _DEFINE_DEPRECATED_HASH_CLASSES
	typedef std::hash_compare<StringBase, std::less<StringBase > > _StringHash;
#else
	typedef stdext::hash_compare<StringBase, std::less<StringBase > > _StringHash;
#endif
#else
    typedef std::hash<StringBase > _StringHash;
#endif
#endif // _MSC_VER

/*
#ifdef _DEBUG
#pragma comment( lib, "StringLibD.lib" )
#else
#pragma comment( lib, "StringLib.lib" )
#endif
*/

/*
typedef StringBase String;

STRINGLIB_API void StringFormatV(LPCTSTR lpszFormat, va_list argList, String &str);

// printf-like formatting using passed string
STRINGLIB_API void StringFormat(String &str, LPCTSTR lpszFormat, ...);

// Upper-cases all the characters in the string.
__forceinline String &toLowerCase(String &str)
{
	std::transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(::tolower) );
	return str;
}
// Lower-cases all the characters in the string.
__forceinline String &toUpperCase(String &str)
{
	std::transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(::toupper) );
	return str;
}
// return nCount characters starting at zero-based nFirst
__forceinline String StringMid(const String &str, int nFirst, int nCount)
{
	String dest;
	String::size_type len = str.length();
	// out-of-bounds requests return sensible things
	if( nFirst < 0 )
		nFirst = 0;
	if( nCount < 0 )
		nCount = 0;

	if( nFirst + nCount > len )
		nCount = len - nFirst;
	if( nFirst > len )
		nCount = 0;

//	ASSERT(nFirst >= 0);
//	ASSERT(nFirst + nCount <= GetData()->nDataLength);

	// optimize case of returning entire string
	if( (0 == nFirst) && (nFirst + nCount == len) )
		return str;

	dest.append(str, nFirst, nCount);
	return dest;
}
// return all characters starting at zero-based nFirst
__forceinline String StringMid(const String &str, int nFirst)
{
	return StringMid(str, nFirst, str.length() - nFirst);
}
// return first nCount characters in string
__forceinline String StringLeft(const String &str, int nCount)
{
	if( nCount < 0 )
		nCount = 0;
	if( nCount >= str.length() )
		return str;

	String dest;
	dest.append(str, 0, nCount);
	return dest;
}
// return nCount characters from end of string
__forceinline String StringRight(const String &str, int nCount)
{
	if( nCount < 0 )
		nCount = 0;
	if( nCount >= str.length() )
		return str;

	String dest;
	dest.append(str, str.length() - nCount, nCount);
	return dest;
}
*/
