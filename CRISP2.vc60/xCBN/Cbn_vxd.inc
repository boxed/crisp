;****************************************************************************
;                                                                           *
; THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY     *
; KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE       *
; IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
; PURPOSE.                                                                  *
;                                                                           *
; Copyright (C) 1993-95  Microsoft Corporation.  All Rights Reserved.       *
;                                                                           *
;****************************************************************************

PAGE 58,132
;******************************************************************************
TITLE CBN.ASM - Windows/386 Configuration Manager DLVxD CBN
;******************************************************************************
;
;   Title:  CBN.ASM - Windows/386 Config Manager DLVxD CBN
;
;   Version:    1.00
;
;   Date:   21-Mar-1993
;
;==============================================================================

;******************************************************************************
;               D E F I N E S
;******************************************************************************

SUCCESS                equ 1
FAILURE                equ 0

W32_SUCCESS            equ 0
W32_FAILURE            equ 1

;******************************************************************************
;               S T R U C T U R E S
;******************************************************************************

shared_data_format     struc
InstanceIndex          dd  ?
hWnd                   dd  ?
msg                    dw  ?
wParam                 dw  ?
lParam                 dd  ?
PostMessageAddr        dd  ?
shared_data_format     ends

DIOCParams             struc
Internal1              dd ?
VMHandle               dd ?
Internal2              dd ?
dwIoControlCode        dd ?
lpvInBuffer            dd ?
cbInBuffer             dd ?
lpvOutBuffer           dd ?
cbOutBuffer            dd ?
lpcbBytesReturned      dd ?
lpoOverlapped          dd ?
hDevice                dd ?
tagProcess             dd ?
DIOCParams             ends

; end of file
