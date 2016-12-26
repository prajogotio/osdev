#ifndef __TIO_OS_FILE_DESCRIPTOR_ITERATOR_H__
#define __TIO_OS_FILE_DESCRIPTOR_ITERATOR_H__

#include "core/file_system.h"
#include "core/stdint.h"

struct FileDescriptorIterator {
  char* buffer;
  int cursor;
};

extern void FileDescriptorIterator_Initialize(struct FileDescriptorIterator* iterator, char* buffer, logical_block_addr addr);
extern bool FileDescriptorIterator_GetNext(struct FileDescriptorIterator* iterator, struct FileDescriptor** descriptor);

#endif  //__TIO_OS_FILE_DESCRIPTOR_ITERATOR_H__