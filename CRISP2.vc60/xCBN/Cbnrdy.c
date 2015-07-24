
/* =====[ CBNRDY.C ]=========================================================

   Description:     Ready function.

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

   ======================================================================= */

//;*****************************************************************************
//;
//; Name: _box_ready      ---   quick check of box
//;
//; Input:        ah        box ready command
//;               EDX       printer port number
//;
//; Output:       EDI       data buffer
//;               EBX       error flag  ( 0 : ok  else error )
//;
//;*****************************************************************************

_box_ready:

    mov     al, ah                      ; create real opcode
    xor     al, SEC_BYTE

    ;swapl                              ; swaps al
    push    ECX
    push    EDX

    xor     dx, dx
    mov     dh, ah                      ; backup ah
    mov     ECX, 04h
    mov     ah, al                      ; copy ah to al
    shl     al, cl
    shr     ah, cl
    or      al, ah                      ; make the byte
    mov     ah, dh                      ; restore ah

    pop     EDX
    pop     ECX

    push    ECX                         ; save ECX
    push    EDX                         ; save originnal port number

    ;mov     ECX, 1                     ; test only one port
    ;jmp     box_ready_one              ; go on testing

    cmp     EDX, 4                      ; port number = 11, 12 or 13 ?
    jb      box_ready_all               ; port number = 1, 2 or 3, test all
    sub     EDX, 10                     ; create the right port number
    mov     ECX, 1                      ; test only one port
    jmp     box_ready_one               ; go on testing

box_ready_all:

    mov     ECX, 3                      ; maximal 3 ports

box_ready_one:

    push    EDX                         ; save original port number

box_ready_10:

    push    EAX                         ; save real original opcode

    ;-------------------------------
    push    EBX                         ; save EBX

    mov     EBX, EDX                    ; port number into EBX
    dec     EBX
    shl     EBX, 2                      ; adjust for dword
    add     EBX, offset LPT_SRC         ; offset for printer ports
    mov     EDX, [EBX]

    pop     EBX                         ; restore EBX
    ;-------------------------------

    cmp     EDX, PORT1                  ; it's valid port 1?
    je      box_ready_20
    cmp     EDX, PORT2                  ; it's valid port 2?
    je      box_ready_20
    cmp     EDX, PORT3                  ; it's valid port 3?
    jne     box_ready_22                ; no valid port, try next

box_ready_20:

    call    _bios_port_test             ; tests port like BIOS does
    jc      box_ready_22                ; port doesn't exist, try next one

#ifdef  STUPID
    call    _stupid                     ; perform the stupid macro, NODEBUG
#endif

//;***********************************************************************

    call    _box_reset                   ; reset box
    call    _send_cmd                   ; send box_ready command

    mov     al, 0CBH                    ; get box_ready password
    call    _put_byte                   ; send 'CB'

    push    ECX                         ; save loop counter
    mov     EAX, 400                    ; mEAX wait time = 4 ms
    mov     EBX, 1                      ; get EBX=1 bytes (block length)
    mov     ECX, 1                      ; get 1 time (1 block)
    call    _get_nData
    pop     ECX                         ; restore loop counter
    cmp     EBX, 0                      ; EBX == 0 ?

    jne     box_ready_22
    jmp     box_ready_ret               ; yes, return without error

box_ready_22:

    pop     EAX                         ; restore real original opcode
    pop     EDX                         ; restore last port number

    cmp     EDX, 2                      ; was it port1 or port2 ?
    jbe     box_ready_24
    mov     EDX, 1                      ; it was port 3, try port 1 now
    jmp     box_ready_26

box_ready_24:

    add      EDX,    1                  ; it was port 1 or 2, try next port

box_ready_26:

    push    EDX                         ; save new port number

    ;krloop  box_ready_10               ; try next logical port

    dec     cx
    cmp     cx,0
    jbe     done_11
    jmp     box_ready_10

done_11:


box_ready_err1:

    pop     EDX                         ; restore last port number
    pop     EDX                         ; restore original port number

    cmp     EDX, 4                      ; port number = 11, 12 or 13 ?
    jb      box_search_all              ; port number = 1, 2 or 3, test all
    jmp     box_ready_end               ; end of box_ready

box_search_all:

    push    EDX                         ; save original port number
    mov     ECX, 3                      ; perform mEAX. 3 loops

box_search_p10:

    pop     EDX
    cmp     EDX, 1                      ; was it port 1 ?
    je      box_search_p11
    cmp     EDX, 2                      ; was it port 2 ?
    je      box_search_p12
    mov     EDX, 1                      ; it was port 3, prepare port 1 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT3
    jmp     box_search_p20

box_search_p11:

    add     EDX,    1                   ; it was port 1, prepare port 2 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT2
    jmp     box_search_p20

box_search_p12:

    add     EDX,    1                   ; it was port 2, prepare port 3 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT1

box_search_p20:                         ; test port if box is there

    push    ECX                         ; save loop count  (init.3)
    push    EAX                         ; save real original opcode

    call    _bios_port_test             ; tests port like BIOS does
    jc      box_search_p22              ; port doesn't exist, try next one

#ifdef  STUPID
    call    _stupid                      ; perform the stupid macro, NODEBUG
#endif

    call    _box_reset                  ; reset box
    call    _send_cmd                   ; send box_ready command

    mov     al, 0CBH                    ; get box_ready password
    call    _put_byte                   ; send 'CB'
    push    ECX                         ; save loop counter
    mov     EAX, 400                    ; mEAX wait time = 4 ms
    mov     EBX, 1                      ; get EBX=1 bytes (block length)
    mov     ECX, 1                      ; get 1 time (1 block)
    call    _get_nData
    pop     ECX                         ; restore loop counter
    cmp     EBX, 0                      ; EBX == 0 ?
    je      box_search_ret              ; no error ok

box_search_p22:

    pop     EAX                         ;        restore real opcode
    pop     ECX                         ;        restore loop count (init.3)
    ;krloop  box_search_p10             ; try next address
    loop    box_search_p10              ; try next address

box_search_err:

    pop     EDX                         ; restore original port number

    pop     ECX                         ; restore ECX
    mov     EBX, BOX_READY_ERR          ; no box was found
    ret

box_search_ret:

    pop     EAX                         ; restore real original opcode
    pop     ECX                         ; restore loop count (init.3)
    pop     ECX                         ; dummy restore to reach ECX (EDX)

    pop     ECX                         ; restore ECX
    xor     EBX, EBX                    ; no error
    ret

//;*****************************************************************************
//;*  exits in case of box_search wasn't performed
//;*  (port number = 11, 12 or 13 or box was found uESIng port = 1, 2 or 3)
//;*****************************************************************************

box_ready_end:

    pop     ECX                         ; restore ECX
    mov     EBX, BOX_READY_ERR          ; no box was found
    ret

box_ready_ret:

    pop     EAX                         ; restore real original opcode
    pop     EDX                         ; restore last port number
    pop     EDX                         ; restore original port number
    pop     ECX                         ; restore ECX
    xor     EBX, EBX                    ; no error
    ret


/* EOF */
