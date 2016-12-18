org 0x7c00
bits 16
jmp loader

%include "print_hex.asm"

loader:
  mov [BootDrive], dl

ResetDisk:
  mov ah, 0
  mov dl, 0
  int 0x13
  jc ResetDisk

  mov ax, 0x1000
  mov es, ax
  mov bx, bx

ReadFromDisk:
  mov ah, 0x02
  mov al, 1
  mov ch, 0
  mov dh, 0
  mov cl, 2
  mov dl, [BootDrive]
  int 0x13

  jc ReadFromDisk

  mov cx, [es:bx]
  call PrintHex

  mov cx, [es:bx]

  cli
  hlt

BootDrive db 0

times 510 - ($-$$) db 0

dw 0xaa55
dw 0xface
times 510 db 0