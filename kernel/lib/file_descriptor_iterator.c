#include "file_descriptor_iterator.h"
#include "core/ata_pio.h"
#include "core/string.h"

void FileDescriptorIterator_Initialize(struct FileDescriptorIterator* iterator, char* buffer, logical_block_addr addr) {
  memset(iterator, 0, sizeof(struct FileDescriptorIterator));
  iterator->buffer = buffer;
  iterator->cursor = 0;
  iterator->current_buffer_lba = addr;  
  AtaPioReadFromDisk(ATA_PIO_MASTER, addr, 8, iterator->buffer);
}

bool FileDescriptorIterator_GetNext(struct FileDescriptorIterator* iterator, struct FileDescriptor** descriptor) {
  // TODO: implement a check on the last 8 bytes to see if it is the end of
  // the directory file. If not, we need to load the next blocks.
  // Check if cursor points to a beginning of a complete FileDescriptor
  if (iterator->cursor + sizeof(struct FileDescriptor) >= FILE_CONTENT_SIZE) return 0;
  *descriptor = (struct FileDescriptor*) &iterator->buffer[iterator->cursor];
  iterator->cursor += sizeof(struct FileDescriptor);
  return 1;
}
