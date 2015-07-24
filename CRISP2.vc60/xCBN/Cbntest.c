
/* =====[ CBNTEST.C ]=========================================================

   Description:     Test ID function.

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

   ======================================================================= */

//;=========================================================================
//;
//; Name: _id_test_port     ---     reads BIOS port number and tests box
//;
//; Input:          ah      read command
//;                 EDX     port number
//;                 bh      put ? byte ( id )
//;                 bl      get ? byte ( data )
//;                 ESI     box id buffer
//;
//; Output:         EDI     box data buffer
//;                 C       Carry set if error, Carry clear if ok
//;                 EDX     if ok portaddress, if error portnumber
//;
//;=========================================================================

_id_test_port:

    push    ECX
    push    EDX                         ; save original port number

    cmp     EDX, 4                      ; port number = 11, 12 or 13 ?
    jb      id_test_all                 ; port number = 1, 2 or 3, test all

    sub     EDX, 10                     ; create the right port number
    mov     ECX, 1                      ; test only one port
    jmp     id_test_one                 ; go on testing

id_test_all:

    mov     ECX, 3                      ; perform mEAX. 3 loops

id_test_one:

    push    EDX                         ; save port number

id_test_p10:                            ; test port addresses

    push    EBX                         ; save number of bytes
    mov     EBX, EDX                    ; port number into EBX

    dec     EBX
    shl     EBX, 2                      ; adjust for dword
    add     EBX, offset LPT_SRC         ; offset for printer ports
    mov     EDX, [EBX]

    pop     EBX                         ; restore number of bytes

    cmp     EDX, PORT1                  ; it's valid port 1?
    je      id_test_p20
    cmp     EDX, PORT2                  ; it's valid port 2?
    je      id_test_p20
    cmp     EDX, PORT3                  ; it's valid port 3?
    je      id_test_p20

    pop     EDX                         ; not a valid port, try next
    cmp     EDX, 2                      ; was it port1 or port2?
    jbe     id_test_p11
    mov     EDX, 1                      ; it was port 3, try port 1 now
    jmp     id_test_p12

id_test_p11:

    add     EDX,    1                   ; it was port 1 or 2, try next port

id_test_p12:

    push    EDX                         ; save new port number
    loop    id_test_p10                 ; try next port

    jmp     id_test_err                 ; no valid port address available

id_test_p20:                            ; test port if box is there

    push    EBX                         ; save number of bytes
    push    EAX                         ; save real original opcode
    push    ESI                         ; save original ESI for box id buffer

    call    _bios_port_test             ; tests port like BIOS does
    jc      id_test_p22                 ; port doesn't exist, try next one

#ifdef  STUPID
    call    _stupid                    ; perform the stupid macro, NODEBUG
#endif

    call    _box_reset                  ; reset box
    call    _send_cmd                   ; send real original opcode
    xor     ch, ch
    mov     cl, bh                      ; ECX = ? byte (id)

id_test_p21:

    lodsb
    call    _put_byte                   ; send id
    push    ECX                         ; save loop counter ??!! KR
    mov     ECX, D500                   ; delay for calculating ID (min 534)
                                        ; D100 (put_byte_R) + D500 = D600
                                        ; ->4MHz for ID1..3,4..8?
    call    _delay_us
    pop     ECX                         ; restore loop counter
    loop    id_test_p21

    mov     EAX, 400                    ; mEAX wait time = 4 ms
    xor     bh, bh                      ; get EBX=bl bytes (block length)
    mov     ECX, 1                      ; get 1 time (1 block)
    call    _get_nData
    cmp     EBX, 0                      ; EBX == 0 ?
    je      id_test_ret                 ; no error ok

id_test_p22:

    pop     ESI                         ; restore original ESI for box id buffer
    pop     EAX                         ; error, restore real original opcode
    pop     EBX                         ; error, restore number of bytes

id_test_err:

    stc                                 ; set Carry for error
    pop     EDX                         ; restore port number
    pop     EDX                         ; restore original port number
    pop     ECX
    ret

id_test_ret:

    clc                                 ; clear Carry for ok
    pop     EAX                         ; dummy restore for ESI
    pop     EAX                         ; senseless restore (OPC)
    pop     ECX                         ; dummy restore to reach ECX (EBX)
    pop     ECX                         ; dummy restore to reach ECX (EDX)
    pop     ECX                         ; dummy restore to reachg ECX (EDX)
    pop     ECX
    ret




//;=========================================================================
//;
//; Name: _id_search_port   ---     takes port addresses and tests box
//;
//; Input:          ah      read command
//;                 EDX     port number
//;                 bh      put ? byte ( id )
//;                 bl      get ? byte ( data )
//;                 ESI     box id buffer
//;
//; Output:         EDI     box data buffer
//;                 C       Carry set if error, Carry clear if ok
//;                 EDX     if ok portaddress, if error portnumber
//;
//;=========================================================================


_id_search_port:

    push    ECX                         ;
    push    EDX                         ; save original port number
    mov     ECX, 3                      ; perform mEAX. 3 loops

id_search_p10:

    pop     EDX
    cmp     EDX, 1                      ; was it port 1 ?
    je      id_search_p11
    cmp     EDX, 2                      ; was it port 2 ?
    je      id_search_p12
    mov     EDX, 1                      ; it was port 3, prepare port 1 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT3
    jmp     id_search_p20

id_search_p11:


    add     EDX,    1                   ; it was port 1, prepare port 2 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT2
    jmp     id_search_p20

id_search_p12:

    add     EDX,    1                   ; it was port 2, prepare port 3 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT1

id_search_p20:                          ; test port if box is there

    push    ECX                         ; save loop count  (init.3)
    push    EBX                         ; save number of bytes
    push    EAX                         ; save real original opcode
    push    ESI                         ; save original ESI for box id buffer

    call    _bios_port_test             ; tests port like BIOS does
    jc      id_search_p22               ; port doesn't exist, try next one

#ifdef  STUPID
    call    _stupid                      ; perform the stupid macro, NODEBUG
#endif

    call    _box_reset                  ; reset box
    call    _send_cmd                   ; send real original opcode
    xor     ch, ch
    mov     cl, bh                      ; ECX = ? byte (id)

id_search_p21:

    lodsb
    call    _put_byte                   ; send id
    push    ECX                         ; save loop counter ??!! KR
    mov     ECX, D500                   ; delay for calculating ID (min 534)
    ; D100 (put_byte_R) + D500 = D600
    call    _delay_us
    pop     ECX                         ; restore loop counter
    loop    id_search_p21

    mov     EAX, 1000                   ; mEAX wait time = 10 ms
    xor     bh, bh                      ; get EBX=bl bytes
    mov     ECX, 1                      ; get 1 time
    call    _get_nData
    cmp     EBX, 0                      ; EBX == 0 ?
    je      id_search_ret               ; no error ok

id_search_p22:

    pop     ESI                         ; error: restore box id buffer
    pop     EAX                         ;        restore real opcode
    pop     EBX                         ;        restore number of bytes
    pop     ECX                         ;        restore loop count (init.3)
    ;krloop    id_search_p10            ; try next address

    dec     cx
    cmp     cx,0
    jbe         done_111
    jmp     id_search_p10

done_111:


id_search_err:

    stc                                 ; set Carry for error
    pop     EDX                         ; restore original port number
    pop     ECX                         ; restore ECX
    ret

id_search_ret:

    clc                                 ; clear Carry for ok
    pop     EAX                         ; dummy restore for ESI
    pop     EAX                         ; restore real original opcode
    pop     ECX                         ; dummy restore to reach ECX (EBX)
    pop     ECX                         ; restore loop count (init.3)
    pop     ECX                         ; dummy restore to reach ECX (EDX)
    pop     ECX                         ; restore ECX
    ret

/* EOF */
