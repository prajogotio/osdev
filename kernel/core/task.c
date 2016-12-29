#include "task.h"
#include "physical.h"
#include "print.h"
#include "kmalloc.h"
#include "pit.h"

// Both processes are kernel processes, so they share VAS
// Context switch happens in the kernel space
// TODO: User mode? (Ring jumps)

static struct Task* running_task;
struct Task main_task;
static void TaskCreateIretFrame(uint32_t* esp, uint32_t eflags, uint32_t eip);



void TaskInitialize() {
  // Getting CR3 (page directory)
  __asm__ __volatile__(
    "movl %%cr3, %%eax\n\t"
    "movl %%eax, %0\n\t" : "=r"(main_task.registers.cr3):);

  // Getting EFLAGS
  __asm__ __volatile__(
    "pushfl\n\t"
     "movl (%%esp), %%eax\n\t"
     "movl %%eax, %0\n\t"
     "popfl\n\t" : "=r"(main_task.registers.eflags)
  );

  main_task.next = &main_task;
  running_task = &main_task;
}

void TaskCreate(struct Task* task, void (*main)(), uint32_t flags, uint32_t* page_directory) {
  task->registers.eax = 0;
  task->registers.ebx = 0;
  task->registers.ecx = 0;
  task->registers.edx = 0;
  task->registers.esi = 0;
  task->registers.edi = 0;
  task->registers.eflags = flags;
  task->registers.eip = (uint32_t) main;
  // Allocate a kernel stack.
  task->registers.ebp = ((uint32_t) kmalloc(4096)) + 0x9ff;
  task->registers.esp = task->registers.ebp-5*32;
  // Push an IRET frame
  TaskCreateIretFrame((uint32_t*) task->registers.esp, flags, task->registers.eip);
  task->registers.cr3 = (uint32_t) page_directory;
  task->next = 0;
}

static void TaskCreateIretFrame(uint32_t* esp, uint32_t eflags, uint32_t eip) {
  // Push SS, ESP, EFLAGS, CS, EIP
  // For now, we create processes at ring 0 only: SS = 0x10, CS = 0x08
  // ESP is set

  *esp = eip;
  *(esp+1) = 0x08;
  *(esp+2) = eflags;
  *(esp+3) = (uint32_t) esp;      // Not used for IRET to same
  *(esp+4) = 0x10;                // privilege level.
}

struct Task* TaskGetReadyList() {
  return running_task;
}

struct Task* TaskGetNext(struct Task* task) {
  return task->next;
}

void TaskSchedule(struct Task* task) {
  __asm__("cli");
  struct Task* next_task = main_task.next;
  task->next = next_task;
  main_task.next = task;
  __asm__("sti");
}

bool TaskShouldSchedule() {
  return 1;
}


void TaskSetRunningTask(struct Task* new_task) {
  running_task = new_task;
}