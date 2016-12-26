#ifndef __TIO_OS_FILE_SYSTEM_H__
#define __TIO_OS_FILE_SYSTEM_H__

#include "stdint.h"
#include "disk_allocation.h"

#define EMPTY_TYPE              0
#define DIRECTORY_TYPE          1
#define FILE_TYPE               2
#define SELF_DIRECTORY_TYPE     3
#define PARENT_DIRECTORY_TYPE   4


struct __attribute__ ((packed)) FileDescriptor {
  char name[32];
  logical_block_addr start_addr;
  int id;
  int type;
  int filesize;
};

struct __attribute__ ((packed)) File {
  struct FileDescriptor* file_descriptor;
  int cursor;
};

// Directory creation, deletion and traversal
extern void FileSystemInitialize();
extern bool CreateDir(char* dirname);
extern void ListDirectoryContent();
extern bool ChangeDir(char* dirname);

// Given a filename, set the file descriptor to contain relevant information of
// the corresponding file. Returns true if the file is found.
extern bool OpenFile(struct File* file, char* filename);
extern int ReadFile(struct File* fp, char* buffer, size_t read_size);
#endif  //__TIO_OS_FILE_SYSTEM_H__