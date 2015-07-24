
/*
 * =====[ CODEIDEA.C ]======================================================
 *
 * Description:     Test IDEA code algoritm.
 *
 * Compiled:        MS-C.
 *
 * Compiler opt:    ??
 *
 * Revisions:
 *
 *
 *  REV     DATE     BY           DESCRIPTION
 *  ----  --------  ----------  --------------------------------------
 *  0.00  01.01.95  Peter Glen  Initial version.
 *  0.00  01.10.95  Peter Glen  Simulation version released.
 *  0.00  10.02.95  Peter Glen  Library define removed.
 *  0.00  18.05.95  Peter Glen  WIN 95 initial version.
 *
 * =======================================================================
 */

/* -------- System includes:  -------------------------------------------- */

#ifndef     SIXTEEN_BIT
#include    <windows.h>
#else
#include "mydefine.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef TEST            // JK aktiv nur fÅr Testroutine
#include <stddef.h>
#include <dos.h>
#include <time.h>
#endif

/* -------- Includes:  --------------------------------------------------- */

#include "macros.h"
#include "idea.h"

#define  SECTOR_SIZE 512

char userkey[16] = "0000000000000000";
WORD iv[4] = {0};
short  loop = 0;

/* -------- Strings: ----------------------------------------------------- */


/* -------- Implementation: ---------------------------------------------- */
//JK pcUserKey , da sonst Compiler Warning: DS != SS, Var auf SS
void IDEA_Code(unsigned char ucCode_Code, char  * pcDataBuffer, //JK
                    unsigned short ulLen, unsigned char  * pcUserKey) //JK
{
    unsigned short uiCryptLen = SECTOR_SIZE;


    while (TRUE)
        {
        for (loop = 0; loop < 4; loop++)        /* clear iv buffer */
            iv[loop] = 0;

        if (ulLen < SECTOR_SIZE)
            uiCryptLen = (unsigned int) ulLen;

        initcfb_idea((unsigned short  *) iv, pcUserKey, (boolean) ucCode_Code);
        ideacfb(pcDataBuffer, uiCryptLen);

        if (ulLen < SECTOR_SIZE)
            break;

        ulLen -= SECTOR_SIZE;
        pcDataBuffer += SECTOR_SIZE;    /* KR 04.07.94: cryptlen > 512 Byte */
        }

}

#include "idea.c"

/* EOF */
