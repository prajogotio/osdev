org 0x7c00
bits 16

jmp loader

%include "boot/inc/stdio.inc"
%include "boot/inc/load_from_disk.inc"
%include "boot/debugger/print_hex.asm"

KERNEL_SECTORS            dd 52
KERNEL_STARTING_SECTOR    dd 5
KERNEL_COPY_BASE          dd 0x1000

total_head                db 0
sectors_per_track         db 0

loader:
  mov si, WelcomeMsg
  call Puts16
  mov [BootDrive], dl

  ; Place sp at a safe location
  mov sp, 0x9000


  ; Get sector_per_track and total_head
  mov ah, 8
  mov dl, byte [BootDrive]
  int 0x13

  mov [total_head], dh      ; number of heads -1
  and cl, 0x3f
  mov [sectors_per_track], cl

  ; Load Stage 2
  mov ax, 0x50
  mov es, ax
  mov bx, 0
  mov al, 3
  mov cl, 2
  mov dl, [BootDrive]
  call LoadFromDisk


  mov si, WelcomeMsg
  call Puts16

  ; Load Kernel at KERNEL_COPY_BASE
  mov ax, [KERNEL_COPY_BASE]
  shr ax, 4
  mov es, ax
  mov bx, [KERNEL_COPY_BASE]
  and bx, 0xf
  mov al, [KERNEL_SECTORS]          ; load al sectors.
  mov cl, [KERNEL_STARTING_SECTOR]  ; starting from cl -th sector.
  mov dl, [BootDrive]
  call LoadFromDisk


  ; Send kernel copy info to over to stage 2
  mov eax, [KERNEL_COPY_BASE]
  push eax
  mov eax, [KERNEL_SECTORS]
  push eax


  ; Jump to stage 2
  jmp 0x500


BootDrive db 0

WelcomeMsg db "Welcome...", 0

times 510 - ($-$$) db 0
dw 0xaa55