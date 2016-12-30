#include "file_system.h"
#include "physical.h"
#include "string.h"
#include "print.h"
#include "virtual.h"
#include "ata_pio.h"
#include "kmalloc.h"
#include "math.h"
#include "lib/file_descriptor_iterator.h"

#define FILE_READ_SIZE     0x1000/512   // We want to read exactly 1 block, which is 0x1000/512 sectors.


static struct FileDescriptor* cwd_;
static char* buffer_page_;  // pointer to buffer page of size 4096.
static struct FileDescriptorIterator* iterator_;

static void CopyFileDescriptor(struct FileDescriptor* source, struct FileDescriptor* dest);
static bool FlushFileDescriptor(struct FileDescriptor* file_descriptor);

void FileSystemInitialize() {
  cwd_ = (struct FileDescriptor*) kmalloc(sizeof(struct FileDescriptor));
  memset(cwd_, 0, sizeof(struct FileDescriptor));
  cwd_->name[0] = '/';
  cwd_->name[1] = 0;
  cwd_->start_addr = 2000 * 4096/512; // Root directory is placed at block 2000
  cwd_->id = 0;
  cwd_->type = DIRECTORY_TYPE;
  cwd_->filesize = 0;
  // Allocate VAS to buffer_page_
  buffer_page_ = (char*) kmalloc(4096);
  memset(buffer_page_, 0, 4096);
  // Allocate a reusable iterator
  iterator_ = (struct FileDescriptorIterator*) kmalloc(sizeof(struct FileDescriptorIterator));
  memset(iterator_, 0, sizeof (struct FileDescriptorIterator));
  DiskInitialize();
}

bool CreateDir(char *dirname) {
  FileDescriptorIterator_Initialize(iterator_, buffer_page_, cwd_->start_addr);
  struct FileDescriptor* fd;
  while (FileDescriptorIterator_GetNext(iterator_, &fd)) {
    if (fd->type == EMPTY_TYPE) {
      // Found an empty space for our new directory descriptor
      // Update information
      fd->id = DiskAllocateFileId();
      strcpy(dirname, fd->name);
      fd->type = DIRECTORY_TYPE;
      // Allocate a block to this directory and store the LBA here.
      fd->start_addr = DiskAllocateBlock();
      fd->parent_start_addr = cwd_->start_addr;

      // Write to disk
      AtaPioWriteToDisk(ATA_PIO_MASTER, cwd_->start_addr, FILE_READ_SIZE, buffer_page_);

      // Write recursive pointer to itself (i.e. '.') as the first entry
      struct FileDescriptor* self_dir = (struct FileDescriptor*) buffer_page_;
      memcpy((char*)fd, (char*)self_dir, sizeof(struct FileDescriptor));
      self_dir->type = SELF_DIRECTORY_TYPE;

      struct FileDescriptor* parent_dir = (struct FileDescriptor*) (buffer_page_ + sizeof(struct FileDescriptor));
      memcpy((char*) cwd_, (char*) parent_dir, sizeof(struct FileDescriptor));
      parent_dir->type = PARENT_DIRECTORY_TYPE;

      memset((char*) (buffer_page_ + 2 * sizeof(struct FileDescriptor)), 0, 4096 - 2 * sizeof(struct FileDescriptor));

      AtaPioWriteToDisk(ATA_PIO_MASTER, self_dir->start_addr, 8, buffer_page_);
      

      // Flush file descriptor to update cwd info.
      // Do this after the above, because they share buffer_page_!
      // TODO: change this design so we allocate new buffer_page when
      // necessary to reduce complexity of sharing
      cwd_->filesize += sizeof(struct FileDescriptor);
      FlushFileDescriptor(cwd_);

      return 1;
    }
  }
  // TODO: allocate new block for this directory if space is not enough
  // TODO: check dirname does not contain special chars
  // TODO: check for uniqueness of dirname
  return 0;
}

void ListDirectoryContent() {
  FileDescriptorIterator_Initialize(iterator_, buffer_page_, cwd_->start_addr);
  struct FileDescriptor* fd;
  while (FileDescriptorIterator_GetNext(iterator_, &fd)) {
    if (fd->type == EMPTY_TYPE) {
      return;
    }
    if (fd->type == FILE_TYPE) {
      PrintString("    ");
    } else if (fd->type == DIRECTORY_TYPE) {
      PrintString("dir ");
    }

    if (fd->type == SELF_DIRECTORY_TYPE) {
      PrintString(".   Current directory: ");
      PrintString(fd->name);
      PrintString("\n");
    } else if (fd->type == PARENT_DIRECTORY_TYPE) {
      PrintString("..  Parent directory: ");
      PrintString(fd->name);
      PrintString("\n");
    } else {
      PrintString(fd->name);
      PrintString(" size:");
      PrintInt(fd->filesize);
      PrintString(" start_addr:");
      PrintHex(fd->start_addr);
      PrintString(" id: ");
      PrintInt(fd->id);
      PrintString("\n");
    }
  }
  PrintString("\n");
  // TODO: handle next block of directory entry
}

bool ChangeDir(char * dirname) {
  FileDescriptorIterator_Initialize(iterator_, buffer_page_, cwd_->start_addr);
  struct FileDescriptor* fd;

  while (FileDescriptorIterator_GetNext(iterator_, &fd)) {
    if (fd->type == EMPTY_TYPE) {
      break;
    }
    if ((fd->type == DIRECTORY_TYPE && strcmp(fd->name, dirname) == 0) ||
        (fd->type == SELF_DIRECTORY_TYPE && strcmp(dirname, ".") == 0) ||
        (fd->type == PARENT_DIRECTORY_TYPE && strcmp(dirname, "..") == 0)) {
      memcpy((char*) fd, (char*) cwd_, sizeof(struct FileDescriptor));
      cwd_->type = DIRECTORY_TYPE;
      return 1;
    }
  }
  return 0;
}

bool CreateFile(char *filename) {
  FileDescriptorIterator_Initialize(iterator_, buffer_page_, cwd_->start_addr);
  struct FileDescriptor* fd;

  while (FileDescriptorIterator_GetNext(iterator_, &fd)) {
    if (fd->type == EMPTY_TYPE) {
      fd->id = DiskAllocateFileId();
      strcpy(filename, fd->name);
      fd->type = FILE_TYPE;
      fd->start_addr = DiskAllocateBlock();
      DiskMemsetBlock(fd->start_addr, 0);
      fd->parent_start_addr = cwd_->start_addr;

      AtaPioWriteToDisk(ATA_PIO_MASTER, cwd_->start_addr, FILE_READ_SIZE, buffer_page_);
      return 1;
    }
  }
  return 0;
}

bool OpenFile(struct File** file, char* filename) {
  // allocate space for file and its members
  *file = (struct File*) kmalloc(sizeof(struct File));
  memset(*file, 0, sizeof(struct File));
  (*file)->file_descriptor = (struct FileDescriptor*) kmalloc(sizeof(struct FileDescriptor));
  memset((*file)->file_descriptor, 0, sizeof(struct FileDescriptor));

  FileDescriptorIterator_Initialize(iterator_, buffer_page_, cwd_->start_addr);

  struct FileDescriptor* fd;
  while (FileDescriptorIterator_GetNext(iterator_, &fd)) {
    if (strcmp(fd->name, filename) == 0) {
      // Set file descriptor to its value from disk
      CopyFileDescriptor(fd, (*file)->file_descriptor);
      (*file)->cursor = 0;
      return 1;
    }
  }
  // TODO: implementation to go through the directory linked list.
  return 0;
}

int WriteFile(struct File* file, char* buffer, size_t size) {
  // Handle one block first
  // TODO: load next block if needed
  // TODO: free up blocks that are not used anymore
  AtaPioReadFromDisk(ATA_PIO_MASTER, file->file_descriptor->start_addr, 8, buffer_page_);

  int write_size = min(size, FILE_CONTENT_SIZE - file->cursor);
  memcpy(buffer, &buffer_page_[file->cursor], write_size);
  AtaPioWriteToDisk(ATA_PIO_MASTER, file->file_descriptor->start_addr, 8, buffer_page_);
  // Update cursor the cursor by amount of byte written.
  file->cursor += write_size;
  // TODO: Handle special case: if file->cursor is at FILE_CONTENT_SIZE,
  // allocate new block to this file.

  // Update the filesize if cursor now points to area larger than the filesize
  if (file->file_descriptor->filesize <= file->cursor) {
    file->file_descriptor->filesize = file->cursor;
    // Update the file descriptor filesize and flush to disk
    FlushFileDescriptor(file->file_descriptor);
  }
  return write_size;
}

extern int ReadFile(struct File* fp, char* buffer, size_t read_size) {
  // Handle one block first
  // TODO: load next block if needed
  AtaPioReadFromDisk(ATA_PIO_MASTER, fp->file_descriptor->start_addr, 8, buffer_page_);
  // Only read up to end of block or end of file.
  int true_read_size = min(
    read_size,
    min(FILE_CONTENT_SIZE-fp->cursor, 
        fp->file_descriptor->filesize - fp->cursor));
  memcpy(&buffer_page_[fp->cursor], buffer, true_read_size);
  return true_read_size;
}

static void CopyFileDescriptor(struct FileDescriptor* source, struct FileDescriptor* dest) {
  memcpy((char*)source, (char*)dest, sizeof(struct FileDescriptor));
}

static bool FlushFileDescriptor(struct FileDescriptor* file_descriptor) {
  if (file_descriptor->id == 0) {
    // 'file_descriptor' is root directory descriptor. Currently we
    // don't store root directory information on disk.
    return 0;
  }
  // Load and traverse the directory listing to find the file descriptor
  FileDescriptorIterator_Initialize(iterator_, buffer_page_, file_descriptor->parent_start_addr);
  struct FileDescriptor* fd;
  while (FileDescriptorIterator_GetNext(iterator_, &fd)) {
    if (fd->id == file_descriptor->id) {
      // Overwrite fd content and flush to disk
      memcpy((char*) file_descriptor, (char*) fd, sizeof(struct FileDescriptor));
      AtaPioWriteToDisk(ATA_PIO_MASTER, iterator_->current_buffer_lba, 8, iterator_->buffer);
      return 1;
    }
  }
  // File descriptor not found!
  return 0;
}


void CloseFile(struct File** file) {
  // Free all allocated memories during OpenFile.
  kfree((*file)->file_descriptor);
  kfree((*file));
  // Set file to 0
  *file = 0;
}
