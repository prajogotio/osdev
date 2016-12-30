#ifndef __TIO_OS_FILE_SYSTEM_H__
#define __TIO_OS_FILE_SYSTEM_H__

#include "stdint.h"
#include "disk_allocation.h"


#define EMPTY_TYPE              0
#define DIRECTORY_TYPE          1
#define FILE_TYPE               2
#define SELF_DIRECTORY_TYPE     3
#define PARENT_DIRECTORY_TYPE   4

#define FILE_CONTENT_SIZE  4088         // The remaining 8 bytes are used for
                                        // pointers to next and prev block

struct __attribute__ ((packed)) FileDescriptor {
  char name[32];
  logical_block_addr start_addr;    // start address of the file
  int id;
  int type;
  int filesize;

  // We need a way to reference this file descriptor itself on disk.
  // As long as we can get to the start of the directory that contains this
  // descriptor, we search the directory by id.
  logical_block_addr parent_start_addr;  // Start address of the directory that
                                         // contains this descriptor.
};

struct File {
  struct FileDescriptor* file_descriptor;
  int cursor;
};

// Directory creation, deletion and traversal
extern void FileSystemInitialize();
extern bool CreateDir(char* dirname);
extern void ListDirectoryContent();
extern bool ChangeDir(char* dirname);

// File operation
extern bool CreateFile(char *filename);
extern bool OpenFile(struct File** file, char* filename);
extern void CloseFile(struct File** file);
extern int WriteFile(struct File* file, char* buffer, size_t size);
extern int ReadFile(struct File* fp, char* buffer, size_t read_size);

#endif  //__TIO_OS_FILE_SYSTEM_H__