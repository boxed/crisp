
/* =====[ CBNRES.C ]=========================================================

   Description:     Reset function.

   Compiled:        MS-VC.

   Compiler opt:    ??

   Note:            This file has been extracted from BOXIO.

                    If you opposed to it:

                        type in the main file ALT-R cbrres.c
                        this will read this include file to the main file,
                        so everything is as before.

   Revisions:

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  07.02.95  Peter Glen    Extracting modules.

   ======================================================================= */

//;=========================================================================
//;
//; Name  : _box_reset      ---     resets the box before any action
//;
//; Input:          EDX     port address given by host routine
//;
//;=========================================================================

_box_reset:

    pushf
    //cli                               ; do not allow interrupt requests
    call    disable
    push    EAX
    push    EBX
    push    ECX
    push    EDX                         ; save printer port address

#ifdef  HACK
    call    _crack_the_hack             ;perform hacker test, NODEBUG
#endif

    add     EDX,2                       ; EDX =  printer control port
    call    read_port                   ;
    or      al,4                        ; turn power on, INIT = 1
    and     al,17H                      ; turn power on, SLT IN = 1
    call    write_port
    sub     EDX,2                       ; EDX =  printer data port

    mov     al,0bfh                     ; set clk = 1, set reset = 1
    call    write_port                  ; d5 = 1, d6 = 0
    mov     ECX, D25                    ; changed from 100 to 25 KR 16.05.94
    call    _delay_us                   ; delay 25us

                                        ; perform reset ( reset = lo )
    mov     al,7fh                      ; set clk = 1, set reset = 0
    call    write_port                  ; d5 = 1, d6 = 1
    mov     ECX, D50                    ; changed from 100 to 50 KR 16.05.94
    call    _delay_us                   ; delay 50us

    mov     al,0bfh                     ; set clk = 1, set reset = 1
    call    write_port                  ; d5 = 1, d6 = 0

    mov     ECX, 10                     ; about 1ms  16.05.94 WO & KR

delay1:

    push    ECX                         ; 2MHz: 512 us min; ->4MHz
    mov     ECX, D100                   ; delay 100us
    call    _delay_us                   ;
    pop     ECX
    loop    delay1

    pop     EDX                         ; restore printer port address
    pop     ECX
    pop     EBX
    pop     EAX
    popf
    ret

//;=========================================================================

