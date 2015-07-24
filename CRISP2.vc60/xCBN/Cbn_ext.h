
/* =====[ CBN_EXT.H ]=====================================================

   Description:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ---------   --------------------------------------
      0.00  09.02.95  Peter Glen  Initial version.
      0.00  15.02.95  Peter Glen  Rewrite finished.

   ======================================================================= */

#ifndef ext_defined
#define ext_defined

/* -------- Protos ------------------------------------------------------- */

void    IDEA_Code(          unsigned char       code_code,
                            char        *buff,
                            unsigned    long len,
                            unsigned char       *pass
                 );

short    cb560_ext(         unsigned short    iOpCode,
                            unsigned short    iPortNr,
                            void    *pcInData,
                            void    *pcOutData,
                            unsigned short    iOutlen
                   );

short    cb560_tst(unsigned short iPortNr);

#ifndef DECLARE
    extern
#endif
    short   gl_iRetValue
#ifdef DECLARE
    = 0
#endif
    ;

#endif

/* EOF */

