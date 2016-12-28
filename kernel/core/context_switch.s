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
; , *old, *new, RET
; 
; Update old_reg first
  mov eax, [esp+4]   ; Address of old_reg

  ; update old_reg->eip  idx 32
  mov ebx, [esp+48]
  mov [eax+32], ebx

  ; update old_reg->eax  idx 0
  mov ebx, [esp+44]
  mov [eax], ebx

  ; update old_reg->ecx  idx 8
  mov ebx, [esp+40]
  mov [eax+8], ebx

  ; update old_reg->edx  idx 12
  mov ebx, [esp+36]
  mov [eax+12], ebx

  ; update old_reg->ebx  idx 4
  mov ebx, [esp+32]
  mov [eax+4], ebx

  ; update old_reg->esp  idx 24
  ; move esp to just at EIP [for now we ignore privilege switch]
  mov ebx, esp
  add ebx, 48
  mov [eax+24], ebx

  ; update old_reg->ebp  idx 28
  mov ebx, [esp+24]
  mov [eax+28], ebx

  ; update old_reg->esi  idx 16
  mov ebx, [esp+20]
  mov [eax+16], ebx

  ; update old_reg->edi  idx 20
  mov ebx, [esp+16]
  mov [eax+20], ebx

  ; update old_reg->eflags   36
  mov ebx, [esp+56]
  mov [eax+36], ebx

  ; update old_reg->cr3      40
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
  ; load ebx from new_reg->ebx
  mov ebx, [eax + 4]
  ; load ecx from new_reg->ecx
  mov ecx, [eax + 8]
  ; load edx from new_reg->edx
  mov edx, [eax + 12]
  ; load esi from new_reg->esi
  mov esi, [eax + 16]  
  ; load edi from new_reg->edi
  mov edi, [eax + 20]
  ; load esp from new_reg->esp      ; Important: set ESP to point to where
  mov esp, [eax + 24]               ; IRET should be called
  ; load ebp from new_reg->ebp
  mov ebp, [eax + 28]

  ; Finally push eax to stack
  push dword [eax]

  ; load eax value to eax
  pop eax

  ; Calling InterruptDone(0) (IRQ 0)
  push 0
  call InterruptDone
  add esp, 4

  iret
