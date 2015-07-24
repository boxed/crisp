
/* =====[ INTERFCE.H ]=========================================================

   Description:     Internal interface for IDEA and assemler and
                    pointerless.

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  mm/dd/95  Peter Glen    Initial version.

   ======================================================================= */

#ifndef interfce_defined
#define interfce_defined

#ifdef  EXPORT
#define EXTERN
#else
#define EXTERN extern
#endif

#define VERSION_32 202


// Types to handle the input and output:

// typedef struct
//     {
//     unsigned long      iOpCode;               // input code
//     unsigned long      iPortNr;               // port number
//     unsigned char      input[12];             // input pass and all params
//     unsigned long      ret_val;               // output ret_val
//     unsigned long      len;                   // output length (not used)
//     unsigned char      firstbyte[2];          // output data (used as pointer)
//     //  The structure can be arbitrary long, according to the function used
//     } MARX_I_O;


// Protos to call back from _asm:


EXTERN short  CB560(
                        unsigned long iOpCode,
                        unsigned long iPortNr,
                        unsigned char FAR *pcInData,
                        unsigned char FAR *pcOutData
                 );

EXTERN void  IDEA_Code(
                        unsigned char code_code,
                        char FAR *buff,
                        unsigned short len,
                        unsigned char FAR *pass
                      );


EXTERN short  cb560_ext( unsigned short    iOpCode,
                         unsigned  short    iPortNr,
                         void    FAR *pcInData,
                         void    FAR *pcOutData,
                         unsigned  short    iOutlen

                     );


EXTERN short    cb560_nt (     unsigned short     iOpCode,
                        unsigned short     iPortNr,
                        void               *pcInData,
                        void               *pcOutData,
                        unsigned short     iOutlen
                   );

EXTERN short    cb560_95 (     unsigned short     iOpCode,
                        unsigned short     iPortNr,
                        void               *pcInData,
                        void               *pcOutData,
                        unsigned short     iOutlen
                   );

/*
 * Global for storing error codes and stuff:
 */

EXTERN short    gl_iRetValue;
EXTERN short    gl_version;
EXTERN short    gl_instance;
EXTERN short    gl_ext_ret_val;
EXTERN char     name1[];

#undef  EXTERN
#undef  EXPORT

#endif

/* EOF */
