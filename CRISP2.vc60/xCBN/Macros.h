
/* =====[ MACROS.H ]=======================================================

   Description:     Predefined macros for work.

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  04.02.92  Peter Glen    Header constructed.
      0.00  06.08.93  Peter Glen    Comment restucture, conv. to upper case
      0.00  06.08.93  Peter Glen    Safe free and close added

   ======================================================================= */

#ifndef   macros_defined
#define   macros_defined

/* -------- LOOPS -------------------------------------------------------- */

#define    Repeat(var_1,cycle)    {                                         \
                                  unsigned int var_1;                       \
                                  for (var_1 = 0; var_1 < cycle; var_1++)   \
                                       {                                    \


#define    Repeat_end                  }                                    \
                                  }


#define FOREVER      {while (TRUE)                                          \
                            {


#define ENDFOREVER          }                                               \
                        }

/* -------- COUNTS: ------------------------------------------------------ */

#define    DEC_TILL_NULL(var_1)   {if (var_1)                               \
                                       {                                    \
                                       var_1--;                             \
                                       }                                    \
                                   }

/* -------- DEBUGS: ------------------------------------------------------ */

#define    PRINT_ADDRESS(a)    {printf ("Address: %lp\n",&a);}              \

#define    PRINT_VAR(a)        {printf ("Address: %lp\n",&a);               \
                                printf ("Val    : %lX\n",a);                \
                                printf ("*Val   : %lX\n",*a);               \
                               }

#define    PRINT_PTR(a)        {printf ("Segment: %X\n",FP_SEG(a));         \
                                printf ("Offset : %X\n",FP_OFF(a));         \
                               }

/* -------- INTERRUPTS: -------------------------------------------------- */

#define     ENABLE      _asm { sti }

#define     DISABLE     _asm { cli }

                                                  /* send end of interrupt */
#define EOI         _asm                                                    \
                        {                                                   \
                        mov al,20h                                          \
                        out 20h,al                                          \
                        }

/* -------- MAKE FAR POINTRES: ------------------------------------------- */

/*  Schon in dos.h!!!!!!!!
#define MK_FP(seg,offset)                                                   \
    ((void _far *)(((unsigned long)(seg)<<16) | (unsigned)(offset)))
*/
#define MK_LONG(ptr)                                                        \
    ((unsigned long) (((unsigned long)FP_SEG(ptr) << 4) + FP_OFF(ptr))  )

#define MK_FP_LONG(ptr)                                                     \
    MK_FP((unsigned int)((ptr) >> 4), (unsigned int)((ptr) & 0xf))


#define MK_FP_LONG_OPT(ptr)                                                 \
    MK_FP( (unsigned int)(((ptr) - 0x8000) >> 4),                           \
                (unsigned int)(0x8000 + ((ptr) & 0xf)) )

#define PARAGRAPHS(x)   ((FP_OFF(x) + 15) >> 4)

/* -------- RESOURCES ---------------------------------------------------- */

#define SAFE_FREE(buffer)                                                   \
        if(buffer)                                                          \
            free(buffer);                                                   \
            buffer = NULL;

#define SAFE_FCLOSE(fp)                                                     \
        if(fp)                                                              \
            fclose(fp);                                                     \
            fp = NULL;

/*
 * MAKE_PTR(type, name)                                                     \
 *
 * Description:  Make a pointer and initialize it to NULL.
 *
 * Input:      type     : type of pointer
 *             name     : variable name
 *
 * Example:    MAKE_PTR(FILE, fp)                                           \
 *
 */

#define MAKE_PTR(type, name)                                                \
        type  *name = NULL;


/* -------- SWAPS: ------------------------------------------------------- */

#define     SWAP_BYTES(numb)    ((numb << 8) | (numb >> 8))

#define     SWAP_WORDS(numb)    ((numb << 16) | (numb >> 16))


/* -------- ROTATES: ----------------------------------------------------- */

#define     ROTATE_LONG_LEFT(x, n)  (((x) << (n))  | ((x) >> (32 - (n))))
#define     ROTATE_LONG_RIGHT(x, n) (((x) >> (n))  | ((x) << (32 - (n))))

#define     ROTATE_INT_LEFT(x, n)  (((x) << (n))  | ((x) >> (16 - (n))))
#define     ROTATE_INT_RIGHT(x, n) (((x) >> (n))  | ((x) << (16 - (n))))

#define     ROTATE_CHAR_LEFT(x, n)  (((x) << (n))  | ((x) >> (8 - (n))))
#define     ROTATE_CHAR_RIGHT(x, n) (((x) >> (n))  | ((x) << (8 - (n))))

#else
#endif

