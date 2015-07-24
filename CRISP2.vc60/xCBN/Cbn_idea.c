
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

// #include "cbn_.h"

/*
 * Get the interface of the assembler module:
 */

// #include "interfce.h"

////////////////////////////////////////////////////////////////////////////

/****************************************************************************
*   Einbindung der CbN_ fÅr BORLAND C/C++
*
*   erstellt am 23.02.94 von kr
*
******************************************************************************/

/* --- Constants: ------------------------------------------------------------- */

#define IDEA_BOX_KEY        8         /* number of bytes returned from BOX */

#define CODE_IT             0                      /* action code for code */
#define DECODE_IT           1
#define PASSWORD_SIZE       16
#define SCRAMBLE_SIZE       32                        /* buffer dimentions */

short  DLLCALL CbN_IDEA_Encrypt(
                                    unsigned short  iPortNr,
                                    unsigned short  iIdNr,
                                    unsigned char   FAR *pcSCodeId,
                                    unsigned char   FAR *pcBuff,
                                    unsigned long   ulLen
                                 )

{
    unsigned char cInField[4];
    unsigned char cOutField[17] = {0};
    unsigned short   iCount;
    unsigned short   iOpCode;
    short   iRetValue;
    unsigned char ucMethod;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);

    iOpCode = 0x29;
    iRetValue = cb560_ext(iOpCode, iPortNr, cInField, cOutField,
                          (unsigned short) IDEA_BOX_KEY);

    for( iCount = 8; iCount<16; iCount++)
        cOutField[iCount] = (unsigned char) (cOutField[iCount-8] ^ cOutField[iCount-1]);

    cOutField[16] = 0;
    ucMethod = CODE_IT;

    if ( !iRetValue)
            IDEA_Code(ucMethod, pcBuff, (unsigned short)ulLen, cOutField);

    return(iRetValue);
}


short  DLLCALL CbN_IDEA_Decrypt(
                                    unsigned short  iPortNr,
                                    unsigned short  iIdNr,
                                    unsigned char   FAR *pcSCodeId,
                                    unsigned char   FAR *pcBuff,
                                    unsigned long   ulLen

                                  )
{
    unsigned char cInField[4];
    unsigned char cOutField[17] = {0};
    unsigned short   iOpCode;
    unsigned short   iCount;
    short   iRetValue;
    unsigned char ucMethod;

    cInField[0] = (unsigned char) (iIdNr & 0xff);
    cInField[1] = *(pcSCodeId+2);
    cInField[2] = *(pcSCodeId+1);
    cInField[3] = *(pcSCodeId);

    iOpCode = 0x29;
    iRetValue = cb560_ext(iOpCode, iPortNr, cInField, cOutField,
                          (unsigned short) IDEA_BOX_KEY);

    for( iCount = 8; iCount<16; iCount++)
        cOutField[iCount] = (unsigned char) (cOutField[iCount-8] ^ cOutField[iCount-1]);

    cOutField[16] = 0;

    ucMethod = DECODE_IT;

    if ( !iRetValue)
        IDEA_Code(ucMethod, pcBuff, (unsigned short)ulLen, cOutField);

    return(iRetValue);
}

/* EOF */

