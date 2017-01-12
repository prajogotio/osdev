#include "page_replacement.h"
#include "ata_pio.h"
#include "virtual.h"
#include "string.h"
#include "print.h"
#include "kmalloc.h"

struct WorkingSet* current_working_set = 0;

static uint8_t page_reserve_[PAGING_RESERVE_SIZE/8]; // each byte manages 8 pages

static uint32_t PageGetDiskLba();
static void PageFreeReserveBit(int i);

void PageEvict(uint32_t virtual_address) {
  struct PageInfo* page = current_working_set->page_in_memory->next;
  while (page != current_working_set->page_in_memory &&
         page->virtual_address != virtual_address) {
    page = page->next;
  }
  if (page == current_working_set->page_in_memory || page->virtual_address != virtual_address) {
    __asm__("cli");
    PrintString("FATAL: PageEvict: Virtual address not found.\n");
    for(;;);
  }
  if (page->can_evict != PAGE_EVICTABLE) {
    __asm__("cli");
    PrintString("FATAL: PageEvict: evicting non-evictable page\n");
    for(;;);
  }
  // Remove the page from its current linked list
  struct PageInfo* page_next = page->next;
  struct PageInfo* page_prev = page->prev;
  page_prev->next = page_next;
  page_next->prev = page_prev;

  // Place the page on evicted page linked list
  struct PageInfo* page_evicted_next = current_working_set->page_evicted->next;
  page->next = page_evicted_next;
  page_evicted_next->prev = page;
  page->prev = current_working_set->page_evicted;
  current_working_set->page_evicted->next = page;

  // Set the LBA of that page on disk
  page->lba_on_disk = PageGetDiskLba();
  AtaPioWriteToDisk(ATA_PIO_MASTER, page->lba_on_disk, 4096/512, (char*)page->virtual_address);

  // Invalidate the page table entry of page.
  struct ptable* pt = (struct ptable*) VmmGetPageTablePointer((virtual_addr)page->virtual_address);
  pagetable_entry* pte = (struct pagetable_entry*) VmmPtableLookupEntry(pt, virtual_address);
  PteDeleteAttribute(pte, PTE_PRESENT);
  PteDeleteAttribute(pte, PTE_DIRTY);
  PteDeleteAttribute(pte, PTE_ACCESSED);
}

void PageReplace(uint32_t virtual_address, uint32_t physical_address) {
  struct PageInfo* ptr = current_working_set->page_evicted->next;
  while (ptr != current_working_set->page_evicted &&
         ptr->virtual_address != virtual_address) {
    ptr = ptr->next;
  }
  int is_new_page = 0;
  if (ptr == current_working_set->page_evicted || ptr->virtual_address != virtual_address) {
    is_new_page = 1;
    ptr = (struct PageInfo*) kmalloc(sizeof(struct PageInfo));
    ptr->virtual_address = virtual_address;
    ptr->can_evict = PAGE_EVICTABLE;
  }

  VmmMapPage((void*) physical_address, (void*) virtual_address);
  if (!is_new_page) {
    // Bring it back to memory (assuming not a page directory entry/page table entry)
    // TODO: handle those cases
    AtaPioReadFromDisk(ATA_PIO_MASTER, ptr->lba_on_disk, 4096/512, (char*) virtual_address);
    PageFreeReserveBit(ptr->lba_on_disk*512/4096);
    // Remove page from evicted list
    struct PageInfo* page_prev = ptr->prev;
    struct PageInfo* page_next = ptr->next;
    page_prev->next = page_next;
    page_next->prev = page_prev;
  }
  // Add to page_in_memory linked list
  struct PageInfo* page_in_memory_head = current_working_set->page_in_memory->next;
  current_working_set->page_in_memory->next = ptr;
  ptr->prev = current_working_set->page_in_memory;
  ptr->next = page_in_memory_head;
  page_in_memory_head->prev = ptr;
}

void PageSwap(uint32_t virt_victim, uint32_t virt_new) {
  uint32_t physical_address = (uint32_t) VmmGetPhysicalAddress((void*) virt_victim);
  PrintString("Evicting: ");
  PrintHex(virt_victim);
  PrintString(" [");
  PrintHex(physical_address);
  PrintString("] Bring in: ");
  PrintHex(virt_new);
  PrintString("\n");

  // Perform the swapping
  PageEvict(virt_victim);
  PageReplace(virt_new, physical_address);
  VmmFlushTlbEntry(virt_victim);
  VmmFlushTlbEntry(virt_new);
}

extern void PageReplacementInitialize() {
  memset(page_reserve_, 0, PAGING_RESERVE_SIZE/8);
}

extern void PageWorkingSetInitialize(struct WorkingSet* working_set) {
  working_set->page_evicted = (struct PageInfo*) kmalloc(sizeof(struct PageInfo));
  working_set->page_evicted->next = working_set->page_evicted;
  working_set->page_evicted->prev = working_set->page_evicted;

  working_set->page_in_memory = (struct PageInfo*) kmalloc(sizeof(struct PageInfo));
  working_set->page_in_memory->next = working_set->page_in_memory;
  working_set->page_in_memory->prev = working_set->page_in_memory;
}

extern void PageWorkingSetInsertPage(struct WorkingSet* working_set, uint32_t virtual_address) {
  struct PageInfo* page = (struct PageInfo*) kmalloc(sizeof(struct PageInfo));
  page->virtual_address = virtual_address;
  page->can_evict = PAGE_EVICTABLE;
  struct PageInfo* head = working_set->page_in_memory->next;
  working_set->page_in_memory->next = page;
  page->prev = working_set->page_in_memory;
  page->next = head;
  head->prev = page;
}

extern void PageSetCurrentWorkingSet(struct WorkingSet* working_set) {
  current_working_set = working_set;
}

static uint32_t PageGetDiskLba() {
  for (int i = 0; i < PAGING_RESERVE_SIZE; ++i) {
    if (page_reserve_[i / 8] & (1 << (i % 8))) continue;
    page_reserve_[i/8] |= (1 << (i%8));
    return (i + PAGING_RESERVE_START_BLOCK) * 4096/512;
  }
  // We cannot evict the page because our reserve is full.
  // Handle this case in the future.
  __asm__("cli");
  PrintString("FATAL: Page Eviction failed. Out of memory\n");
  for(;;);
  return -1;
}

static void PageFreeReserveBit(int i) {
  page_reserve_[i/8] &= ~(1 << (i%8));
}

void PageReplacementTest() {
  PrintString("Page Replacement Test.\n");

  struct WorkingSet* test_ws = (struct WorkingSet*) kmalloc(sizeof(struct WorkingSet));
  PageWorkingSetInitialize(test_ws);
  PageSetCurrentWorkingSet(test_ws);

  char *my_block = (char*) kmalloc(4096);
  strcpy("Canary...\n", my_block);
  PageWorkingSetInsertPage(current_working_set, (uint32_t)my_block);
  PrintString(my_block);

  char *x = 0x23232323;
  uint32_t page = (uint32_t) x & ~0xfff;
  strcpy("New Word in block...\n", x);
  PrintString(x);
  PrintString(my_block);

  char *y = 0x43411f04;
  uint32_t page2 = (uint32_t) y & ~0xfff;
  strcpy("What a feeling!\n", y);
  PrintString(x);
  PrintString(my_block);
  PrintString(y);
  for(;;);
}