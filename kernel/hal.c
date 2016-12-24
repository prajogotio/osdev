#include "hal.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "print.h"
#include "keyboard.h"
#include "virtual.h"

uint32_t * Hal_memory_information = 0;

static void InitializeMemoryManagement();
static void MmapAllocationTesting();
static void WriteToMemory(void* position, char * str);
static void InitializeDiskManager();

int HalInitialize() {
  InitializeGdt();
  InitializeIdt(0x8);
  InitializePic(0x20, 0x28);
  InitializePit();
  PitStartCounter(100, PIT_OCW_COUNTER_0, PIT_OCW_MODE_SQUAREWAVEGEN);
  InitializeMemoryManagement();
  InitializeKeyboard();
  InitializeDiskManager();
  PrintString("HAL Initialized!\n");
  return 0;
}

int HalShutdown() {
  return 0;
}

void GenerateInterrupt(int n) {
  // broken.
  __asm__(
    "movl generate_interrupt, %%ebx\n\t"
    "movb %0, 1(%%ebx)\n\t"
    "generate_interrupt:\n\t"
    "int $0\n\t"
    : : "a" (n));
}


void WriteToIoPort(unsigned int port, char value) {
  __asm__("outb %%al, %%dx": : "a"(value), "d"(port));
}

uint8_t ReadFromIoPort(unsigned int port) {
  char value = 0;
  __asm__("inb %%dx, %%al" : "=a"(value) : "d"(port));
  return value;
}

void InterruptDone(unsigned intno) {
  if (intno > 16) return;
  if (intno >= 8) PicSendCommand(PIC_OCW2_MASK_EOI, 1);
  PicSendCommand(PIC_OCW2_MASK_EOI, 0);
}

void SetInterruptVector(int intno, IRQ_HANDLER handler) {
  InstallInterruptHandler(intno, IDT_DESC_PRESENT|IDT_DESC_BIT32, 0x8, handler);
}

static void InitializeMemoryManagement() {
  uint32_t kernel_size = *Hal_memory_information;
  uint32_t * memory_info = Hal_memory_information + 1;
  uint32_t * memory_map_table = Hal_memory_information + 5;
  PrintString("Kernel size: ");
  PrintInt(kernel_size);
  PrintString("\n");
  PrintString("Extended Memory between 1MB to 16MB (in KB): ");
  PrintHex((int) *memory_info);
  PrintString("\nExtended Memory between > 16MB (in 64KB): ");
  PrintHex((int) *(memory_info+1));
  PrintString("\nConfigured Memory between 1MB to 16MB (in KB): ");
  PrintHex((int) *(memory_info+2));
  PrintString("\nConfigured Memory between > 16MB (in 64KB): ");
  PrintHex((int) *(memory_info+3));
  PrintString("\n");

  int memory_low_size = *(memory_info);       // in KB
  int memory_high_size = *(memory_info+1);    // in 64 KB
  int total_memory_in_kb = 1024 + memory_low_size + memory_high_size * 64;
  PrintString("Size of available memory: ");
  PrintInt(total_memory_in_kb);
  PrintString(" KB\n");
  PrintString("MemoryMap entries: ");

  // Initialize memory map
  // int mmap_address = KERNEL_OFFSET + kernel_size;
  int mmap_address = KERNEL_OFFSET + kernel_size;
  PrintString("Initializing mmap at ");
  PrintHex(mmap_address);
  PrintString("\n");
  MmapInitialize(total_memory_in_kb * 1024, (uint32_t *) mmap_address);

  int entry_size = *memory_map_table;
  PrintString("Number of SMAP entries: ");
  PrintInt(entry_size);
  PrintString("\n");

  for (int index = 0; index < entry_size; ++index) {
    int offset = 1+index*6;
    uint32_t base_address = (int) *(memory_map_table + offset);
    uint32_t length = (uint32_t) *(memory_map_table + 2 + offset);
    uint32_t type = (uint32_t) * (memory_map_table + 4 + offset);    
    PrintString("E: ");
    PrintString("  Base address: ");
    PrintHex(base_address);
    PrintString("  Length: ");
    PrintHex(length);
    PrintString("  Type: ");
    PrintHex(type);

    // PrintString("  ACPI_NULL: ");
    // PrintHex((int) *(memory_map_table+5+offset));
    PrintString("\n");
  

    if (type == 1) {
      MmapInitializeRegion(base_address, length);
    }
  }
  // Our kernel sector should be protected from allocation.
  // Also protect our Mmap table
  // Since we map 3gb virtual 0x00100000 physical, use that physical base
  MmapDeinitializeRegion(0x00100000, kernel_size + 4096);

  // Protect the first block of memory.
  MmapDeinitializeRegion(0, 4096);

  MmapAllocationTesting();

  // Reinitialize page directory table
  VmmInitialize();
}

static void MmapAllocationTesting() {
  PrintString("Testing memory allocation: \n");

  // Test memory allocation.
  int* address_1 = (int *) MmapAllocateBlocks(1);
  WriteToMemory(address_1, "[A] was here...");
  PrintString("Page [A] of size 1 is allocated at: ");
  PrintHex((int) address_1);
  PrintString("\n");

  int* address_1000 = (int *) MmapAllocateBlocks(1000);
  PrintString("Page [B] of size 1000 is allocated at: ");
  PrintHex((int) address_1000);
  PrintString("\n");  
  
  int* address_100 = (int *) MmapAllocateBlocks(100);
  PrintString("Page [C] of size 100 is allocated at: ");
  PrintHex((int) address_100);
  PrintString("\n");

  MmapFreeBlocks(address_1, 1);
  PrintString("[A] is deallocated\n");

  int* address_2 = (int *) MmapAllocateBlocks(1);
  PrintString("Page [D] of size 1 is allocated at: ");
  PrintHex((int) address_2);
  PrintString("\n[D] checks what's on memory: ");
  PrintString((char *)address_2);
  PrintString("\n");

  address_1 = (int *) MmapAllocateBlocks(1);
  PrintString("[A] is reallocated at: ");
  PrintHex((int) address_1);
  PrintString("\n");


  MmapFreeBlocks(address_1, 1);

  MmapFreeBlocks(address_2, 1);

  MmapFreeBlocks(address_100, 100);

  MmapFreeBlocks(address_1000, 1000);

  MmapMemoryInformation();
}

static void InitializeDiskManager() {
  AtaPioInitialize();
}

static void WriteToMemory(void* position, char * str) {
  char* index = (char *) position;
  while (*str) {
    *(index++) = *(str++);
  }
  *index = 0;
}
