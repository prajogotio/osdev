%ifndef __TIO_OS_PUTS32_INC__
%define __TIO_OS_PUTS32_INC__

bits 32

%define VIDEO_MEM 0xB8000
%define COLS 80
%define LINES 25
%define CHAR_ATTRIB 7

Putch32_curX_ dd 0
Putch32_curY_ dd 0

; Put character to monitor
; BL : character to print
Putch32:
  pusha

  xor eax, eax
  mov ecx, COLS*2
  mov al, byte [Putch32_curY_]
  mul ecx
  push eax  ; save result of COLS * 2 * curY

  mov eax, [Putch32_curX_]
  add eax, eax
  pop ecx
  add eax, ecx ; COLS * 2 * curY + 2 * curX

  mov edi, VIDEO_MEM
  add edi, eax

  cmp bl, 0x0A      ; check for newline character
  je  Putch32_MoveToNextRow

  ; Print character
  mov dl, bl
  mov dh, CHAR_ATTRIB
  mov word [edi], dx

  ; Update character
  inc dword [Putch32_curX_]
  cmp dword [Putch32_curX_], COLS
  je Putch32_MoveToNextRow
  jmp Putch32_Done

Putch32_MoveToNextRow:
  mov dword [Putch32_curX_], 0
  inc dword [Putch32_curY_]

Putch32_Done:
  popa
  ret


; Put string
; ebx: address of beginning of string
Puts32:
  pusha
  mov edi, ebx

Puts32_Loop:
  mov dl, byte [ebx]
  or dl, dl
  jz Puts32_Done

  xor ebx, ebx
  mov bl, dl
  call Putch32
  inc edi
  mov ebx, edi
  jmp Puts32_Loop

Puts32_Done:
  mov bx, word [Putch32_curY_]
  shl bx, 8
  add bx, word [Putch32_curX_]
  call MoveCursor

  popa
  ret


; bh = y pos
; bl = x pos
MoveCursor:
  pusha

  xor eax, eax
  mov ecx, COLS
  mov al, bh
  mul ecx         ; y * COLS
  add al, bl      ; y * COLS + x
  mov ebx, eax

  ; set low byte index
  mov al, 0x0f
  mov dx, 0x03d4
  out dx, al

  mov al, bl
  mov dx, 0x03d5
  out dx, al

  ; set high byte index
  xor eax, eax
  mov al, 0x0e
  mov dx, 0x3d4
  out dx, al

  mov al, bh
  mov dx, 0x03d5
  out dx, al

  popa
  ret

ClearScreen32:
  pusha
  cld
  mov edi, VIDEO_MEM
  mov cx, 2000
  mov ah, CHAR_ATTRIB
  mov al, ' '
  rep stosw           ; repeat cx times, and increase edi by 1
                      ; (cld --> direction flag = 0 so edi is increased)
  mov word [Putch32_curX_], 0
  mov word [Putch32_curY_], 0
  popa
  ret

; ecx the number
PrintHex32:
  pusha
  mov bl, ' '
  call Putch32
  mov bl, 'X'
  call Putch32
PrintHex32_While:
  or ecx, ecx

  je PrintHex32_Done

  xor ebx, ebx
  mov bl, cl
  and bl, 0xf
  mov bl, [HexCharacter + bx]
  call Putch32

  shr ecx, 4
  jmp PrintHex32_While

PrintHex32_Done:
  mov bl, 'I'
  call Putch32
  popa
  ret

HexCharacter db "0123456789abcdef", 0
%endif ; __TIO_OS_PUTS32_INC__