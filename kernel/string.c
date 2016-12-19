#include "string.h"
#include "print.h"

void* memset(void* addr, unsigned char value, unsigned int size) {
  unsigned char * index = (unsigned char *) addr;
  int blocks = size;
  for (; blocks >= 0; --blocks) {
    *index = value;
    ++index;
  }
  return addr;
}
