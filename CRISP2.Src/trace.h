#pragma once

#define		DO_NOT_USE_BOOST_FORMAT

#include <boost/format.hpp>

#ifndef _T
#	ifdef UNICODE
#		define _T L
#	else
#		define _T
#	endif
#endif

namespace _tstd
{
#ifdef UNICODE
	typedef std::wstring string;
	typedef wchar_t _tchar;
#else
	typedef std::string string;
	typedef char _tchar;
#endif

	typedef boost::basic_format<_tchar> boost_format;

	_tstd::string format(LPCTSTR pszFormat, ...);

#define		DEFINE_FORMAT(TYPE) 		Format & operator % (const TYPE &value)
#define		DEFINE_FORMAT2(TYPE) 		Format & operator % (TYPE value)
}

class Format
{
public:
	Format(const _tstd::string &_format);
	Format(LPCTSTR _format);

	DEFINE_FORMAT(char);
	DEFINE_FORMAT(unsigned char);
	DEFINE_FORMAT(short);
	DEFINE_FORMAT(unsigned short);
	DEFINE_FORMAT(int);
	DEFINE_FORMAT(unsigned int);
	DEFINE_FORMAT(long);
	DEFINE_FORMAT(unsigned long);
	DEFINE_FORMAT(float);
	DEFINE_FORMAT(double);
// 		DEFINE_FORMAT2(char*);
// 		DEFINE_FORMAT2(wchar_t*);
	DEFINE_FORMAT2(const char*);
	DEFINE_FORMAT2(const wchar_t*);
	//template <class _Tx>
	//Format & operator % (const _Tx &value);

	_tstd::string str() const;

protected:
	_tstd::boost_format m_format;
};

const _tstd::string& GetInstallPath();
_tstd::string GetLastErrorString();

#define _FMT		_tstd::format

_tstd::string GetUserErrorAndTrace(const TCHAR* in);
inline _tstd::string GetUserErrorAndTrace(const _tstd::string& in) { return GetUserErrorAndTrace(in.c_str()); }

void Trace(const TCHAR* in);
void Trace(const Format& in);
inline void Trace(const _tstd::string &sInput) { Trace(sInput.c_str()); }

