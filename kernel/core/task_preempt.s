[bits 32]

extern TaskGetReadyList
extern TaskGetNext
extern TaskSetRunningTask
extern TaskContextSwitch
extern PrintHex

section .text
global TaskPreempt


TaskPreempt:
  call TaskGetReadyList
  ; eax now contains the address to running task 
  mov ebx, eax  ; save to ebx

  push eax
  call TaskGetNext
  add esp, 4

  ; eax now contains the addres of the next task

  push eax
  call TaskSetRunningTask
  add esp, 4

  push eax   ; push second argument next_reg
  push ebx   ; push first argument old_reg
  call TaskContextSwitch
