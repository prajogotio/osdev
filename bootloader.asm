org 0x7c00
bits 16

jmp loader

%include "inc/stdio.inc"
%include "inc/load_from_disk.inc"

KERNEL_SECTORS db 16

loader:
  mov si, WelcomeMsg
  call Puts16
  mov [BootDrive], dl

  ; Load Stage 2
  mov ax, 0x50
  mov es, ax
  mov bx, 0
  mov al, 2
  mov cl, 2
  mov dl, [BootDrive]
  call LoadFromDisk

  ; Load Kernel at 0x900
  mov ax, 0x90
  mov es, ax
  mov bx, 0
  mov al, [KERNEL_SECTORS]    ; load al sectors.
  mov cl, 4                 ; starting from cl -th sector.
  mov dl, [BootDrive]
  call LoadFromDisk

  ; Jump to stage 2
  jmp 0x500


BootDrive db 0

WelcomeMsg db "Welcome...", 0

times 510 - ($-$$) db 0
dw 0xaa55