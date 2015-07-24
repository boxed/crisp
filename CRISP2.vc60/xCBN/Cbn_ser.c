
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

#include    <windows.h>
#include    <winioctl.h>

#include    <stddef.h>
#include    <stdlib.h>
#include    <stdio.h>

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
****************************************************************************/

short dummy2        = 0;
short gl_ret_val    = 0;
short gl_ext_ret_val= 0;

////////////////////////////////////////////////////////////////////////////

short DLLCALL CbN_ReadSER(
                                unsigned short iPortNr,
                                unsigned char FAR *pcSCodeSer,
                                unsigned long FAR *pulSerRet
                         )
{
    unsigned char cInField[2];
    unsigned char cOutField[4];
    unsigned short  iOpCode;
    short  iRetValue;

    cInField[0] = *(pcSCodeSer+1);
    cInField[1] = *(pcSCodeSer);
    iOpCode = 0x10;

    iRetValue = cb560_ext(iOpCode, iPortNr,
                                cInField, cOutField, sizeof(long));

    cOutField[3] = 0;
    *pulSerRet = *(unsigned long *) cOutField;

    return(iRetValue);
}


////////////////////////////////////////////////////////////////////////////

short   DLLCALL CbN_BoxReady(
                                unsigned short iPortNr,
                                unsigned char FAR *pcBoxName
                            )
{
    unsigned char cInField[1];              /* dummy2 vaiable */
    unsigned char cOutField[2];
    unsigned short  iOpCode;
    short  iRetValue;

    iOpCode = 0x13;

    iRetValue = cb560_ext(iOpCode, iPortNr,
                                cInField, cOutField, sizeof(short));

    *pcBoxName = *(unsigned char *) cOutField;
    return(iRetValue);
}

////////////////////////////////////////////////////////////////////////////

/*
 * Pointerless functions:
 */

/*
 * We have a local segment, and this segment is used by IDEA.
 * Windows will point DS to the segment of the application, and
 * read/write to the segment overwrites application data.
 *
 * Correction:  Make the module point to the local DATA segment.
 *              We made all the initialized, and that pulled  it
 *              to DATA instead of _BSS. The andvantage is that we made
 *              a single segment, and it is sufficient to chan DS.
 *
 * I feel that it is an assembler path, there is a way for doing it
 * legally with switches and options. The question is that will the
 * legal way do it correctly with all modules, 16 bit, 32 bit 95
 * NT etc ....
 *
 * Well the assembler will do it correct in all those situations.
 *
 *  P.G.
 *
 *  History:    Date       Author     Comment
 *              ??/??/93              Created
 *              16/06/95   PG         Corrected bad segment overwrites.
 *
 *   //_asm push    ds                 // save segment
 *   //_asm mov     ax, seg loop1      // local segment
 *   //_asm mov     ds,ax
 *
 *
 * This comment is repeated in modules that this fix was needed.
 *
 */


long   DLLCALL CBN_R_READSER( unsigned short iPortNr, long SCodeSer )

{
    long    ser_num = 0;
    short  iRetValue;

    //_asm push    ds                              // save segment
    //_asm mov     ax, seg dummy2                  // local segment
    //_asm mov     ds,ax

    iRetValue = CbN_ReadSER(iPortNr,
                            (unsigned char FAR *)&SCodeSer,
                                (unsigned long FAR *)&ser_num);
    if(iRetValue)
          ser_num = 0;                          // error, make blank number

    /*
     * Far pointers were used, restore segments:
     */
    //_asm mov     ax, seg dummy2                  // local segment
    //_asm mov     ds,ax

    gl_ret_val  = iRetValue;

    //_asm    pop ds

    return(ser_num);
}

short   DLLCALL CBN_R_BOXREADY(unsigned short iPortNr)

{
    short           RetValue;
    unsigned char   box_name = 0;

    //_asm push    ds                              // save segment
    //_asm mov     ax, seg dummy2                  // local segment
    //_asm mov     ds,ax

    RetValue = CbN_BoxReady(iPortNr, (unsigned char FAR *)&box_name);

    if(RetValue)
        box_name = 0;

    /*
     * Far pointers were used, restore segments:
     */
    //_asm mov     ax, seg dummy2                  // local segment
    //_asm mov     ds,ax

    gl_ret_val  = RetValue;

    //_asm    pop ds

    return(box_name);
}


short   DLLCALL CBN_LAST_ERROR(void)

{
    short ret_val;

    //_asm push    ds                              // save segment
    //_asm mov     ax, seg dummy2                  // local segment
    //_asm mov     ds,ax

    ret_val = gl_ret_val;

    //_asm    pop ds

    return(ret_val);
}

////////////////////////////////////////////////////////////////////////////

/*
 * Version function:
 *
 * This function is supplied to make the customer support more easy.
 *
 */

short   DLLCALL CBN_VERSION()

{
    short ver;

    //_asm push    ds                              // save segment
    //_asm mov     ax, seg dummy2                  // local segment
    //_asm mov     ds,ax

    ver = gl_version;

    //_asm    pop ds

    return(ver);
}

/*
 * Version function:
 *
 * This function is supplied to make the customer support more easy.
 * The code is a little complex for version function, but anything
 * for a customer.
 *
 */

#include "marxdev.h"

short   DLLCALL CBN_VXD_VERSION()

{
    unsigned long  ret_val;

    // The following parameters are used in the IOCTL call:

    BOOL    IoctlResult;
    HANDLE  hndFile;            // Handle to device, obtain from CreateFile
    LONG    IoctlCode;
    LONG    DataLength = 1;
    DWORD   ReturnedLength;     // Number of bytes returned
    char    *devname;
    MARX_I_O    *ptr = NULL;
    LONG    data1;

    devname = name1;

    /*
     * Open device:
     */
    hndFile = CreateFile(
                devname,                        // Open the Device "file"
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if (hndFile == INVALID_HANDLE_VALUE)        // Was the device opened?
        {
        #ifdef PG_DEBUG
        wsprintf(test_buf, "Cannot locate 95 device: MARXPORT");
        MessageBox ( GetFocus(), (LPCTSTR) test_buf,
                (LPCTSTR) "TEST", MB_OK | MB_SYSTEMMODAL);
        #endif
        ret_val = 0;
        goto endd;
        }
    /*
     * Device there, all set, call the ioctrl:
     */
    IoctlCode = MARX_IOCTL_TEST;

    IoctlResult = DeviceIoControl(
                            hndFile,            // Handle to device
                            IoctlCode,          // MARX IO Control code
                            &data1,             // Buffer to driver.
                            sizeof(LONG),
                            &data1,             // Buffer from driver.
                            sizeof(LONG),
                            &ReturnedLength,    // Bytes placed in DataBuffer
                            NULL                // NULL means await compleion
                            );

    if (IoctlResult)                            // Did the IOCTL succeed?
        {
        ret_val = (short)data1;
        }
    else
        {
        // Box IOCTRL failed

        ret_val = 0;

        #ifdef PG_DEBUG
        wsprintf(test_buf, "Ioctl failed with code %ld\n", GetLastError() );
        MessageBox ( GetFocus(), (LPCTSTR) test_buf,
                (LPCTSTR) "TEST", MB_OK | MB_SYSTEMMODAL);
        #endif
        }
    /*
     * Close device:
     */
    if (!CloseHandle(hndFile))                  // Close the Device "file".
        {
        #ifdef PG_DEBUG
        wsprintf(test_buf, "Cannot close 95 device: MARXPORT");
        MessageBox ( GetFocus(), (LPCTSTR) test_buf,
                (LPCTSTR) "TEST", MB_OK | MB_SYSTEMMODAL);

        #endif
        }
    else
        {
        // Box IOCTRL failed, we throw away error code ...
        }

   endd:

    return((short)ret_val);
}

////////////////////////////////////////////////////////////////////////////

/* EOF */

