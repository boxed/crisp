
/* =====[ xxx.c ]=========================================================

   Description:     NT devive driver interface part.

   Compiled:        MS-VC.

   Compiler opt:    See makefile.

   Revisions:

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  mm/dd/95  Peter Glen    Initial version.
      0.00  03/26/96  Peter Glen    To MJ delivered, he can reproduce the err
      0.00  03/27/96  Peter Glen    Malloc for copy of the buffer to retry
      0.00  04/03/96  Peter Glen    Removed retry part.

   ======================================================================= */

/* -------- System includes:  -------------------------------------------- */

#include    <windows.h>
#include    <winioctl.h>

#include    <stddef.h>
#include    <stdlib.h>
#include    <stdio.h>

/* -------- Includes:  --------------------------------------------------- */

#include "marxdev.h"
#include "marxcode.h"
#include "interfce.h"

#define UNDER_WIN95
#define MARX_IMPORT
#include "cbn_.h"


/* static short    cb560_try(     unsigned short     iOpCode,
                        unsigned short     iPortNr,
                        void               *pcInData,
                        void               *pcOutData,
                        unsigned short     iOutlen
                   ); */


/* -------- Defines: ----------------------------------------------------- */

//define PG_DEBUG

/* -------- Strings: ----------------------------------------------------- */

char    name1[]  =  MARX_95DEVICE_NAME1;

/* -------- Implementation: ---------------------------------------------- */


//*******************************************************************
//
// RETRY CODE, DEACTIVATED
//
//*******************************************************************

//static short gl_retry = 1;


//short    cb560_95(      unsigned short     iOpCode,
//                        unsigned short     iPortNr,
//                        void               *pcInData,
//                        void               *pcOutData,
//                        unsigned short     iOutlen
//                   )
//
//
//{
//    unsigned short  ret_val, retry;
//    char FAR *mem = NULL;
//
//    retry = gl_retry;           // atomic number
//
//    /*
//     * Allocate a copy of buffer, so retries will have a new life:
//     */
//
//    mem = malloc(iOutlen);
//
//    #ifdef PG_DEBUG
//    wsprintf(test_buf, "Cannot allocate 95 retry memory: MARXPORT");
//    MessageBox ( GetFocus(), (LPCTSTR) test_buf,
//            (LPCTSTR) "TEST", MB_OK | MB_SYSTEMMODAL);
//    #endif
//
//    if( !mem)
//        {
//        ret_val = BOX_DRIVER_ERROR;             // no memory, flag error
//        goto endd;
//        }
//
//    while (TRUE)
//        {
//        retry--;
//
//        memcpy(mem, pcOutData, iOutlen);      // copy to our memory
//
//        //
//        // The function is called with local buffer:
//        //
//        ret_val = cb560_try(iOpCode, iPortNr,
//                    (unsigned char FAR *)pcInData,
//                            (unsigned char FAR *)mem, iOutlen);
//
//        //
//        // See if we have succeded:
//        //
//        if (!ret_val)
//            {
//            memcpy(pcOutData, mem, iOutlen);
//            break;
//            }
//
//        //
//        // See if we run out of retries:
//        //
//        if (!retry)
//            break;
//
//        //
//        // Now have a secure box problem:
//        //
//        if (ret_val == BOX_BOX_READY_ERR)
//            break;
//
//        //
//        // Please note that the prvious section can be described as:
//        //
//        // if (ret_val == BOX_BOX_READY_ERR || !retry)
//        //     break;
//        //
//        // But it is less readable.
//        //
//
//        }
//   endd:
//
//    if (mem)                    // free resources
//        free(mem);
//
//    return(ret_val);
//}
//

/*
 * This is a visible part for 95 DLL level:
 *
 *  History:
 *
 *     The time of this operation is too long for the '95 kernel to
 *     tolarate a lock. We cannot do anything against it, but we have to wait
 *     with a task switch.
 *
 *     In that case, we retry the task switch.
 *
 *  Tuning:
 *
 *     I have done 3 retries. Do it to your taste.
 *
 *           If to high, crypo box will not feel "snappy"
 *
 *           If to low, risk of fail.
 *
 */

short    cb560_95(     unsigned short     iOpCode,
                        unsigned short     iPortNr,
                        void               *pcInData,
                        void               *pcOutData,
                        unsigned short     iOutlen
                   )


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

    short     fullsize =  iOutlen + offsetof(MARX_I_O, firstbyte);
    ptr =   malloc(fullsize + 1);

    devname = name1;

    if(!ptr)
        {
        /*
         * Failed to get enough memory:
         */
        #ifdef PG_DEBUG
        wsprintf(test_buf, "Cannot allocate 95 memory: MARXPORT");
        MessageBox ( GetFocus(), (LPCTSTR) test_buf,
                (LPCTSTR) "TEST", MB_OK | MB_SYSTEMMODAL);
        #endif
        ret_val = BOX_DRIVER_ERROR;
        goto endd;
        }
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
        ret_val = BOX_DRIVER_ERROR;
        goto endd;
        }

    /*
     * Decorate the memory with the contents of input/output:
     */

    ptr->iOpCode = (unsigned long) iOpCode;
    ptr->iPortNr = (unsigned long) iPortNr;
    ptr->ret_val = (unsigned long) 0;
    memcpy(ptr->input, pcInData, sizeof(ptr->input));
    memcpy(ptr->firstbyte, pcOutData, iOutlen);

    /*
     * We have a plain buffer, scramble it:
     */
    predecode_buffer((UCHAR *)ptr, iOutlen +
                                        offsetof(MARX_I_O,firstbyte));
    /*
     * Device there, all set, call the ioctrl:
     */
    IoctlCode = MARX_IOCTL_ID;

    IoctlResult = DeviceIoControl(
                            hndFile,            // Handle to device
                            IoctlCode,          // MARX IO Control code
                            ptr,                // Buffer to driver.
                            iOutlen + offsetof(MARX_I_O,firstbyte),
                            ptr,                // Buffer from driver.
                            iOutlen + offsetof(MARX_I_O,firstbyte),
                            &ReturnedLength,    // Bytes placed in DataBuffer
                            NULL                // NULL means await compleion
                            );

    if (IoctlResult)                            // Did the IOCTL succeed?
        {
        /*
         * We have a coded buffer, unscramble it:
         */

        precode_buffer((UCHAR *)ptr, iOutlen +
                                        offsetof(MARX_I_O,firstbyte));
        /*
         * Output results:
         */
        ret_val = ptr->ret_val;

        if(!ret_val)
            memcpy(pcOutData, ptr->firstbyte, iOutlen);
        }
    else
        {
        // Box IOCTRL failed

        ret_val = BOX_DRIVER_ERROR;

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

    if(ptr)
        free(ptr);

    return((short)ret_val);
}

//*******************************************************************
//
// RETRY CODE, DEACTIVATED
//
//*******************************************************************

///*
// * Set number of retries for the CBN 95 subsystem.
// *
// *  Win '95 multitask may stop atomic box operation, this will allow a
// *  seemless retry, so the box I/O will do irt thing correctly.
// *
// *  Current value:      1   (No retries, just execute)
// *
// *  Recommended value:  3   (only if problems occur)
// *
// *  Function will not allow: num < 1
// *
// *  In:
// *
// *      New number of retries
// *
// *  Out:
// *
// *      Old number of retries
// *
// */
//
//short    CBN_SET_RETRY(short num)
//
//{
//    short ret_val;
//
//    ret_val = gl_retry;         // save old value
//
//    if (num > 1)                // set new
//        gl_retry = num;
//
//    return(ret_val);
//}

/* EOF */
