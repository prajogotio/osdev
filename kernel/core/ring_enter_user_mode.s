[bits 32]
extern PrintHex
extern VmmGetPhysicalAddress

section .text
global RingEnterUserMode

; Assumption: current esp is on kernel stack
; Kernel stack should already be mapped into the cr3 pdirectory

; registers are not preserved

RingEnterUserMode:
  mov ebx, esp          ; [ebx + 4] points to cr3,
                        ; [ebx + 8] points to user esp

  ; Set segment to 0x23
  cli
  mov ax, 0x23
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax


  ; load new page directory
  mov eax, [ebx + 4]
  mov cr3, eax

  push dword 0x23
  push dword [ebx + 8]      ; User ESP.
  pushfd                    ; Use the same flag as kernel.
  push dword 0x1b           ; CS for user space is 0x18,
                            ; RPL is 0x3 (ring 3)
                            ; so 0x18 + 0x3 = 0x1b.
  push 0x00400000           ; When IRET is called, we will jump
                            ; to 0x1b : 0x00000000

  iretd
