
/* =====[ CBN_PROG.H ]=========================================================

   Description:     Placeholder for PROG prototyps.

   Notes:   To use it there are two metods: as definition   (EXPORT)
                                            as prototyping. (IMPORT)

            If EXPORT is defined, we define and initialize variables,
            otherwise we supply prototypes.

            If we want to initialize, it is done like:

            int this variable
            #ifdef EXPORT
            = FALSE
            #endif
            ;               !!!!  note the semicolon !!!!

            EXPORT is undefined by this process, for the next include file
            it should be stated again.


      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  mm/dd/95  Peter Glen    Initial version.

   ======================================================================= */

#ifndef cbn_prog_defined
#define cbn_prog_defined

#ifdef  EXPORT
#define EXTERN
#else
#define EXTERN extern
#endif

/* -------- Defines: ----------------------------------------------------- */


/* -------- Macros: ------------------------------------------------------ */


/* -------- Protos ------------------------------------------------------- */


/* -------- Declarations: ------------------------------------------------ */


/* -------- Definitions: ------------------------------------------------- */

#undef  EXTERN
#undef  EXPORT

#endif

/* EOF */
