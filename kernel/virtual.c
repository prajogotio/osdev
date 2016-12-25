#include "virtual.h"
#include "string.h"
#include "print.h"
#include "hal.h"

struct pdirectory* current_directory_ = 0;
physical_addr cur_pdbr_ = 0;

static void VmmPtableClear(struct ptable* table);
static void VmmTest();
static void PagefaultHandler();
static struct ptable* GetPageTablePointer(virtual_addr virtual);

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
  if ((*e & PDE_PRESENT) != PDE_PRESENT) {
    physical_addr ptable_addr = (physical_addr) MmapAllocateBlocks(1);
    if (ptable_addr == 0) {
      // Cannot allocate block!
      return;
    }

    // Assign to the page-directory first.
    PdeAddAttribute(e, PDE_PRESENT);
    PdeAddAttribute(e, PDE_WRITABLE);
    PdeSetFrame(e, ptable_addr);

    // Retrieve the page table
    struct ptable * table = GetPageTablePointer((virtual_addr) virtual);
    memset(table, 0, BLOCK_SIZE);
  }

  struct ptable* table = GetPageTablePointer((virtual_addr) virtual);
  pagetable_entry* page = VmmPtableLookupEntry(table, (virtual_addr) virtual);

  PteSetFrame(page, (physical_addr) physical);
  PteAddAttribute(page, PTE_PRESENT);
}

static struct ptable* GetPageTablePointer(virtual_addr virtual) {
  // Compute index to directory table
  uint32_t index = PAGE_DIRECTORY_INDEX(virtual);
  // Create the address usable for recursive ptable mapping address resolution
  return (struct ptable*) (0xFFC00000 | (index << 12));
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

  // map 1mb physical to 3gb virtual
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

  // Recursive mapping for page-table address resolution. 
  // Set the last entry in directory table to point to the directory table itself.
  page_directory_entry* recursive_mapping = VmmPdirectoryLookupEntry(directory, 0xFFC00000);
  PdeAddAttribute(recursive_mapping, PDE_PRESENT);
  PdeAddAttribute(recursive_mapping, PDE_WRITABLE);
  PdeSetFrame(recursive_mapping, (physical_addr) directory);


  // Enable paging.
  cur_pdbr_ = (physical_addr) directory;
  VmmSwitchPdirectory(directory);
  PmmEnablePaging(1);

  // Pagefault interrupt
  SetInterruptVector(14, PagefaultHandler);

  VmmTest();
}

static void PagefaultHandler() {
  PrintString("*** Pagefault by JOGOG. Not implemented yet...");
  for(;;);
}

static void VmmTest() {
  PrintString("Pdbr is set at: ");
  PrintHex((uint32_t) cur_pdbr_);
  PrintString(" [physical]\n");

  // Virtual 3gb - 1mb physical mapping test
  PrintString("Test: Virtual 0xc0001234: ");
  PrintHex(*(uint32_t*) 0xc0001234);
  PrintString(" Physical 0x00101234: ");
  PrintHex(*(uint32_t*) 0x00101234);  // identity mapping for first 4mb
  PrintString("\n");

  // Test VmmMap
  // Memory mapping only works for pages.
  uint32_t * some_memory = (uint32_t*) MmapAllocateBlocks(1);
  *(some_memory+123) = 0xfaceface;  
  uint32_t * virtual_memory = (uint32_t*) 0xfefef000;

  VmmMapPage((void*) some_memory, (void*) virtual_memory);

  PrintString("Mapped "); PrintHex((physical_addr) some_memory); PrintString(" physical to "); PrintHex((virtual_addr) virtual_memory); PrintString(" virtual\n");
  PrintString("Test entry: [virtual] ");
  PrintHex((virtual_addr) (virtual_memory+123));
  PrintString(" := ");
  PrintHex(*(uint32_t*) (virtual_memory+123));
  PrintString(" [physical] ");
  PrintHex((physical_addr) (some_memory+123));
  PrintString(" := ");
  PrintHex(*(uint32_t*) (some_memory+123));
  PrintString("\n");
  PrintString("Check: physical addr of ");
  PrintHex((int) virtual_memory);
  PrintString(" [virtual] : ");
  PrintHex((int)VmmGetPhysicalAddress(virtual_memory));
  PrintString(" [physical]\n");

}

void* VmmGetPhysicalAddress(void* virtual) {
  struct ptable* table = GetPageTablePointer((virtual_addr) virtual);
  pagetable_entry* entry = VmmPtableLookupEntry(table, (virtual_addr) virtual);
  physical_addr addr = (physical_addr) PAGE_GET_PHYSICAL_ADDRESS(entry);
  // virtual & 0xfff computes the 12 bit offset into page block.
  addr |= ((virtual_addr) virtual) & 0xfff;
  return (void*) addr;
}