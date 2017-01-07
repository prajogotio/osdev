#ifndef __TIO_OS_VIRTUAL_H__
#define __TIO_OS_VIRTUAL_H__

#include "stdint.h"
#include "page_table_entry.h"
#include "page_directory_entry.h"

typedef uint32_t virtual_addr;

#define PAGES_PER_TABLE         1024
#define PAGES_PER_DIRECTORY     1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

#define PTABLE_ADDR_SPACE_SIZE 0x400000
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

#define PAGE_SIZE 4096

struct ptable {
  pagetable_entry entries[PAGES_PER_TABLE];
};

struct pdirectory {
  page_directory_entry entries[PAGES_PER_DIRECTORY];
};

extern struct pdirectory* current_directory_;

extern bool VmmAllocatePage(pagetable_entry* e);
extern bool VmmFreePage(pagetable_entry* e);
extern pagetable_entry* VmmPtableLookupEntry(struct ptable* p, virtual_addr addr);
extern page_directory_entry* VmmPdirectoryLookupEntry(struct pdirectory* p, virtual_addr addr);
extern bool VmmSwitchPdirectory(struct pdirectory* directory, uint32_t pdbr);
extern void VmmFlushTlbEntry(virtual_addr addr);
extern void VmmMapPage(void* physical, void* virtual);
extern void* VmmGetPhysicalAddress(void* virtual);
extern void VmmInitialize();
extern void* VmmGetCurrentPageDirectory();
extern struct ptable* VmmGetPageTablePointer(virtual_addr virtual);
extern void VmmMapIfNotPresent(virtual_addr addr);

#endif  //__TIO_OS_VIRTUAL_H__