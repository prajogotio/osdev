#include "page_directory_entry.h"



void PdeAddAttribute(page_directory_entry* e, uint32_t attribute) {
  *e |= attribute;
}

void PdeDeleteAttribute(page_directory_entry* e, uint32_t attribute) {
  *e &= ~attribute;
}

void PdeSetFrame(page_directory_entry* e, physical_addr addr) {
  *e = (*e & ~PDE_FRAME) | addr;
}

bool PdeIsPresent(page_directory_entry e) {
  return e & PDE_PRESENT;
}

bool PdeIsUser(page_directory_entry e) {
  return e & PDE_USER;
}

bool PdeIs4Mb(page_directory_entry e) {
  return e & PDE_4MB;
}

bool PdeIsWritable(page_directory_entry e) {
  return e & PDE_WRITABLE;
}

physical_addr PdePhysicalFrameNumber(page_directory_entry e) {
  return e & PDE_FRAME;
}

void PdeEnableGlobal(page_directory_entry e) {
  // Not implemented.
}