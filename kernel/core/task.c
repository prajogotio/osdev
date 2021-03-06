#include "task.h"
#include "physical.h"
#include "print.h"
#include "kmalloc.h"
#include "pit.h"
#include "string.h"
#include "virtual.h"
#include "ring.h"

#define ENABLE_TASKING 1

// Both processes are kernel processes, so they share VAS
// Context switch happens in the kernel space
// TODO: User mode? (Ring jumps)

struct Task* running_task;
struct Task main_task;
static int is_initialized_ = 0;
static void TaskCreateIretAndPushadFrame(struct Task* task, uint32_t user_esp);


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
  main_task.privilege_mode = KERNEL_MODE;
  main_task.page_directory_addr = (uint32_t) VmmGetCurrentPageDirectory();
  main_task.next = &main_task;
  running_task = &main_task;
  is_initialized_ = 1;
}

void TaskCreate(struct Task* task, void (*main)(), uint32_t flags, uint32_t* page_directory) {
  // TODO: when user process is implemented, create a new page directory and
  // map user stack and kernel region to the directory.
  task->registers.eax = 0;
  task->registers.ebx = 0;
  task->registers.ecx = 0;
  task->registers.edx = 0;
  task->registers.esi = 0;
  task->registers.edi = 0;
  task->registers.eflags = flags;
  task->registers.eip = (uint32_t) main;
  // Allocate kernel stack.
  // TODO: when user process is implemented, TSS will be used
  // to lead esp0 and ss0, so esp and ebp below should be set
  // to user stack instead.
  char* kernel_stack_page = (char*) kmalloc(4096);
  memset((char*)kernel_stack_page, 0, 4096);
  task->kernel_stack_addr = ((uint32_t)kernel_stack_page) + 0x1000;

  // esp pointing to kernel stack.
  // esp that points to user stack is stored on kernel stack using the
  // IRETD mechanism.
  task->registers.esp = task->kernel_stack_addr;
  task->registers.ebp = task->kernel_stack_addr;
  
  task->registers.cr3 = (uint32_t) page_directory;
  task->page_directory_addr = (uint32_t) VmmGetCurrentPageDirectory();
  task->privilege_mode = KERNEL_MODE;
  task->next = 0;
  task->working_set = 0;      // Working set is not used for kernel processes.
  // Push an IRET and PUSHAD frame
  TaskCreateIretAndPushadFrame(task, 0 /* user space stack, not used */);
}

void TaskCreateUserProcess(struct Task* task, void (*main)(), uint32_t flags) {
  task->registers.eax = 0;
  task->registers.ebx = 0;
  task->registers.ecx = 0;
  task->registers.edx = 0;
  task->registers.esi = 0;
  task->registers.edi = 0;
  // Start from 0x00000000 user address
  task->registers.eip = 0;

  // Assign a kernel stack
  char* kernel_stack_page = (char*) kmalloc(4096);
  memset((char*)kernel_stack_page, 0, 4096);
  task->kernel_stack_addr = ((uint32_t)kernel_stack_page) + 0x1000;

  task->registers.esp = task->kernel_stack_addr;
  task->registers.ebp = task->kernel_stack_addr;
  task->privilege_mode = USER_MODE;
  task->next = 0;

  // Initalize task working set
  task->working_set = (struct WorkingSet*) kmalloc(sizeof(struct WorkingSet));
  WorkingSetInitialize(task->working_set);

  struct pdirectory* user_directory;
  RingInitializeUserDirectory(&user_directory);

  task->registers.cr3 = (uint32_t) VmmGetPhysicalAddress(user_directory);
  task->page_directory_addr = (uint32_t) user_directory;

  // Copy first block of main function to 0x00000000.
  // TODO: handle functions that are larger than that.
  uint32_t user_code = 0x00000000;
  char* user_code_segment = RingTestPageMappingHelper(user_code, user_directory);
  memcpy((char*) main, user_code_segment, 4096);
  WorkingSetInsertPage(task->working_set, user_code);

  // Set user esp to 0x20000000
  uint32_t user_esp = 0x20000000;
  char* user_stack_segment = RingTestPageMappingHelper(user_esp, user_directory);
  WorkingSetInsertPage(task->working_set, user_esp);
  
  // Push an IRET and PUSHAD frame
  TaskCreateIretAndPushadFrame(task, user_esp + 0x1000);
}

static void TaskCreateIretAndPushadFrame(struct Task* task, uint32_t user_esp) {
  // Set up values in kernel stack so that when we call
  // POPAD and IRET, the correct values for the registers
  // are automatically set.

  // Push SS, ESP, EFLAGS, CS, EIP, then PUSHAD
  // For now, we create processes at ring 0 only: SS = 0x10, CS = 0x08
  // ESP is set
  task->registers.esp -= 13*4; // Allocate spaces for IRET and PUSHAD
  uint32_t* esp = (uint32_t*) task->registers.esp;
  // PUSHAD frame
  *esp = task->registers.edi;
  *(esp+1) = task->registers.esi;
  *(esp+2) = task->registers.ebp;
  *(esp+3) = (uint32_t)esp + 4 * 8; // ORIGINAL ESP (points to EIP position in stack)
  *(esp+4) = task->registers.ebx;
  *(esp+5) = task->registers.edx;
  *(esp+6) = task->registers.ecx;
  *(esp+7) = task->registers.eax;

  // IRET FRAME
  *(esp+8) = task->registers.eip;
  *(esp+9) = task->privilege_mode == KERNEL_MODE ? 0x08 : 0x1b;   // CS
  *(esp+10) = task->registers.eflags | 0x200;
  // USER ESP
  *(esp+11) = user_esp;
  *(esp+12) = task->privilege_mode == KERNEL_MODE ? 0x10 /* unused */ : 0x23;
}

struct Task* TaskGetReadyList() {
  return running_task;
}

struct Task* TaskGetNext(struct Task* task) {
  return task->next;
}

uint32_t TaskGetPrivilegeMode(struct Task* task) {
  return task->privilege_mode;
}

uint32_t TaskGetPageDirectoryAddr(struct Task* task) {
  return task->page_directory_addr;
}

uint32_t TaskGetKernelStack(struct Task* task) {
  return task->kernel_stack_addr;
}

struct WorkingSet* TaskGetWorkingSet(struct Task* task) {
  return task->working_set;
}

void TaskSchedule(struct Task* task) {
  __asm__("cli");
  struct Task* next_task = running_task->next;
  task->next = next_task;
  running_task->next = task;
  __asm__("sti");
}

bool TaskShouldSchedule() {
  return is_initialized_ & ENABLE_TASKING;
}


void TaskSetRunningTask(struct Task* new_task) {
  running_task = new_task;
}