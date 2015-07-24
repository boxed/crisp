
/* =====[ CBNSTU.C ]=========================================================

   Description:     Stupid function.

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
      0.00  09.02.95  Peter Glen    Still extracting modules.
      0.00  15.02.95  Peter Glen    Still extracting modules.

   ======================================================================= */

//;=========================================================================
//;
//; Name: _crack_the_hack   ---     tests the hacker with stupid code
//;
//; Input:          EAX     read value of printer port
//;
//; Aufruf:         call    _crack_the_hack ;perform hacker test
//;
//; output:         Carry   error flag  ( clear : ok  else error )
//;
//;=========================================================================

_crack_the_hack:                        ; NODEBUG


    push    ESI
    push    EDX
    push    EAX
    push    EDI

    push    EBP
    push    EDX

    add     EAX,    3fh
    xor     al, 12h
    cmp     EAX, 75h
    jg      crack_the_hack_04
    cmp     EAX, 21h
    jbe     crack_the_hack_06

//crack_the_hack_02:

    call    read_port
    add     ah, al
    inc     EDX
    call    read_port
    sar     al, cl

crack_the_hack_04:

    pop     EAX
    pop     EAX
    dec     EDX

crack_the_hack_06:

    call    read_port
    mov     EDX, EAX
    add     EDX,    EAX
    xor     EDX, 03fh
    jz      crack_the_hack_ret

//crack_the_hack_err:

    pop     EDI
    pop     EAX
    pop     EDX
    pop     ESI
    stc
    ret

crack_the_hack_ret:

    pop     EDI
    pop     EAX
    pop     EDX
    pop     ESI
    clc
    ret


//;=========================================================================
//;
//; Name:           _stupid  ---     tests the hacker with stupid code
//;
//;=========================================================================


_stupid:

    push    EDX
    push    ECX
    push    EBX
    push    EAX

    mov     al,ah                       ; create real opcode
    xor     al, SEC_BYTE                ;
    ;swapl                              ; swaps al

    push    ECX
    push    EDX
    xor     dx, dx
    mov     dh, ah                      ; backup ah
    mov     cx, 04h
    mov     ah, al                      ; copy ah to al
    shl     al, cl
    shr     ah, cl
    or      al, ah                      ; make the byte
    mov     ah, dh                      ; restore ah
    pop     EDX
    pop     ECX


    call    _put_stupid                 ; call the stupid put of one byte function

    mov     cx, D10                     ; wait to get data valid
    call    _delay_us                   ; delay 55 us
    mov     al,0cfh                     ; reset = 1, clk = 0

    ;out     dx,al                      ; set clk = lo, d5 = 0, d6 = 1
    call    write_port

    mov     cx, D10
    call    _delay_us                   ; delay 25 us to get box ready
    inc     dx

    ;in      al,dx                       ; read ACK
    call    read_port
    dec     dx
    mov     cx,2
    shl     al,cl                       ; set data bit to c flag
    rcl     ah,1                        ; set data bit to ah

    ;out     dx, al
    call    write_port

    pop     EAX
    pop     EBX
    pop     ECX
    pop     EDX

    ret     0

//;=========================================================================
//;
//; Name: _put_stupid       ---     Stupid write to box (same as _put_byte)
//;                            Should be called inside of Macro stupid
//; Input:          al      data
//;                 dx      Data port address of printer
//;
//;=========================================================================

_put_stupid:                            ; NODEBUG

    push    ECX
    push    EAX                          ; kr
    call    _pre_put_stupid
    or      al,90h                      ; reset = 1, clk = 1
    or      al,4fh                      ; d3,d2,d1,d0 = 1 , d6 = 1

    ;out     dx,al                      ; set clk = hi, set data bits hi
    call    write_port

    mov     cx, D10                     ; 10us
    call    _delay_us                   ; get data from ROM
    pop     EAX                          ; kr
    pop     ECX
    ret

//;=========================================================================

_pre_put_stupid:

    mov     ah,al                       ; ** put lo nibble ** & backup data
    and     al,0fh                      ; clear high byte of al, preserve lo byte
    or      al,0f0h                     ; reset = 1, clk = 1
    ;out     dx,al                      ; set clk hi 10 us, d5 = 1, d6 = 1
    call    write_port
    mov     cx, D10                     ; 10 us
    call    _delay_us                   ;

    and     al,0cfh                     ; reset = 1, clk = 0, d5 = 0
    ;out     dx,al                      ; set clk lo 10 us , preserve the rest
    call    write_port
    mov     cx, D10                     ; 10 us  ->4MHz
    call    _delay_us

    or      al,0b0h                     ; reset = 1, clk = 1, d5 = 1
    ;out     dx,al                      ; set clk hi for 10 us
    call    write_port
    mov     cx, D10                     ; 10 us
    call    _delay_us

    mov     al,ah                       ; ** put hi nibble **
    mov     cx,4
    shr     al,cl
    or      al,90h                      ; reset = 1, clk = 1
    ;out     dx,al                      ; set clk hi 10 us,
    call    write_port
    mov     cx, D10                     ; 10us
    call    _delay_us

    and     al,08fh                     ; reset = 1, clk = 0, d5 = 0, d6 = 0
    ;out     dx,al                      ; set clk lo 10 us , preserve the rest
    call    write_port
    mov     cx, D10                     ; set clk lo 10 us
    call    _delay_us
    ret

//;=========================================================================


