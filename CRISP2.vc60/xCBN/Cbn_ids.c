
/*
 * =====[ WINNT BIND ]=====================================================
 *
 * Description:     Windows 95 binding. Based upon the binding for
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
 *  0.00  01.10.95  Peter Glen  Simulation version released.
 *  0.00  15.02.95  Peter Glen  Rewrite finished.
 *  0.00  18.05.95  Peter Glen  WIN 95 initial version.
 *
 * =======================================================================
 */

#ifndef     SIXTEEN_BIT
#include    <windows.h>
#else
#include "mydefine.h"
#endif

#define UNDER_WIN95
#define MARX_IMPORT
#include "cbn_.h"

/*
 * ID functions:
 */

#define GEN_ID_CODE(funcname, opcode)                               \
                                                                    \
short   DLLCALL funcname(       unsigned short    iPortNr,          \
                                unsigned char   *pcSCodeId,         \
                                unsigned long   *pulIdRet)          \
{                                                                   \
    unsigned char cInField[3];                                      \
    unsigned char cOutField[4];                                     \
    unsigned short  iOpCode;                                        \
    short  iRetValue;                                               \
                                                                    \
    cInField[0] = *(pcSCodeId + 2);                                 \
    cInField[1] = *(pcSCodeId + 1);                                 \
    cInField[2] = *(pcSCodeId);                                     \
    iOpCode = opcode;                                               \
                                                                    \
    iRetValue = cb560_ext(iOpCode, iPortNr, cInField,               \
                                    cOutField, sizeof(long));       \
    cOutField[2] = 0;                                               \
    cOutField[3] = 0;                                               \
                                                                    \
    if(!iRetValue)                                                  \
        *pulIdRet = *(unsigned long *) cOutField;                   \
    else                                                            \
        *pulIdRet = (unsigned long) 0;                              \
                                                                    \
    gl_iRetValue =  iRetValue;                                      \
                                                                    \
    return (iRetValue);                                             \
}                                                                   \

/*
 * Pointerless functions:
 */

#define GEN_R_ID_CODE(funcname, tocall)                            \
                                                                   \
long   DLLCALL funcname(unsigned short iPortNr, long cSCodeId)     \
                                                                   \
{                                                                  \
    long lRetValue = 0;                                            \
                                                                   \
    gl_iRetValue =  tocall(iPortNr, (unsigned char *) &cSCodeId,   \
                                          &lRetValue);             \
                                                                   \
    return (lRetValue);                                            \
}                                                                  \

GEN_ID_CODE(CbN_ReadID1, 1);
GEN_ID_CODE(CbN_ReadID2, 2);
GEN_ID_CODE(CbN_ReadID3, 3);
GEN_ID_CODE(CbN_ReadID4, 4);
GEN_ID_CODE(CbN_ReadID5, 5);
GEN_ID_CODE(CbN_ReadID6, 6);
GEN_ID_CODE(CbN_ReadID7, 7);
GEN_ID_CODE(CbN_ReadID8, 8);


GEN_R_ID_CODE(CBN_R_READID1, CbN_ReadID1);
GEN_R_ID_CODE(CBN_R_READID2, CbN_ReadID2);
GEN_R_ID_CODE(CBN_R_READID3, CbN_ReadID3);
GEN_R_ID_CODE(CBN_R_READID4, CbN_ReadID4);
GEN_R_ID_CODE(CBN_R_READID5, CbN_ReadID5);
GEN_R_ID_CODE(CBN_R_READID6, CbN_ReadID6);
GEN_R_ID_CODE(CBN_R_READID7, CbN_ReadID7);
GEN_R_ID_CODE(CBN_R_READID8, CbN_ReadID8);


/* EOF */

