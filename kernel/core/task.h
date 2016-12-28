#ifndef __TIO_OS_TASK_H__
#define __TIO_OS_TASK_H__

#include "stdint.h"

struct Registers {
  uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
};

struct Task {
  struct Registers registers;
  struct Task* next;
};

extern struct Task main_task;

extern void TaskInitialize();

// Helper functions for assembly functions
extern struct Task* TaskGetReadyList();
extern struct Task* TaskGetNext(struct Task* task);

extern void TaskCreate(struct Task* task, void (*main)(), uint32_t flags, uint32_t* page_directory);
extern void TaskPreempt();
extern void TaskContextSwitch(struct Registers* old_reg, struct Registers* new_reg);
extern void TaskSchedule(struct Task* task);
extern bool TaskShouldSchedule();
extern void TaskSetRunningTask(struct Task* new_task);

#endif  //__TIO_OS_TASK_H__