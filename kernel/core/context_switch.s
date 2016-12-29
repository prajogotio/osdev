[bits 32]

extern PrintString
extern PrintHex
extern InterruptDone

section .text
global TaskContextSwitch

; Implementing void TaskContextSwitch(struct Registers* old_reg, struct Registers* new_reg)
; Set up the values in old_reg and new_reg as if IRET is just going to be
; called
;            0    4    8    12   16   20   24   28   32   36      40
; Registers: eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3

TaskContextSwitch:
; Now stack contains:
;  64    60      56  52   48   44   40   36   32        28   24   20   16   12
; SS?, ESP?, EFLAGS, CS, EIP, eax, ecx, edx, ebx, orig_esp, ebp, esi, edi, RET,
;      8     4    0
; , *new, *old, RET
; 
; Update old_reg first
  mov eax, [esp+4]   ; Address of old_reg

  ; update old_reg->esp  idx 24
  ; move esp to EDI position on stack (i.e. after stack pointer after initial PUSHAD)
  lea ebx, [esp+16]
  mov [eax+24], ebx

  ; store old_reg->cr3      40
  mov ebx, cr3
  mov [eax+40], ebx


  ; Now, load new_reg to registers
  mov eax, [esp+8]

  ; Load the new page directory
  mov ebx, [eax+40]
  mov cr3, ebx

  ; Writing to cr3 will flush the TLB
  ; In effect, we enter a new VAS
  ; However, since kernel space is mapped the same way on every process,
  ; (i.e. kernel code, kernel stack and kernel heap are mapped to the same
  ;  physical address on every process VAS) the addresses referenced
  ; by esp is still valid.
  ; In other words, eax still points to a valid address of the Register structure of the new process  
  ; load esp from new_reg->esp      ; Important: set ESP to point to where
  mov esp, [eax + 24]               ; IRET should be called
  ; Now esp is pointing at the kernel stack of the new process

  ; Calling InterruptDone(0) (IRQ 0)
  push 0
  call InterruptDone
  add esp, 4

  ; Return as per normal
  popad
  iret
