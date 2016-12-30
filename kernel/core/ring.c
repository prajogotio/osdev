#include "ring.h"
#include "gdt.h"
#include "string.h"
#include "physical.h"
#include "virtual.h"
#include "kmalloc.h"

static struct TssEntry tss;

void TssFlush(uint16_t tss_selector) {
  __asm__("ltr %%ax" : : "a"(tss_selector));
}

void TssInstall(uint32_t index, uint16_t kernel_segment_selector, uint16_t kernel_esp) {

  uint32_t base = (uint32_t) &tss;
  GdtSetDescriptor(index, base, base + sizeof(struct TssEntry),
    GDT_DESC_ACCESS|GDT_DESC_EXEC_CODE|GDT_DESC_DPL|GDT_DESC_MEMORY,
    0);
  memset((void*) &tss, 0, sizeof(struct TssEntry));
  tss.ss0 = kernel_segment_selector;
  tss.esp0 = kernel_esp;

  TssFlush(index * sizeof(struct GdtDescriptor));
}


void RingTestUserMode() {
  // Allocate a kernel stack for this user
  // (Note: we will be abandoning our current kernel stack,
  // which means we have memory leak. Let's not care about this
  // right now as we are just testing how to enter user mode)
  uint32_t kernel_esp = (uint32_t) kmalloc(4096);

  // Create a new page directory
}