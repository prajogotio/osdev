#ifndef __TIO_OS_PTE_H__
#define __TIO_OS_PTE_H__

#include "stdint.h"
#include "physical.h"

typedef uint32_t pagetable_entry;

enum PAGE_PTE_FLAGS {
  PTE_PRESENT           = 1,
  PTE_WRITABLE          = 2,
  PTE_USER              = 4,
  PTE_WRITETHOUGH       = 8,
  PTE_NOT_CACHEABLE     = 0x10,
  PTE_ACCESSED          = 0x20,
  PTE_DIRTY             = 0x40,
  PTE_PAT               = 0x80,
  PTE_CPU_GLOBAL        = 0x100,
  PTE_LV4_GLOBAL        = 0x200,
  PTE_FRAME             = 0x7FFFF000
};

extern void PteAddAttribute(pagetable_entry* e, uint32_t attribute);
extern void PteDeleteAttribute(pagetable_entry* e, uint32_t attribute);
extern void PteSetFrame(pagetable_entry* e, physical_addr);
extern bool PteIsPresent(pagetable_entry e);
extern bool PteIsWritable(pagetable_entry e);
extern physical_addr PtePhysicalFrameNumber(pagetable_entry e);

#endif  //__TIO_OS_PTE_H__