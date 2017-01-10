#include "syscall.h"
#include "hal.h"
#include "idt.h"
#include "print.h"

#define MAX_SYSCALL 2


static void SyscallPrintString(uint32_t arg_addr);
static void SyscallTest(uint32_t arg_addr);

static void (*syscall_vector_[]) (uint32_t arg_addr) = {
  SyscallPrintString,
  SyscallTest,
};



void SyscallHandler() {
  __asm__("pusha");
  int syscall_index = -1;
  uint32_t arg_addr = -1;
  __asm__("movl %%eax, %0": "=r"(syscall_index) :);
  __asm__("movl %%ebx, %0": "=r"(arg_addr) :);
  if (syscall_index >= MAX_SYSCALL || syscall_index < 0) {
    PrintString("Syscall Error: Index out of bound: ");
    PrintInt(syscall_index);
    PrintString("\n");
    for(;;);
  }
  void (*function_to_call)() = syscall_vector_[syscall_index];
  function_to_call(arg_addr);
  __asm__("popa");
  __asm__("leave");
  __asm__("iret");
}

void SyscallInitialize() {
  SetUserInterruptVector(0x80, SyscallHandler);
}

static void SyscallPrintString(uint32_t arg_addr) {
  PrintString((char*)arg_addr);
  PrintString("\n");
}

static void SyscallTest(uint32_t arg_addr) {
  PrintString("System call test!\n");
}
