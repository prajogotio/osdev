#ifndef __TIO_OS_TASK_H__
#define __TIO_OS_TASK_H__

#include "stdint.h"

#define KERNEL_MODE 0
#define USER_MODE   3

struct Registers {
  uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
};

struct Task {
  struct Registers registers;
  uint32_t page_directory_addr; // Virtual address.
  uint32_t privilege_mode;      // Either KERNEL_MODE or USER_MODE.
  uint32_t kernel_stack_addr;   // Unused if task is at KERNEL_MODE.
                                // Virtual address.
  struct Task* next;
};

extern struct Task main_task;

extern void TaskInitialize();

// Helper functions for assembly functions
extern struct Task* TaskGetReadyList();
extern struct Task* TaskGetNext(struct Task* task);
extern uint32_t TaskGetPrivilegeMode(struct Task* task);
extern uint32_t TaskGetPageDirectoryAddr(struct Task* task);
extern uint32_t TaskGetKernelStack(struct Task* task);

extern void TaskCreate(struct Task* task, void (*main)(), uint32_t flags, uint32_t* page_directory);
extern void TaskCreateUserProcess(struct Task* task, void (*main)(), uint32_t flags);

extern void TaskPreempt();
extern void TaskContextSwitch(struct Registers* old_reg, struct Registers* new_reg);
extern void TaskSchedule(struct Task* task);
extern bool TaskShouldSchedule();
extern void TaskSetRunningTask(struct Task* new_task);

#endif  //__TIO_OS_TASK_H__