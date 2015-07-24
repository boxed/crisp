// THIS FILE WAS ADDED BY PETER OON 23 NOVEMBER 2004

#include "StdAfx.h"

#include "crash.h"
#include "trace.h"
#include <tlhelp32.h>

#include "Crash/StackWalker.h"

struct ModuleInfo
{
	const char *name;
	unsigned long base, size;
};

// ARRGH.  Where's psapi.h?!?

struct Win32ModuleInfo
{
	DWORD base, size, entry;
};

typedef BOOL (__stdcall *PENUMPROCESSMODULES)(HANDLE, HMODULE *, DWORD, LPDWORD);
typedef DWORD (__stdcall *PGETMODULEBASENAME)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef BOOL (__stdcall *PGETMODULEINFORMATION)(HANDLE, HMODULE, ModuleInfo*, DWORD);

typedef HANDLE (__stdcall *PCREATETOOLHELP32SNAPSHOT)(DWORD, DWORD);
typedef BOOL (WINAPI *PMODULE32FIRST)(HANDLE, LPMODULEENTRY32);
typedef BOOL (WINAPI *PMODULE32NEXT)(HANDLE, LPMODULEENTRY32);

ModuleInfo *CrashGetModules(void *&ptr)
{
	void *pMem = VirtualAlloc(NULL, 65536, MEM_COMMIT, PAGE_READWRITE);

	if (!pMem) {
		ptr = NULL;
		return NULL;
	}

	// This sucks.  If we're running under Windows 9x, we must use
	// TOOLHELP.DLL to get the module list.  Under Windows NT, we must
	// use PSAPI.DLL.  With Windows 2000, we can use both (but prefer
	// PSAPI.DLL).

	HMODULE hmodPSAPI = LoadLibrary("psapi.dll");

	if (hmodPSAPI) {
		// Using PSAPI.DLL.  Call EnumProcessModules(), then GetModuleFileNameEx()
		// and GetModuleInformation().

		PENUMPROCESSMODULES pEnumProcessModules = (PENUMPROCESSMODULES)GetProcAddress(hmodPSAPI, "EnumProcessModules");
		PGETMODULEBASENAME pGetModuleBaseName = (PGETMODULEBASENAME)GetProcAddress(hmodPSAPI, "GetModuleBaseNameA");
		PGETMODULEINFORMATION pGetModuleInformation = (PGETMODULEINFORMATION)GetProcAddress(hmodPSAPI, "GetModuleInformation");
		HMODULE *pModules, *pModules0 = (HMODULE *)((char *)pMem + 0xF000);
		DWORD cbNeeded;

		if (pEnumProcessModules && pGetModuleBaseName && pGetModuleInformation
			&& pEnumProcessModules(GetCurrentProcess(), pModules0, 0x1000, &cbNeeded))
		{

			ModuleInfo *pMod, *pMod0;
			char *pszHeap = (char *)pMem, *pszHeapLimit;

			if (cbNeeded > 0x1000) cbNeeded = 0x1000;

			pModules = (HMODULE *)((char *)pMem + 0x10000 - cbNeeded);
			memmove(pModules, pModules0, cbNeeded);

			pMod = pMod0 = (ModuleInfo *)((char *)pMem + 0x10000 - sizeof(ModuleInfo) * (cbNeeded / sizeof(HMODULE) + 1));
			pszHeapLimit = (char *)pMod;

			do
			{
				HMODULE hCurMod = *pModules++;
				Win32ModuleInfo mi;

				if (pGetModuleBaseName(GetCurrentProcess(), hCurMod, pszHeap, pszHeapLimit - pszHeap)
					&& pGetModuleInformation(GetCurrentProcess(), hCurMod, (ModuleInfo*)&mi, sizeof mi)) {

					char *period = NULL;

					pMod->name = pszHeap;

					while(*pszHeap++)
						if (pszHeap[-1] == '.')
							period = pszHeap-1;

					if (period)
					{
						*period = 0;
						pszHeap = period+1;
					}

					pMod->base = mi.base;
					pMod->size = mi.size;
					++pMod;
				}
			} while((cbNeeded -= sizeof(HMODULE *)) > 0);

			pMod->name = NULL;

			FreeLibrary(hmodPSAPI);
			ptr = pMem;
			return pMod0;
		}

		FreeLibrary(hmodPSAPI);
	}
	else
	{
		// No PSAPI.  Use the ToolHelp functions in KERNEL.

		HMODULE hmodKERNEL32 = LoadLibrary("kernel32.dll");

		PCREATETOOLHELP32SNAPSHOT pCreateToolhelp32Snapshot = (PCREATETOOLHELP32SNAPSHOT)GetProcAddress(hmodKERNEL32, "CreateToolhelp32Snapshot");
		PMODULE32FIRST pModule32First = (PMODULE32FIRST)GetProcAddress(hmodKERNEL32, "Module32First");
		PMODULE32NEXT pModule32Next = (PMODULE32NEXT)GetProcAddress(hmodKERNEL32, "Module32Next");
		HANDLE hSnap;

		if (pCreateToolhelp32Snapshot && pModule32First && pModule32Next)
		{
			if ((HANDLE)-1 != (hSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0))) {
				ModuleInfo *pModInfo = (ModuleInfo *)((char *)pMem + 65536);
				char *pszHeap = (char *)pMem;
				MODULEENTRY32 me;

				--pModInfo;
				pModInfo->name = NULL;

				me.dwSize = sizeof(MODULEENTRY32);

				if (pModule32First(hSnap, &me))
					do {
						if (pszHeap+strlen(me.szModule) >= (char *)(pModInfo - 1))
							break;

						strcpy(pszHeap, me.szModule);

						--pModInfo;
						pModInfo->name = pszHeap;

						char *period = NULL;

						while(*pszHeap++);
							if (pszHeap[-1]=='.')
								period = pszHeap-1;

						if (period) {
							*period = 0;
							pszHeap = period+1;
						}

						pModInfo->base = (unsigned long)me.modBaseAddr;
						pModInfo->size = me.modBaseSize;

					} while(pModule32Next(hSnap, &me));

				CloseHandle(hSnap);

				FreeLibrary(hmodKERNEL32);

				ptr = pMem;
				return pModInfo;
			}
		}

		FreeLibrary(hmodKERNEL32);
	}

	VirtualFree(pMem, 0, MEM_RELEASE);

	ptr = NULL;
	return NULL;
}

static bool IsExecutableProtection(DWORD dwProtect) {
	MEMORY_BASIC_INFORMATION meminfo;

	// Windows NT/2000 allows Execute permissions, but Win9x seems to
	// rip it off.  So we query the permissions on our own code block,
	// and use it to determine if READONLY/READWRITE should be
	// considered 'executable.'

	VirtualQuery(IsExecutableProtection, &meminfo, sizeof meminfo);

	switch((unsigned char)dwProtect) {
	case PAGE_READONLY:				// *sigh* Win9x...
	case PAGE_READWRITE:			// *sigh*
		return meminfo.Protect==PAGE_READONLY || meminfo.Protect==PAGE_READWRITE;

	case PAGE_EXECUTE:
	case PAGE_EXECUTE_READ:
	case PAGE_EXECUTE_READWRITE:
	case PAGE_EXECUTE_WRITECOPY:
		return true;
	}
	return false;
}

static bool IsValidCall(char *buf, int len) {
	// Permissible CALL sequences that we care about:
	//
	//	E8 xx xx xx xx			CALL near relative
	//	FF (group 2)			CALL near absolute indirect
	//
	// Minimum sequence is 2 bytes (call eax).
	// Maximum sequence is 7 bytes (call dword ptr [eax+disp32]).

	if (len >= 5 && buf[-5] == (char)0xE8)
		return true;

	// FF 14 xx					CALL [reg32+reg32*scale]

	if (len >= 3 && buf[-3] == (char)0xFF && buf[-2]==0x14)
		return true;

	// FF 15 xx xx xx xx		CALL disp32

	if (len >= 6 && buf[-6] == (char)0xFF && buf[-5]==0x15)
		return true;

	// FF 00-3F(!14/15)			CALL [reg32]

	if (len >= 2 && buf[-2] == (char)0xFF && (unsigned char)buf[-1] < 0x40)
		return true;

	// FF D0-D7					CALL reg32

	if (len >= 2 && buf[-2] == (char)0xFF && (buf[-1]&0xF8) == 0xD0)
		return true;

	// FF 50-57 xx				CALL [reg32+reg32*scale+disp8]

	if (len >= 3 && buf[-3] == (char)0xFF && (buf[-2]&0xF8) == 0x50)
		return true;

	// FF 90-97 xx xx xx xx xx	CALL [reg32+reg32*scale+disp32]

	if (len >= 7 && buf[-7] == (char)0xFF && (buf[-6]&0xF8) == 0x90)
		return true;

	return false;
}

static const char *CrashGetModuleBaseName(HMODULE hmod, char *pszBaseName) {
	char szPath1[MAX_PATH];
	char szPath2[MAX_PATH];

	__try {
		DWORD dw;
		char *pszFile, *period = NULL;

		if (!GetModuleFileName(hmod, szPath1, sizeof szPath1))
			return NULL;

		dw = GetFullPathName(szPath1, sizeof szPath2, szPath2, &pszFile);

		if (!dw || dw>sizeof szPath2)
			return NULL;

		strcpy(pszBaseName, pszFile);

		pszFile = pszBaseName;

		while(*pszFile++)
			if (pszFile[-1]=='.')
				period = pszFile-1;

		if (period)
			*period = 0;
	} __except(1) {
		return NULL;
	}

	return pszBaseName;
}

struct PEHeader {
	DWORD		signature;
	WORD		machine;
	WORD		sections;
	DWORD		timestamp;
	DWORD		symbol_table;
	DWORD		symbols;
	WORD		opthdr_size;
	WORD		characteristics;
};

struct PESectionHeader {
	char		name[8];
	DWORD		virtsize;
	DWORD		virtaddr;
	DWORD		rawsize;
	DWORD		rawptr;
	DWORD		relocptr;
	DWORD		linenoptr;
	WORD		reloc_cnt;
	WORD		lineno_cnt;
	DWORD		characteristics;
};

struct PEExportDirectory {
	DWORD		flags;
	DWORD		timestamp;
	WORD		major;
	WORD		minor;
	DWORD		nameptr;
	DWORD		ordbase;
	DWORD		addrtbl_cnt;
	DWORD		nametbl_cnt;
	DWORD		addrtbl_ptr;
	DWORD		nametbl_ptr;
	DWORD		ordtbl_ptr;
};

struct PE32OptionalHeader {
	WORD		magic;					// 0
	char		major_linker_ver;		// 2
	char		minor_linker_ver;		// 3
	DWORD		codesize;				// 4
	DWORD		idatasize;				// 8
	DWORD		udatasize;				// 12
	DWORD		entrypoint;				// 16
	DWORD		codebase;				// 20
	DWORD		database;				// 24
	DWORD		imagebase;				// 28
	DWORD		section_align;			// 32
	DWORD		file_align;				// 36
	WORD		majoros;				// 40
	WORD		minoros;				// 42
	WORD		majorimage;				// 44
	WORD		minorimage;				// 46
	WORD		majorsubsys;			// 48
	WORD		minorsubsys;			// 50
	DWORD		reserved;				// 52
	DWORD		imagesize;				// 56
	DWORD		hdrsize;				// 60
	DWORD		checksum;				// 64
	WORD		subsystem;				// 68
	WORD		characteristics;		// 70
	DWORD		stackreserve;			// 72
	DWORD		stackcommit;			// 76
	DWORD		heapreserve;			// 80
	DWORD		heapcommit;				// 84
	DWORD		loaderflags;			// 88
	DWORD		dictentries;			// 92

	// Not part of header, but it's convienent here

	DWORD		export_RVA;				// 96
	DWORD		export_size;			// 100
};

struct PE32PlusOptionalHeader {
	WORD		magic;					// 0
	char		major_linker_ver;		// 2
	char		minor_linker_ver;		// 3
	DWORD		codesize;				// 4
	DWORD		idatasize;				// 8
	DWORD		udatasize;				// 12
	DWORD		entrypoint;				// 16
	DWORD		codebase;				// 20
	__int64		imagebase;				// 24
	DWORD		section_align;			// 32
	DWORD		file_align;				// 36
	WORD		majoros;				// 40
	WORD		minoros;				// 42
	WORD		majorimage;				// 44
	WORD		minorimage;				// 46
	WORD		majorsubsys;			// 48
	WORD		minorsubsys;			// 50
	DWORD		reserved;				// 52
	DWORD		imagesize;				// 56
	DWORD		hdrsize;				// 60
	DWORD		checksum;				// 64
	WORD		subsystem;				// 68
	WORD		characteristics;		// 70
	__int64		stackreserve;			// 72
	__int64		stackcommit;			// 80
	__int64		heapreserve;			// 88
	__int64		heapcommit;				// 96
	DWORD		loaderflags;			// 104
	DWORD		dictentries;			// 108

	// Not part of header, but it's convienent here

	DWORD		export_RVA;				// 112
	DWORD		export_size;			// 116
};

static const char *CrashLookupExport(HMODULE hmod, unsigned long addr, unsigned long &fnbase) {
	char *pBase = (char *)hmod;

	// The PEheader offset is at hmod+0x3c.  Add the size of the optional header
	// to step to the section headers.

	PEHeader *pHeader = (PEHeader *)(pBase + ((long *)hmod)[15]);

	if (pHeader->signature != 'EP')
		return NULL;

#if 0
	PESectionHeader *pSHdrs = (PESectionHeader *)((char *)pHeader + sizeof(PEHeader) + pHeader->opthdr_size);

	// Scan down the section headers and look for ".edata"

	int i;

	for(i=0; i<pHeader->sections; i++) {
		MessageBox(NULL, pSHdrs[i].name, "section", MB_OK);
		if (!memcmp(pSHdrs[i].name, ".edata", 6))
			break;
	}

	if (i >= pHeader->sections)
		return NULL;
#endif

	// Verify the optional structure.

	PEExportDirectory *pExportDir;

	if (pHeader->opthdr_size < 104)
		return NULL;

	switch(*(short *)((char *)pHeader + sizeof(PEHeader))) {
	case 0x10b:		// PE32
		{
			PE32OptionalHeader *pOpt = (PE32OptionalHeader *)((char *)pHeader + sizeof(PEHeader));

			if (pOpt->dictentries < 1)
				return NULL;

			pExportDir = (PEExportDirectory *)(pBase + pOpt->export_RVA);
		}
		break;
	case 0x20b:		// PE32+
		{
			PE32PlusOptionalHeader *pOpt = (PE32PlusOptionalHeader *)((char *)pHeader + sizeof(PEHeader));

			if (pOpt->dictentries < 1)
				return NULL;

			pExportDir = (PEExportDirectory *)(pBase + pOpt->export_RVA);
		}
		break;

	default:
		return NULL;
	}

	// Hmmm... no exports?

	if ((char *)pExportDir == pBase)
		return NULL;

	// Find the location of the export information.

	DWORD *pNameTbl = (DWORD *)(pBase + pExportDir->nametbl_ptr);
	DWORD *pAddrTbl = (DWORD *)(pBase + pExportDir->addrtbl_ptr);
	WORD *pOrdTbl = (WORD *)(pBase + pExportDir->ordtbl_ptr);

	// Scan exports.

	const char *pszName = NULL;
	DWORD bestdelta = 0xFFFFFFFF;
	int i;

	addr -= (DWORD)pBase;

	for(i = 0; i < pExportDir->nametbl_cnt; i++)
	{
		DWORD fnaddr;
		int idx;

		idx = pOrdTbl[i];
		fnaddr = pAddrTbl[idx];

		if (addr >= fnaddr) {
			DWORD delta = addr - fnaddr;

			if (delta < bestdelta) {
				bestdelta = delta;
				fnbase = fnaddr;

				if (pNameTbl[i])
					pszName = pBase + pNameTbl[i];
				else {
					static char buf[8];

					wsprintf(buf, "ord%d", pOrdTbl[i]);
					pszName = buf;
				}

			}
		}
	}

	return pszName;
}

void CrashHandlerTranslator(unsigned int, EXCEPTION_POINTERS* pExc)
{
	CrashHandler(pExc);
	throw UnhandledException();
}

const TCHAR* ConvertExceptionCodeToString(DWORD dwException)
{
#define CONVERT_EXCEPTION_CODE_TO_STRING(exception) \
	case EXCEPTION_##exception: return _T(#exception)

	switch (dwException)
	{
	CONVERT_EXCEPTION_CODE_TO_STRING(ACCESS_VIOLATION);
	CONVERT_EXCEPTION_CODE_TO_STRING(DATATYPE_MISALIGNMENT);
	CONVERT_EXCEPTION_CODE_TO_STRING(BREAKPOINT);
	CONVERT_EXCEPTION_CODE_TO_STRING(SINGLE_STEP);
	CONVERT_EXCEPTION_CODE_TO_STRING(ARRAY_BOUNDS_EXCEEDED);
	CONVERT_EXCEPTION_CODE_TO_STRING(FLT_DENORMAL_OPERAND);
	CONVERT_EXCEPTION_CODE_TO_STRING(FLT_DIVIDE_BY_ZERO);
	CONVERT_EXCEPTION_CODE_TO_STRING(FLT_INEXACT_RESULT);
	CONVERT_EXCEPTION_CODE_TO_STRING(FLT_INVALID_OPERATION);
	CONVERT_EXCEPTION_CODE_TO_STRING(FLT_OVERFLOW);
	CONVERT_EXCEPTION_CODE_TO_STRING(FLT_STACK_CHECK);
	CONVERT_EXCEPTION_CODE_TO_STRING(FLT_UNDERFLOW);
	CONVERT_EXCEPTION_CODE_TO_STRING(INT_DIVIDE_BY_ZERO);
	CONVERT_EXCEPTION_CODE_TO_STRING(INT_OVERFLOW);
	CONVERT_EXCEPTION_CODE_TO_STRING(PRIV_INSTRUCTION);
	CONVERT_EXCEPTION_CODE_TO_STRING(IN_PAGE_ERROR);
	CONVERT_EXCEPTION_CODE_TO_STRING(ILLEGAL_INSTRUCTION);
	CONVERT_EXCEPTION_CODE_TO_STRING(NONCONTINUABLE_EXCEPTION);
	CONVERT_EXCEPTION_CODE_TO_STRING(STACK_OVERFLOW);
	CONVERT_EXCEPTION_CODE_TO_STRING(INVALID_DISPOSITION);
	CONVERT_EXCEPTION_CODE_TO_STRING(GUARD_PAGE);
	CONVERT_EXCEPTION_CODE_TO_STRING(INVALID_HANDLE);
	default: return NULL;
	}

#undef CONVERT_EXCEPTION_CODE_TO_STRING
}

LONG __stdcall CrashHandler(struct _EXCEPTION_POINTERS *pExc)
{
	Trace(_FMT(_T("CrashHandler: %s. Address: 0x%08X"),
		ConvertExceptionCodeToString(pExc->ExceptionRecord->ExceptionCode),
		pExc->ExceptionRecord->ExceptionAddress));
	if (pExc->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		Trace(_FMT(_T("Tried to %s 0x%08X"),
			(0 == pExc->ExceptionRecord->ExceptionInformation[0] ? _T("read from"): _T("write to")),
			pExc->ExceptionRecord->ExceptionInformation[1]));
	}
	const CONTEXT *const pContext = (const CONTEXT *)pExc->ContextRecord;
	char *lpAddr = (char *)pContext->Esp;
	HANDLE hprMe = GetCurrentProcess();
	int limit = 100;
	unsigned long data;
	char buf[512];
	TCHAR tmp[1024];

	//return 1;

	// Get some module names.

	void *pModuleMem;
	ModuleInfo *pModules = CrashGetModules(pModuleMem);

	// Retrieve stack pointers.
	// Not sure if NtCurrentTeb() is available on Win95....

	NT_TIB *pTib;

	__asm {
		mov	eax, fs:[0]_NT_TIB.Self
		mov pTib, eax
	}

	char *pStackBase = (char *)pTib->StackBase;

	// Walk up the stack.  Hopefully it wasn't fscked.

	data = pContext->Eip;
	do
	{
		bool fValid = true;
		int len;
		MEMORY_BASIC_INFORMATION meminfo;

		VirtualQuery((void *)data, &meminfo, sizeof meminfo);
		
		if (!IsExecutableProtection(meminfo.Protect) || meminfo.State!=MEM_COMMIT)
		{
//			Report(hwnd, hFile, "Rejected: %08lx (%08lx)", data, meminfo.Protect);
			fValid = false;
		}
		if (data != pContext->Eip)
		{
			len = 7;

			*(long *)(buf + 0) = *(long *)(buf + 4) = 0;

			while(len > 0 && !ReadProcessMemory(GetCurrentProcess(), (void *)(data-len), buf+7-len, len, NULL))
				--len;

			fValid &= IsValidCall(buf+7, len);
		}
		
		if (fValid)
		{
//			if (VDDebugInfoLookupRVA(&g_debugInfo, data, buf, sizeof buf) >= 0)
//			{
//				Report(hwnd, hFile, "%08lx: %s()", data, buf);
//				--limit;
//			}
//			else
			{
				ModuleInfo *pMods = pModules;
				ModuleInfo mi;
				char szName[MAX_PATH];

				mi.name = NULL;

				if (pMods) {
					while(pMods->name) {
						if (data >= pMods->base && (data - pMods->base) < pMods->size)
							break;

						++pMods;
					}

					mi = *pMods;
				} else {

					// Well, something failed, or we didn't have either PSAPI.DLL or ToolHelp
					// to play with.  So we'll use a nastier method instead.

					mi.base = (unsigned long)meminfo.AllocationBase;
					mi.name = CrashGetModuleBaseName((HMODULE)mi.base, szName);
				}

				if (mi.name)
				{
					unsigned long fnbase;
					const char *pExportName = CrashLookupExport((HMODULE)mi.base, data, fnbase);

					if (pExportName)
					{
						_stprintf(tmp, _T("%08lx: %s!%s [%08lx+%lx+%lx]\n"), data, mi.name, pExportName, mi.base, fnbase, (data-mi.base-fnbase));
						::OutputDebugString(tmp);
					}
					else
					{
						_stprintf(tmp, _T("%08lx: %s!%08lx\n"), data, mi.name, data - mi.base);
						::OutputDebugString(tmp);
					}
				}
				else
				{
					_stprintf(tmp, _T("%08lx: %08lx\n"), data, data);
					::OutputDebugString(tmp);
				}
				--limit;
			}
		}

		if (lpAddr >= pStackBase)
			break;

		lpAddr += 4;
	} while(limit > 0 && ReadProcessMemory(hprMe, lpAddr-4, &data, 4, NULL));

	// All done, close up shop and exit.

	if (pModuleMem)
		VirtualFree(pModuleMem, 0, MEM_RELEASE);

	StackWalker sw;
	sw.ShowCallstack();
	return true;
}