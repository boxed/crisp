
/* =====[ CBN_ASM.H ]=========================================================

   Description:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------  --------------------------------------
      0.00  01.02.95  Peter Glen  Initial version.
      0.00  12.02.95  Peter Glen  Moved to 2 files: cbn_asm.h _cbn_asm.h

   ======================================================================= */

#ifndef cbnasm_defined
#define cbnasm_defined

#ifdef  EXPORT
#define EXTERN
#else
#define EXTERN extern
#endif


/* -------- Defines: ----------------------------------------------------- */

#define NUM_PORTS 3

/* -------- Protos ------------------------------------------------------- */

EXTERN short    CB560(      unsigned long    iOpCode,
                            unsigned long    iPortNr,
                            unsigned char    *pcInData,
                            unsigned char    *pcOutData
                      );

EXTERN int  LPT_SRC[NUM_PORTS];
EXTERN int  LPT_REF[NUM_PORTS];

EXTERN int    PORT1;
EXTERN int    PORT2;
EXTERN int    PORT3;


#undef  EXTERN
#undef  EXPORT

#endif


