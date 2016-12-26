#ifndef __TIO_OS_STRING_H__
#define __TIO_OS_STRING_H__

#include "stdint.h"

extern void* memset(void* addr, unsigned char value, size_t size);
extern void memcpy(char* source, char* dest, size_t size);
extern int strcmp(char* first, char* second);
extern void strcpy(char* source, char* dest);
extern size_t strlen(char* str);

#endif  //__TIO_OS_STRING_H__