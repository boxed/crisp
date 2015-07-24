
/* =====[ CBN_NT.C ]=========================================================

   Description:     NT devive driver interface part.

   Compiled:        MS-VC.

   Compiler opt:    See makefile.

   Revisions:

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  mm/dd/95  Peter Glen    Initial version.

   ======================================================================= */

/* -------- System includes:  -------------------------------------------- */

#include    <windows.h>
#include    <winioctl.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/* -------- Includes:  --------------------------------------------------- */

#include "marxdev.h"
#include "marxcode.h"
#include "interfce.h"

#define UNDER_WIN95
#define MARX_IMPORT
#include "cbn_.h"

/* -------- Defines: ----------------------------------------------------- */

#define MAX_PORT  3
//define PG_DEBUG

/* -------- Macros: ------------------------------------------------------ */


/* -------- Forward references for procedures: --------------------------- */

short    cb560_nt1 (    unsigned short     iOpCode,
                        unsigned short     iPortNr,
                        void               *pcInData,
                        void               *pcOutData,
                        unsigned short     iOutlen
                   );

/* -------- Strings: ----------------------------------------------------- */

/*
 * Array of pointers:
 */

char    static name1[]  =  MARX_DEVICE_NAME1;
char    static name2[]  =  MARX_DEVICE_NAME2;
char    static name3[]  =  MARX_DEVICE_NAME3;


/*
 * Make it 1 based by repeating port 1:
 */

char    *names[] = {name1, name1, name2, name3};


/* -------- Implementation: ---------------------------------------------- */


/*
 * This is a visible part for 95 DLL level:
 */

short    cb560_nt (     unsigned short     iOpCode,
                        unsigned short     iPortNr,
                        void               *pcInData,
                        void               *pcOutData,
                        unsigned short     iOutlen
                   )
{

    short ret_val = 0, ret_val2 = 0, solid = 0;

    if(iPortNr >= 10)
        {
        solid = 1;                      // no auto search
        iPortNr -= 10;
        }
    iPortNr = min(iPortNr, 3);          // safety port number limit

    /*
     * See if we have got a NT crypto box, do action:
     */
    if(solid)
        {
        ret_val = cb560_nt1(iOpCode, iPortNr,
                      (unsigned char FAR *)pcInData,
                              (unsigned char FAR *)pcOutData, iOutlen);
        }
    else
        {
        short port;
        short ports[2 * MAX_PORT + 2];

        ret_val = BOX_DRIVER_ERROR;

        /*
         * Scan ports as in spec:
         *
         * Please note how much easier it is to do it from high level.
         */
        for(port = 0; port < MAX_PORT; port++)
          {
            ports[port+1] = port+1;
            ports[port+ 1 + MAX_PORT ] = port+1;

            #ifdef PG_DEBUG
            //DebugDump(&ports[port + iPortNr], 4, "port info:");
            #endif
          }

        for(port = 0; port < MAX_PORT; port++)
          {
            ret_val2 = cb560_nt1(iOpCode, ports[port + iPortNr],
                      (unsigned char FAR *)pcInData,
                              (unsigned char FAR *)pcOutData, iOutlen);

            /*
             * The box responded, we need search no further:
             */
            if(!ret_val2)
                {
                ret_val = ret_val2;
                break;
                }
            if(ret_val2 != BOX_DRIVER_ERROR)
                {
                ret_val = ret_val2;
                }
          }
        /*
         * Yes, that's it. I bet that this was a two weeks job in
         * assembler.
         * The best part is that it can be built in to new code (NT, '95)
         * within no time.
         *
         * Here is how it works:
         *
         *       ports[port + iPortNr] will overwrap to the end of the
         *       array which is initialized with the correct port number.
         *
         * Reality check:
         * --------------
         *
         *  The PS2 has 4 printer ports. This code can be adapted by
         *  defining MAX_PORT at 4 instead of 3.
         *  Imagine the effort required in assembler.
         *
         */
        }
    return(ret_val);
}

/*
 *
 */

short    cb560_nt1(     unsigned short     iOpCode,
                        unsigned short     iPortNr,
                        void               *pcInData,
                        void               *pcOutData,
                        unsigned short     iOutlen
                   )

{
    unsigned long   ret_val;

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

    iPortNr = min(iPortNr, 3);            // limit the port
    devname = names[iPortNr];             // get the name of the port

    if(!ptr)
        {
        /*
         * Failed to get enough memory:
         */
        #ifdef TEST_DRIVER
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
                devname,                         // Open the Device "file"
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if (hndFile == INVALID_HANDLE_VALUE)        // Was the device opened?
        {
        #ifdef TEST_DRIVER
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
                            iOutlen +
                            offsetof(MARX_I_O,firstbyte),
                            ptr,                // Buffer from driver.
                            iOutlen +
                            offsetof(MARX_I_O,firstbyte),
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

        #ifdef TEST_DRIVER
        wsprintf(test_buf, "Ioctl failed with code %ld\n", GetLastError() );
        MessageBox ( GetFocus(), (LPCTSTR) test_buf,
                (LPCTSTR) "TEST", MB_OK | MB_SYSTEMMODAL);
        #endif
        }
    /*
     * Close device:
     */
    if (!CloseHandle(hndFile))                   // Close the Device "file".
        {
        #ifdef TEST_DRIVER
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


/*
 * Return TRUE for existing port:
 */
#ifdef _test560
short    cb560_tst(unsigned short  iPortNr)

{
    long   ret_val = FALSE;

    // The following parameters are used in the IOCTL call:
    //BOOL    IoctlResult;

    HANDLE  hndFile;            // Handle to device, obtain from CreateFile
    LONG    IoctlCode;
    char    *devname;
    DWORD   ReturnedLength;     // Number of bytes returned
    MARX_I_O    iobuf;

    iPortNr = min(iPortNr, 3);            // limit the port
    devname = names[iPortNr];             // get the name of the port

    iobuf.iOpCode = 0;
    iobuf.iPortNr = 0;
    iobuf.ret_val = 0;

    /*
     * Open device:
     */
    hndFile = CreateFile(
                devname,                         // Open the Device "file"
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if (hndFile == INVALID_HANDLE_VALUE)        // Was the device opened?
        {
        #ifdef TEST_DRIVER
        wsprintf(test_buf, "Cannot locate 95 device: MARXPORT");
        MessageBox ( GetFocus(), (LPCTSTR) test_buf,
                (LPCTSTR) "TEST", MB_OK | MB_SYSTEMMODAL);
        #endif

        goto end;
        }
    IoctlCode = MARX_IOCTL_TEST;
    DeviceIoControl(
                hndFile,                // Handle to device
                IoctlCode,              // MARX IO Control code
                &iobuf,                 // Buffer to driver.
                sizeof(MARX_I_O),
                &iobuf,                 // Buffer to driver.
                sizeof(MARX_I_O),
                &ReturnedLength,        // Bytes placed in DataBuffer
                NULL                    // NULL means await compleion
                );

    CloseHandle(hndFile);               // Close the Device "file".
    ret_val =  iobuf.ret_val;

   end:
    return((short)ret_val);
}
#endif

/* EOF */