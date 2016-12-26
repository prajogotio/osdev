#ifndef __TIO_OS_DISK_ALLOCATION_H__
#define __TIO_OS_DISK_ALLOCATION_H__

#include "stdint.h"

typedef uint32_t logical_block_addr;    // Only the first 28 bits are used.

extern void DiskInitialize();
extern void DiskFormat();
extern logical_block_addr DiskAllocateBlock();
extern void DiskFreeBlock(logical_block_addr addr);
extern int DiskAllocateFileId();
extern void DiskMemsetBlock(logical_block_addr addr, char val);


#endif  //__TIO_OS_DISK_ALLOCATION_H__