
/* =====[ DUMP.C ]======================================================

   Description:     Dump a memory in C, mainframe style.


   Compiled:        MS-C.

   Compiler opt:    ??

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  xx.xx.94  Peter Glen    Initial version.
      0.00  07/08/95  Peter Glen    Comment added, cleaned.

   ======================================================================= */

/* -------- System includes:  -------------------------------------------- */

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <dos.h>
#include <malloc.h>

#include "dump.h"

//static  int     pstat(_HEAPINFO *heap);
//static  int     hstat(int stat);
//

static  char dump_str[1024];

/* -------- Implementation: ---------------------------------------------- */

/*
 * Dump a memory to a debugger window. (dbwin.exe needs to be started)
 * This is a most useful thing you will ever have in Visual 'C'. Limitless
 * look at whatever you want to see in C or C++.
 *
 * Usage example:
 *
 *   char    cptr[12];
 *
 *   DebugDump(cptr, 36, "DUMP of cptr string:");
 *
 * Input:
 *
 *      char FAR *dptr   :   poiter to mem to dump
 *      int len         :   length of mem to dump
 *      char FAR *str   :   optional message
 *
 *  Exceptions:
 *
 *      char FAR *dptr   = NULL no mem dump
 *      char FAR *str    = NULL no banner
 *
 * The dump is done in the following fashion:
 *
 *  Optional message (pointer value):
 *
 *  -  00 00 00 00 00 00 00 00  - ccccccc  -
 *  -  00 00 00 00 00 00 00 00  - ccccccc  -
 *  -  00 00 00 00 00 00 00 00  - ccccccc  -
 *  -  00 00 00 00 00 00 00 00  - ccccccc  -
 *  -  00 00 00 00 00 00 00 00  - ccccccc  -
 *
 *  The buffer is dumped with modulus eight as the character count.
 *  This is a nice way to examine the mem at the end of the buffer.
 *  Like null termination, \r \n charaters etc ...
 *
 *  In protected mode it can cause problems. (none so far)
 *
 *  Limits:
 *
 *  Generally, it is designed to examine a short buffer of 1024 lenth.
 *  If you need to examine a larger piece, use the pointer constructively
 *  to look at the place of interest.
 *
 */

int     DebugDump(void FAR *dptr, int len, char FAR *str)

{
    char    *dump_ptr = dump_str;
    int     slen = 0;
    char    FAR *ptr = dptr;

    if(str)
        slen = sprintf(dump_str, "%Fs (ptr = %Fp)\n\n", str, ptr);

    if (dptr)
        {
        while(TRUE)
            {
            int aa;

            if(len <= 0)                                  /* end of buffer */
                break;

            if(slen >=  sizeof(dump_str)-46)        /* end of local buffer */
                break;

            slen += sprintf(dump_str + slen, "  -  ");

            for(aa = 0; aa < 8; aa++)
                slen += sprintf(dump_str + slen, "%02x ", *(ptr+aa) & 0xff);

            slen += sprintf(dump_str + slen, "  -  ");

            for(aa = 0; aa < 8; aa++)
                {
                char chh = *(ptr+aa);

                if(!isprint((int)chh))                 /* convert to USC if not prt */
                    chh = '.';

                slen += sprintf(dump_str + slen, "%c", chh);
                }
            slen += sprintf(dump_str + slen, "  -  ");
            slen += sprintf(dump_str + slen, "\n");

            len -= 8; ptr += 8;
            }
        }
    OutputDebugString(dump_str);

    return(0);
}

int     DebugPrintf(char *fmt, ...)

{
    va_list marker;

    va_start(marker, fmt);
    vsprintf(dump_str, fmt, marker);
    va_end(marker);

    MessageBox(NULL, dump_str, "Debug", MB_OK);
    //OutputDebugString(dump_str);

    return(0);
}


//int     DebugHeap(void)
//
//{
//
//    _HEAPINFO heap;
//    int stat;
//    int entry = 0;
//
//    DebugPrintf("Start heap dump->\n");
//
//    heap._pentry = NULL;
//
//    while (TRUE)
//        {
//        stat = heapwalk(&heap);
//
//        entry++;
//        pstat(&heap);
//
//        if (stat != _HEAPOK )
//            break;
//        }
//    hstat(stat);
//    DebugPrintf("End heap dump, %u entries dumped.\n", entry);
//    return(0);
//}
//
//static  int     pstat(_HEAPINFO *heap)
//
//{
//    DebugPrintf("%Fp %3s Size: %u\t",
//                   heap->_pentry,
//                       (heap->_useflag == _USEDENTRY ? "F" : "U"),
//                            heap->_size);
//
//    DebugDump(  heap->_pentry, 8, NULL);
//
//    return(0);
//}
//
//
//static  int     hstat(int stat)
//
//{
//    DebugPrintf("Heap termination: ");
//
//    switch(stat)
//        {
//        case  _HEAPEMPTY    :  DebugPrintf("_HEAPEMPTY\n"); break;
//        case  _HEAPOK       :  DebugPrintf("_HEAPOK\n"); break;
//        case  _HEAPBADBEGIN :  DebugPrintf("_HEAPBADBEGIN\n"); break;
//        case  _HEAPBADNODE  :  DebugPrintf("_HEAPBADNODE\n"); break;
//        case  _HEAPEND      :  DebugPrintf("_HEAPEND\n"); break;
//        case  _HEAPBADPTR   :  DebugPrintf("_HEAPBADPTR\n"); break;
//        default:                DebugPrintf("_UNKNOWN\n"); break;
//
//        }
//    return(0);
//}

/* EOF */


