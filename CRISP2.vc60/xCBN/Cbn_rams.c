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


#define GEN_RAM_CODE(funcname, opcode)                               \
                                                                     \
short   DLLCALL funcname(          unsigned short   iPortNr,         \
                                 unsigned short   iIdNr,             \
                                 unsigned char  *pcSCodeId,          \
                                 unsigned char  *pcPassRam,          \
                                 unsigned short iStartAdr,           \
                                 unsigned short uiLength,            \
                                 unsigned char  * pcOutData)         \
{                                                                    \
    unsigned char cInField[12];                                      \
    unsigned short iOpCode;                                          \
    short  iRetValue;                                                \
                                                                     \
    cInField[0] = (unsigned char) (iIdNr & 0xff);                    \
    cInField[1] = *(pcSCodeId + 2);                                  \
    cInField[2] = *(pcSCodeId + 1);                                  \
    cInField[3] = *pcSCodeId;                                        \
    cInField[4] = *(pcPassRam + 3);                                  \
    cInField[5] = *(pcPassRam + 2);                                  \
    cInField[6] = *(pcPassRam + 1);                                  \
    cInField[7] = *pcPassRam;                                        \
    cInField[8] = (unsigned char) (uiLength & 0xff);                 \
    cInField[9] = (unsigned char) ((uiLength >> 8) & 0xff);          \
    cInField[10] = (unsigned char) (iStartAdr & 0xff);               \
    cInField[11] = (unsigned char) ((iStartAdr >> 8) & 0xff);        \
                                                                     \
    iOpCode = opcode;                                                \
                                                                     \
    iRetValue = cb560_ext(iOpCode, iPortNr, cInField, pcOutData,     \
                                            uiLength);               \
                                                                     \
    gl_iRetValue =  iRetValue;                                       \
                                                                     \
    return (iRetValue);                                              \
}                                                                    \


#define GEN_INC_DEC_CODE(funcname, opcode)                           \
                                                                     \
short   DLLCALL funcname(       unsigned short  iPortNr,             \
                                unsigned short    iIdNr,             \
                                unsigned char   *pcSCodeId,          \
                                unsigned char   *pcPassRam,          \
                                unsigned short  iAdrCount,           \
                                unsigned short  *piNewCount)         \
{                                                                    \
    unsigned char   cInField[10];                                    \
    unsigned short    iOpCode;                                       \
    short     iRetValue;                                             \
                                                                     \
                                                                     \
    cInField[0] = (unsigned char) (iIdNr & 0xff);                    \
    cInField[1] = *(pcSCodeId + 2);                                  \
    cInField[2] = *(pcSCodeId + 1);                                  \
    cInField[3] = *pcSCodeId;                                        \
    cInField[4] = *(pcPassRam + 3);                                  \
    cInField[5] = *(pcPassRam + 2);                                  \
    cInField[6] = *(pcPassRam + 1);                                  \
    cInField[7] = *pcPassRam;                                        \
    cInField[8] = (unsigned char) (iAdrCount & 0xff);                \
    cInField[9] = (unsigned char) ((iAdrCount >> 8) & 0xff);         \
                                                                     \
    iOpCode = opcode;                                                \
                                                                     \
    iRetValue = cb560_ext(iOpCode, iPortNr, cInField, piNewCount,    \
                                            sizeof(short));          \
                                                                     \
    gl_iRetValue =  iRetValue;                                       \
                                                                     \
    return (iRetValue);                                              \
}


#define GEN_RW_CNT_CODE(funcname, FuncToCall)                        \
                                                                     \
short   DLLCALL funcname(               unsigned short    iPortNr,   \
                                unsigned short    iIdNr,             \
                                unsigned char    *pcSCodeId,         \
                                unsigned char    *pcPassRam,         \
                                unsigned short  iAdrCount,           \
                                unsigned short    *piNewCount)       \
{                                                                    \
    short     iRetValue;                                             \
                                                                     \
    iRetValue = FuncToCall(     iPortNr,                             \
                                iIdNr,                               \
                                pcSCodeId,                           \
                                pcPassRam,                           \
                                iAdrCount,                           \
                                (short)sizeof(int),                  \
                                (unsigned char *) piNewCount         \
                           );                                        \
                                                                     \
    gl_iRetValue =  iRetValue;                                       \
                                                                     \
    return (iRetValue);                                              \
}                                                                    \


GEN_RAM_CODE(CbN_ReadRAM1,    0x14);
GEN_RAM_CODE(CbN_ReadRAM2,    0x15);

GEN_RAM_CODE(CbN_WriteRAM1,   0x16);
GEN_RAM_CODE(CbN_WriteRAM2,   0x17);

GEN_INC_DEC_CODE(CbN_IncRAM1, 0x18);
GEN_INC_DEC_CODE(CbN_IncRAM2, 0x20);

GEN_INC_DEC_CODE(CbN_DecRAM1, 0x19);
GEN_INC_DEC_CODE(CbN_DecRAM2, 0x21);


/*
 * These functions intend to replace the increment/decrement, by reading
 * and writing the RAM counter.
 * It will use 4 bytes, same as an integer on 32 bit machines.
 *
 * How to use within application:
 *
 *         count = CbN_Read_Cnt_RAM1
 *
 *         if (count == 0)
 *            deny execution of monitored (payed) function
 *         else
 *            {
 *            execute monitored (payed) function
 *            if(monitored (payed) function succeeded)
 *                 {
 *                 decrement counter
 *                 CbN_Write_Cnt_RAM1
 *                 }
 *             else
 *                 {
 *                 user could not use counted function (ex. printer offline)
 *                 do not decrement counter
 *                 do not CbN_Write_Cnt_RAM1
 *                 }
 *            }
 *
 * Macro calls macro, trust it. Visual 'C' makes it right.
 *
 */

GEN_RW_CNT_CODE(CbN_Read_Cnt_RAM1,  CbN_ReadRAM1);
GEN_RW_CNT_CODE(CbN_Read_Cnt_RAM2,  CbN_ReadRAM2);

GEN_RW_CNT_CODE(CbN_Write_Cnt_RAM1, CbN_WriteRAM1);
GEN_RW_CNT_CODE(CbN_Write_Cnt_RAM2, CbN_WriteRAM2);


/* EOF */

