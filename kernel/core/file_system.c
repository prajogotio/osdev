#include "file_system.h"
#include "physical.h"
#include "string.h"
#include "print.h"
#include "virtual.h"
#include "ata_pio.h"
#include "kmalloc.h"

#define FILE_READ_SIZE     0x1000/512   // We want to read exactly 1 block, which is 0x1000/512 sectors.

static struct FileDescriptor* cwd_;
static char* buffer_page_;  // pointer to buffer page of size 4096.

static void CopyFileDescriptor(struct FileDescriptor* source, struct FileDescriptor* dest);

void FileSystemInitialize(struct FileDescriptor* dir_desc) {
  cwd_ = dir_desc;
  // Allocate VAS to buffer_page_
  buffer_page_ = (char*) kmalloc(4096);
  memset(buffer_page_, 0, 4096);
}

bool CreateDir(char *dirname) {
  AtaPioReadFromDisk(ATA_PIO_MASTER, cwd_->start_addr, FILE_READ_SIZE, buffer_page_);
  int sz_of_entry = sizeof(struct FileDescriptor);
  for (int i = 0; i < 4096 - 4; i += sz_of_entry) {
    struct FileDescriptor* fd = (struct FileDescriptor*) &buffer_page_[i];
    if (fd->type == EMPTY_TYPE) {
      // Found an empty space for our new directory descriptor
      // Update information
      strcpy(dirname, fd->name);
      fd->type = DIRECTORY_TYPE;
      // Write to disk
      AtaPioWriteToDisk(ATA_PIO_MASTER, cwd_->start_addr, FILE_READ_SIZE, buffer_page_);
      return 1;
    }
  }
  // TODO: allocate new block for this directory if space is not enought
  // TODO: check dirname does not contain special chars
  // TODO: check for uniqueness of dirname
  return 0;
}

void ListDirectoryContent() {
  // Print to screen
  AtaPioReadFromDisk(ATA_PIO_MASTER, cwd_->start_addr, FILE_READ_SIZE, buffer_page_);
  int sz_of_entry = sizeof(struct FileDescriptor);
  for (int i = 0; i < 4096 - 4; i += sz_of_entry) {
    struct FileDescriptor* fd = (struct FileDescriptor*) &buffer_page_[i];
    if (fd->type == EMPTY_TYPE) {
      return;
    }
    if (fd->type == FILE_TYPE) {
      PrintString("    ");
    } else if (fd->type == DIRECTORY_TYPE) {
      PrintString("dir ");
    }
    PrintString(fd->name);
    PrintString(" ");
    PrintInt(fd->filesize);
    PrintString(" ");
    PrintHex(fd->start_addr);
    PrintString("\n");
  }
  PrintString("\n");
  // TODO: handle next block of directory entry
}

bool ChangeDir(char * dirname) {
  AtaPioReadFromDisk(ATA_PIO_MASTER, cwd_->start_addr, FILE_READ_SIZE, buffer_page_);
  int sz_of_entry = sizeof(struct FileDescriptor);
  for (int i = 0; i < 4096 - 4; i += sz_of_entry) {
    struct FileDescriptor* fd = (struct FileDescriptor*) &buffer_page_[i];
    if (fd->type == EMPTY_TYPE) {
      break;
    }
    if (fd->type == DIRECTORY_TYPE && strcmp(fd->name, dirname) == 0) {
      memcpy(fd, cwd_, sizeof(struct FileDescriptor));
      return 1;
    }
  }
  return 0;
}

bool OpenFile(struct File* file, char* filename) {
  // Get the first page containing the directory info memory
  AtaPioReadFromDisk(ATA_PIO_MASTER, cwd_->start_addr,
                     FILE_READ_SIZE, buffer_page_);
  int sz_of_entry = sizeof(struct FileDescriptor); // number of bytes per entry in the directory page
  // 4096 - 4 because the last 4 bytes are reserved for LBA to the next entries in the same directory (if any).
  for (int i = 0; i < 4096 - 4; i += sz_of_entry) {
    struct FileDescriptor* fd = (struct FileDescriptor*) &buffer_page_[i];
    if (strcmp(fd->name, filename) == 0) {
      CopyFileDescriptor(fd, file->file_descriptor);
      file->cursor = 0;
      return 1;
    }
  }
  // TODO: implementation to go through the directory linked list.
  return 0;
}

extern int ReadFile(struct File* fp, char* buffer, size_t read_size) {
  // Special case: cursor is already at unreadable position. Returns 0
  if (fp->cursor >= fp->file_descriptor->filesize || fp->cursor < 0) {
    return 0;
  }

  logical_block_addr cur_addr = fp->file_descriptor->start_addr;
  AtaPioReadFromDisk(ATA_PIO_MASTER, cur_addr, FILE_READ_SIZE, buffer_page_);
  int loaded_size = 4092;

  while (loaded_size <= fp->cursor) {
    cur_addr = buffer_page_[4092];
    AtaPioReadFromDisk(ATA_PIO_MASTER, cur_addr, FILE_READ_SIZE, buffer_page_);
    loaded_size += 4092;
  }
  int bytes_transferred = 0;
  for (; bytes_transferred < read_size; ++bytes_transferred) {
    if (fp->cursor >= fp->file_descriptor->filesize) {
      // We reach EOF before we transferred requested read_size.
      break;
    }
    // i points to the position of the cursor in current loaded file segment in buffer.
    int i = fp->cursor-loaded_size-4092;
    if (i >= 4092) {
      // Load next page!
      cur_addr = buffer_page_[4092];
      AtaPioReadFromDisk(ATA_PIO_MASTER, cur_addr, FILE_READ_SIZE, buffer_page_);
      loaded_size += 4092;
      i = 0;
    }
    ++fp->cursor;
    *buffer++ = buffer_page_[i];
  }
  return bytes_transferred;
}


static void CopyFileDescriptor(struct FileDescriptor* source, struct FileDescriptor* dest) {
  memcpy((char*)source, (char*)dest, sizeof(struct FileDescriptor));
}
