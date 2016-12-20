#include "virtual.h"
#include "string.h"

struct pdirectory* current_directory_ = 0;
physical_addr cur_pdbr_ = 0;

static void VmmPtableClear(struct ptable* table);

bool VmmAllocatePage(pagetable_entry* e) {
  void* frame_addr = MmapAllocateBlocks(1);
  if (!frame_addr) return 0;
  PteSetFrame(e, (physical_addr) frame_addr);
  PteAddAttribute(e, PTE_PRESENT);
  return 1;
}

bool VmmFreePage(pagetable_entry* e) {
  void * frame_addr = (void*) PtePhysicalFrameNumber(*e);
  if (frame_addr) {
    MmapFreeBlocks(frame_addr, 1);
  }
  PteDeleteAttribute(e, PTE_PRESENT);
}

inline pagetable_entry* VmmPtableLookupEntry(struct ptable* p, virtual_addr addr) {
  if (p) {
    return &p->entries[PAGE_TABLE_INDEX(addr)];
  }
  return 0;
}

inline page_directory_entry* VmmPdirectoryLookupEntry(struct pdirectory* p, virtual_addr addr) {
  if (p) {
    return &p->entries[PAGE_DIRECTORY_INDEX(addr)];
  }
  return 0;
}

inline bool VmmSwitchPdirectory(struct pdirectory* directory) {
  if (!directory) return 0;
  current_directory_ = directory;
  PmmLoadPdbr(cur_pdbr_);
  return 1;
}

void VmmFlushTlbEntry(virtual_addr addr) {
  __asm__("cli\n\t"
          "invlpg %0\n\t"
          "sti\n\t" : : "m" (addr));
}

void VmmPtableClear(struct ptable* table) {
  memset(table, 0, 4096);
}

void VmmMapPage(void* physical, void* virtual) {
  struct pdirectory* page_directory = current_directory_;
  page_directory_entry* e = VmmPdirectoryLookupEntry(page_directory, (virtual_addr) virtual);
  if ((*e & PTE_PRESENT) != PTE_PRESENT) {
    // Page table is not present. Allocate 1 block to hold the table.
    struct ptable * table = (struct ptable*) MmapAllocateBlocks(1);
    if (!table) return;
    memset(table, 0, BLOCK_SIZE);
    PdeAddAttribute(e, PDE_PRESENT);
    PdeAddAttribute(e, PDE_WRITABLE);
    PdeSetFrame(e, (physical_addr) table);
  }
  struct ptable* table = (struct ptable*) PAGE_GET_PHYSICAL_ADDRESS(e);
  pagetable_entry* page = VmmPtableLookupEntry(table, (virtual_addr) virtual);
  PteSetFrame(page, (physical_addr) physical);
  PteAddAttribute(page, PTE_PRESENT);
}

void VmmInitialize() {
  // Allocate default page table
  struct ptable* table_default = (struct ptable*) MmapAllocateBlocks(1);
  if (!table_default) return;
  // Allocate 3GB page table
  struct ptable* table_3gb = (struct ptable*) MmapAllocateBlocks(1);
  if (!table_3gb) return;

  VmmPtableClear(table_default);
  VmmPtableClear(table_3gb);

  // first 4mb are identity map
  for (int i = 0, frame=0x0, virt=0x00000000; i<1024; ++i, frame+=4096, virt+=4096) {
    pagetable_entry page = 0;
    PteAddAttribute(&page, PTE_PRESENT);
    PteSetFrame(&page, frame);
    *VmmPtableLookupEntry(table_default, virt) = page;
  }

  // map 1mb to 3gb
  for (int i = 0, frame=0x100000, virt=0xc0000000; i<1024;i++, frame+=4096, virt+=4096) {
    pagetable_entry page=0;
    PteAddAttribute(&page, PTE_PRESENT);
    PteSetFrame(&page, frame);
    *VmmPtableLookupEntry(table_3gb, virt) = page;
  }


  struct pdirectory* directory = (struct pdirectory*) MmapAllocateBlocks(1);
  if (!directory) return;
  memset(directory, 0, BLOCK_SIZE);

  page_directory_entry* entry_3gb = VmmPdirectoryLookupEntry(directory, (virtual_addr) 0xc0000000);
  PdeAddAttribute(entry_3gb, PDE_PRESENT);
  PdeAddAttribute(entry_3gb, PDE_WRITABLE);
  PdeSetFrame(entry_3gb, (physical_addr) table_3gb);

  page_directory_entry* entry_default = VmmPdirectoryLookupEntry(directory, 0x00000000);
  PdeAddAttribute(entry_default, PDE_PRESENT);
  PdeAddAttribute(entry_default, PDE_WRITABLE);
  PdeSetFrame(entry_default, (physical_addr) table_default);

  cur_pdbr_ = (physical_addr) directory;
  VmmSwitchPdirectory(directory);
  PmmEnablePaging(1);
}