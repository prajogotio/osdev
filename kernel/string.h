#ifndef __TIO_OS_STRING_H__
#define __TIO_OS_STRING_H__

#include "stdint.h"

extern void* memset(void* addr, unsigned char value, size_t size);
extern int strcmp(char* first, char* second);
#endif  //__TIO_OS_STRING_H__