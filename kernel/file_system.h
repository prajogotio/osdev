#ifndef __TIO_OS_FILE_SYSTEM_H__
#define __TIO_OS_FILE_SYSTEM_H__

#include "stdint.h"

typedef uint32_t logical_block_addr;    // Only the first 28 bits are used.

struct __attribute__ ((packed)) FileDescriptor {
  char name[32];
  logical_block_addr start_addr;
  bool is_directory;
  int filesize;
};

struct __attribute__ ((packed)) File{
  struct FileDescriptor* file_descriptor;
  int cursor;
}

// Given a filename, set the file descriptor to contain relevant information of
// the corresponding file. Returns true if the file is found.
extern bool OpenFile(char* filename, struct FileDescriptor* file);
extern int ReadFile(struct FileDescriptor* fp, char* buffer, size_t read_size);

#endif  //__TIO_OS_FILE_SYSTEM_H__