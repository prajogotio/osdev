printhex_charset db "0123456789abcdef", 0
printhex_output db "0x0000", 0
PrintHex:
  pusha
  ; bx is the indexing register
  ; cx is the number

  mov dx, 4

PrintHexWhile:
  mov bx, 0xf
  and bx, cx
  add bx, printhex_charset
  mov ax, [bx]              ; Stores the corresponding character.
  mov bx, printhex_output
  add bx, dx
  add bx, 1
  mov [bx], al              ; Sets the character to the correct position.

  dec dx
  jz PrintHexDisplay
  shr cx, 4
  jmp PrintHexWhile

PrintHexDisplay:
  mov bx, printhex_output
PrintHexDisplayWhile:
  mov ah, 0x0e
  mov al, [bx]
  int 0x10

  inc bx
  cmp byte [bx], 0
  je PrintHexFinish
  jmp PrintHexDisplayWhile

PrintHexFinish:
  popa
  ret
