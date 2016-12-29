#include "ring.h"
#include "gdt.h"

static struct TssEntry tss;

void TssFlush(uint16_t tss_selector) {
  __asm__("ltr (%0)" : : "a"(tss_selector));
}

void TssInstall(uint32_t index, uint16_t kernel_segment_selector, uint16_t kernel_esp) {

  uint32_t base = (uint32_t) &tss;
  GdtSetDescriptor(index, base, base + sizeof(struct TssEntry),
    GDT_DESC_ACCESS|GDT_DESC_EXEC_CODE|GDT_DESC_DPL|GDT_DESC_MEMORY,
    0);
  memset((void*) &tss, 0, sizeof(struct TssEntry));
  tss.ss0 = kernel_segment_selector;
  tss.esp0 = kernel_esp;

  TssFlush(idx * sizeof(struct GdtDescriptor));
}
