#include "string.h"
#include "print.h"

void* memset(void* addr, unsigned char value, unsigned int size) {
  unsigned char * index = (unsigned char *) addr;
  int blocks = size;
  for (; blocks > 0; --blocks) {
    *index = value;
    ++index;
  }
  return addr;
}

int strcmp(char* first, char* second) {
  while (*first != 0 && *second != 0) {
    if (*first > *second) return 1;
    if (*first < *second) return -1;
    ++first;
    ++second;
  }
  if (*first == *second) return 0;
  return *first > *second ? 1 : -1;
}

extern void memcpy(char* source, char* dest, size_t size) {
  for (; size > 0; --size) {
    *dest++ = *source++;
  }
}
