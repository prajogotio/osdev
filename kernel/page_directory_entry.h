#ifndef __TIO_OS_PDE_H__
#define __TIO_OS_PDE_H__

#include "stdint.h"
#include "physical.h"

typedef uint32_t page_directory_entry;

enum PAGE_PDE_FLAGS {
  PDE_PRESENT       = 1,
  PDE_WRITABLE      = 2,
  PDE_USER          = 4,
  PDE_PWT           = 8,
  PDE_PCD           = 0x10,
  PDE_ACCESSED      = 0x20,
  PDE_DIRTY         = 0x40,
  PDE_4MB           = 0x80,
  PDE_CPU_GLOBAL    = 0x100,
  PDE_LV4_GLOBAL    = 0x200,
  PDE_FRAME         = 0x7FFFF000
};

extern void PdeAddAttribute(page_directory_entry* e, uint32_t attribute);
extern void PdeDeleteAttribute(page_directory_entry* e, uint32_t attribute);
extern void PdeSetFrame(page_directory_entry*, physical_addr);
extern bool PdeIsPresent(page_directory_entry e);
extern bool PdeIsUser(page_directory_entry);
extern bool PdeIs4Mb(page_directory_entry);
extern bool PdeIsWritable(page_directory_entry);
extern physical_addr PdePhysicalFrameNumber(page_directory_entry);
extern void PdeEnableGlobal(page_directory_entry);

#endif  //__TIO_OS_PDE_H__