org 0x7c00
bits 16

jmp loader

%include "boot/inc/stdio.inc"
%include "boot/inc/load_from_disk.inc"
%include "boot/debugger/print_hex.asm"


loader:
  mov si, WelcomeMsg
  call Puts16

  mov [BootDrive], dl

  ; Place sp at a safe location
  mov sp, 0x9000

  ; Load Stage 2 at 0x500 RAM
  mov ax, 0x50
  mov es, ax
  mov bx, 0
  mov al, 3           ; Second stage has size 3 x 512 bytes
  mov cl, 2           ; Second stage starts at the second sector.
  mov dl, [BootDrive]
  call LoadFromDisk

  ; Jump to stage 2
  jmp 0x500

BootDrive   db  0
WelcomeMsg  db  "MBR block loaded. Jumping to Stage 2...", 0

times 510 - ($-$$) db 0
dw 0xaa55             ; Magic number for MBR