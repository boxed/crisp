
/* =====[ MYDEFINE.H ]=========================================================

   Description:     Header file for own use.

   Revisions:

      REV     DATE     BY    DESCRIPTION
      ----  --------  -----  --------------------------------------
      0.00  05.09.91  Peter  Initial version.
      0.00  12.10.92  Peter  Header formatted.
      0.00  28.05.93  Peter  BOOL to unsigned character.
      0.00  24.08.93  Peter  Corrected for compatibility with WINDOWS.H
      0.00  01.01.95  Peter  Corrected for compatibility with WINDOWS NT

   ======================================================================= */

/*  ---------- Defines for own use: -------------------------------------- */

#ifndef     mydefine_defined
#define     mydefine_defined

/*
 * Some boolean constants
 */

#ifndef     TRUE
#define     TRUE    1
#define     FALSE   0
#endif

/*
 * Some bit defines
 */

#define     BIT_0   1
#define     BIT_1   2
#define     BIT_2   4
#define     BIT_3   8
#define     BIT_4   16
#define     BIT_5   32
#define     BIT_6   64
#define     BIT_7   128

#define     BIT_8   256
#define     BIT_9   512
#define     BIT_10  1024
#define     BIT_11  2048
#define     BIT_12  4096
#define     BIT_13  8192
#define     BIT_14  16384
#define     BIT_15  32768

#define     SOUND_CLOCK 1193180                          /* system defines */
                                                   /* some file attributes */
#define     R_ATTR  1                                         /* read only */
#define     H_ATTR  2                                            /* hidden */
#define     S_ATTR  4                                            /* system */
#define     V_ATTR  8                                      /* volume label */
#define     D_ATTR  0x10                                      /* directory */
#define     A_ATTR  0x20                                        /* archive */

#define     ALL_ATTR  R_ATTR | A_ATTR

#define     ROOT_DIR    "\\"                           /* some directories */
#define     CURRENT_DIR "."
#define     PARENT_DIR  ".."
#define     MAX_DIR     65                     /* maximum path name length */
#define     MAX_LEVEL   32                     /* maximum dir dept nesting */

#ifdef      _MAX_PATH                      /* include only if we have FILE */
typedef struct                                          /* path components */
    {
    char    path[_MAX_PATH];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    fname[_MAX_FNAME];
    char    ext[_MAX_EXT];
    char    dummy[_MAX_FNAME];
    } PATH;
#endif

#define     BW                  07               /* some screen attributes */
#define     WB                  0x70
#define     BWH                 15
#define     DISP_NORMAL         0x07
#define     DISP_REVERSE        0x70
#define     DISP_UNDERLINE      0x01
#define     DISP_NONDISPLAY     0x00
#define     DISP_INTENSITY      0x08    /* These bits should be OR - ed in */
#define     DISP_BLINK          0x80

/*
 * Keys and characters. Defined as integers.
 *
 */

#define     BEL                 0x07
#define     BS                  0x08
#define     TAB                 0x09
#define     LF                  0x0A
#define     VT                  0x0B
#define     FF                  0x0C
#define     CR                  0x0D
#define     END_OF_FILE         0x1A
#define     ESC                 0x1B
#define     SPACE               0x20

#define     F1                  0x3b00
#define     F2                  0x3c00
#define     F3                  0x3d00
#define     F4                  0x3e00
#define     F5                  0x3f00
#define     F6                  0x4000
#define     F7                  0x4100
#define     F8                  0x4200
#define     F9                  0x4300
#define     F10                 0x4400
#define     F11                 0x8500
#define     F12                 0x8600

#define     RIGHT_ARROW         0x4d00
#define     LEFT_ARROW          0x4b00
#define     UP_ARROW            0x4800
#define     DOWN_ARROW          0x5000

#define     HOME                0x4700
#define     BEGIN               0x4700
#define     END                 0x4f00
#define     PGUP                0x4900
#define     PGDOWN              0x5100
#define     INS                 0x5200
#define     DEL                 0x5300
#define     NUMLOCK             0x4500
#define     SCRLOCK             0x4600
#define     GREYMINUS           0x4a00
#define     GREYPLUS            0x4e00

#define     ALT_Q               0x1000
#define     ALT_W               0x1100
#define     ALT_E               0x1200
#define     ALT_R               0x1300
#define     ALT_T               0x1400
#define     ALT_Y               0x1500
#define     ALT_U               0x1600
#define     ALT_I               0x1700
#define     ALT_O               0x1800
#define     ALT_P               0x1900

#define     ALT_A               0x1E00
#define     ALT_S               0x1F00
#define     ALT_D               0x2000
#define     ALT_F               0x2100
#define     ALT_G               0x2200
#define     ALT_H               0x2300
#define     ALT_J               0x2400
#define     ALT_K               0x2500
#define     ALT_L               0x2600

#define     ALT_Z               0x2C00
#define     ALT_X               0x2D00
#define     ALT_C               0x2E00
#define     ALT_V               0x2F00
#define     ALT_B               0x3000
#define     ALT_N               0x3100
#define     ALT_M               0x3200

#define     ALT_F1              0x6800
#define     ALT_F2              0x6900
#define     ALT_F3              0x6A00
#define     ALT_F4              0x6B00
#define     ALT_F5              0x6C00
#define     ALT_F6              0x6D00
#define     ALT_F7              0x6E00
#define     ALT_F8              0x6F00
#define     ALT_F9              0x7000
#define     ALT_F10             0x7100

#define     ALT_SPACE           0x2000

/*
 * Useful types. Mostly defined by every one.
 * Soft define things:
 */

#ifndef uchar
typedef unsigned char   uchar;
#endif

#ifndef uint
typedef unsigned int    uint;
#endif

#ifndef ulong
typedef unsigned long   ulong;
#endif

#ifndef WORD
typedef unsigned short  WORD;
#endif

#ifndef BYTE
typedef unsigned char   BYTE;
#endif

#ifndef UCHAR
typedef unsigned char   UCHAR;
#endif

#ifndef USHORT
typedef unsigned int    USHORT;
#endif

#ifndef UINT
typedef unsigned int    UINT;
#endif

#ifndef LONG
typedef long   LONG;
#endif

#ifndef ULONG
typedef unsigned long   ULONG;
#endif

#ifndef DWORD
typedef unsigned long   DWORD;
#endif

#ifndef BOOL
typedef unsigned int    BOOL;
#endif

/*
 * Not so common useful types. Mostly defined by every one.
 */

typedef char            byte_1;
typedef unsigned char   ubyte_1;

typedef short           byte_2;
typedef unsigned short  ubyte_2;
typedef unsigned short  ushort;

typedef long            byte_4;
typedef unsigned long   ubyte_4;

typedef unsigned        bit;                                   /* bitfield */
typedef int             logical;                       /* logical/boolean  */

/*
 * Pointer to function types.
 */

typedef void            (*PFV)();    /* ptr to a funct ret  void           */
typedef char            (*PFC)();    /* ptr to a funct ret  char           */
typedef unsigned char   (*PFUC)();   /* ptr to a funct ret  unsigned char  */
typedef int             (*PFI)();    /* ptr to a funct ret  int            */
typedef unsigned int    (*PFUI)();   /* ptr to a funct ret  unsigned int   */
typedef long            (*PFL)();    /* ptr to a funct ret  long           */
typedef unsigned long   (*PFUL)();   /* ptr to a funct ret  unsigned long  */
typedef float           (*PFF)();    /* ptr to a funct ret  float          */
typedef double          (*PFD)();    /* ptr to a funct ret  double         */
typedef char            (*PFB1)();   /* ptr to a funct ret  char           */
typedef unsigned char   (*PFUB1)();  /* ptr to a funct ret  unsigned char  */
typedef short           (*PFB2)();   /* ptr to a funct ret  short          */
typedef unsigned short  (*PFUB2)();  /* ptr to a funct ret  unsigned short */
typedef long            (*PFB4)();   /* ptr to a funct ret  long           */
typedef unsigned long   (*PFUB4)();  /* ptr to a funct ret  unsigned long  */

typedef char           *(*PFPC)();   /* ptr to a funct ret  ptr to char    */
typedef int            *(*PFPI)();   /* ptr to a funct ret  ptr to int     */

/*
 * Pointer to pointer to functions.
 *
 */

typedef PFI             (*PPFI)();
typedef PFC             (*PPFC)();
typedef PFUC            (*PPFUC)();
typedef PFUB4           (*PPFUB4)();
typedef PFV             (*PPFV)();

#endif




