#include "stdafx.h"

#include "trace.h"
#include "globals.h"
#include "time.h"

//////////////////////////////////////////////////////////////////////////

namespace _tstd
{
	_tstd::string format(LPCTSTR pszFormat, ...)
	{
		va_list argList;
		va_start(argList, pszFormat);
		int nSize = 1024;
		int nArraySize = 1024;
		std::vector<TCHAR> buffer;
		do
		{
			buffer.resize(nArraySize);
			nSize = _vsntprintf(&*buffer.begin(), nArraySize, pszFormat, argList);
			nArraySize *= 2;
		}
		while( nSize >= nArraySize );
		va_end(argList);
		return _tstd::string(&*buffer.begin());
	}

	//////////////////////////////////////////////////////////////////////////
}

// c-tors
Format::Format(const _tstd::string &_format) : m_format(_format) {}
Format::Format(LPCTSTR _format) : m_format(_format) {}

// #define		INSTATIATE_FORMAT(TYPE) 		template Format & Format::operator % (const TYPE &value)	{ m_format % value; return *this; }
#define		INSTATIATE_FORMAT(TYPE) 		Format & Format::operator % (const TYPE &value)	{ m_format % value; return *this; }
#define		INSTATIATE_FORMAT2(TYPE) 		Format & Format::operator % (TYPE value)	{ m_format % value; return *this; }

INSTATIATE_FORMAT(char);
INSTATIATE_FORMAT(unsigned char);
INSTATIATE_FORMAT(short);
INSTATIATE_FORMAT(unsigned short);
INSTATIATE_FORMAT(int);
INSTATIATE_FORMAT(unsigned int);
INSTATIATE_FORMAT(long);
INSTATIATE_FORMAT(unsigned long);
INSTATIATE_FORMAT(float);
INSTATIATE_FORMAT(double);
INSTATIATE_FORMAT2(const char*);
INSTATIATE_FORMAT2(const wchar_t*);

_tstd::string Format::str() const { return m_format.str(); }

//////////////////////////////////////////////////////////////////////////

const _tstd::string&	 GetInstallPath()
{
	static _tstd::string installPath;
	if (installPath.empty())
	{
		TCHAR filename[MAX_PATH];
		ZeroMemory(filename, sizeof(filename));
		GetModuleFileName(hInst, filename, MAX_PATH);

		// set last backslash to NULL to get the path
		for (int i = MAX_PATH-1; i > 0; i--)
		{
			if (filename[i] == _T('\\'))
			{
				filename[i] = NULL;
				break;
			}
		}
		installPath = filename;
	}
	return installPath;
}

bool FileExists(const _tstd::string& inPath)
{
	OFSTRUCT of;
	ZeroMemory(&of, sizeof(of));
	of.cBytes = sizeof(of);
	return OpenFile(inPath.c_str(), &of, OF_EXIST) != HFILE_ERROR;
}

_tstd::string GetTimeFormatted()
{
	time_t now;
	time(&now);
	TCHAR buf[1024];
	struct tm *t = localtime(&now);
	//return _tasctime(when);
	_tcsftime(buf, 1024, _T("%Y-%m-%d %H:%M:%S"), t);
	return buf;
}

class Tracer
{
public:
	static Tracer& instance()
	{
		static Tracer t;
		return t;
	}

	void Write(const TCHAR* pszIn)
	{
		DWORD bytesWritten = 0;
		_tstd::string output = _FMT(_T("%s %d - %s\r\n"), GetTimeFormatted().c_str(), (clock()-mLastClock), pszIn);
		WriteFile(mFile, output.c_str(), output.size()*sizeof(TCHAR), &bytesWritten, NULL);
		mSize += bytesWritten;
		
		if (mSize >= 1024*1024*10) // 10 meg
		{
			_tstd::string newFilename = _FMT(_T("%s\\trace2.txt"), GetInstallPath().c_str());
			if (FileExists(newFilename.c_str()))
			{
				DeleteFile(newFilename.c_str());
			}
			CloseHandle(mFile);
			MoveFile(_FMT(_T("%s\\trace.txt"), GetInstallPath().c_str()).c_str(), newFilename.c_str());
			Bind();
		}
		
		mLastClock = clock();
	}

private:
	Tracer()
		: mSize(0)
		, mFile(INVALID_HANDLE_VALUE)
		, mLastClock(clock())
	{
		Bind();
	}

	void Bind()
	{
		mFile = CreateFile(_FMT(_T("%s\\trace.txt"), GetInstallPath().c_str()).c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (mFile == INVALID_HANDLE_VALUE)
		{
			GetLastError();
			return;
		}
		DWORD r = SetFilePointer(mFile, NULL, NULL, FILE_END);
		if (r == 0xFFFFFFFF) //INVALID_SET_FILE_POINTER
		{
			GetLastError();
			return;
		}
		mSize = GetFileSize(mFile, NULL);
	}

	DWORD mSize;
	HANDLE mFile;
	int mLastClock;
};

void Trace(const TCHAR* in)
{
	OutputDebugString(in);
	OutputDebugString(_T("\n"));
	Tracer::instance().Write(in);
}

void Trace(const Format& in)
{
	Trace(in.str().c_str());
}

_tstd::string GetLastErrorString()
{
	TCHAR buf[1024];
	ZeroMemory(buf, sizeof(buf));
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buf, 1024, NULL);
	_tstd::string result = buf;
	return result;
}

_tstd::string GetUserErrorAndTrace(const TCHAR* in)
{
	Trace(in);
	return _T("A fatal error has occurred. It has been logged in the trace.txt file in your installation directory. Please send this file and a description of the problem to info@calidris.se");
}