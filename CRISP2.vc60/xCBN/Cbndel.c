
/* =====[ CBNRDY.C ]=========================================================

   Description:     Delay function.

   Compiled:        MS-VC.

   Compiler opt:    ??

   Note:            This file has been extracted from BOXIO.

                    If you opposed to it:

                        type in the main file ALT-R cbrdy.c
                        this will read this include file to the main file,
                        so everything is as before.

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  xx.xx.94  Peter Glen    Initial version.
      0.00  07.02.95  Peter Glen    Extracting modules.
      0.00  08.02.95  Peter Glen    Extracting modules.

   ======================================================================= */

//;***********************************************************************
//;
//; _delay_us:
//;
//; Delay a given time
//;
//; Input:  cx      Delay interval (micro seconds)
//;
//;  Note:          This is a real mode code, not an emulation, port
//;                 accessed directlty.
//;
//;***********************************************************************

delay_it:
                pushf

                cmp     ECX,100         ; we comESIder delay longer as 100 us
                ja      no_lock         ; unlocked.

                call    disable
no_lock:
                push    EAX
                push    EBX
                push    EDX

                mov     EDX,43h
                                        ; get initial timer value to EAX
                mov     al,06H          ; -+ ** get timer count **
                out     43H,al          ;  | Latch timer counter
                in      al,40H          ;  | Read LSB of timer counter
                mov     ah,al           ;  |
                in      al,40H          ;  | Read MSB of timer counter
                xchg    ah,al           ; -+
                mov     EDX,EAX         ; backup old timer to EDX
                                        ; get new timer value to EAX
du_10:
                mov     al,06H          ; -+ ** get timer count **
                out     43H,al          ;  | Latch timer counter
                in      al,40H          ;  | Read LSB of timer counter
                mov     ah,al           ;  |
                in      al,40H          ;  | Read MSB of timer counter
                xchg    ah,al           ; -+

                mov     EBX,EDX
                sub     EBX,EAX         ; EBX = initial value - new value
                cmp     EBX,ECX         ; delay interval(EBX) < ECX ?
                jb      du_10           ; if Yes ->timer is'nt over


                pop     EDX             ; No, timer is over
                pop     EBX
                pop     EAX
                popf
                ret

//;***********************************************************************

