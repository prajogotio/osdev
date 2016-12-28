[bits 32]

extern TaskShouldSchedule
extern PitIncreaseTickCount
extern TaskPreempt
extern PrintHex
extern PrintString
extern InterruptDone

section .text
global PitIrq

PitIrq:
  pushad
  call PitIncreaseTickCount

  call TaskShouldSchedule
  cmp eax, 0
  je .just_return

  call TaskPreempt

.just_return:
  push 0
  call InterruptDone
  add esp, 4

  popad
  iret