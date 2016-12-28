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
#define METADATA_BLOCK_ID                 1999
#define ROOT_DIR_BLOCK_ID                 2000
static char* buffer_;
static int size_of_alloc_table_ = DISK_SIZE/8;    // in terms of bytes

struct __attribute__((packed)) DiskManagerMetadata {
  int next_file_id;
};

static void DiskSetAllocationTableEntry(int block_index);
static void DiskUnsetAllocationTableEntry(int block_index);
static void InitializeMetadataBlock();

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
  // Allocate root at block 2000 (arbitrary choice, revise later when needed)
  DiskSetAllocationTableEntry(ROOT_DIR_BLOCK_ID);

  InitializeMetadataBlock();
}

static void InitializeMetadataBlock() {
  // Allocate 1 block for disk manager metadata at block 1999
  DiskSetAllocationTableEntry(METADATA_BLOCK_ID);
  int lba = METADATA_BLOCK_ID * 4096 / 512;
  AtaPioReadFromDisk(ATA_PIO_MASTER, lba, 8, buffer_);
  memset(buffer_, 0, 4096);
  struct DiskManagerMetadata* metadata = (struct DiskManagerMetadata*) buffer_;
  metadata->next_file_id = 1;
  AtaPioWriteToDisk(ATA_PIO_MASTER, lba, 8, buffer_);
}

static void DiskSetAllocationTableEntry(int block_index) {
  int k = block_index / (4096 * 8);
  AtaPioReadFromDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA + 8 * k, 8, buffer_);
  buffer_[block_index/8] |=  1<< (block_index % 8); 
  AtaPioWriteToDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA + 8 * k, 8, buffer_);
}


static void DiskUnsetAllocationTableEntry(int block_index) {
  int k = block_index / (4096 * 8);
  AtaPioReadFromDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA + 8 * k, 8, buffer_);
  buffer_[block_index/8] &=  ~(1<< (block_index % 8));
  AtaPioWriteToDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA + 8 * k, 8, buffer_);
}

logical_block_addr DiskAllocateBlock() {
  for (int i = 0; i < size_of_alloc_table_; i += 4096) {
    // i manages blocks [i * 4096 * 8, (i + 1) 4096 * 8 - 1]
    AtaPioReadFromDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA + 8 * i, 8, buffer_);
    for (int j = 0; j < 4096 * 8; ++j) {
      if (buffer_[j/8] & (1 << (j%8))) continue;
      // We found an empty one!
      buffer_[j/8] |= 1 << (j % 8);

      AtaPioWriteToDisk(ATA_PIO_MASTER, ALLOCATION_TABLE_LBA + 8 * i, 8, buffer_);
      // Return the LBA of that block
      return (i * 4096 * 8 + j) * 4096 / 512;
    }
  }
  return 0;
}

void DiskFreeBlock(logical_block_addr addr) {
  // Compute the block index
  int block_index = addr * 512 / 4096;
  DiskUnsetAllocationTableEntry(block_index);
}

int DiskAllocateFileId() {
  AtaPioReadFromDisk(ATA_PIO_MASTER, METADATA_BLOCK_ID * 4096 / 512, 8, buffer_);
  struct DiskManagerMetadata* metadata = (struct DiskManagerMetadata*) buffer_;
  int ret_id = metadata->next_file_id++;
  AtaPioWriteToDisk(ATA_PIO_MASTER, METADATA_BLOCK_ID * 4096 / 512, 8, buffer_);
  return ret_id;
}

void DiskMemsetBlock(logical_block_addr addr, char val) {
  AtaPioReadFromDisk(ATA_PIO_MASTER, addr, 8, buffer_);
  memset(buffer_, val, 4096);
  AtaPioWriteToDisk(ATA_PIO_MASTER, addr, 8, buffer_);
}

