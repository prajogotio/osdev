#include "kernel.h"
#include "physical.h"
#include "virtual.h"
#include "string.h"

// Allocatable virtual address is defined as the following:
// 0xd0000000 to 0xefffffff
// Or in other words: virtual page 0xd0000 to 0xeffff
// Set 0xf0000000 to point to the buffer for the virtual page bitmap

static uint32_t* vas_bitmap = 0xf0000000;

void KernelMallocInitialize() {
  // physical_addr vas_bitmap_addr = (uint32_t*) MmapAllocateBlocks(1);
  // VmmMapPage((physical_addr) vas_bitmap_addr, (virtual_addr) vas_bitmap);
  // memset(vas_bitmap, )
}