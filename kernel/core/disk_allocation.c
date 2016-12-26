#include "disk_allocation.h"
#include "ata_pio.h"
#include "kmalloc.h"
#include "math.h"
#include "string.h"

// Approach used: bitmap allocation table
// Location of allocation table: at LBA 5MB mark, i.e. 0x2800
// Since we allocate disk by blocks of 4096 bytes, and each bit can
// represent 1 block, if disk size is D bytes, we need D / (4096 * 8) bytes
// of bitmap allocation table

#define DISK_SIZE                         131072 // number blocks of 4096 bytes
                                                 // i.e. 512MB
#define ALLOCATION_TABLE_LBA              5 * 1024 * 1024 / 512  // location in terms of sector (512B)
#define ALLOCATION_TABLE_BLOCK_LOCATION   5 * 1024 * 1024 / 4096 // location in terms of block (4096B)
static char* buffer_;
static int size_of_alloc_table_ = DISK_SIZE/8;    // in terms of bytes

extern void DiskInitialize() {
  buffer_ = (char*) kmalloc(4096);
  memset(buffer_, 0, 4096);
}


extern void DiskFormat() {
  // number of 512 sectors needed
  int size_in_sector = size_of_alloc_table_ / 512;
  if (size_of_alloc_table_ % 512) {
    ++size_in_sector;
  }
  memset(buffer_, 0, 4096);

  // Deallocate kernel area (the lower 2MB)
  memset(buffer_, 0xff, 64);

  AtaPioWriteToDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA, min(size_in_sector, 8), buffer_);
  int cur_lba = ALLOCATION_TABLE_LBA + min(size_in_sector, 8);
  size_in_sector -= 8;
  memset(buffer_, 0, 4096);

  while (size_in_sector > 0) {
    AtaPioWriteToDisk(ATA_PIO_MASTER, cur_lba, min(size_in_sector, 8), buffer_);
    cur_lba += min(size_in_sector, 8);
    size_in_sector -= 8;
  }

  size_in_sector = size_of_alloc_table_ / 512;
  if (size_of_alloc_table_ % 512) {
    ++size_in_sector;
  }

  // Deallocate blocks occupied by the allocation table
  int left_block_idx = ALLOCATION_TABLE_BLOCK_LOCATION;
  int right_block_idx = (ALLOCATION_TABLE_LBA + size_in_sector) * 512 / 4096;
  // Each block contains 4096 * 8 bits, each bit represent allocation of 1 block of memory.
  // k = i / (4096 * 8) is the k-th block from ALLOCATION_TABLE_BLOCK_LOCATION
  int k = 0;
  for (int i = 0; i < DISK_SIZE; i += 4096 * 8, ++k) {
    if (i <= left_block_idx && left_block_idx < i+4096*8) {
      AtaPioReadFromDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA + k * 8, 8, buffer_);
      for (; left_block_idx < min(i + 4096*8, right_block_idx); ++left_block_idx) {
        buffer_[left_block_idx/8] |= 1<<(left_block_idx%8);
      }
      // Write 4096 bytes = 8 x 512 bytes sector at position ALLOCATION_TABLE_LBA + k * 8 (the corresponding block)
      AtaPioWriteToDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA + k * 8, 8, buffer_);
    }
  }
  // TODO: initialize root directory?
}

