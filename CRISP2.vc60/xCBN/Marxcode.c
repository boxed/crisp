
/* =====[ MARXCODE.C ]=========================================================

   Description:     Simple coding algoritm in 'C'.

                    Used in:    Coding the MARXDEV communication.

   Notes:           This file is intended to be used as a source include
                    file. No provision is made to protect multiple
                    including etc ...
   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  30.05.93  Peter Glen    Initial version.
      0.00  15.02.95  Peter Glen    Gone to WINDOWS 95.
      0.00  18.05.95  Peter Glen    WIN 95 initial version.

   ======================================================================= */

#include    <windows.h>
#include    <winioctl.h>

#include    <stddef.h>
#include    <stdlib.h>
#include    <stdio.h>

/* -------- Includes:  --------------------------------------------------- */

#include "marxdev.h"
#include "marxcode.h"
#include "interfce.h"


/*
 * Define the NOPRE if precoding is not desired. For test only.
 */

//define  NOPRE

#define MAGIC_NUMBER 0xaa           // any number would do

/*
 * Precode buffer. Pointer to string and length is passed.
 *
 * Simple algorytm:     XOR first byte, sum the following with  n-1
 *                      (and XOR the sum)
 *
 */

void    precode_buffer(UCHAR *str, ULONG len)

{
    register UCHAR   chh;

    #ifdef  NOPRE
    return;
    #endif

    if(!len--)
        return;                                              /* safety net */

    *str++ ^= MAGIC_NUMBER;
    while(len--)
        {
        *str ^= MAGIC_NUMBER;

        chh  =  *str;
        chh += *(str-1);
        *str = chh;

        str++;
        }
}

/*
 * Decode buffer. Pointer to string and length is passed.
 *
 */

void    predecode_buffer(UCHAR *str, ULONG len)

{
    register UCHAR   chh;
    register UCHAR   prev;

    #ifdef  NOPRE
    return;
    #endif

    if(!len--)                                               /* safety net */
        return;

    prev =  *str;
    *str++ ^= MAGIC_NUMBER;

    while(len--)
        {
        chh  =  *str;
        chh  -= prev;
        prev =  *str;
        *str =  chh;

        *str ^= MAGIC_NUMBER;
        str++;
        }
}

/* EOF */

