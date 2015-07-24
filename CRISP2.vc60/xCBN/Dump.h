
/* =====[ DUMP.H ]=========================================================

   Description:

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
      0.00  07/05/95  Peter Glen    Wsprintf suchs, changed to sprintf
      0.00  07/08/95  Peter Glen    Comment added, cleaned.

   ======================================================================= */

#ifndef dump_defined
#define dump_defined

#ifdef  EXPORT
#define EXTERN
#else
#define EXTERN extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -------- Defines: ----------------------------------------------------- */

#define DDMP  DebugDump         // for those of us lasy to type
#define DPRT  DebugPrintf       // for those of us lasy to type

/* -------- Protos ------------------------------------------------------- */

int     DebugDump(void FAR *dptr, int len, char FAR *str);
int     DebugPrintf(char *fmt, ...);
//int     DebugHeap();


#ifdef __cplusplus
}
#endif


#undef  EXTERN
#undef  EXPORT

#endif

/* EOF */


