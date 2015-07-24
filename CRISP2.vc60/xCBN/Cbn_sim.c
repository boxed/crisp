
/* =====[ CBN_SIM.C ]====================================================

   Description:     Simulation of crypto box.

   Compiled:        MS-C.

   Compiler opt:    ??

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  xx.xx.94  Peter Glen    Initial version.
      0.00  18.05.95  Peter Glen  WIN 95 initial version.

   ======================================================================= */

/* -------- System includes:  -------------------------------------------- */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>
#include <string.h>


/* -------- Includes:  --------------------------------------------------- */

static  unsigned char   ram1[RAM1LENG];
static  unsigned char   ram2[RAM2LENG];
static  void    encrypt(char  *str, short len);

/* -------- Data: -------------------------------------------------------- */

#define BUFLEN  48
char    buf[BUFLEN+1];

/* -------- Implementation: ---------------------------------------------- */

/*
 * Simulating the crypto box:
 */

short    cb560_sim(         ushort    iOpCode,
                            ushort    iPortNr,
                            uchar    *pcInData,
                            uchar    *pcOutData)
{
    short   ret_val = 0;
    unsigned short len, start;
    unsigned char *ptr;

    TESTPORT();

    /*
     * Giant switch for OPCODES:
     */
    switch(iOpCode)
        {
        ///////////////////////////////////////////////////////////////////
        // ID-s:

        case 1:
                *((long*)pcOutData) = 0x1415;
                break;

        case 2:
                *((long*)pcOutData) = 0x1416;
                break;

        case 3:
                *((long*)pcOutData) = 0x1417;
                break;

        case 4:
                *((long*)pcOutData) = 0x1418;
                break;

        case 5:
                *((long*)pcOutData) = 0x1419;
                break;

        case 6:
                *((long*)pcOutData) = 0x1420;
                break;

        case 7:
                *((long*)pcOutData) = 0x1421;
                break;

        case 8:
                *((long*)pcOutData) = 0x1422;
                break;

        ///////////////////////////////////////////////////////////////////
        // Ser-num:

        case 0x10:
                *((long*)pcOutData) = 0x1423;
                break;

        ///////////////////////////////////////////////////////////////////
        // Ready:

        case 0x13:
                *((short*)pcOutData) = 0xaa;
                break;

        ///////////////////////////////////////////////////////////////////
        // ram1:

        case 0x14:
                /*
                 * Big endian, small endian:
                 */
                start = ((unsigned short)(pcInData[11])) >> 8 | pcInData[10];
                len   = ((unsigned short)(pcInData[9]))  >> 8 |  pcInData[8];

                start = min(start, RAM1LENG);     /* Correcting dimentions */
                len   = min(len, RAM1LENG-start); /* Correcting dimentions */
                memcpy(pcOutData, ram1 + start, len);
                break;

        case 0x16:
                /*
                 * Big endian, small endian:
                 */
                start = ((unsigned short)(pcInData[11])) >> 8 | pcInData[10];
                len   = ((unsigned short)(pcInData[9]))  >> 8 |  pcInData[8];

                start = min(start, RAM1LENG);     /* Correcting dimentions */
                len   = min(len, RAM1LENG-start); /* Correcting dimentions */
                memcpy(ram1 + start, pcOutData, len);
                break;

        case 0x18:
                start = ((unsigned short)(pcInData[9])) >> 8 | pcInData[8];
                start = min(start, RAM1LENG);     /* Correcting dimentions */

                ptr = ram1 + start;
                (*ptr)++;
                *((unsigned short  *) pcOutData) = *ptr;
                break;

        case 0x19:
                start = ((unsigned short)(pcInData[9])) >> 8 | pcInData[8];
                start = min(start, RAM1LENG);     /* Correcting dimentions */

                ptr = ram1 + start;
                (*ptr)--;
                *((unsigned short  *) pcOutData) = *ptr;
                break;


        ///////////////////////////////////////////////////////////////////
        // ram2:

        case 0x15:
                /*
                 * Big endian, small endian:
                 */
                start = ((unsigned short)(pcInData[11])) >> 8 | pcInData[10];
                len   = ((unsigned short)(pcInData[9]))  >> 8 |  pcInData[8];

                start = min(start, RAM2LENG);     /* Correcting dimentions */
                len   = min(len, RAM2LENG-start); /* Correcting dimentions */

                memcpy(pcOutData, ram2 + start, len);
                break;

        case 0x17:
                /*
                 * Big endian, small endian:
                 */
                start = ((unsigned short)(pcInData[11])) >> 8 | pcInData[10];
                len   = ((unsigned short)(pcInData[9]))  >> 8 |  pcInData[8];

                start = min(start, RAM2LENG);     /* Correcting dimentions */
                len   = min(len, RAM2LENG-start); /* Correcting dimentions */

                memcpy(ram2 + start, pcOutData, len);
                break;

        case 0x20:
                start = ((unsigned short)(pcInData[9])) >> 8 | pcInData[8];
                start = min(start, RAM2LENG);     /* Correcting dimentions */

                ptr = ram2 + start;
                (*ptr)++;
                *((unsigned short  *) pcOutData) = *ptr;
                break;

        case 0x21:
                start = ((unsigned short)(pcInData[9])) >> 8 | pcInData[8];
                start = min(start, RAM2LENG);     /* Correcting dimentions */

                ptr = ram2 + start;
                (*ptr)--;
                *((unsigned short  *) pcOutData) = *ptr;
                break;

        ///////////////////////////////////////////////////////////////////
        // crypt:

        case 0x11:
                len   = ((unsigned short)(pcInData[4]))  >> 8 |  pcInData[5];
                encrypt(pcOutData, len);
                break;

        case 0x12:
                len   = ((unsigned short)(pcInData[4]))  >> 8 |  pcInData[5];
                encrypt(pcOutData, len);
                break;

        }
    return(ret_val);
}

static  void    encrypt(char  *str, short len)

{
        while(TRUE)
            {
            if(!len)
                break;
            *str ^= 0xaa;
            str++; len--;
            }
}
