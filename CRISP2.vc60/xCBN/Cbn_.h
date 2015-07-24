
/* =====[ CBN_.H ]=========================================================

   Description:     Prototype (header) file for 16/32 bit DLL.

   ======================================================================= */
/*
 ************************************************************************
 *
 * DLL CALL HEADER FOR MARX CRYPTO BOX.
 *
 * COPYRIGHT:
 * ----------
 * Copyright (C) Marx Datentechnik GmbH 1995, 1996
 *
 * DISCLAIMER OF WARRANTIES:
 * -------------------------
 *
 * The following [enclosed] code is sample code created by Peter Glen.
 * This sample code is provided to you solely for the purpose of assisting
 * you in the development of your applications.  The code is provided "AS IS",
 * without warranty of any kind.  The author and propriator shall not be
 * liable for any damages arising out of your use of the sample code,even if
 * they have been advised of the possibility of such damages.
 *
 ************************************************************************
 *
 * The functions prototyped here, are found in the CBN???.DLL
 *
 * Further notes:
 * --------------
 *
 * These prototypes are telling the compiler how to use the DLL, and what
 * functions (names) are in it.
 * This is why, this header file is READ ONLY for the end user. If error
 * occurs during compilation, the error cannot be repaired by modifying
 * this header. The error should be repaired in the calling convention etc ...
 *
 * The DLL uses the same calling convention as the WIN API-s, so all
 * compilers should be able to use them.
 *
 * To use this DLL, include IMPORT library (???.LIB) in your project.
 * It contains all names, and dummy entries, so the linker will
 * think the functions are linked in.
 * In reality, they are resolved at run time, via the DLL mechanism.
 *
 ************************************************************************
 *
 * If you are including this prototype header from C++ environment, tell the
 * compiler that they are all 'C' funtions:
 *
 *        EXTERN "C"
 *        {
 *        #include "cbn_.h"
 *        }
 *
 * This include file should stop C++ decorated function name generation.
 * Make sure if you use a different compiler, or environment, that the
 * called function names are corresponding with the name declared in this
 * header.
 *
 * Multi platform include examples:
 *
 *  For  WINDOWS 95:
 *
 *          #define UNDER_WIN95
 *          EXTERN "C"
 *              {
 *              #include "cbn_.h"
 *              }
 *
 *  For  WINDOWS 3.1, 3.11:
 *
 *         #define UNDER_WIN31
 *          EXTERN "C"
 *              {
 *              #include "cbn_.h"
 *              }
 *
 *  For  WINDOWS NT:
 *
 *         #define UNDER_WIN_NT
 *          EXTERN "C"
 *              {
 *              #include "cbn_.h"
 *              }
 */

/*
 * The following defines control interface to various platforms:
 *
 *      UNDER_WIN95     // For platform WINDOWS 95
 *      UNDER_WIN31     // For platform WINDOWS 3.1, 3.11
 *      UNDER_WIN_NT    // For platform WINDOWS NT
 *
 */

/*
 * Single inclusion:
 */

#ifndef CBN_defined
#define CBN_defined

/*
 * The following trick will simplify the export/import of DLL functions in
 * (NT) environment:
 *
 * a. No more .def files. Simply include the .lib file, the rest is done
 *    for you.
 *
 * b. The definition EXTERN will be seen by the compiler as:
 *
 *        "__declspec(dllimport)"
 *
 *        So all is done for you, just say:
 *
 *        #include "cbn_.h"
 *
 * c. If you are working from a C++ environment, tell the compiler
 *        the MARX CRYPTO BOX DLL IS A 'C' DLL BY:
 *
 *        EXTERN "C"
 *        {
 *        #include "cbn_.h"
 *        }
 *
 */

/*
 * The following will simplify 32/16 bit inclusion:
 */

#ifdef  MARX_IMPORT
    #define EXTERN
#endif

#ifdef  UNDER_WIN_NT

    #ifdef  MARX_IMPORT
        #define DLLCALL __declspec(dllexport)
    #else
        #define DLLCALL __declspec(dllimport)
    #endif

#endif

#ifdef  UNDER_WIN95

 #ifndef  FAR
    #define FAR
 #endif

 #define DLLCALL WINAPI
 #define EXTERN

#endif


// Default values, assume Windows 16 bit -> 3.1, 3.11 etc ...

 #ifndef  PASCAL
    #define PASCAL  pascal
 #endif

 #ifndef  FAR
    #define FAR _far
 #endif

 #ifndef DLLCALL
    #define DLLCALL  FAR PASCAL
 #endif

 #ifndef EXTERN
    #define EXTERN extern
 #endif


/*
 * The following return codes (and more) may be returned from
 * the CRYPTO BOX:
 */

#define BOX_OK              0x00
#define BOX_PARAM_ERR       0x01
#define BOX_NOT_FOUND       0x02
#define BOX_CRYPT_ERR       0x03
#define BOX_UNI_CRYPT_ERR   0x04
#define BOX_READ_RAM_ERR    0x05
#define BOX_WRITE_RAM_ERR   0x06
#define BOX_RAM_COUNT_ERR   0x07
#define BOX_BOX_READY_ERR   0x08
#define BOX_SEMAFOR_ERROR   0x10
#define BOX_DRIVER_ERROR    0x11
#define BOX_COMM_ERROR      0x12
#define BOX_RESTART_ERR     0x13
#define BOX_WHAT_WINDOWS    0x14

/*
 ************************************************************************
 *
 * Note:
 *
 *      In case of error code values above the listed and defined values,
 *      please contact us.
 *
 ************************************************************************
 */

/*
 * The following codes define box dimentions:
 */

#define CRYPTLENG      128
#define RAM1LENG        50
#define RAM2LENG       433
#define IDEALENG       224+1

/*
 * The following differences are important:
 * ----------------------------------------
 *
 * The NT library will not use the port number variable, because it
 * communicates with a device driver. The port is actually selected by
 * installing a different device driver for a different port.
 *
 * The 16/32 library will work 100% compatible with the older library
 * under windows 3.1, 3.11 etc ...
 *
 * But if it detects NT host, it will attempt to talk with the NT crypto box.
 * If all fails, the DLL will attempt to talk to the local crypto box.
 * This all is transparent to the user. It is stated only, for information
 * purposes, as my belief is that greater understanding helps productivity.
 *
 * The '95 library will not use the port numbers in the following order:
 *
 *  Port 0x378, 0x278, 0x3bc
 *
 * The confusion came from dropping the old monochrome port to the back
 * of the list. Microsoft decided to do it that way.
 *
 * If the call returns BOX_RESTART_ERR, (restartable error) the program
 * may retry the operation. Retry can be done as:
 *
 *    Until a timeout (like 2 seconds)
 *    Or number of times (like 10 tries)
 *
 */

/*
 * These are the functions offered by the CRYPTO BOX:
 */

/* -------- Declarations: ------------------------------------------------ */


EXTERN short DLLCALL CbN_ReadID1(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned long  FAR *plIdReturn
                                 );

EXTERN short DLLCALL CbN_ReadID2(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned long  FAR *plIdReturn
                                 );

EXTERN short DLLCALL CbN_ReadID3(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned long  FAR *plIdReturn
                                 );

EXTERN short DLLCALL CbN_ReadID4(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned long  FAR *plIdReturn
                                 );

EXTERN short DLLCALL CbN_ReadID5(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned long  FAR *plIdReturn
                                 );

EXTERN short DLLCALL CbN_ReadID6(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned long  FAR *plIdReturn
                                 );

EXTERN short DLLCALL CbN_ReadID7(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned long  FAR *plIdReturn
                                 );

EXTERN short DLLCALL CbN_ReadID8(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned long  FAR *plIdReturn
                                 );

EXTERN short DLLCALL CbN_ReadSER(
                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcSCodeSer,
                                    unsigned long  FAR *plSerNum
                                 );

EXTERN short DLLCALL CbN_ReadRAM1(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcPasswRam1,
                                    unsigned short iStartAdr,
                                    unsigned short iLength,
                                    unsigned char  FAR *pcOutData
                                 );

EXTERN short DLLCALL CbN_ReadRAM2(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcPasswRam2,
                                    unsigned short iStartAdr,
                                    unsigned short iLength,
                                    unsigned char  FAR *pcOutData
                                 );

EXTERN short DLLCALL CbN_WriteRAM1(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcPasswRam1,
                                    unsigned short iStartAdr,
                                    unsigned short iLength,
                                    unsigned char  FAR *pcOutData
                                 );

EXTERN short DLLCALL CbN_WriteRAM2(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcPasswRam2,
                                    unsigned short iStartAdr,
                                    unsigned short iLength,
                                    unsigned char  FAR *pcOutData
                                 );

EXTERN short DLLCALL CbN_IncRAM1(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcPasswRam1,
                                    unsigned short iCounterAdr,
                                    unsigned short FAR *piNewCount
                                 );

EXTERN short DLLCALL CbN_DecRAM1(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcPasswRam1,
                                    unsigned short iCounterAdr,
                                    unsigned short FAR *piNewCount
                                 );

EXTERN short DLLCALL CbN_IncRAM2(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcPasswRam2,
                                    unsigned short iCounterAdr,
                                    unsigned short FAR *piNewCount
                                 );

EXTERN short DLLCALL CbN_DecRAM2(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcPasswRam2,
                                    unsigned short iCounterAdr,
                                    unsigned short FAR *piNewCount
                                 );


EXTERN short DLLCALL CbN_Decrypt(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned short iSeed,
                                    unsigned short iLength,
                                    unsigned char  FAR *pcOutData
                                 );

EXTERN short DLLCALL CbN_Encrypt(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned short iSeed,
                                    unsigned short iLength,
                                    unsigned char  FAR *pcOutData
                                 );

EXTERN short DLLCALL CbN_IDEA_Encrypt(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcBuff,
                                    unsigned long  Length
                                 );

EXTERN short DLLCALL CbN_IDEA_Decrypt(
                                    unsigned short iPortNr,
                                    unsigned short iIdNr,
                                    unsigned char  FAR *pcSCodeId,
                                    unsigned char  FAR *pcBuff,
                                    unsigned long  Length
                                 );


EXTERN short DLLCALL CbN_BoxReady(

                                    unsigned short iPortNr,
                                    unsigned char  FAR *pcBoxName
                                 );

/*
 *  Pointerless functions:
 */

EXTERN short DLLCALL CBN_LAST_ERROR( void);
EXTERN short DLLCALL CBN_R_BOXREADY( unsigned short iPortNr);
EXTERN long  DLLCALL CBN_R_READSER(  unsigned short iPortNr, long pcSCodeSer);

EXTERN long DLLCALL CBN_R_READID1(   unsigned short iPortNr, long cSCodeId);
EXTERN long DLLCALL CBN_R_READID2(   unsigned short iPortNr, long cSCodeId);
EXTERN long DLLCALL CBN_R_READID3(   unsigned short iPortNr, long cSCodeId);

EXTERN long DLLCALL CBN_R_READID4(   unsigned short iPortNr, long cSCodeId);
EXTERN long DLLCALL CBN_R_READID5(   unsigned short iPortNr, long cSCodeId);
EXTERN long DLLCALL CBN_R_READID6(   unsigned short iPortNr, long cSCodeId);
EXTERN long DLLCALL CBN_R_READID7(   unsigned short iPortNr, long cSCodeId);
EXTERN long DLLCALL CBN_R_READID8(   unsigned short iPortNr, long cSCodeId);

/*
 *  Version control, misc:
 */

EXTERN short   DLLCALL CBN_VERSION();
EXTERN short   DLLCALL CBN_VXD_VERSION();

/*
 *  Applied only to the old compatibility lib:
 */

EXTERN short   DLLCALL CBN_DISABLE_DDE(short flag);

#endif

/* EOF */