#ifndef __TIO_OS_PHYSICAL_H__
#define __TIO_OS_PHYSICAL_H__

#include "stdint.h"

#define BLOCKS_PER_BYTE     8
#define BLOCK_SIZE          4096
#define BLOCK_ALIGN         4096

typedef uint32_t physical_addr;

// Addresses must be multiples of BLOCK_SIZE
extern void MmapInitialize(uint32_t memory_size, uint32_t* mmap_pointer);
extern void MmapInitializeRegion(uint32_t base_address, size_t size);
extern void MmapDeinitializeRegion(uint32_t base_address, size_t size);
extern void MmapFreeBlocks(void* block_address, int size);
extern void* MmapAllocateBlocks(int size);

extern void MmapSanityCheck();

extern void PmmEnablePaging(bool);
extern void PmmLoadPdbr(physical_addr);


#endif  //__TIO_OS_PHYSICAL_H__