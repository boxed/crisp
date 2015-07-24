
/* =====[ CBN BIND ]=========================================================

   Description:     Implementation of MARX CRYPTO BOX Library.

   Based on the:    Einbindung der cbn .. see below ...

   Procedures:      SEE the comment after the header.

   Notes:           This code is IDENTICAL of Cb9 WATCOM binding codes.
                    The only change is made are in the header Cb9.h

   Reference:       It calls the assembler subroutine Cb9

   Name of library: Cb9MSC6

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  02.09.94  Peter Glen    Initial version.
      0.01  18.06.95  Peter Glen    Segment bug corrected.

   ======================================================================= */

/*
 * Get the interface of the DLL module:
 *
 * NEVER, NEVER leave out this include.
 *
 * The compiler will do an enourmous job helping you catch all kinds of
 * errors. Remember, the compiler is your biggest help that you can get.
 *
 * Use it.
 *
 */

#define UNDER_WIN95
#define MARX_IMPORT
#include "cbn_.h"

/*
 * Get the interface of the assembler module:
 */

#include "interfce.h"

////////////////////////////////////////////////////////////////////////////

/****************************************************************************
*   Einbindung der CbN_ fÅr BORLAND C/C++
*
*   erstellt am 23.02.94 von kr
*
*   Modell: SMALL
*
****************************************************************************/

short DLLCALL CbWID4(       unsigned short iPortNr,
                            unsigned short iIdNr,
                            unsigned char  FAR *pcSCodeId,
                            unsigned char FAR *pcPwMaster,
                            unsigned char FAR *pcIdCode,
                            unsigned char FAR *pcIdRet
                        )
{
    unsigned char cOutField[1];
    unsigned char cInField[14];
    unsigned short  iOpCode;
    unsigned short  iRetValue;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);
    cInField[4] = *(pcPwMaster+4);
    cInField[5] = *(pcPwMaster+3);
    cInField[6] = *(pcPwMaster+2);
    cInField[7] = *(pcPwMaster+1);
    cInField[8] = *(pcPwMaster);
    cInField[9] = *(pcIdCode+2);
    cInField[10] = *(pcIdCode+1);
    cInField[11] = *(pcIdCode);
    cInField[12] = *(pcIdRet+1);
    cInField[13] = *(pcIdRet);

    iOpCode = 0x24;

    iRetValue = CB560(iOpCode, iPortNr, cInField, cOutField);

    return(iRetValue);
}

short DLLCALL CbWID5(       unsigned short iPortNr,
                            unsigned short iIdNr,
                            unsigned char  FAR *pcSCodeId,
                            unsigned char FAR *pcPwMaster,
                            unsigned char FAR *pcIdCode,
                            unsigned char FAR *pcIdRet)
{
    unsigned char cOutField[1];
    unsigned char cInField[14];
    unsigned short  iOpCode;
    unsigned short  iRetValue;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);
    cInField[4] = *(pcPwMaster+4);
    cInField[5] = *(pcPwMaster+3);
    cInField[6] = *(pcPwMaster+2);
    cInField[7] = *(pcPwMaster+1);
    cInField[8] = *(pcPwMaster);
    cInField[9] = *(pcIdCode+2);
    cInField[10] = *(pcIdCode+1);
    cInField[11] = *(pcIdCode);
    cInField[12] = *(pcIdRet+1);
    cInField[13] = *(pcIdRet);

    iOpCode = 0x25;

    iRetValue = CB560(iOpCode, iPortNr, cInField, cOutField);

    return(iRetValue);
}

short DLLCALL CbWID6(       unsigned short iPortNr,
                            unsigned short iIdNr,
                            unsigned char  FAR *pcSCodeId,
                            unsigned char FAR *pcPwMaster,
                            unsigned char FAR *pcIdCode,
                            unsigned char FAR *pcIdRet)
{
    unsigned char cOutField[1];
    unsigned char cInField[14];
    unsigned short  iOpCode;
    unsigned short  iRetValue;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);
    cInField[4] = *(pcPwMaster+4);
    cInField[5] = *(pcPwMaster+3);
    cInField[6] = *(pcPwMaster+2);
    cInField[7] = *(pcPwMaster+1);
    cInField[8] = *(pcPwMaster);
    cInField[9] = *(pcIdCode+2);
    cInField[10] = *(pcIdCode+1);
    cInField[11] = *(pcIdCode);
    cInField[12] = *(pcIdRet+1);
    cInField[13] = *(pcIdRet);

    iOpCode = 0x26;

    iRetValue = CB560(iOpCode, iPortNr, cInField, cOutField);

    return(iRetValue);
}

short DLLCALL CbWID7(       unsigned short iPortNr,
                            unsigned short iIdNr,
                            unsigned char  FAR *pcSCodeId,
                            unsigned char FAR *pcPwMaster,
                            unsigned char FAR *pcIdCode,
                            unsigned char FAR *pcIdRet)
{
    unsigned char cOutField[1];
    unsigned char cInField[14];
    unsigned short  iOpCode;
    unsigned short  iRetValue;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);
    cInField[4] = *(pcPwMaster+4);
    cInField[5] = *(pcPwMaster+3);
    cInField[6] = *(pcPwMaster+2);
    cInField[7] = *(pcPwMaster+1);
    cInField[8] = *(pcPwMaster);
    cInField[9] = *(pcIdCode+2);
    cInField[10] = *(pcIdCode+1);
    cInField[11] = *(pcIdCode);
    cInField[12] = *(pcIdRet+1);
    cInField[13] = *(pcIdRet);

    iOpCode = 0x27;

    iRetValue = CB560(iOpCode, iPortNr, cInField, cOutField);

    return(iRetValue);
}

short DLLCALL CbWID8(       unsigned short iPortNr,
                            unsigned short iIdNr,
                            unsigned char  FAR *pcSCodeId,
                            unsigned char FAR *pcPwMaster,
                            unsigned char FAR *pcIdCode,
                            unsigned char FAR *pcIdRet)
{
    unsigned char cOutField[1];
    unsigned char cInField[14];
    unsigned short  iOpCode;
    unsigned short  iRetValue;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);
    cInField[4] = *(pcPwMaster+4);
    cInField[5] = *(pcPwMaster+3);
    cInField[6] = *(pcPwMaster+2);
    cInField[7] = *(pcPwMaster+1);
    cInField[8] = *(pcPwMaster);
    cInField[9] = *(pcIdCode+2);
    cInField[10] = *(pcIdCode+1);
    cInField[11] = *(pcIdCode);
    cInField[12] = *(pcIdRet+1);
    cInField[13] = *(pcIdRet);

    iOpCode = 0x28;

    iRetValue = CB560(iOpCode, iPortNr, cInField, cOutField);

    return(iRetValue);
}

short DLLCALL CbWPwR2(   unsigned short iPortNr,
                            unsigned short iIdNr,
                            unsigned char FAR *pcSCodeId,
                            unsigned char FAR *pcPwMaster,
                            unsigned char FAR *pcPwRam)
{
    unsigned char cOutField[1];
    unsigned char cInField[13];
    unsigned short  iOpCode;
    unsigned short  iRetValue;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);
    cInField[4] = *(pcPwMaster+4);
    cInField[5] = *(pcPwMaster+3);
    cInField[6] = *(pcPwMaster+2);
    cInField[7] = *(pcPwMaster+1);
    cInField[8] = *(pcPwMaster);
    cInField[9] = *(pcPwRam+3);
    cInField[10] = *(pcPwRam+2);
    cInField[11] = *(pcPwRam+1);
    cInField[12] = *(pcPwRam);

    iOpCode = 0x23;

    iRetValue = CB560(iOpCode, iPortNr, cInField, cOutField);

    return(iRetValue);
}

/* EOF */

