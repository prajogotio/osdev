#ifndef __TIO_OS_PAGE_REPLACEMENT_H__
#define __TIO_OS_PAGE_REPLACEMENT_H__

#include "stdint.h"

#define PAGING_RESERVE_START_BLOCK        2048    // Reserved for LRU paging
#define PAGING_RESERVE_SIZE               256     // 256 pages

#define PAGE_EVICTABLE                         1
// Maximum number of pages user can have on RAM at once.
#define MAX_USER_PHYSICAL_PAGE_ALLOCATION      4

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
  int num_pages_in_memory;
};

extern struct WorkingSet* current_working_set;
extern void WorkingSetInitialize(struct WorkingSet* working_set);
extern void WorkingSetInsertPage(struct WorkingSet* working_set, uint32_t virtual_address);

extern void PageReplacementInitialize();

extern void PageEvict(struct WorkingSet* working_set, uint32_t virt_victim);
extern void PageReplace(struct WorkingSet* working_set, uint32_t virtual_address, uint32_t physical_address);
extern void PageSwap(struct WorkingSet* working_set, uint32_t virt_victim, uint32_t virt_new);

extern void PageReplacementTest();

#endif  //__TIO_OS_PAGE_REPLACEMENT_H__