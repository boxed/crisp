
/* =====[ xxx.c ]=========================================================

   Description:

   Compiled:        MS-VC.

   Compiler opt:    See makefile.

   Revisions:

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  mm/dd/95  Peter Glen    Initial version.

   ======================================================================= */

/* -------- System includes:  -------------------------------------------- */

#define __export
#define __huge

#include    <windows.h>
#include    <winioctl.h>


/* -------- Includes:  --------------------------------------------------- */


#include "marxdev.h"

/* -------- Defines: ----------------------------------------------------- */


/* -------- Macros: ------------------------------------------------------ */


/* -------- Forward references for procedures: --------------------------- */

void    main(int argc,char *argv[]);

/* -------- Declarations: ------------------------------------------------ */


/* -------- Definitions: ------------------------------------------------- */


/* -------- Data: -------------------------------------------------------- */


/* -------- Strings: ----------------------------------------------------- */


/* -------- Implementation: ---------------------------------------------- */


void    main(int argc,char *argv[])

{

    printf("IOCTL_GPD_READ_PORT_UCHAR   EQU  %u\n", IOCTL_GPD_READ_PORT_UCHAR );
    printf("IOCTL_GPD_READ_PORT_USHORT  EQU  %u\n", IOCTL_GPD_READ_PORT_USHORT );
    printf("IOCTL_GPD_READ_PORT_ULONG   EQU  %u\n", IOCTL_GPD_READ_PORT_ULONG );
    printf("IOCTL_GPD_WRITE_PORT_UCHAR  EQU  %u\n", IOCTL_GPD_WRITE_PORT_UCHAR );
    printf("IOCTL_GPD_WRITE_PORT_USHORT EQU  %u\n", IOCTL_GPD_WRITE_PORT_USHORT );
    printf("IOCTL_GPD_WRITE_PORT_ULONG  EQU  %u\n", IOCTL_GPD_WRITE_PORT_ULONG );
    printf("MARX_IOCTL_ID               EQU  %u\n", MARX_IOCTL_ID );
    printf("MARX_IOCTL_RAM              EQU  %u\n", MARX_IOCTL_RAM );
    printf("MARX_IOCTL_CRYPT            EQU  %u\n", MARX_IOCTL_CRYPT );
    printf("MARX_IOCTL_TEST             EQU  %u\n", MARX_IOCTL_TEST );

}

/* EOF */
