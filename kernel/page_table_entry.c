#include "page_table_entry.h"


void PteAddAttribute(pagetable_entry* e, uint32_t attribute) {
  *e |= attribute;
}

void PteDeleteAttribute(pagetable_entry* e, uint32_t attribute) {
  *e &= ~attribute;
}

void PteSetFrame(pagetable_entry* e, physical_addr addr) {
  *e = (*e & ~PTE_FRAME) | addr;
}

bool PteIsPresent(pagetable_entry e) {
  return e & PTE_PRESENT;
}

bool PteIsWritable(pagetable_entry e) {
  return e & PTE_WRITABLE;
}

physical_addr PtePhysicalFrameNumber(pagetable_entry e) {
  return e & PTE_FRAME;
}