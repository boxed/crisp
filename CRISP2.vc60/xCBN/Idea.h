
/* =====[ IDEA.H ]=========================================================

   Description:

   Notes:   To use it there are two metods: as definition   (EXPORT)
                                            as prototyping. (IMPORT)

            All is controlled by defining EXPORT. (or not)
            If EXPORT is defined, we define and initialize variables,
            else we supply prototypes.

            If we want to initialize, it is done like:

            int this variable
            #ifdef EXPORT
            = FALSE
            #endif
            ;               !!!!  note the semicolon !!!!

            EXPORT is undefined by this process, for the next include file
            it should be stated again.

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------  --------------------------------------
      0.00  31.01.95  Peter Glen  Documented.
      0.00  10.02.95  Peter Glen  Dropped into windows NT.

   ======================================================================= */

#ifndef  IDEA_DEFINED
#define  IDEA_DEFINED

#ifdef  EXPORT
#define _EXTERN
#else
#define _EXTERN extern
#endif

/* -------- Defines: ----------------------------------------------------- */

typedef unsigned char boolean;                 /* values are TRUE or FALSE */
typedef unsigned char byte;                            /* values are 0-255 */
typedef byte *byteptr;                                  /* pointer to byte */
typedef char *string;                 /* pointer to ASCII character string */
typedef unsigned short word16;                       /* values are 0-65535 */
typedef unsigned long word32;                   /* values are 0-4294967295 */

#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#ifndef min
#define min(a,b) ( (a)<(b) ? (a) : (b) )
#define max(a,b) ( (a)>(b) ? (a) : (b) )
#endif

#define REG register

#define IDEAKEYSIZE 16
#define IDEABLOCKSIZE 8

void initkey_idea(char  key[16], boolean decryp);                    // JK
void idea_ecb(short  * inbuf, short  * outbuf);                   // JK
void initcfb_idea(word16  iv0[4], char  key[16], boolean decryp); // JK
void ideacfb(char  * buf, int count);             // JK Buffer aus Anwendung
void close_idea(void);

#undef  _EXTERN
#undef  EXPORT

#endif


/* --------- END OF IDEA.H ----------------------------------------------- */
