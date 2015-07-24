
/* =====[ _CBN_ASM.H ]=========================================================

   Description:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------  --------------------------------------
      0.00  01.02.95  Peter Glen  Initial version.
      0.00  12.02.95  Peter Glen  Moved to 2 files: cbn_asm.h _cbn_asm.h

   ======================================================================= */

#ifndef _cbnasm_defined
#define _cbnasm_defined

#ifdef  EXPORT
#define EXTERN
#else
#define EXTERN extern
#endif

//;  ษอออออออออออออออออออออออออออออออออออออออออป
//;  บ                                         บ
//;  บ OpCodes fr Interface Tabelle<->BOXI/O  บ
//;  บ                                         บ
//;  ศอออออออออออออออออออออออออออออออออออออออออผ

#define     GET_ID1_OPC         0xA9
#define     GET_ID2_OPC         0xB9
#define     GET_ID3_OPC         0x89
#define     GET_ID4_OPC         0x99
#define     GET_ID5_OPC         0x0E9
#define     GET_ID6_OPC         0x0F9
#define     GET_ID7_OPC         0x0C9
#define     GET_ID8_OPC         0x0D9
#define     GET_SERIAL_OPC      0x29
#define     READ_ALL_OPC        0x39
#define     ENCRYPT_OPC         0x09
#define     DECRYPT_OPC         0x19
#define     BOX_READY_OPC       0x69
#define     READ_RAM1_OPC       0x79
#define     READ_RAM2_OPC       0x49
#define     WRITE_RAM1_OPC      0x59
#define     WRITE_RAM2_OPC      0x0A8
#define     INC_RAM1_OPC        0x0B8
#define     DEC_RAM1_OPC        0x88
#define     INC_RAM2_OPC        0x98
#define     DEC_RAM2_OPC        0x0E8
#define     WR_ID_OFF_OPC       0x0F8
#define     WR_PW_R2_OPC        0x0C8
#define     WR_ID4_OPC          0x0D8
#define     WR_ID5_OPC          0x28
#define     WR_ID6_OPC          0x38
#define     WR_ID7_OPC          0x08
#define     WR_ID8_OPC          0x18
#define     UNI_CRYPT_OPC       0x68
#define     WR_NSERNR_OPC       0x78
#define     RD_UNLIMIT_OPC      0x48
#define     RD_MINLIM_OPC       0x58
#define     WR_USERLIM_OPC      0x0AF
#define     WR_MARXLIM_OPC      0x0BF

//;  ษอออออออออออออออออออออออออออออออออออออออออป
//;  บ                                         บ
//;  บ   OpCodes fr Interface User<->Tabelle  บ
//;  บ                                         บ
//;  ศอออออออออออออออออออออออออออออออออออออออออผ

#define     GET_ID1_VIR         0x01
#define     GET_ID2_VIR         0x02
#define     GET_ID3_VIR         0x03
#define     GET_ID4_VIR         0x04
#define     GET_ID5_VIR         0x05
#define     GET_ID6_VIR         0x06
#define     GET_ID7_VIR         0x07
#define     GET_ID8_VIR         0x08
#define     GET_SERIAL_VIR      0x10

#define     READ_ALL_VIR        0x55

#define     ENCRYPT_VIR         0x11
#define     DECRYPT_VIR         0x12

#define     BOX_READY_VIR       0x13

#define     READ_RAM1_VIR       0x14
#define     READ_RAM2_VIR       0x15
#define     WRITE_RAM1_VIR      0x16
#define     WRITE_RAM2_VIR      0x17
#define     INC_RAM1_VIR        0x18
#define     DEC_RAM1_VIR        0x19
#define     INC_RAM2_VIR        0x20
#define     DEC_RAM2_VIR        0x21
#define     WR_ID_OFF_VIR       0x56
#define     WR_PW_R2_VIR        0x23

#define     WR_ID4_VIR          0x24
#define     WR_ID5_VIR          0x25
#define     WR_ID6_VIR          0x26
#define     WR_ID7_VIR          0x27
#define     WR_ID8_VIR          0x28

#define     UNI_CRYPT_VIR       0x29

#define     WR_NSERNR_VIR       0x50
#define     RD_UNLIMIT_VIR      0x51
#define     RD_MINLIM_VIR       0x52
#define     WR_USERLIM_VIR      0x53
#define     WR_MARXLIM_VIR      0x54

//;  ษอออออออออออออออออออออออออออออออออออออออออป
//;  บ                                         บ
//;  บ            Fehler-Konstanten            บ
//;  บ                                         บ
//;  ศอออออออออออออออออออออออออออออออออออออออออผ

//;*****************************************************************************
//;* Error Codes unter 10H sind fr den Kunden zum Auswerten bestimmt
//;* Error Codes ber 10H zeigen die genaue Stelle des Fehlers
//;*****************************************************************************

#define     PARAM_ERR           0x01
#define     BOX_NOT_FOUND       0x02

#define     BOX_CRYPT_ERR       0x03
#define     UNI_CRYPT_ERR       0x04
#define     READ_RAM_ERR        0x05
#define     WRITE_RAM_ERR       0x06
#define     RAM_COUNT_ERR       0x07
#define     BOX_READY_ERR       0x08

#define     WR_SP_ACK_ERR       0x11
#define     WR_SERNO_ERR        0x12
#define     RD_UNLIMIT_ERR      0x13
#define     RD_MINLIM_ERR       0x14
#define     WR_UL_ERR           0x15
#define     WR_ML_ERR           0x16
#define     SYNCRO_ERR          0x17
#define     WRONG_OPC           0x1b

//;  ษอออออออออออออออออออออออออออออออออออออออออป
//;  บ                                         บ
//;  บ              Konstanten                 บ
//;  บ                                         บ
//;  ศอออออออออออออออออออออออออออออออออออออออออผ

#define     SEC_BYTE            0x0A3
#define     ID_OFFSET           0x9F
#define     ID_W_OFFSET         0x93

#define     REAL_RAM2_RD        0x0AE
#define     REAL_RAM2_WR        0x0B0
#define     REAL_DEC_2          0x0B4
#define     REAL_INC_2          0x0B3

//;***********************************************************************
//;
//; BIOS DATA and PRINTER BASE definitions:
//;
//;Pharlap: segment 34h maps first MB of memory
//;Pharlap: physical offset in first MB of memory
//;
//;***********************************************************************

#ifdef  _PHARLAP
#define     BIOS_DATA           0x0034
#define     PRINTER             0x0408
#endif

#ifdef  _NLM_BIOS
#define     BIOS_DATA           0x0400
#define     PRINTER             0x0008
#endif

//;
//; Give default values:
//;

#ifndef     BIOS_DATA
#define     BIOS_DATA           0x40
#define     PRINTER             0x8
#endif

//;  ษอออออออออออออออออออออออออออออออออออออออออป
//;  บ                                         บ
//;  บ             Delay-constants             บ
//;  บ                                         บ
//;  ศอออออออออออออออออออออออออออออออออออออออออผ

#define     D10000              23800       //        ;10000 x 2.38 = 23800
#define     D1800               4284        //       ; 1800 x 2.38 = 4284
#define     D1500               3570        //       ; 1500 x 2.38 = 3570
#define     D1200               2856        //       ; 1200 x 2.38 = 2856
#define     D1100               2618        //       ; 1100 x 2.38 = 2618
#define     D1000               2380        //       ; 1000 x 2.38 = 2380
#define     D900                2142        //       ; 900 x 2.38 = 2142
#define     D800                1904        //       ; 800 x 2.38 = 1904
#define     D700                1666        //       ; 700 x 2.38 = 1666
#define     D600                1428        //       ; 600 x 2.38 = 1428
#define     D500                1190        //       ; 500 x 2.38 = 1190
#define     D400                952         //       ; 400 x 2.38 = 952
#define     D350                833         //       ; 350 x 2.38 = 833
#define     D300                714         //       ; 300 x 2.38 = 714
#define     D250                595         //       ; 250 x 2.38 = 595
#define     D200                476         //       ; 200 x 2.38 = 476
#define     D150                357         //       ; 150 x 2.38 = 357
#define     D100                238         //       ; 100 x 2.38 = 238
#define     D65                 155         //
#define     D55                 131         //
#define     D50                 119         //
#define     D45                 108         //
#define     D35                 84          //
#define     D25                 60          //
#define     D10                 24          //

//;****************************************************************************

#undef  EXTERN
#undef  EXPORT

#endif

