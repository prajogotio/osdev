#include "syscall.h"
#include "hal.h"
#include "idt.h"
#include "print.h"

#define MAX_SYSCALL 2


static void SyscallTest();
static void SyscallTestTwo();

static void* syscall_vector_[] = {
  SyscallTest,
  SyscallTestTwo
};



void SyscallHandler() {
  __asm__("pusha");
  int syscall_index = -1;
  __asm__("movl %%eax, %0": "=r"(syscall_index) :);
  if (syscall_index >= MAX_SYSCALL || syscall_index < 0) {
    PrintString("Syscall Error: Index out of bound\n");
    for(;;);
  }
  void (*function_to_call)() = syscall_vector_[syscall_index];
  function_to_call();
  __asm__("popa");
  __asm__("leave");
  __asm__("iret");
}

void SyscallInitialize() {
  SetUserInterruptVector(0x80, SyscallHandler);
}

static void SyscallTest() {
  PrintString("System call!\n");
}

static void SyscallTestTwo() {
  PrintString("Another system call!\n");
}
