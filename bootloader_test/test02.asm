org 0x7c00
bits 16

Start:
  xor bx, bx
  mov ah, 0x0e
  mov al, 'A'
  int 0x10
  cli
  hlt

times 510 - ($-$$) db 0
dw 0xAA55