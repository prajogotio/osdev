[bits 32]

extern PrintString
extern PrintHex
extern InterruptDone
extern TaskGetPrivilegeMode
extern TaskGetPageDirectoryAddr
extern TaskGetKernelStack
extern TssUpdateStack
extern current_directory_

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

  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;;;;;;;; STORE CURRENT CONTEXT ;;;;;;;;;;;;;;;;;;;;;;;;;

  mov eax, [esp+4]   ; Address of old_reg

  ; update old_reg->esp  idx 24
  ; move esp to EDI position on stack (i.e. after stack pointer after initial PUSHAD)
  lea ebx, [esp+16]       ; Address where POPAD should start when we switch
  mov [eax+24], ebx       ; back.

  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ;;;;; UPDATE CURRENT CONTEXT ;;;;;;;;;;;;;;;;;;;;;;;;;

  ; Now, load new_reg to registers
  mov eax, [esp+8]      ; Getting address to new_reg

  ; Load the new page directory
  mov ebx, eax          ; store new_reg to ebx, because eax
                        ; will be overwritten when we call helper functions
  ; Retrieve current page directory address
  push ebx
  call TaskGetPageDirectoryAddr
  add esp, 4
  ; eax now contains the current page directory address

  mov dword [current_directory_], eax
  mov eax, dword [ebx+40]             ; This is the same as task->registers.cr3
  mov cr3, eax

  ; Writing to cr3 will flush the TLB
  ; In effect, we enter a new VAS
  ; However, since kernel space is mapped the same way on every process,
  ; (i.e. kernel code, kernel stack and kernel heap are mapped to the same
  ; physical address on every process VAS) the addresses referenced
  ; by esp is still valid.
  ; In other words, ebx still points to a valid address of the Register
  ; structure of the new process  
  ; load esp from new_reg->esp      ; Important: set ESP to point to where
  mov esp, [ebx + 24]               ; IRET should be called
  ; Now esp is pointing at the kernel stack of the new process

  ; Get kernel stack
  push ebx
  call TaskGetKernelStack
  add esp, 4
  ; eax now contains the address to the kernel stack (esp0)
  ; Update TSS entry
  push eax      ; esp0 (the start address of kernel stack)
  push 0x10     ; ss0 (always KERNEL SS)
  call TssUpdateStack
  add esp, 8

  ; Update segment registers
  ; Check if kernel or user space
  push ebx
  call TaskGetPrivilegeMode
  add esp, 4

  cmp eax, 0
  je .KernelSegment

.UserSegment:
  mov ax, 0x23
  mov es, ax
  mov ds, ax
  mov fs, ax
  mov gs, ax
  jmp .SetSegmentDone
.KernelSegment:
  mov ax, 0x10
  mov es, ax
  mov ds, ax
  mov fs, ax
  mov gs, ax
.SetSegmentDone:
  
  ; Calling InterruptDone(0) (IRQ 0)
  push 0
  call InterruptDone
  add esp, 4

  ; Return as per normal
  popad
  iret
