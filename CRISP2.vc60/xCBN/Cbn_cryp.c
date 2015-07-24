
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

#define MARX_IMPORT
#include "cbn_.h"

////////////////////////////////////////////////////////////////////////////

/****************************************************************************
*   Einbindung der CbN_ fr BORLAND C/C++
*
*   erstellt am 23.02.94 von kr
*
*   Modell: SMALL
*
*   Name der Library:   CbN_bc4s.lib
*
*   Funktionen:
*
*   short CbN_Decrypt(unsigned short iPortNr, unsigned short iIdNr, unsigned char
*                  *pcSCodeId, unsigned short iSeed, *unsigned short uiLength,
*                  unsigned char *pcOutData)
*
*   short CbN_Encrypt(unsigned short iPortNr, unsigned short iIdNr, unsigned char
*                  *pcSCodeId, unsigned short iSeed, *unsigned short uiLength,
*                  unsigned char *pcOutData)
*
****************************************************************************/


short DLLCALL CbN_Decrypt(
                                unsigned short iPortNr,
                                unsigned short iIdNr,
                                unsigned char FAR *pcSCodeId,
                                unsigned short iSeed,
                                unsigned short uiLength,
                                unsigned char FAR *pcOutData
                             )
{
    unsigned char cInField[8];
    unsigned short  iOpCode;
    short  iRetValue;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);
    cInField[4] = (unsigned char) ((uiLength>>8) & 0xff);
    cInField[5] = (unsigned char) (uiLength & 0xff);
    cInField[6] = (unsigned char) (iSeed &0xff);
    cInField[7] = (unsigned char) ((iSeed>>8) &0xff);

    iOpCode = 0x12;

    //iRetValue = CB560(iOpCode, iPortNr, cInField, pcOutData);
    iRetValue = cb560_ext(iOpCode, iPortNr, cInField, pcOutData, uiLength);

    return(iRetValue);
}


short DLLCALL CbN_Encrypt(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char FAR *pcSCodeId,
                                    unsigned short iSeed,
                                    unsigned short uiLength,
                                    unsigned char FAR *pcOutData
                             )
{
    unsigned char cInField[8];            // idnr, 3*scid, 2*len, 2*seed
    unsigned short  iOpCode;
    short  iRetValue;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);
    cInField[4] = (unsigned char) ((uiLength>>8) & 0xff);
    cInField[5] = (unsigned char) (uiLength & 0xff);
    cInField[6] = (unsigned char) (iSeed &0xff);
    cInField[7] = (unsigned char) ((iSeed>>8) &0xff);

    iOpCode = 0x11;

    //iRetValue = CB560(iOpCode, iPortNr, cInField, pcOutData);
    iRetValue = cb560_ext(iOpCode, iPortNr, cInField, pcOutData, uiLength);

    return(iRetValue);
}


/* EOF */

