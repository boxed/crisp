
//;=====[ CBN_ASM.C ]====================================================
//;
//;Desc:        32 bit NT implementation of MARX BOX LIB
//;
//;Return:      Error code from BOX IO
//;
//;I/O:         Makes direct I/O access to printer port via device driver.
//;
//;Compiled:    Visual 'C' 9.00 32 bit (MSVC 2.00 beta and release)
//;
//;Note:        Re-written in 'C' from assembler. Pfuuu ...
//;
//;Options:     Makefile has it.
//;
//;Author:      KR, PG
//;
//;             Based upon the work done by KR, the binding of OS/2,
//;             the binding used in windows.
//;
//;Revisions:
//;
//;   REV     DATE     BY           DESCRIPTION
//;   ----  --------  ----------    ----------------------------------------
//;   0.00  01.12.93  Peter Glen    Initial version.
//;   0.00  02.12.93  Peter Glen    Converted to upper case for easy assembly
//;   0.00  05.12.93  Peter Glen    Bios data area access resolved
//;   0.00  08.01.95  Peter Glen    Ported from OS/2 to NT
//;   0.00  30.01.95  Peter Glen    Continued ...
//;   0.00  31.01.95  Peter Glen    Converted to 'C'
//;   0.00  31.02.95  Peter Glen    Killed automatic port search
//;   0.00  31.02.95  Peter Glen    Fired the person wrote this crap ...
//;   0.00  07.02.95  Peter Glen    Tried hard to make it single port.
//;   0.00  10.02.95  Peter Glen    16 bit compilation.
//;   0.00  11.02.95  Peter Glen    32 bit compilation.
//;   0.00  14.02.95  Peter Glen    Struggle with stack ...
//;   0.00  08.01.96  Peter Glen    Port to '95
//;   0.00  08.01.96  Peter Glen    Cleanup, removed NT specifics.
//;   0.00  18.01.96  Peter Glen    Rearranged for port order with '95
//;
//;=========================================================================

#include "marxdev.h"
#include "_cbn_asm.h"

#define  EXPORT
#include "cbn_asm.h"

//;****************************************************************************
//;
// How it works:
//
// Everthing is like in the old BOXIO except:
//
//    No port search (very very sticky!)
//    Definable hacker protection
//    Definable stupid protection
//    Rewritten in 'C'
//    The port input is channelled to a function, that calls back 'C'
//    The port output is channelled to a function, that calls back 'C'
//    The delay is channelled to a function, that calles back 'C'
//    The previouse functions call KERNEL and DEVICE services from NT
//
// Defines to control compilation:
//
//    SIXTEEN_BIT   to compile from 16 bit (coming from makefile)
//    HACK          to switch on hacker protection
//    STUPID        to switch on another hacker protection
//
//;****************************************************************************

#define    TRUE  1
#define    FALSE 0

//;****************************************************************************
// Control compilation:
//
#define    HACK                   /* define this for HACKER  code output */
//define    STUPID                  /* define this for STUPID code output */

//;****************************************************************************
//;
//; useful search patterns for ETP:
//;
//;            \Sin[ tab]
//;            \Sout[ tab]
//;            \S<[a-z]*: (for labels)
//;
//;****************************************************************************

//;****************************************************************************

//#define PG_DEBUG

/*
 * This is to help debug on 16 bit environment, by converting 32
 * bit things to 16 bit ones.
 *
 * All is done by the preprocessor of the compiler.
 *
 */

#ifdef      SIXTEEN_BIT

#define     EAX ax
#define     EBX bx
#define     ECX cx
#define     EDX dx

#define     EDI di
#define     ESI si

#define     ESP sp
#define     EBP bp

#else

#define     EAX eax
#define     EBX ebx
#define     ECX ecx
#define     EDX edx

#define     EDI edi
#define     ESI esi

#define     ESP esp
#define     EBP ebp

#endif

//
// Rearranged for consistancy with '95
//

#define     DPORT1               0x3bc
#define     DPORT2               0x378
#define     DPORT3               0x278

int     PORT1    = DPORT1;
int     PORT2    = DPORT2;
int     PORT3    = DPORT3;

int     LPT_REF[NUM_PORTS] =    {DPORT1, DPORT2, DPORT3};
//int     LPT_SRC[NUM_PORTS] =    {0, 0, 0};
int     LPT_SRC[NUM_PORTS] =    {DPORT2, DPORT3, DPORT1};

//;****************************************************************************
//;
//;Summary:     Common entry point of BOX IO
//;             This function will call the converted 16 bit functions,
//;             the extended registers are zeroed for compatibility.
//;             (ex: loop cx overrun)
//;
//;Return:      Error   code from BOX IO. (in EAX)
//;
//; Input:      32 bit  OpCode (it is a 32 bit system)
//;             32 bit  PortNr
//;             32 bit Pointer
//;             32 bit Pointer
//;
//;Note:        Following the 'C' 32 bit calling convention (Win API)
//;             the stack is restored as follows:
//;
//;             leave;
//;
//; All arguments are pushed in 'C' order, 32 bit:
//; The following code fragment was generated by the C++ compiler, and
//; it is used as the basic interface for the ASM module:
//;
//;         _iOpCode$ = 8
//;         _iPortNr$ = 12
//;         _pcInData$ = 16
//;         _pcOutData$ = 20
//;
//; (Provided for informational purposes only)
//;
//;****************************************************************************

short    CB560(
                    unsigned long iOpCode,
                    unsigned long iPortNr,
                    unsigned char *pcInData,
                    unsigned char *pcOutData
              )

{
    long    ret_val;
    short   lport;
    short   lval;
    short   ldelay;
    short   cret;

   #ifdef PG_DEBUG
    long port_adr;
    DebugPrintf("Port: %d", iPortNr);
   #endif

    _asm
    {
    push    EBX
    push    ECX
    push    EDX
    push    ESI
    push    EDI
    ;pushf

#ifdef      SIXTEEN_BIT
    push    es
    mov     ax,ds
    mov     es,ax
#endif

    xor     EAX,EAX
    xor     EBX,EBX
    xor     ECX,ECX
    xor     EDX,EDX

    mov     EAX, iOpCode
    mov     EDX, iPortNr
    mov     ESI, pcInData
    mov     EDI, pcOutData

    xchg    al,ah
    call    _box_io

    mov     EAX, EBX
    mov     ret_val,eax

#ifdef      SIXTEEN_BIT
    pop     es
#endif

    ;popf
    pop     EDI
    pop     ESI
    pop     EDX
    pop     ECX
    pop     EBX

//; code exited here ->
    jmp final_exit

//;=========================================================================
//;
//;Summary:         Port input under NT
//;
//; Input:          dx      port to input from
//;
//; Registers:      All preserved exept RET_VAL (EAX)
//;
//; Note:           This function will call back the C++ environment for PORT
//;                 input via device driver.
//;
//; '95 Note:       This function is near identical to the 16 ones.
//;                 The reason for extracting is to keep it clean for
//;                 expansion or box re-write.
//;
//;=========================================================================

read_port:

    push    EBX
    push    ECX
    push    EDX
    push    ESI
    push    EDI

    in      al,dx

    pop     EDI
    pop     ESI
    pop     EDX
    pop     ECX
    pop     EBX

    ret     0

//;=========================================================================
//;
//;Summary:     Port output under NT
//;
//; Input:          dx      port to input from
//;                 al      value to output
//;
//; Registers:      All preserved exept RET_VAL (EAX)
//;
//; Note:           This function will call back the C++ environment for PORT
//;                 output via device driver.
//;
//; '95 Note:       This function is near identical to the 16 ones.
//;                 The reason for extracting is to keep it clean for
//;                 expansion or box re-write.
//;
//;=========================================================================

write_port:

    push    EBX
    push    ECX
    push    EDX
    push    ESI
    push    EDI

    out     dx,al

    pop     EDI
    pop     ESI
    pop     EDX
    pop     ECX
    pop     EBX

    ret     0

//;=========================================================================
//;
//;Summary:     Enable/disable interrups.
//;
//;Registers:   All preserved.
//;
//;Note:        This function will call back the C++ environment for PORT
//;             disableing interrupts and multitasking.
//;
//; '95 Note:       This function is near identical to the 16 ones.
//;                 The reason for extracting is to keep it clean for
//;                 expansion or box re-write.
//;
//;=========================================================================

disable:

#ifdef      SIXTEEN_BIT
    cli
#else
    //cli                           ;this time nothing is done
#endif
    ret   0

enable:

#ifdef      SIXTEEN_BIT
    sti
#else
    //sti                           ;this time nothing is done
#endif
    ret    0

//;=========================================================================
//;
//;Summary:     Delay under NT
//;
//;Registers:   All preserved exept RET_VAL (EAX)
//;
//;Calling:     NTSTATUS KeDelayExecutionThread(WaitMode, Alertable, Interval)
//;
//;This function will call back the NT KERNEL environment for DELAY
//;
//;=========================================================================
//;
//; Description from NT DDK:
//; ------------------------
//;
//; NTSTATUS KeDelayExecutionThread(WaitMode, Alertable, Interval)
//;
//; IN KPROCESSOR_MODE WaitMode;
//; IN BOOLEAN Alertable;
//; IN PLARGE_INTEGER Interval;
//;
//; KeDelayExecutionThread puts the current thread into an alertable or
//; non-alertable wait state for a given interval.
//;
//; Parameter   Description
//; ---------   -----------
//;
//; WaitMode    Specifies the processor mode in which the caller is waiting,
//;             which can be either KernelMode or UserMode.
//;             Lower-level drivers should specify KernelMode.
//;
//; Alertable   Specifies TRUE if the wait is alertable or FALSE if
//;             nonalertable.
//;
//; Interval    Specifies the absolute or relative time, in units of 100
//;             nanoseconds, for which the wait is to occur.
//;
//; Description from 95 DDK:
//; ------------------------
//;
//; '95 Note:       This function is near identical to the 16 ones.
//;                 The reason for extracting is to keep it clean for
//;                 expansion or box re-write.
//;
//;=========================================================================

#include "cbndel.c"

_delay_us:

    push    EBX
    push    ECX
    push    EDX
    push    ESI
    push    EDI

    call    delay_it
    pop     EDI
    pop     ESI
    pop     EDX
    pop     ECX
    pop     EBX

    ret     0

    //delay_us       endp

//;=========================================================================
//;
//;What follws this point is IDENTICAL of the original BOX IO exept:
//;
//; a. the port IO is done in the fashion of:
//;
//;            read_port  for read
//;            write_port for write
//;
//;    These functions will do the same as: (funtionally compatible with)
//;
//;             in  al,dx
//;             out dx,al
//;
//; b. delay is done with the help of NT (offitial delay)
//;
//;            delay for waiting
//;
//; c. cli sti are not allowed.
//;
//; c. The automatic port search is shut out.
//;
//;=========================================================================

//;=========================================================================
//;
//; Name: _box_io           ---     Box I/O handler
//;
//; Input:          ah      Function code
//;                 EBX     Pointer to printer-port (1..4)
//;                 ESI     command data point
//;                 EDI     return data point
//;
//; Output:         EBX     error flag
//;                 carry   clear ok
//;                 carry   set error
//;
//; Flow:           Check for very obvious errors:
//;
//;                    1. Wrong port number
//;                    2. Wrong opcode
//;                    3. Null pointers.
//;
//;=========================================================================

_box_io:

    //push    EBP                       ;no new stack frame !!
    //mov     EBP,ESP

chk_parameter:

    cmp     EDX, 3                      ; Port number > 3 ?
    ja      port_nr_big                 ; given port number is > 3
    cmp     EDX, 1                      ; Port number < 1 ?
    jb      port_nr_err                 ; given port number is < 1 -> err

    jmp     portnr_ok                   ; port no. 1..3 yes, ok

port_nr_big:

    cmp     EDX, 11                     ; Port number < 11 ?
    jb      port_nr_err                 ; given port number is > 3 & < 11
    cmp     EDX, 13                     ; Port number > 13 ?
    jbe     portnr_ok                   ; port no. 11..13 yes, ok

port_nr_err:

    mov     EBX, PARAM_ERR
    jmp     chk_ret

portnr_ok:

    cmp     ESI, 0                     ; given ptr != 0 ?
    jne     ptr1_ok                    ; yes, ok
    mov     EBX, PARAM_ERR
    jmp     chk_ret

ptr1_ok:

    cmp     EDI, 0                     ; given ptr != 0 ?
    jne     ptr2_ok                    ; yes, ok
    mov     EBX, PARAM_ERR
    jmp     chk_ret

ptr2_ok:

    xor         EBX, EBX                ; clear error carrier

//;=========================================================================
//;* check for right OpCode:
//;=========================================================================

chk_opc:

    cmp     ah,  0
    ja      opc_0
    jmp     chk_err

opc_0:

    cld

    cmp     ah, GET_ID8_VIR             ; get IEDX
    ja      opc_1
    jmp     chk_01

opc_1:

    cmp     ah, GET_SERIAL_VIR          ; get serial number
    jne     opc_2
    jmp     chk_10

opc_2:

    cmp     ah, ENCRYPT_VIR             ; encrypt data
    jne     opc_3
    jmp     chk_11

opc_3:

    cmp     ah, DECRYPT_VIR             ; decrypt data
    jne     opc_4
    jmp     chk_12

opc_4:

    cmp     ah, BOX_READY_VIR           ; quick check of box
    jne     opc_5
    jmp     chk_13

opc_5:

    cmp     ah, READ_RAM1_VIR           ; read data from ram1
    jne     opc_6
    jmp     chk_14

opc_6:

    cmp     ah, READ_RAM2_VIR           ; read data from ram2
    jne     opc_7
    jmp     chk_15

opc_7:

    cmp     ah, WRITE_RAM1_VIR          ; write data to ram1
    jne     opc_8
    jmp     chk_16

opc_8:

    cmp     ah, WRITE_RAM2_VIR          ; write data to ram2
    jne     opc_9
    jmp     chk_17

opc_9:

    cmp     ah, INC_RAM1_VIR            ; increment counter ram1
    jne     opc_10
    jmp     chk_18

opc_10:

    cmp     ah, DEC_RAM1_VIR            ; decrement counter ram1
    jne     opc_11
    jmp     chk_19

opc_11:

    cmp     ah, INC_RAM2_VIR            ; increment counter ram2
    jne     opc_12
    jmp     chk_20

opc_12:

    cmp     ah, DEC_RAM2_VIR            ; decrement counter ram2
    jne     opc_13
    jmp     chk_21

opc_13:

    cmp     ah, UNI_CRYPT_VIR           ; uni encryption
    jne     opc_14
    jmp     chk_29

opc_14:


#ifdef  _WRITE_PW_ID

    cmp     ah, WR_PW_R2_VIR            ; write password ram2
    jne     opc_15
    jmp     chk_23

opc_15:

    cmp     ah, WR_ID4_VIR              ; write ID4
    jne     opc_16
    jmp     chk_24

opc_16:

    cmp     ah, WR_ID5_VIR              ; write ID5
    jne     opc_17
    jmp     chk_24

opc_17:

    cmp     ah, WR_ID6_VIR              ; write ID6
    jne     opc_18
    jmp     chk_24

opc_18:

    cmp     ah, WR_ID7_VIR              ; write ID7
    jne     opc_19
    jmp     chk_24

opc_19:

    cmp     ah, WR_ID8_VIR              ; write ID8
    jne     opc_20
    jmp     chk_24

#endif


opc_20:

#ifdef  _NOVELL
    cmp     ah, WR_NSERNR_VIR           ; write NOVELL ser.- nr.
    jne     opc_21
    jmp     chk_50

opc_21:

    cmp     ah, RD_UNLIMIT_VIR          ; read unlimited for int. ram
    jne     opc_22
    jmp     chk_51
#endif

opc_22:


#ifdef  _NLM
    cmp     ah, RD_MINLIM_VIR           ; read minlimit of CBN
    jne     opc_23
    jmp     chk_52
#endif

opc_23:

#ifdef  _NOVELL
    cmp     ah, WR_USERLIM_VIR          ; write userlimit of CBN
    jne     opc_24
    jmp     chk_53

opc_24:

    cmp     ah, WR_MARXLIM_VIR          ; write marxlimit of CBN
    jne     opc_25
    jmp     chk_54
#endif


opc_25:

#ifdef  _READALL
    cmp     ah, READ_ALL_VIR            ; read all values of CB
    jne     opc_26
    jmp     chk_55

opc_26:

    cmp     ah, WR_ID_OFF_VIR           ; write ID offset
    jne     opc_27
    jmp     chk_56
#endif

opc_27:

    jmp     chk_err                     ; not a valid opcode

chk_01:

    add     ah, ID_OFFSET

    ;swaph                              ; swaps ah
    push    ECX
    push    EDX
    xor     dx, dx
    mov     dl, al                      ; backup al
    mov     cx, 04h
    mov     al, ah                      ; copy ah to al
    shl     al, cl
    shr     ah, cl
    or      ah, al                      ; make the byte
    mov     al, dl                      ; restore al
    pop     EDX
    pop     ECX

    xor     ah, SEC_BYTE
    mov     EBX,0302h                   ; bh: ? byte id  bl: ? data return
    call    _get_codes                  ; get id data
    jmp     chk_ret

chk_10:

    mov     ah, GET_SERIAL_OPC
    mov     EBX,0203h                   ; bh: ? byte id  bl: ? data return
    call    _get_codes                  ; get serial data
    jmp     chk_ret

chk_11:

    mov     ah, ENCRYPT_OPC             ; encrypt data
    call    _box_crypt
    jmp     chk_ret

chk_12:

    mov     ah, DECRYPT_OPC             ; encrypt data
    call    _box_crypt
    jmp     chk_ret

chk_13:

    mov     ah, BOX_READY_OPC           ; quick check of box
    call    _box_ready
    jmp     chk_ret

chk_14:

    mov     ah, READ_RAM1_OPC           ; read data from ram1
    call    _read_ram
    jmp     chk_ret

chk_15:

    mov     ah, READ_RAM2_OPC           ; read data from ram2
    call    _read_ram
    jmp     chk_ret

chk_16:

    mov     ah, WRITE_RAM1_OPC          ; write data to ram1
    call    _write_ram
    jmp     chk_ret

chk_17:

    mov     ah, WRITE_RAM2_OPC          ; write data to ram2
    call    _write_ram
    jmp     chk_ret

chk_18:

    mov     ah, INC_RAM1_OPC            ; increment counter ram1
    call    _ram_counter
    jmp     chk_ret

chk_19:

    mov     ah, DEC_RAM1_OPC            ; decrement counter ram1
    call    _ram_counter
    jmp     chk_ret

chk_20:

    mov     ah, INC_RAM2_OPC            ; increment counter ram2
    call    _ram_counter
    jmp     chk_ret

chk_21:

    mov     ah, DEC_RAM2_OPC            ; decrement counter ram2
    call    _ram_counter
    jmp     chk_ret

chk_29:

    mov     ah, UNI_CRYPT_OPC           ; uni encryption algorithm
    call    _uni_crypt
    jmp     chk_ret

#ifdef  _WRITE_PW_ID

chk_23:

    mov     ah, WR_PW_R2_OPC            ; write password ram2
    mov     EBX, 04h                    ; write 4 bytes
    call    _write_special
    jmp     chk_ret

chk_24:

    add     ah, ID_W_OFFSET             ; write IEDX

    swaph                               ; swaps ah

    push    ECX
    push    EDX
    xor     dx, dx
    mov     dl, al                      ; backup al
    mov     cx, 04h
    mov     al, ah                      ; copy ah to al
    shl     al, cl
    shr     ah, cl
    or      ah, al                      ; make the byte
    mov     al, dl                      ; restore al
    pop     EDX
    pop     ECX

    xor     ah, SEC_BYTE
    mov     EBX, 05h                    ; write 5 bytes
    call    _write_special
    jmp     chk_ret
#endif

#ifdef  _NOVELL

chk_50:

    mov     ah, WR_NSERNR_OPC           ; write NOVELL ser.-nr.
    call    _write_nsernr
    jmp     chk_ret

chk_51:

    mov     ah, RD_UNLIMIT_OPC          ; read unlimited for CBN
    call    _read_unlim
    jmp     chk_ret
#endif

#ifdef  _NLM


chk_52:

    mov     ah, RD_MINLIM_OPC           ; read minlimit of CBN
    call    _read_minl
    jmp     chk_ret
#endif

#ifdef  _NOVELL


chk_53:

    mov     ah, WR_USERLIM_OPC          ; write userlimit of CBN
    call    _write_userl
    jmp     chk_ret

chk_54:

    mov     ah, WR_MARXLIM_OPC          ; write marxlimit of CBN
    call    _write_marxl
    jmp     chk_ret
#endif


#ifdef  _READALL

chk_55:

    mov     ah, READ_ALL_OPC            ; read all values of CB
    call    _read_all
    jmp     chk_ret

chk_56:

    mov     ah, WR_ID_OFF_OPC           ; write ID offset
    call    _write_id_off
    jmp     chk_ret

#endif


chk_err:


    mov     EBX, WRONG_OPC              ; OPC <= 0 or not in table

chk_ret:

    //pop     EBP                         ; restore EBP
    //sti                                 ; set interrupt flag
    call    enable
    ret

#include "cbnres.c"

//;=========================================================================
//;
//; Name: _get_codes        ---     Read data from BOXII
//;
//; Input:          ah      read command
//;                 EDX     port number given by user
//;                 bh      put ? byte ( id )
//;                 bl      get ? byte ( data )
//;                 ESI     box id buffer
//;
//; Output:         EDI     box data buffer
//;                 EBX     error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_get_codes:

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

    push    EDX                         ; save original portnumber

    call    _id_test_port               ; reads port from BIOS, tests box
    jnc     gtc_start_cmd               ; if no error, perform command

    cmp     EDX, 3                      ; port number = 1, 2 or 3 ?
    ja      gtc_port_err                ; no, no other tests

    pop     EDX                         ; get original portnumber
    push    EDX                         ; to ensure same behaviour
    call    _id_search_port             ; takes next port, tests box
    jc      gtc_port_err                ; if error jmp to exit

gtc_start_cmd:

#ifdef  HACK
    call    _crack_the_hack             ; perform hacker test, NODEBUG
#endif

    pop     ECX                         ; dummy pop to reach EAX

gtc_ret:

    pop     EAX
    ret

gtc_port_err:

    mov     EBX, BOX_NOT_FOUND
    pop     EDX
    pop     EAX
    ret

//;=========================================================================
//;
//; Name: _box_crypt        ---     encrypt data with box's crypter
//;
//; Input:          ah      encryption command
//;                 EDX     printer port number
//;                 ESI     parameter buffer
//;                 EDI     data buffer
//;
//; Output:         EDI     data buffer
//;                 EBX     error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_box_crypt:

    mov     al, ah                      ; create real opcode
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


    push    EAX                         ; save original real opcode
    push    EDX                         ; save original portnumber

    call    _test_port                  ; reads port from BIOS, tests box
    jnc     crypt_start                 ; if no error perform command

    cmp     EDX, 3                      ; port number = 1, 2 or 3 ?
    ja      enc_port_err                ; no, no other tests

    pop     EDX                         ; get original portnumber
    push    EDX                         ; to ensure same behaviour
    call    _search_port                ; takes next port, tests box
    jc      enc_port_err                ; if error jmp to exit


crypt_start:


#ifdef  HACK
    call    _crack_the_hack             ; perform hacker test, NODEBUG
#endif

    pop     EAX                         ; dummy pop to reach EAX (EDX)
    pop     EAX                         ; restore original real opcode
    call    _send_cmd                   ; send real box_crypt command

    lodsb                               ; load HiLength from buffer
    mov     ah, al                      ; save HiLength
    lodsb                               ; load LowLength from buffer
    and     EAX, EAX                    ; test Length for 0
    jz      box_crypt_err_0             ; Length was 0, it's wrong...
    push    EAX                         ; save complete valid Length

    call    _put_byte                   ; send byte LowLength
    mov     al, ah                      ; get HiLength ready to send
    call    _put_byte                   ; send byte HiLength

    lodsb                               ; load LowSeed from buffer
    call    _put_byte                   ; send byte LowSeed
    lodsb                               ; load HiSeed from buffer
    call    _put_byte                   ; send byte HiSeed

box_crypt_10:

    pop     EAX                         ; restore complete valid Length
    push    EAX                         ; save it again for data restauration
    mov     ECX, EAX                    ; set loop counter = Length
    mov     ESI, EDI                    ; -+
    ;mov     EAX, es                    ;  |- get address of string to crypt
    ;mov     ds, EAX                    ; -+

box_crypt_12:

    lodsb                               ; get one byte of string to crypt
    call    _put_byte                   ; send one byte to crypt it
    push    ECX                         ; save loop counter

    mov     EAX, 400                    ; mEAX wait time = 4 ms
    mov     EBX, 1                      ; get 1 byte
    mov     ECX, 1                      ; get 1 times
    call    _get_nData                  ;
    cmp     EBX, 0                      ; error in get_nData?
    jnz     box_crypt_err_1             ; yes, error in get_nData

    pop     ECX                         ; restore loop counter
    loop    box_crypt_12                ; perform loop Length times

box_crypt_14:

    mov     ECX, D55                    ; delay 55 us (min 10us)
    call    _delay_us                   ; for getting ACK = hi

    mov     EAX, 400                    ; mEAX wait time = 4 ms
    mov     EBX, 1                      ; get 1 byte
    call    _get_testData               ; get last (n+1) byte
    mov     ah, al                      ; save it to ah

    //sti                                 ; allow interrupt requests
    call    enable
    ; all work with box is done

    std                                 ; set string operation backwards
    dec     ESI                         ; point to last byte of buffer
    dec     EDI                         ; point to last byte of buffer
    pop     ECX                         ; restore complete valid Length

box_crypt_16:                           ; data restauration

    lodsb                               ; get next byte
    mov     bl, al                      ; backup 1. part
    and     ah, 0f0h                    ; make hi nibble
    and     al, 0fh                     ; make lo nibble
    or      al, ah                      ; get the whole byte
    stosb                               ; store it
    mov     ah, bl                      ; get saved copy of 1. part (now 2. part)
    loop    box_crypt_16                ; do this with all bytes

    cld                                 ; set string operations forward as normal

box_crypt_ret:

    xor     EBX, EBX                    ; no error, ok
    ret

enc_port_err:

    mov     EBX, BOX_NOT_FOUND
    pop     EDX
    pop     EAX
    ret

box_crypt_err_0:

    mov     EBX, BOX_CRYPT_ERR
    ret

box_crypt_err_1:

    pop     ECX
    pop     EAX
    mov     EBX, BOX_CRYPT_ERR
    ret

//;=========================================================================
//;
//; Name: _uni_crypt        ---     get crypt_key for software implemented
//;                               crypt algorithm
//;
//; Input:          ah      opcode
//;                 EDX     printer port number
//;                 ESI     parameter buffer
//;                 EDI     data buffer
//;
//; Output:         EDI     data buffer
//;                 EBX     error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_uni_crypt:

    mov     al, ah                      ; create real opcode
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

    push    EAX                         ; save original real opcode
    push    EDX                         ; save original portnumber

    call    _test_port                  ; reads port from BIOS, tests box
    jnc     uni_start                   ; if no error perform command

    cmp     EDX, 3                      ; port number = 1, 2 or 3 ?
    ja      uni_port_err                ; no, no other tests

    pop     EDX                         ; get original portnumber
    push    EDX                         ; to ensure same behaviour
    call    _search_port                ; takes next port, tests box
    jc      uni_port_err                ; if error jmp to exit

uni_start:

    pop     EAX                         ; dummy pop to reach EAX (EDX)
    pop     EAX                         ; restore original real opcode
    call    _send_cmd                   ; send real box_crypt command

uni_crypt_10:

    ; get 4 bytes ROM codes
    mov     EAX, 400                    ; mEAX wait time = 4 ms
    mov     EBX, 4                      ; get EBX=4 bytes (block length)
    mov     ECX, 1                      ; get 1 time (1 block)
    call    _get_nData
    cmp     EBX, 0                      ; error during get_nData?
    jnz     uni_crypt_err1              ; if error jmp to err

    mov     ECX, D50                    ; delay min 10us to get ACK = hi
    call    _delay_us                   ; KR 18.05.94 ->4MHz

    ; get 4 bytes RAM codes
    mov     EAX, 400                    ; mEAX wait time = 4 ms
    mov     EBX, 4                      ; get EBX=3 bytes (block length)
    mov     ECX, 1                      ; get 1 time (1 block)
    call    _get_nData
    cmp     EBX, 0                      ; error during get_nData?
    jnz     uni_crypt_err1              ; if error jmp to err

uni_crypt_ret:

    xor     EBX, EBX                    ; no error, clear EBX
    ret

uni_crypt_err1:

    mov     EBX, UNI_CRYPT_ERR          ; error occured
    ret

uni_port_err:

    mov     EBX, BOX_NOT_FOUND
    pop     EDX
    pop     EAX
    ret

//;=========================================================================
//;
//; Name: _read_ram         ---     read data from RAM12 of box
//;
//; Input:          ah      read command
//;                 EDX     printer port number
//;                 ESI     parameter buffer
//;
//;
//; Output:         EDI     data buffer
//;                 EBX     error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_read_ram:

    mov     al, ah                      ; create real opcode
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

    push    EAX                         ; save original real opcode
    push    EDX                         ; save original portnumber

    call    _test_port                  ; reads port from BIOS, tests box
    jnc     read_ram_10                 ; if no error perform command

    cmp     EDX, 3                      ; port number = 1, 2 or 3 ?
    ja      rd_port_err                 ; no, no other tests

    pop     EDX                         ; get original portnumber
    push    EDX                         ; to ensure same behaviour
    call    _search_port                ; takes next port, tests box
    jc      rd_port_err                 ; if error jmp to exit

read_ram_10:

    pop     EAX                         ; dummy pop to reach EAX (EDX)
    pop     EAX                         ; restore original real opcode
    call    _send_cmd                   ; send real box_crypt command

    mov     ECX, 4                      ; 4 bytes RamPass to send

    //cli
    call    disable
read_ram_12:

    push    EAX                         ; save opcode for compare

    lodsb                               ; load 1 byte of RamPass
    call    _put_byte                   ; send the byte

    pop     EAX                         ; restore opcode for compare
    cmp     al, REAL_RAM2_RD            ; read attempt to RAM2 ?
    jnz     read_ram_13
    push    ECX                         ; RAM2: (min 560us) 100+500+45=645us
    mov     ECX, D500                   ; RAM1: (min 110us) 100+45=145us
    call    _delay_us                   ; check 1 byte in ext. RAM
    pop     ECX                         ; ->4MHz

read_ram_13:

    nop
    loop    read_ram_12                 ; do it 4 times

    //sti
    call    enable
read_ram_14:

    lodsb                               ; load LoLength from buffer
    mov     ah, al                      ; save LoLength
    lodsb                               ; load HiLength from buffer
    xchg    al, ah                      ; get the right order hi:lo
    and     EAX, EAX                    ; test Length for 0
    jz      rd_ram_err                  ; Length was 0, it's wrong...
    push    EAX                         ; save complete valid Length

    call    _put_byte                   ; send byte LoLength
    mov     al, ah                      ; get HiLength ready to send
    call    _put_byte                   ; send byte HiLength

    lodsb                               ; load LoAddr from buffer
    call    _put_byte                   ; send byte LoAddr
    lodsb                               ; load HiAddr from buffer
    call    _put_byte                   ; send byte HiAddr

read_ram_16:

    pop     EAX                         ; restore complete valid Length
    mov     ECX, EAX                    ; set loop counter = Length

read_ram_18:

    push    ECX                         ; save loop counter
    mov     EAX, 400                    ; mEAX wait time = 4 ms
    mov     EBX, 1                      ; get EBX=1 bytes (block length)
    mov     ECX, 1                      ; get 1 time (1 block)
    call    _get_nData

    mov     ECX, D45                    ; KR delay for get ACK = hi from box
    call    _delay_us                   ; wait time between data blocks

    pop     ECX                         ; restore loop counter
    cmp     EBX, 0                      ; EBX == 0 ?

    jnz     rd_ram_err                  ; yes, error in get_nData

    loop    read_ram_18                 ; no error, ready to send next byte

rd_ram_ret:

    xor     EBX, EBX                    ; no error
    ret

rd_ram_err:

    mov     EBX, READ_RAM_ERR           ; error during send
    ret

rd_port_err:

    pop     EDX                         ; save original portnumber
    pop     EAX                         ; save original real opcode

    mov     EBX, BOX_NOT_FOUND          ; no box found
    ret

//;=========================================================================
//;
//; Name: _write_ram        ---     write data to RAM12 of box
//;
//; Input:          ah      write command
//;                 EDX     printer port number
//;                 ESI     parameter buffer
//;                 EDI     data buffer
//;
//; Output:         EBX     error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_write_ram:

    mov     al, ah                      ; create real opcode
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

    push    EAX                         ; save original real opcode
    push    EDX                         ; save original portnumber

    call    _test_port                  ; reads port from BIOS, tests box
    jnc     write_ram_10                ; if no error perform command

    cmp     EDX, 3                      ; port number = 1, 2 or 3 ?
    ja      wr_port_err                 ; no, no other tests

    pop     EDX                         ; get original portnumber
    push    EDX                         ; to ensure same behaviour
    call    _search_port                ; takes next port, tests box
    jc      wr_port_err                 ; if error jmp to exit

write_ram_10:

    //cli
    call    disable
    pop     EAX                         ; dummy pop to reach EAX (EDX)
    pop     EAX                         ; restore original real opcode
    call    _send_cmd                   ; send real box_crypt command

    mov     ECX, 4                      ; 4 bytes RamPass to send

write_ram_12:

    push    EAX                         ; save opcode for compare

    lodsb                               ; load 1 byte of RamPass
    call    _put_byte                   ; send the byte

    pop     EAX                         ; restore opcode for compare
    cmp     al, REAL_RAM2_WR            ; write attempt to RAM2 ?
    jnz     write_ram_13
    push    ECX                         ; RAM2: (min 560us) 100+500+45=645us
    mov     ECX, D500                   ; RAM1: (min 110us) 100+45=145us
    call    _delay_us                   ; check 1 byte in ext. RAM
    pop     ECX                         ; ->4MHz

write_ram_13:

    nop
    loop    write_ram_12                ; do it 4 times
    //sti
    call    enable

write_ram_14:

    lodsb                               ; load LoLength from buffer
    mov     ah, al                      ; save LoLength
    lodsb                               ; load HiLength from buffer
    xchg    al, ah                      ; get the right order hi:lo
    and     EAX, EAX                    ; test Length for 0
    jz      wr_ram_err                  ; Length was 0, it's wrong...
    push    EAX                         ; save complete valid Length

    call    _put_byte                   ; send byte LoLength
    mov     al, ah                      ; get HiLength ready to send
    call    _put_byte                   ; send byte HiLength

    lodsb                               ; load LoAddr from buffer
    call    _put_byte                   ; send byte LoAddr
    lodsb                               ; load HiAddr from buffer
    call    _put_byte                   ; send byte HiAddr

    mov     ECX, D350                   ; delay for ewen RAM (overall min 400us)
    call    _delay_us                   ; now:100+350+45=495us  ->4MHz

write_ram_16:

    pop     EAX                         ; restore complete valid Length
    mov     ECX, EAX                    ; set loop counter = Length
    mov     ESI, EDI                    ; -+
    ;mov     EAX, es                    ;  |- get address of data to send
    ;mov     ds, EAX                    ; -+

write_ram_18:

    lodsb                               ; load 1 byte of data
    call    _put_byte                   ; send the byte

    ; no wait time to get ACK = lo
    mov     EAX, 1500                   ; mEAX wait time = 15ms (Mind the VERSA)
    call    _chk_ack                    ; check ACK = lo, wait for ACK = hi
    cmp     EBX, 0                      ; error during wait ?
    jnz     wr_ram_err                  ; yes, error in _chk_ack

    push    ECX                         ; save number of bytes to send
    mov     ECX, D25                    ; delay to get box ready for next byte
    call    _delay_us                   ; KR 18.05.94 (min 40us) now: 25+45=70us
    pop     ECX                         ; restore number of bytes, ->4MHz

    loop    write_ram_18                ; no error, ready to send next byte

    mov     ECX, D300                   ; delay for ewds RAM (overall min 320us)
    call    _delay_us                   ; now: 25us+300us = 325us
    ; KR 18.05.94 ->4MHz

wr_ram_ret:

    xor     EBX, EBX                    ; no error
    ret

wr_ram_err:

    mov     EBX, WRITE_RAM_ERR          ; error during send
    ret

wr_port_err:

    pop     EDX                         ; save original portnumber
    pop     EAX                         ; save original real opcode
    mov     EBX, BOX_NOT_FOUND          ; no box found
    ret

//;=========================================================================
//;
//; Name: _ram_counter      ---     increment/decrement function
//;
//; Input:          ah      opcode
//;                 EDX     printer port number
//;                 ESI     parameter buffer
//;                 EDI     data buffer
//;
//; Output:         EBX     error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_ram_counter:

    mov     al, ah                      ; create real opcode
    xor     al, SEC_BYTE                ;
    ;swapl                              ; swaps al;
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

    push    EAX                         ; save original real opcode
    push    EDX                         ; save original portnumber

    call    _test_port                  ; reads port from BIOS, tests box
    jnc     ram_count_10                ; if no error perform command

    cmp     EDX, 3                      ; port number = 1, 2 or 3 ?
    ja      ram_port_err                ; no, no other tests

    pop     EDX                         ; get original portnumber
    push    EDX                         ; to ensure same behaviour
    call    _search_port                ; takes next port, tests box
    jc      ram_port_err                ; if error jmp to exit

ram_count_10:

    pop     EAX                         ; dummy pop to reach EAX (EDX)
    pop     EAX                         ; restore original real opcode
    call    _send_cmd                   ; send real box_crypt command

    mov     ECX, 4                      ; 4 bytes RamPass to send

ram_count_12:

    push    EAX                         ; save opcode for compare

    lodsb                               ; load 1 byte of RamPass
    call    _put_byte                   ; send the byte

    pop     EAX                         ; restore opcode for compare
    cmp     al, REAL_DEC_2              ; decr attempt to RAM2 ?
    jz      ram_count_13a               ; yes, decr, jmp to delay
    cmp     al, REAL_INC_2              ; incr attempt to RAM2 ?
    jnz     ram_count_13                ; no, no delay else goto delay

ram_count_13a:

    push    ECX                         ; RAM2: (min 540us) 100+500+45=645us
    mov     ECX, D500                   ; RAM1: (min 110us) 100+45=145us
    call    _delay_us                   ; check 1 byte in ext. RAM
    pop     ECX                         ; ->4MHz
ram_count_13:

    nop
    loop    ram_count_12                ; do it 4 times

ram_count_14:

    lodsb                               ; load LoAddr from buffer
    call    _put_byte                   ; send byte LowAddr
    lodsb                               ; load HiAddr from buffer
    call    _put_byte                   ; send byte HiAddr

ram_count_16:

    ; wait for writing 2 bytes
    mov     EAX, 2000                   ; mEAX wait time = 20 ms
    mov     EBX, 2                      ; get EBX=2 bytes (block length)
    mov     ECX, 1                      ; get 1 time (1 block)
    call    _get_nData
    cmp     EBX, 0                      ; EBX == 0 ?
    jnz     ram_count_err1              ; yes, error in get_nData

ram_count_ret:

    xor     EBX, EBX                    ; no error
    ret

ram_port_err:

    pop     EDX                         ; save original portnumber
    pop     EAX                         ; save original real opcode
    mov     EBX, BOX_NOT_FOUND          ; no box found
    ret

ram_count_err1:

    mov     EBX, RAM_COUNT_ERR          ; error during incr/decr routine
    ret

#include "cbnrdy.c"

//;*****************************************************************************
//; NOVELL - Functions
//;*****************************************************************************

#ifdef  _NLM

//;****************************************************************************
//;   nlm.inc contains :
//;        _read_minl       ---     read the low of address 0 or 1 rESPectively
//;                        of internal EEPROM
//;****************************************************************************

//include   nlm.inc

#endif


#ifdef  _NOVELL

//;****************************************************************************
//;   novell.inc contains :
//;        _write_userl     ---     write address 1 (usrlim) of internal EEPROM
//;        _write_marxl     ---     write address 0 (mrxlim) of internal EEPROM
//;        _read_unlim      ---     read specified address of internal EEPROM
//;        _write_nsernr    ---     write Novell box ser.-no.
//;****************************************************************************

//include   novell.inc

#endif
//;*****************************************************************************
//; END of NOVELL Functions
//;*****************************************************************************


//;*****************************************************************************
//; Prog-Utility Functions
//;*****************************************************************************

#ifdef  _WRITE_PW_ID

//;****************************************************************************
//;   prog.inc contains :
//;        _write_special   ---     write IEDX and RAM passwords
//;****************************************************************************

//include   prog.inc

#endif
//;*****************************************************************************
//; END of Prog-Utility Functions
//;*****************************************************************************


//;*****************************************************************************
//; Internal Functions
//;*****************************************************************************

#ifdef  _READALL

//;****************************************************************************
//;   prog.inc contains :
//;        _read_all        ---     read all internals
//;        _write_id_off    ---     read all internals
//;****************************************************************************

//include   internal.inc

#endif

//;*****************************************************************************
//; END of Internal Functions
//;*****************************************************************************


//;*****************************************************************************
//; Mid and Low Level Functions
//;*****************************************************************************

#include "cbntest.c"

//;=========================================================================
//;
//; Name: _search_port      ---     reads BIOS port number and tests box
//;
//; Input:          ah      command of host routine
//;                 EDX     port number
//;                 ESI     box cmd & id buffer
//;
//; Output:         C       Carry set if error, Carry clear if ok
//;                 EDX     if ok portaddress, if error portnumber
//;
//;=========================================================================

_search_port:

    push    EAX                         ; save opcode of host routine
    push    ECX                         ;
    push    EDX                         ; save original port number
    mov     ECX, 3                      ; perform mEAX. 3 loops

search_port10:

    pop     EDX
    cmp     EDX, 1                      ; was it port 1 ?
    je      search_port11
    cmp     EDX, 2                      ; was it port 2 ?
    je      search_port12
    mov     EDX, 1                      ; it was port 3, prepare port 1 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT3
    jmp     search_port20

search_port11:

    add     EDX,    1                   ; it was port 1, prepare port 2 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT2
    jmp     search_port20

search_port12:

    add     EDX,    1                   ; it was port 2, prepare port 3 next
    push    EDX                         ; save next port number to test
    mov     EDX, PORT1

search_port20:                          ; test port if box is there

    push    ESI                         ; for data buffer in case of error

    call    _bios_port_test             ; tests port like BIOS does
    jc      search_port22               ; port doesn't exist, try next one

    call    _test_box                   ; tests the port
    cmp     EBX, 0                      ; error ?
    je      search_port_ret             ; no error, return ok

search_port22:

    pop     ESI                         ; error: restore ESI for data buffer
    loop    search_port10               ; try next address

//search_port_err:

    stc                                 ; set Carry for error
    pop     EDX                         ; restore original port number
    pop     ECX                         ; restore ECX
    pop     EAX                         ; restore opcode of host routine
    ret

search_port_ret:

    clc                                 ; clear Carry for ok
    pop     EAX                         ; dummy restore to reach ECX (ESI)
    pop     ECX                         ; dummy restore to reach ECX (EDX)
    pop     ECX                         ; restore ECX
    pop     EAX                         ; restore opcode of host routine
    ret

//;=========================================================================
//;
//; Name: _test_port        ---     reads BIOS port number and tests box
//;
//; Input:          ah      command of host routine
//;                 EDX     port number
//;                 ESI     box cmd & id buffer
//;
//; Output:         C       Carry set if error, Carry clear if ok
//;                 EDX     if ok portaddress, if error portnumber
//;
//;=========================================================================

_test_port:

    push    EAX                         ; save opcode of host routine
    push    EDX                         ; save original port number
    push    ECX

    cmp     EDX, 4                      ; port number = 11, 12 or 13 ?
    jb      test_all                    ; port number = 1, 2 or 3, test all
    sub     EDX, 10                     ; create the right port number

    mov     ECX, 1                      ; test only one port
    jmp     test_one                    ; go on testing

test_all:

#ifdef  HACK
    call    _crack_the_hack             ;perform hacker test, NODEBUG
#endif
    mov     ECX, 3                      ; perform mEAX. 3 loops

test_one:

    push    EDX                         ; save port number

test_port10:                            ; test port addresses

    push    EBX                         ; save EBX
    mov     EBX, EDX                    ; port number into EBX

    dec     EBX
    shl     EBX, 2                      ; adjust for dword
    add     EBX, offset LPT_SRC         ; offset for printer ports
    mov     EDX, [EBX]

   #ifdef PG_DEBUG
    mov  port_adr, EDX
    push EBX
    push ECX
    push EDX
    push ESI
    }
    DebugPrintf("Port IO address: %d", port_adr);
    _asm
    {
      pop ESI
      pop EDX
      pop ECX
      pop EBX
   #endif
    pop     EBX                         ; restore EBX

    cmp     EDX, PORT1                  ; it's valid port 1?
    je      test_port20
    cmp     EDX, PORT2                  ; it's valid port 2?
    je      test_port20
    cmp     EDX, PORT3                  ; it's valid port 3?
    je      test_port20

    pop     EDX                         ; not a valid port, try next
    cmp     EDX, 2                      ; was it port1 or port2?
    jbe     test_port11
    mov     EDX, 1                      ; it was port 3, try port 1 now
    jmp     test_port12

test_port11:

    add     EDX,    1                   ; it was port 1 or 2, try next port

test_port12:

    push    EDX                         ; save new port number
    loop    test_port10                 ; try next port

    jmp     test_port_err               ; no valid port address available

test_port20:                            ; test port if box is there

    push    ESI                         ; for data buffer in case of error

    call    _bios_port_test             ; tests port like BIOS does
    jc      test_port22                 ; port doesn't exist, try next one

    call    _test_box                   ; tests the port
    cmp     EBX, 0                      ; error ?
    je      test_port_ret               ; no error, return ok

test_port22:

    pop     ESI                         ; error: restore ESI for data buffer

test_port_err:

    stc                                 ; set Carry for error
    pop     EDX                         ; restore port number
    pop     ECX                         ; restore ECX
    pop     EDX                         ; restore original port number
    pop     EAX                         ; restore opcode of host routine
    ret

test_port_ret:

    clc                                 ; clear Carry for ok
    pop     ECX                         ; dummy restore to reach ECX (ESI)
    pop     ECX                         ; dummy restore to reach ECX (EDX)
    pop     ECX                         ; restore ECX
    pop     EAX                         ; dummy pop to reach EAX (EDX)
    pop     EAX                         ; restore opcode of host routine
    ret

//;=========================================================================
//;
//; Name: _test_box         ---     test box at given port with given ID
//;
//; Input:          ah      command of host routine
//;                 EDX     port address
//;                 ESI     box cmd & id buffer
//;
//; Output:         EBX     BOX_NOT_FOUND if error, 0 if ok
//;
//;=========================================================================

_test_box:

    push    EAX                         ; save opcode of host routine
    push    ECX                         ; save ECX

#ifdef  STUPID
    call    _stupid                      ; perform the stupid macro, NODEBUG
#endif

    call    _box_reset                  ; reset box
    lodsb                               ; get opcode of Get_IEDX from buffer
    mov     ah, al                      ; create real opcode
    add     al, ID_OFFSET
    mov     EBX, 0302h                  ;
    call    _send_cmd                   ; send real opcode
    xor     ch, ch
    mov     cl, bh                      ; put 3 bytes SCodeID

test_box10:

    lodsb                               ; load byte from buffer
    call    _put_byte                   ; send byte
    push    ECX                         ; save loop counter ??!! KR
    mov     ECX, D100                   ; delay for calculating ID
    call    _delay_us
    pop     ECX                         ; restore loop counter
    loop    test_box10                  ; do it 3 times

    mov     EAX, 400                    ; mEAX wait time = 4 ms
    xor     bh, bh                      ; get EBX=bl bytes (block length)
    call    _get_testData               ; do not save the bytes we got
    cmp     EBX, 0                      ; EBX == 0 ?
    je      test_box_ret                ; no error ok

//test_box_err:

    mov     EBX, BOX_NOT_FOUND          ; error
    pop     ECX                         ; restore ECX
    pop     EAX                         ; restore opcode of host routine
    ret

test_box_ret:

    xor     EBX, EBX                    ; no error, EBX = 0
    pop     ECX                         ; restore ECX
    pop     EAX                         ; restore opcode of host routine
    ret


//;=========================================================================
//;
//; Name: _get_nData        ---     reads n bytes data from box
//;
//; Input:          ECX     get ? times
//;                 EBX     get ? bytes
//;                 EAX     mEAX wait time
//;
//;                 EDX     Data port address of printer
//;
//; Output:         EDI     box data or ids buffer
//;                 EBX     error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_get_nData:

    pushf
    //cli
    call    disable

    push    EAX
    push    ECX
    push    EBX

    mov     ECX, EAX                    ; EAX = mEAX wait time
    push    ECX
    mov     ECX, 100                    ; check ACK = 1 100 times

get_nData_10:

    inc     EDX                         ; -+
    call    read_port                   ;  | get ACK
    dec     EDX                         ; -+
    test    al,40h                      ; check ACK is hi ?
    jnz     get_nData_11                ; yes get start bit
    loop    get_nData_10                ; try again
    pop     ECX                         ; restore ECX to be correct
    jmp     get_nData_err               ; sorry it's wrong

get_nData_11:                           ; ** synch start **

    pop     ECX                         ; get mEAX wait time into ECX!!!
    push    EDX                         ; dummy to get a good loop

get_nData_12:

    pop     EDX                         ; restore EDX pushed below
    mov     al,0cfh                     ; reset = 1, clk = 0
    call    write_port                  ; set clk = lo , d5 = 0, d6 = 1
    inc     EDX
    call    read_port                   ; get start bit (ACK = 0)
    dec     EDX
    test    al,40h                      ; start bit ok ?
    jz      get_nData_14                ; yes, perform syncro
    inc     EDX
    call    read_port                   ; check start bit again
    dec     EDX
    test    al,40h                      ; start bit ok ?
    jz      get_nData_14                ; yes, perform syncro

    push    EDX                         ; save port address

    push    ECX                         ; save big loop counter
    mov     ECX, D10                    ;
    call    _delay_us                   ; delay very short, but do it 400 times
    pop     ECX                         ; restore big loop counter

    loop    get_nData_12                ; do the loop 400 times before error
    ; (4 ms)

    pop     EDX                         ; restore EDX in case of error
    jmp     get_nData_err               ; mEAX wait time is up,sorry it's wrong

get_nData_14:

    call    _syncro                     ; syncro ACK = 1010
    cmp     EBX, 0                      ; error in syncro ?
    jnz     get_nData_err               ; error during syncro

    pop     EBX                         ; pop   number of bytes
    push    EBX                         ; push  number of bytes
    mov     ECX, EBX                    ; get EBX bytes

get_nData_20:

    call    _get_byte                   ; get data
    stosb                               ; restore data to stream
    loop    get_nData_20

    mov     al,0bfh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 1, d6 = 0

    pop     EBX
    pop     ECX
    pop     EAX

    dec     cx
    cmp     cx,0
    jbe         done_1
    jmp     _get_nData
    ;loop   _get_nData

done_1:

    xor     EBX, EBX                    ; no error, clear EBX
    popf
    ret

get_nData_err:

    pop     EBX
    pop     ECX
    pop     EAX
    mov     EBX, READ_RAM_ERR
    popf
    ret

//;=========================================================================
//;
//; Name: _get_testData     ---     reads 1 bytes data from box
//;                                   doesn't save the data byte read from box
//; Input:          EAX     mEAX wait time
//;                 EBX     get ? bytes
//;                 EDX     Data port address of printer
//;
//; Output:         al      box data buffer
//;                 EBX     error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_get_testData:

    pushf
    //cli
    call    disable

    push    EAX
    push    ECX
    push    EBX

    mov     ECX, EAX                    ; EAX = mEAX wait time
    push    ECX
    mov     ECX, 100                    ; check ACK = 1 100 times

get_test_10:

    inc     EDX                         ; -+
    call    read_port                   ;  | get ACK
    dec     EDX                         ; -+
    test    al,40h                      ; check ACK is hi ?
    jnz     get_test_11                 ; yes get start bit
    loop    get_test_10                 ; try again
    pop     ECX                         ; restore ECX to be correct
    jmp     get_test_err                ; sorry it's wrong

get_test_11:                            ; ** synch start **

    pop     ECX                         ; get mEAX wait time into ECX!!!

    push    EDX                         ; dummy to get a good loop

get_test_12:

    pop     EDX                         ; restore EDX pushed below
    mov     al,08fh                     ; reset = 1, clk = 0
    call    write_port                  ; set clk = lo, d5 = 0, d6 = 0
    inc     EDX
    call    read_port                   ; get start bit (ACK = 0)
    dec     EDX
    test    al,40h                      ; start bit ok ?
    jz      get_test_14                 ; yes, perform syncro
    inc     EDX
    call    read_port                   ; check start bit again
    dec     EDX
    test    al,40h                      ; start bit ok ?
    jz      get_test_14                 ; yes, perform syncro

    push    EDX                         ; save port address

    push    ECX                         ; save big loop counter
    mov     ECX, D10                    ;
    call    _delay_us                   ; delay very short, but do it 400 times
    pop     ECX                         ; restore big loop counter

    loop    get_test_12                 ; new round to get the start bit

    pop     EDX                         ; restore EDX in case of error
    jmp     get_test_err                ; mEAX wait time is up,sorry it's wrong

get_test_14:

    call    _syncro                     ; syncro ACK = 1010
    cmp     EBX, 0                      ; error in syncro ?
    jnz     get_test_err                ; error during syncro

    pop     EBX                         ; pop   number of bytes
    push    EBX                         ; push  number of bytes
    mov     ECX, EBX                    ; get EBX bytes
get_test_20:

    call    _get_byte                   ; get data
    loop    get_test_20
    push    EAX                         ; save data

    mov     al,0ffh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 1, d6 = 1

    pop     EAX                         ; restore saved data
    pop     EBX
    pop     ECX
    pop     EBX                         ; dummy restore for EAX

    xor     EBX, EBX                    ; no error, clear EBX
    popf
    ret

get_test_err:

    pop     EBX
    pop     ECX
    pop     EAX
    mov     EBX, READ_RAM_ERR
    popf
    ret

//;=========================================================================
//;
//; Name: _bios_port_test   ---     tests the given parallel port address
//;                             like BIOS does
//;
//; Input:          EDX     address of parallel printer port to test
//;
//;
//; output:       Carry     error flag  ( clear : ok  else error )
//;
//;=========================================================================

_bios_port_test:

    push    EAX                         ; save EAX

    mov     al, 55h
    call    write_port                  ; write 0x55 to outport
    call    read_port                   ; read from outport
    cmp     al, 55h                     ; EDId we read the same back?
    jz      bios_port_test_ret          ; yes, it's the same .. port exists

//bios_port_test_err:                     ; the given  port doesn't exist

    stc                                 ; set Carry to inEDIcate the error
    pop     EAX                         ; restore EAX
    ret

bios_port_test_ret:

    clc                                 ; clear Carry to inEDIcate the OK
    pop     EAX                         ; restore EAX
    ret

//;=========================================================================
//;
//; Name: _syncro           ---     syncro routine: get 1010 via ACK
//;
//;                 while CLK = hi data are sent by box
//;                 While CLK = lo data are read by PC
//;
//; output:         EBX      error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_syncro:                                ; start for syncro (1010)

    ; ->4 MHz
    pushf
    //cli                                 ; do not allow interrupt requests
    call    disable

    mov     al,0dfh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 0, d6 = 1
    mov     ECX, D55
    call    _delay_us                   ; delay 55 us for valid data
    mov     al,0efh                     ; reset = 1, clk = 0
    call    write_port                  ; set clk = lo, d5 = 1, d6 = 1
    mov     ECX, D25
    call    _delay_us                   ; delay 25 us to get box ready
    inc     EDX
    call    read_port                   ; get 1. syncro bit (1010)
    test    al,40h                      ; start bit ok ?     ^
    jz      syncro_ack_err              ; sorry it's wrong (maybe again kr)
    dec     EDX
    mov     al,0bfh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 1, d6 = 0
    mov     ECX, D55
    call    _delay_us                   ; delay 55 us for valid data
    mov     al,0cfh                     ; reset = 1, clk = 0
    call    write_port                  ; set clk = lo, d5 = 0, d6 = 1
    mov     ECX, D25
    call    _delay_us                   ; delay 25 us to get box ready
    inc     EDX
    call    read_port                   ; get 2. syncro bit (1010)
    test    al,40h                      ; start bit ok ?      ^
    jnz     syncro_ack_err              ; sorry it's wrong (maybe again kr)
    dec     EDX
    mov     al,09fh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 0, d6 = 0
    mov     ECX, D55
    call    _delay_us                   ; delay 55 us for valid data
    mov     al,0efh                     ; reset = 1, clk = 0
    call    write_port                  ; set clk = lo, d5 = 1, d6 = 1
    mov     ECX, D25
    call    _delay_us                   ; delay 25 us to get box ready
    inc     EDX
    call    read_port                   ; get 3. syncro bit (1010)
    test    al,40h                      ; start bit ok ?       ^
    jz      syncro_ack_err              ; sorry it's wrong (maybe again kr)
    dec     EDX
    mov     al,0bfh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 1, d6 =0
    mov     ECX, D55
    call    _delay_us                   ; delay 55 us for valid data
    mov     al,08fh                     ; reset = 1, clk = 0
    call    write_port                  ; set clk = lo, d5 = 0, d6 = 0
    mov     ECX, D25
    call    _delay_us                   ; delay 25 us to get box ready
    inc     EDX
    call    read_port                   ; get 4. syncro bit (1010)
    test    al,40h                      ; start bit ok ?        ^
    jnz     syncro_ack_err              ; sorry it's wrong (maybe again kr)

    dec     EDX
    xor     EBX, EBX                    ; no error
    mov     al,0dfh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 0, d6 = 1
    popf
    ret

syncro_ack_err:

    mov     EBX, SYNCRO_ERR
    mov     al,0ffh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 1, d6 = 1
    popf
    ret

//;=========================================================================
//;
//; Name: _chk_ack          ---     check ACK = lo and wait for ACK = hi
//;
//;
//; Input:          EAX     mEAX wait time
//;
//; output:         EBX      error flag  ( 0 : ok  else error )
//;
//;=========================================================================

_chk_ack:

    pushf
    //cli                                 ; do not allow interrupt requests
    call    disable
    push    ECX

    mov     ECX, D25                    ; KR test delay (wc 12 us)
    call    _delay_us                   ; delay very short to get box ready

    mov     ECX, EAX                    ; get mEAX wait time into ECX
    push    ECX                         ; save mEAX wait time
    mov     ECX, 100                    ; KR WO try 100 times to get ACK = lo

_chk_ack_04:

    inc     EDX
    call    read_port                   ; get ACK = 0
    dec     EDX
    test    al,40h                      ; ACK = lo ?
    jnz     chk_ack_06                  ; if yes, check again else next round

    inc     EDX
    call    read_port                   ; get ACK = 0
    dec     EDX
    test    al,40h                      ; ACK = lo ?
    jz      chk_ack_08                  ; if yes, wait for ACK = 1 else next round

chk_ack_06:

    nop
    loop    _chk_ack_04                 ; next round to get ACK = lo
    pop     ECX                         ; restore mEAX wait time
    jmp     chk_ack_err                 ; couldn't get ACK = lo, error

chk_ack_08:

    pop     ECX                         ; restore mEAX wait time

chk_ack_10:

    inc     EDX
    call    read_port                   ; get ACK = 1
    dec     EDX
    test    al,40h                      ; ACK = hi ?
    jnz     chk_ack_12                  ; yes, ACK got hi

    push    ECX                         ; save big loop counter
    mov     ECX, D10                    ;
    call    _delay_us                   ; delay very short, but do it EAX times
    pop     ECX                         ; restore big loop counter

    loop    chk_ack_10                  ; do the loop EAX times before error

    jmp     chk_ack_err                 ; mEAX wait time is up, it's wrong

chk_ack_12:

    xor     EBX, EBX                    ; no error, ACK = hi
    pop     ECX
    popf
    ret

chk_ack_err:

    mov     EBX, WR_SP_ACK_ERR          ; ACK was not lo or ACK won't get hi
    pop     ECX
    popf
    ret

//;=========================================================================
//;
//; Name: _get_byte         ---     read a byte from box
//;
//; Input:          EDX     Data port address of printer
//;                 EBX     point addresss of synch timer
//;
//; Output:         al      data
//;
//;=========================================================================

_get_byte:                              ; -> 4 MHz

    pushf
    //cli                                 ; do not allow interrupt requests
    call    disable
    push    ECX
    push    EBX
    mov     ECX,8                       ; get 8 bits

gtbt_10:

    mov     EBX,ECX
    mov     ECX, D55                    ; wait to get data valid
    call    _delay_us                   ; delay 55 us
    mov     al,0cfh                     ; reset = 1, clk = 0
    call    write_port                  ; set clk = lo, d5 = 0, d6 = 1
    mov     ECX, D25
    call    _delay_us                   ; delay 25 us to get box ready
    inc     EDX
    call    read_port                   ; read ACK
    dec     EDX
    mov     ECX,2
    shl     al,cl                       ; set data bit to c flag
    rcl     ah,1                        ; set data bit to ah

    mov     al,0ffh                     ; reset = 1, clk = 1
    call    write_port                  ; set clk = hi, d5 = 1, d6 = 1

    mov     ECX,EBX
    loop    gtbt_10                     ; next bit
    mov     al,ah
    pop     EBX
    pop     ECX
    popf
    ret

//;=========================================================================
//;
//; Name: _put_byte         ---     Write one byte to box
//;
//; Input:          al      data
//;                 EDX     Data port address of printer
//;
//;=========================================================================

_put_byte:

    push    ECX
    push    EAX                         ; kr
    call    _pre_put_byte
    or      al,90h                      ; reset = 1, clk = 1
    or      al,4fh                      ; d3,d2,d1,d0 = 1 , d6 = 1
    call    write_port                  ; set clk = hi, set data bits hi
    mov     ECX, D100                   ; 100us
    call    _delay_us                   ; get data from ROM
    pop     EAX                         ; kr
    pop     ECX
    ret

//;=========================================================================

_pre_put_byte:

    pushf
    //cli                                 ; do not allow interrupt requests
    call    disable

    mov     ah,al                       ; ** put lo nibble ** & backup data
    and     al,0fh                      ; clear high byte of al, preserve lo byte
    or      al,0f0h                     ; reset = 1, clk = 1
    call    write_port                  ; set clk hi 65 us, d5 = 1, d6 = 1
    mov     ECX, D65                    ; 65 us
    call    _delay_us                   ;

    and     al,0cfh                     ; reset = 1, clk = 0, d5 = 0
    call    write_port                  ; set clk lo 65 us , preserve the rest
    mov     ECX, D65                    ; 65 us  ->4MHz
    call    _delay_us

    or      al,0b0h                     ; reset = 1, clk = 1, d5 = 1
    call    write_port                  ; set clk hi for 25 us
    mov     ECX, D25                    ; 25 us
    call    _delay_us

    mov     al,ah                       ; ** put hi nibble **
    mov     ECX,4
    shr     al,cl
    or      al,90h                      ; reset = 1, clk = 1
    call    write_port                  ; set clk hi 25 us,
    mov     ECX, D25                    ; 25us
    call    _delay_us

    and     al,08fh                     ; reset = 1, clk = 0, d5 = 0, d6 = 0
    call    write_port                  ; set clk lo 65 us , preserve the rest
    mov     ECX, D65                    ; set clk lo 65 us
    call    _delay_us
    popf
    ret

//;=========================================================================
//;
//; Name: _send_cmd         ---     Write a cmd byte to box
//;
//; Input:          al      data
//;                 EDX     Data port address of printer
//;
//;=========================================================================

_send_cmd:

    push    ECX
    push    EAX

    mov     ECX, D45                    ; delay to get the box ready for
    call    _delay_us                   ; reaEDIng opcode KR 19.05.94

    call    _pre_put_byte
    or      al,0d0h                     ; reset = 1, clk = 1, d6 = 1
    and     al,0f0h                     ; d3,d2,d1,d0 = 0
    call    write_port                  ; set clk hi 100 us, set data bits lo
    mov     ECX, D100                   ; 100us  ->4MHz
    call    _delay_us

    pop     EAX
    pop     ECX
    ret

#include "cbnstu.c"

    }
  final_exit:

  return((short)ret_val);
        ;
}


/* EOF */