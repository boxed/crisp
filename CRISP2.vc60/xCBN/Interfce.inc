IFDEF LARGE_MOD
    .MODEL LARGE; Test wie gro· Large-DLL ohne IDEA-Anpassung!!!
ELSE
IFDEF MEDIUM_MOD
    .MODEL MEDIUM
ELSE
    .MODEL SMALL; JK ACHTUNG!!! ZeigerÅbergabe aber mit FAR
ENDIF
ENDIF
    .CODE
;****************************************************************************
;   File:       cb560.asm
;   Funktion:   Aufruf des BOXIO
;
;   Aufruf:     CB560(int iOpCode, int iPortNr, char *pcInData,
;                                               char *pcOutData)
;   Return:     ax , enthÑlt bx (error flag)
;****************************************************************************
;   Lage der Parameter auf dem Stack
;   [bp+0]  gesicherter bp
;   [bp+2]  RÅcksprungadresse
;   [bp+4]  leftmost argument from C (2 Byte wegen Small Model)
;   alles + 2 fÅr LARGE-Test
;****************************************************************************
IFDEF LARGE_MOD
OPC         equ     [bp+6]              ; int OpCode
PORTNR      equ     [bp+8]              ; int PortNr
INDEX       equ     [bp+0ah]            ; FAR Pointer command data
VALUE       equ     [bp+0eh]            ; FAR Pointer return data
ELSE
IFDEF MEDIUM_MOD
OPC         equ     [bp+6]              ; int OpCode
PORTNR      equ     [bp+8]              ; int PortNr
INDEX       equ     [bp+0ah]            ; FAR Pointer command data
VALUE       equ     [bp+0eh]            ; FAR Pointer return data
ELSE
OPC         equ     [bp+4]              ; int OpCode
PORTNR      equ     [bp+6]              ; int PortNr
INDEX       equ     [bp+8]              ; FAR Pointer command data
VALUE       equ     [bp+0ch]            ; FAR Pointer return data
ENDIF
ENDIF
        PUBLIC _CB560
_CB560     proc near
        push bp
        mov  bp,sp
        push ds                         ; sichern der verwendeten Register
        push es
        push si
        push di
        push bx
        push cx
        push dx

        xor  ax,ax                      ; lîschen von ax
        xor  bx,bx

        les  di, VALUE                  ; es:di command data pointer
        lds  si, INDEX                  ; ds:si return data pointer
        mov  dx, PORTNR                 ; dx    printer port number
        mov  ax, OPC                    ;
        xchg al,ah                      ; ah    OpCode

        call _box_io                    ; Box rufen

        cld                             ; Direction Flag wieder auf forwÑrts

        mov  ax, bx                     ; error flag aus bx nach ax als

        pop  dx                         ; verwendete Register restaurieren
        pop  cx
        pop  bx
        pop  di
        pop  si
        pop  es
        pop  ds
        pop  bp
        ret                             ; Return-Wert

_CB560  endp

;EOF
