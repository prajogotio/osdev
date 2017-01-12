#ifndef __TIO_OS_PAGE_REPLACEMENT_H__
#define __TIO_OS_PAGE_REPLACEMENT_H__

#include "stdint.h"

#define PAGING_RESERVE_START_BLOCK        2048    // Reserved for LRU paging
#define PAGING_RESERVE_SIZE               256     // 256 pages

#define PAGE_EVICTABLE 1

struct PageInfo {
  uint32_t virtual_address;
  uint32_t lba_on_disk;
  int can_evict;
  struct PageInfo* prev;
  struct PageInfo* next;
};

struct WorkingSet {
  struct PageInfo *page_evicted;
  struct PageInfo *page_in_memory;
};

extern struct WorkingSet* current_working_set;
extern void PageWorkingSetInitialize(struct WorkingSet* working_set);
extern void PageWorkingSetInsertPage(struct WorkingSet* working_set, uint32_t virtual_address);

extern void PageReplacementInitialize();
extern void PageSetCurrentWorkingSet(struct WorkingSet* working_set);

extern void PageEvict(uint32_t virt_victim);
extern void PageReplace(uint32_t virtual_address, uint32_t physical_address);
extern void PageSwap(uint32_t virt_victim, uint32_t virt_new);

extern void PageReplacementTest();

#endif  //__TIO_OS_PAGE_REPLACEMENT_H__