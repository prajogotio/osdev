#include "ring.h"
#include "gdt.h"
#include "string.h"
#include "physical.h"
#include "virtual.h"
#include "kmalloc.h"
#include "print.h"

static struct TssEntry tss;
static char* RingTestPageMappingHelper(virtual_addr addr, struct pdirectory* user_directory);

void TssFlush() {
  __asm__("mov $0x2b, %ax\n\t"
          "ltr %ax\n\t");
}

void TssInstall(uint32_t index, uint16_t kernel_segment_selector, uint32_t kernel_esp) {

  uint32_t base = (uint32_t) &tss;
  GdtSetDescriptor(index, base, base + sizeof(struct TssEntry),
    GDT_DESC_ACCESS|GDT_DESC_EXEC_CODE|GDT_DESC_DPL|GDT_DESC_MEMORY,
    0);
  memset((void*) &tss, 0, sizeof(struct TssEntry));
  tss.ss0 = kernel_segment_selector;
  tss.esp0 = kernel_esp;
  TssFlush();
}


void RingTestUserMode() {
  // Allocate a kernel stack for this user
  // (Note: we will be abandoning our current kernel stack,
  // which means we have memory leak. Let's not care about this
  // right now as we are just testing how to enter user mode)
  kmalloc(4096);
  uint32_t kernel_esp = (uint32_t) kmalloc(4096);
  memset((char*) kernel_esp, 0, 4096);
  kernel_esp += 0x1000;
  TssInstall(5, 0x10, kernel_esp);
  
  // // User page directory
  struct pdirectory* user_directory = (struct pdirectory*) kmalloc(4096);
  memset((char*) user_directory, 0, 4096);

  uint32_t directory_physical_addr = (uint32_t) VmmGetPhysicalAddress(user_directory);
  // Recursive mapping
  page_directory_entry* self_entry = (page_directory_entry*) VmmPdirectoryLookupEntry(user_directory, 0xFFC00000);
  PdeAddAttribute(self_entry, PDE_PRESENT);
  PdeAddAttribute(self_entry, PDE_WRITABLE);
  PdeSetFrame(self_entry, directory_physical_addr);

  // Kernel space mapping
  // We can just copy the page directory entries of the kernel
  // page directory from 0xc00 onwards to just before 0xffc
  for (int i = 768; i < 1023; ++i) {
    uint32_t vd = ((uint32_t) i) * 0x400000;
    page_directory_entry* kernel_pdir_entry = VmmPdirectoryLookupEntry(VmmGetCurrentPageDirectory(), vd);
    if (!(*(uint32_t*)kernel_pdir_entry & 1)) continue;
    page_directory_entry* user_pdir_entry = VmmPdirectoryLookupEntry(user_directory, vd);
    memcpy((char*) kernel_pdir_entry, (char*) user_pdir_entry, 4);
  }


  uint32_t user_code = 0x00000000;
  char* user_code_segment = RingTestPageMappingHelper(user_code, user_directory);

  // Set user esp to 0x20000000
  uint32_t user_esp = 0x20000000;
  char* user_stack_segment = RingTestPageMappingHelper(user_esp, user_directory);

  memcpy((char*) RingTestUserFunction, user_code_segment, 4096);
  
  PrintString("Entering user mode...");
  RingEnterUserMode((uint32_t) user_directory, (uint32_t) VmmGetPhysicalAddress(user_directory), (uint32_t)(user_esp+0x1000));
}


static char* RingTestPageMappingHelper(virtual_addr addr, struct pdirectory* user_directory) {
  char* user_segment = (char*) kmalloc(4096);
  memset((char*)user_segment, 0x0, 4096);
  page_directory_entry* user_dir_entry = (page_directory_entry*) VmmPdirectoryLookupEntry(user_directory, addr);
  void* user_pt = (void*) kmalloc(4096);
  memset((char*)user_pt, 0x0, 4096);
  PdeSetFrame(user_dir_entry, (uint32_t) VmmGetPhysicalAddress(user_pt));
  PdeAddAttribute(user_dir_entry, PDE_PRESENT);
  PdeAddAttribute(user_dir_entry, PDE_WRITABLE);
  PdeAddAttribute(user_dir_entry, PDE_USER);
  void* user_pte = (void*) VmmPtableLookupEntry(user_pt, addr);
  PteSetFrame(user_pte, (uint32_t) VmmGetPhysicalAddress(user_segment));
  PteAddAttribute(user_pte, PTE_PRESENT);
  PteAddAttribute(user_pte, PTE_WRITABLE);
  PteAddAttribute(user_pte, PTE_USER);
  return user_segment;
}

void RingTestUserFunction() {
  // We are in user mode, looping!
  for (;;);
}