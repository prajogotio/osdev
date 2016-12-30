[bits 32]
extern PrintString
extern PrintHex

section .text
global AtaPioReadOneSector
AtaPioReadOneSector:
  push ebp
  mov ebp, esp
  pushad
  cld

  mov edi, dword [ebp+8]
  mov edx, 0x1f0
  mov ecx, 256
  rep insw

  popad
  leave
  ret

textmsg db "Hello",0x0a,0