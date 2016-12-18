org 0x7c00
bits 16

start:  jmp loader

Print:
  mov ah, 0x0e
  mov al, [bx]
  int 0x10
  inc bx
  cmp byte [bx], 0
  je PrintDone
  jmp Print

PrintDone:
  ret


msg db "Welcome to My Operating System!", 0


loader:
  mov bx, msg
  call Print
  cli
  hlt

times 510 - ($-$$) db 0
dw 0xAA55