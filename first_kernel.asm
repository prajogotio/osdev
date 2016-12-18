bits 32

org 0x00100000
jmp main

%include "inc/Puts32.inc"

WelcomeKernelMsg db "Tio Kernel is HERE", 0

main:
  call ClearScreen32
  mov ebx, WelcomeKernelMsg
  call Puts32

  cli
  hlt

times 1024 - ($ - $$) db 0