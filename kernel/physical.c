#include "physical.h"
#include "string.h"
#include "print.h"

#define BLOCKS_PER_ENTRY 32

static uint32_t physical_memory_size_ = 0;
static int current_used_blocks_ = 0;
static int max_available_blocks_ = 0;
static uint32_t* memory_map_ = 0;

static void MmapSet(int bit);
static void MmapUnset(int bit);
static int MmapTest(int bit);
static int MmapGetFreeBlockCount();


static inline void MmapSet(int bit) {
  if (!MmapTest(bit)) {
    ++current_used_blocks_;
  }
  memory_map_[bit/BLOCKS_PER_ENTRY] |= (1 << (bit % BLOCKS_PER_ENTRY));
}

static inline void MmapUnset(int bit) {
  if (MmapTest(bit)) {
    --current_used_blocks_;
  }
  memory_map_[bit/BLOCKS_PER_ENTRY] &= ~(1 << (bit % BLOCKS_PER_ENTRY));
}

static inline bool MmapTest(int bit) {
  return memory_map_[bit/BLOCKS_PER_ENTRY] & (1 << (bit % BLOCKS_PER_ENTRY));
}

int MmapGetFirstFreeBlock() {
  for (uint32_t i = 0; i < max_available_blocks_; ++i) {
    if (!MmapTest(i)) return i;
  }
  return -1;
}

// Find the first consecutive free blocks
int MmapGetFirstFreeBlocks(int size) {
  int chunk_length = 0;
  for (int i = 0; i < max_available_blocks_; ++i) {
    if (MmapTest(i)) {
      chunk_length = 0;
      continue;
    }
    chunk_length++;
    if (chunk_length == size) {
      return i - size + 1;
    }
  }
  return -1;
}

// memory_size : number of bytes in physical memory.
void MmapInitialize(uint32_t memory_size, uint32_t* mmap_pointer) {
  physical_memory_size_ = memory_size;
  memory_map_ = mmap_pointer;
  max_available_blocks_ = physical_memory_size_ / BLOCK_SIZE;
  current_used_blocks_ = max_available_blocks_;

  PrintString("Blocks initialized: ");
  PrintInt(max_available_blocks_);
  PrintString(" Number of entries: ");
  PrintInt(max_available_blocks_ / 32);
  PrintString("\n");
  memset(memory_map_, 0xff, max_available_blocks_ / BLOCKS_PER_BYTE);
}


void MmapInitializeRegion(uint32_t base_address, size_t size) {
  // Initialize less than required if not aligned.
  // Align to nearest block forward.
  int alignment = base_address / BLOCK_ALIGN * BLOCK_ALIGN;
  if (base_address % BLOCK_ALIGN) {
    alignment += BLOCK_ALIGN;
  }
  // align length inward.
  size -= (alignment - base_address);
  size = size / BLOCK_ALIGN * BLOCK_ALIGN;

  int index = alignment / BLOCK_SIZE;
  int number_of_blocks = size / BLOCK_SIZE;

  for (; number_of_blocks > 0; --number_of_blocks) {
    MmapUnset(index++);
  }
}

void MmapDeinitializeRegion(physical_addr base_address, size_t size) {
  // Deinitialize more than needed if not aligned.
  int alignment = base_address / BLOCK_ALIGN * BLOCK_ALIGN;
  size += alignment - base_address;

  int index = alignment / BLOCK_SIZE;
  int number_of_blocks = size / BLOCK_SIZE;

  if (size % BLOCK_ALIGN) {
    number_of_blocks++;
  }

  for (; number_of_blocks > 0; --number_of_blocks) {
    MmapSet(index++);
  }
}

static int MmapGetFreeBlockCount() {
  return (int) max_available_blocks_ - (int) current_used_blocks_;
}

void* MmapAllocateBlocks(int size) {
  if (MmapGetFreeBlockCount() <= 0) return 0;

  int block_index = MmapGetFirstFreeBlocks(size);

  if (block_index == -1) return 0;
  for (int i = 0; i < size; ++i) {
    MmapSet(block_index + i);
  }
  unsigned int physical_addr = block_index * BLOCK_SIZE;
  return (void*) physical_addr;
}

void MmapFreeBlocks(void* block_address, int size) {
  int block_index = (unsigned int) block_address / BLOCK_SIZE;
  for (; size > 0; --size) {
    MmapUnset(block_index++);
  }
}

void PmmEnablePaging(bool enable) {
  if (enable) {
    __asm__("mov %cr0, %eax\n\t"
            "orl $0x80000000, %eax\n\t"
            "mov %eax, %cr0\n\t");
  } else {
    __asm__("mov %cr0, %eax\n\t"
            "orl $0x7FFFFFFF, %eax\n\t"
            "mov %eax, %cr0\n\t");
  }
}

void PmmLoadPdbr(physical_addr addr) {
  __asm__("movl %%eax, %%cr3\n\t" : : "a"(addr));
}

void MmapMemoryInformation() {
  PrintString("Memory status:\n");
  PrintString("Total blocks allocated: ");
  PrintInt(max_available_blocks_);
  PrintString(" Total blocks used: ");
  PrintInt(current_used_blocks_);
  PrintString(" Remaining blocks: ");
  PrintInt(MmapGetFreeBlockCount());
  PrintString("\n");

  int block_set = 0;
  int block_not_set = 0;
  int run_size = 0;
  for (int i = 0; i < max_available_blocks_; ++i) {
    if (!MmapTest(i)) {
      block_not_set++;
      run_size++;
    }
    if ((MmapTest(i) && run_size != 0) || (!MmapTest(i) && i == max_available_blocks_ - 1)) {
      if (run_size != 0) {
        // special case: if this run extends till the end loop, increase i by 1 to remove off by one error
        if (i == max_available_blocks_ - 1) {
          ++i;
        }
        PrintString("Run detected: size: ");
        PrintInt(run_size);
        PrintString(" [");
        int start = i - run_size;
        PrintHex(start * 4096);
        PrintString(" - ");
        PrintHex(i * 4096 - 1);
        PrintString("]\n");
        if (i == max_available_blocks_) {
          break;
        }
      }
    }
    if (MmapTest(i)) {
      block_set++;
      run_size = 0;
    }
  }
  PrintString("True count mem set: ");
  PrintInt(block_set);
  PrintString(" True count mem unset: ");
  PrintInt(block_not_set);
  PrintString(" True mem total: ");
  PrintInt(block_set + block_not_set);
  PrintString("\n");
}
