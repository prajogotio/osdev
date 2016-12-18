bits 16

org 0x500

jmp main

%include "inc/stdio.inc"
%include "inc/install_gdt.inc"

SIZE_OF_KERNEL_IMAGE dd 512*10

LoadingMsg db "Preparing to load operating system...", 0x0A, 0x0D, 0x00
DiskLoadedMsg db "Disk Loaded!", 0x0A, 0x0D, 0
WelcomeMsg db "    Welcome to TIO OS!!!!!", 0x0A, "tixxo os tio os tio os yoo", 0
main:

  mov si, DiskLoadedMsg
  call Puts16
  cli
  xor ax, ax
  mov ds, ax
  mov es, ax
  mov ax, 0x9000
  mov ss, ax
  mov sp, 0xFFFF
  sti

  mov si, LoadingMsg
  call Puts16

  call InstallGdt

  cli
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  jmp 0x8:ProtectedMode


bits 32
ProtectedMode:
  ; Clear registers and set stack pointer
  mov ax, 0x10
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov esp, 0x90000
  
  ; Enable A20
  mov al, 2
  out 0x92, al

  call ClearScreen32

  mov ebx, WelcomeMsg
  call Puts32

  ; Move Kernel to 0x00100000
  ; from 0x900
  mov ecx, 0

.CopyKernelWhileLoop:
  mov ebx, [0x900 + ecx]
  mov [0x00100000 + ecx], ebx

  inc ecx
  cmp ecx, [SIZE_OF_KERNEL_IMAGE]
  je .CopyKernelDone
  jmp .CopyKernelWhileLoop

.CopyKernelDone:
  ; Jump to Kernel
  mov ebx, [0x00100000]
  jmp 0x00100000

Stop:
  cli
  hlt

%include "inc/Puts32.inc"


times 1024 - ($ - $$) db 0