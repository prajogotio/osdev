bits 16

org 0x500

jmp main

%include "inc/stdio.inc"
%include "inc/install_gdt.inc"
%include "bootloader_test/print_hex.asm"

LoadingMsg db "Preparing to load operating system...", 0x0A, 0x0D, 0x00
DiskLoadedMsg db "Disk Loaded!", 0x0A, 0x0D, 0
WelcomeMsg db "    Welcome to TIO OS!!!!!", 0x0A, "tixxo os tio os tio os yoo", 0

main:

kernel_size:
  dd 0

  ; Retrieve kernel_size = 512 * kernel_sector stored in stack
  pop eax
  mov ecx, 512
  mul ecx
  mov [kernel_size], eax


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


  ; Getting memory information
  xor eax, eax
  xor ebx, ebx
  xor ecx, ecx
  xor edx, edx
  mov ax, 0xe801
  int 0x15

memory_information:
  dd 0
  dd 0
  dd 0
  dd 0

  ; Place memory information into the above structure
  push edx    ; Configured Mem > 16MB, in 64KB blocks
  push ecx    ; Configured Mem 1MB - 16MB in KB
  push ebx    ; Extended Mem > 16MB, in 64KB blocks
  push eax    ; Extended Mem 1MB - 16MB in KB

  mov ebx, memory_information
  pop eax
  mov dword [ebx], eax
  pop eax
  mov dword [ebx+4], eax
  pop eax
  mov dword [ebx+8], eax
  pop eax
  mov dword [ebx+12], eax


  ; Going to protected mode
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
  cmp ecx, [kernel_size]
  je .CopyKernelDone
  jmp .CopyKernelWhileLoop

.CopyKernelDone:
  ; Jump to Kernel
  ; Set address of memory_info to ebx
  mov ebx, memory_information
  jmp 0x00100000

Stop:
  cli
  hlt

%include "inc/Puts32.inc"


times 1024 - ($ - $$) db 0