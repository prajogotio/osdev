%ifndef __TIO_OS_STDIO_INC__
%define __TIO_OS_STDIO_INC__

bits 16

Puts16:
  pusha

.LoopPuts16:
  lodsb
  or al, al
  jz Puts16Done
  mov ah, 0x0e
  int 0x10
  jmp .LoopPuts16

Puts16Done:
  popa
  ret

%endif ;__TIO_OS_STDIO_INC__