#ifndef __TIO_OS_KMALLOC_H__
#define __TIO_OS_KMALLOC_H__

#include "stdint.h"

// Returns the virtual address containing the requested memory buffer.
extern void* kmalloc(size_t size);
extern void kfree(void* address);
extern void KmallocInitialize();

// Debugging
extern void KmallocDisplayLists();

#endif  //__TIO_OS_KMALLOC_H__