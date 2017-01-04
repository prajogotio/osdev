[bits 32]
extern PrintHex
extern VmmGetPhysicalAddress
extern VmmSwitchPdirectory
extern RingTestUserFunction

section .text
global RingEnterUserMode

; Assumption: current esp is on kernel stack
; Kernel stack should already be mapped into the cr3 pdirectory

; registers are not preserved

RingEnterUserMode:
  mov ebx, esp          ; [ebx + 4] points to cr3,
                        ; [ebx + 8] points to current_dir
                        ; [ebx + 12] points to user esp

  ; Set segment to 0x23
  cli
  mov ax, 0x23
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  ; load new page directory
  push dword [ebx+8]
  push dword [ebx+4]
  call VmmSwitchPdirectory
  add esp, 8

  push dword 0x23
  push dword [ebx+12]
  pushfd                    ; Use the same flag as kernel.
  push 0x1b                 ; CS for user space is 0x18,
                            ; RPL is 0x3 (ring 3)
                            ; so 0x18 | 0x3 = 0x1b.

  push 0                    ; When IRET is called, we will jump
                            ; to 0x1b : 0x00000000

  iretd

  