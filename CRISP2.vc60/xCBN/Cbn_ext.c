/*
 * =====[ WINNT BIND ]=====================================================
 *
 * Description:     Windows 95 binding. Based upon the binding for
 *                  windows 3.1, modified for 32 bit access.
 *                    This is the branch code for
 *
 *                        'NT,
 *                        '95,
 *                        '32s
 *
 * Files needed:    See readme.txt
 *
 * Compiled:        VISUAL C++ 1,5 or 2.0
 *
 * Note:            Coded communication between the DLL and DEVICE.
 *
 * Revisions:
 *
 *    REV     DATE     BY           DESCRIPTION
 *    ----  --------  ----------  --------------------------------------
 *    0.00  01.01.95  Peter Glen  Initial version.
 *    0.00  31.01.95  Peter Glen  Documented.
 *    0.00  31.04.95  Peter Glen  Device got on line.
 *    0.00  15.02.95  Peter Glen  Rewrite finished.
 *    0.00  18.05.95  Peter Glen  WIN 95 initial version.
 *    0.00  27.02.96  Peter Glen  WIN 95/NT platform recognition.
 *
 * =======================================================================
 */

/* -------- System includes:  -------------------------------------------- */

#include    <windows.h>
#include    <winioctl.h>

#include    <stddef.h>
#include    <stdlib.h>
#include    <stdio.h>

/* -------- Includes:  --------------------------------------------------- */

#define UNDER_WIN95
#define MARX_IMPORT
#include "cbn_.h"

#include "marxdev.h"
#include "interfce.h"

/* -------- Defines: ----------------------------------------------------- */


/*
 * This is a visible part for 32 bit DLL level:
 */

short    cb560_ext(     unsigned short     iOpCode,
                        unsigned short     iPortNr,
                        void               *pcInData,
                        void               *pcOutData,
                        unsigned short     iOutlen
                   )

{
short   ret_val;
OSVERSIONINFO VersionEx;

    VersionEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&VersionEx)) { VersionEx.dwPlatformId = 0xFFFFL;}

    gl_ext_ret_val = 0;

                 
	if     (VersionEx.dwPlatformId == VER_PLATFORM_WIN32_NT)		// Is NT active ?
		ret_val = cb560_nt(iOpCode, iPortNr, (unsigned char FAR *)pcInData,	(unsigned char FAR *)pcOutData, iOutlen);

	else if (VersionEx.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) // Is '95
		ret_val = cb560_95(iOpCode, iPortNr, (unsigned char FAR *)pcInData,	(unsigned char FAR *)pcOutData, iOutlen);

	else if (VersionEx.dwPlatformId == VER_PLATFORM_WIN32s )			//	or Win32s active ?
		ret_val = CB560((unsigned long) iOpCode, (unsigned long) iPortNr, (unsigned char FAR *)pcInData, (unsigned char FAR *)pcOutData);

	else
		ret_val = BOX_WHAT_WINDOWS;
	return(ret_val);
}

/* EOF */