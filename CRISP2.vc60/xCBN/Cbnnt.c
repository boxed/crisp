
/*
 * =====[ WINNT BIND ]=====================================================
 *
 * Description:     Windows NT binding. Based upon the binding for
 *                  windows 3.1, modified for 32 bit access.
 *
 * Files needed:    See readme.txt
 *
 * Compiled:        VISUAL C++ 1,5 or 2.0
 *
 * Summary:
 *
 * Revisions:
 *
 *  REV     DATE     BY           DESCRIPTION
 *  ----  --------  ----------  --------------------------------------
 *  0.00  01.01.95  Peter Glen  Initial version.
 *
 * =======================================================================
 */

#include <windows.h>
#include "interfce.h"

#define  MARX_IMPORT
#include "cbn_.h"

//define PG_DEBUG

/*
 ****************************************************************************
 *
 *  FUNCTION:    DllMain
 *
 *  INPUTS:      hDLL       - handle of DLL
 *               dwReason   - indicates why DLL called
 *               lpReserved - reserved
 *
 *  RETURNS:     TRUE (always)
 *
 *               Note that the retuRn value is used only when
 *               dwReason == DLL_PROCESS_ATTACH.
 *
 *               Normally the function would return TRUE if DLL initial-
 *               ization succeeded, or FALSE it it failed.
 *
 *  COMMENTS:    On test mode (PG_DEBUG) the functions will display a
 *               dialog box informing user of each notification message
 *               and the name of the attaching/detaching process/thread.
 *               For more information see "DllMain" in the Win32 API reference.
 *
 *****************************************************************************
 *
 */

BOOL WINAPI DllMain (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)

{
    #ifdef PG_DEBUG
    #define     BUFSIZE 48
    char buf[BUFSIZE+1];
    #endif

    switch (dwReason)
    {

        //DLL is attaching to the address space of the current process.
        case DLL_PROCESS_ATTACH:

            #ifdef PG_DEBUG

            GetModuleFileName (NULL, (LPTSTR) buf, BUFSIZE);
            MessageBox ( GetFocus(),
                    (LPCTSTR) buf,
                    (LPCTSTR) "THE_DLL: Process attaching",
                    MB_OK | MB_SYSTEMMODAL);
            #endif
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        //The calling process is detaching the DLL from its address space.
        case DLL_PROCESS_DETACH:

          #ifdef PG_DEBUG
          MessageBox  (GetFocus(),
                      (LPCTSTR) "THE_DLL: Process detaching",
                      (LPCTSTR) buf,
                      MB_OK | MB_SYSTEMMODAL );
          #endif
          break;
      }
    return TRUE;
}

/*
 * Sorry about this mess, but the entry to the DLL execpts only a single
 * module, multiple modules are not allowed.
 *
 * (I cannot answer why)
 *
 */

#include "cbn_ser.c"
#include "cbn_ids.c"
#include "cbn_ram1.c"
#include "cbn_ram2.c"
#include "cbn_cryp.c"
#include "cbn_idea.c"
#include "codeidea.c"


/* EOF */