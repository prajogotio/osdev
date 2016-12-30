bits 16

org 0x500

jmp main

%define KERNEL_PROTECTED_BASE 0xc0100000

%include "boot/inc/stdio.inc"
%include "boot/inc/install_gdt.inc"
%include "boot/debugger/print_hex_32.asm"

%define KERNEL_SECTORS        255
%define KERNEL_START_SECTOR   0x4

LoadingMsg           db "Entering Protected Mode", 0x0A, 0x0D, 0x00
SecondStageLoadedMsg db "Second stage bootloader loaded!", 0x0A, 0x0D, 0
WelcomeMsg           db "Loading kernel image and enabling paging.", 0x0A, "Setting up...", 0x0A, 0
KernelLoadedMsg      db 0x0A, "Kernel image loaded.", 0x0A, 0

main:
  ; Initialization of stage 2.
  mov si, SecondStageLoadedMsg
  call Puts16
  
  ; Initialize segment registers and stack pointer.
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

  ; Get memory map
  push 0
  pop es
  mov di, memory_map_table

  call GetMemoryMap

  ; Going to protected mode
  mov si, LoadingMsg
  call Puts16

  ; Install GDT for protected mode
  call InstallGdt

  ; Enable protected mode
  cli
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  ; Long jump to set cs register to the correct segment selector (0x8).
  jmp 0x8:ProtectedMode


bits 32
ProtectedMode:
  ; Clear registers and set stack pointer
  mov ax, 0x10
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov esp, 0x90000
  
  call ClearScreen32

  mov ebx, WelcomeMsg
  call Puts32

  ; Enable A20 to enable 4GB addressing
  mov al, 2
  out 0x92, al

  ; Install temporary page directory and paging (identity mapping and 3GB virtual to 0x00100000 mapping)
  call EnablePaging

  ; Load kernel to KERNEL_PROTECTED_BASE virtual (which is 0x00100000 physical)
  ; Load up to 255 sectors first
  ; Kernel image starts at 5-th sector on disk.
  ; Hence LBA = 4 (LBA stands for logical block addressing, each block is 512 bytes)
  mov cl, KERNEL_SECTORS
  mov eax, KERNEL_START_SECTOR
  mov edi, KERNEL_PROTECTED_BASE
  call AtaPioLbaRead

  mov ebx, KernelLoadedMsg
  call Puts32

  ; Update kernel_size information
  mov dword [kernel_size], KERNEL_SECTORS * 512

  ; Jump to Kernel
  ; Set address of memory info table to ebx
  mov ebx, kernel_memory_table

  mov ax, 0x10
  mov es, ax
  mov ds, ax
  mov fs, ax
  mov gs, ax

  mov esp, 0xc0300000
  mov ebp, esp
  
  jmp 0x8:KERNEL_PROTECTED_BASE


%include "boot/inc/Puts32.inc"
%include "boot/inc/memory_map.inc"
%include "boot/inc/paging.inc"
%include "boot/inc/read_from_disk_protected_mode.inc"

kernel_copy_base
  dd 0
kernel_memory_table:
kernel_size:
  dd 0
memory_information:
  dd 0
  dd 0
  dd 0
  dd 0
memory_map_table:


times 1536 - ($ - $$) db 0