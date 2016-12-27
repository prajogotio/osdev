#include "kmalloc.h"
#include "physical.h"
#include "virtual.h"
#include "string.h"
#include "print.h"

#define KMALLOC_BASE_ADDR 0xf0000000

struct __attribute__ ((packed)) KmallocNode {
  struct KmallocNode* prev;
  struct KmallocNode* next;
  int start;
  int size;
};

static struct KmallocNode* free_list_head_;
static struct KmallocNode* allocated_list_head_;
static struct KmallocNode* free_pointer_;
static const int size_of_node_ = sizeof(struct KmallocNode);

static struct KmallocNode* KmallocGetNextUsableAddress();
static void KmallocAddToFreePointer(struct KmallocNode* ptr);
static void KmallocTest();

void KmallocInitialize() {
  // 0xf0000000 points to head of allocated_list
  allocated_list_head_ = (struct KmallocNode*) KMALLOC_BASE_ADDR;
  memset(allocated_list_head_, 0, size_of_node_);

  // It is an empty list
  allocated_list_head_->next = allocated_list_head_;
  allocated_list_head_->prev = allocated_list_head_;



  // Initialize free list
  free_list_head_ = (struct KmallocNode*) (KMALLOC_BASE_ADDR + size_of_node_);
  memset(free_list_head_, 0, size_of_node_);

  free_list_head_->next = (struct KmallocNode*) (KMALLOC_BASE_ADDR + size_of_node_*2);
  memset(free_list_head_->next, 0, size_of_node_);

  free_list_head_->prev = free_list_head_;

  // The first entry contains the full allocatable VAS
  // from 0xd0000000 to 0xefffffff
  struct KmallocNode* first_entry = free_list_head_->next;
  first_entry->prev = free_list_head_;
  first_entry->next = free_list_head_;
  first_entry->start = 0xd0000000;
  first_entry->size = 0x20000000;

  // free_pointer points to the next usable address in [0xf0000000-0xffc00000)
  free_pointer_ = (struct KmallocNode*) (KMALLOC_BASE_ADDR + size_of_node_*3);
  memset(free_pointer_, 0, size_of_node_);

  free_pointer_->next = (struct KmallocNode*) (free_pointer_+size_of_node_);
  memset(free_pointer_->next, 0, size_of_node_);
  free_pointer_->prev = free_pointer_;
  // Set the only entry to points to free_pointer_
  free_pointer_->next->prev = free_pointer_;
  free_pointer_->next->next = free_pointer_;

  // KmallocTest();
}

static struct KmallocNode* KmallocGetNextUsableAddress() {
  struct KmallocNode* ret_addr = free_pointer_->next;

  // Remove the first entry from the list.
  free_pointer_->next = free_pointer_->next->next;
  free_pointer_->next->prev = free_pointer_;

  // If it was the last entry, allocate a new one
  if (free_pointer_->next == free_pointer_) {
    free_pointer_->next = (struct KmallocNode*) (ret_addr + size_of_node_);
    memset(free_pointer_->next, 0, size_of_node_);
    free_pointer_->next->next = free_pointer_;
    free_pointer_->next->prev = free_pointer_;
  }

  return ret_addr;
}

static void KmallocAddToFreePointer(struct KmallocNode* ptr) {
  struct KmallocNode* first_entry = free_pointer_->next;
  first_entry->prev = ptr;
  ptr->next = first_entry;
  ptr->prev = free_pointer_;
  free_pointer_->next = ptr;
}

void* kmalloc(size_t size) {
  // Traverse the free linked list, start from the first entry
  struct KmallocNode* ptr = free_list_head_->next;
  while (ptr->size < size && ptr != free_list_head_) {
    ptr = ptr->next;
  }
  if (ptr == free_list_head_) {

    // We cannot find suitable allocation.
    // TODO: handle this situation.
    PrintString("VAS allocation error: Out of VAS to allocate.");
    for(;;);

    return 0;
  }
  void* ret_addr = (void*) ptr->start;

  // Allocate this one! (First fit allocation)

  // Update allocated list
  struct KmallocNode* new_node = KmallocGetNextUsableAddress();
  new_node->size = size;
  new_node->start = ptr->start;

  struct KmallocNode* alloc_first_entry = allocated_list_head_->next;
  allocated_list_head_->next = new_node;
  new_node->prev = allocated_list_head_;
  new_node->next = alloc_first_entry;
  alloc_first_entry->prev = new_node;

  // Update free list
  int remaining_size = ptr->size - size;
  if (remaining_size > 0) {
    ptr->size = remaining_size;
    ptr->start += size;
  } else {
    // Remove from free list, and add the node to free_pointer to be reused
    struct KmallocNode* prev_ptr = ptr->prev;
    struct KmallocNode* next_ptr = ptr->next;
    prev_ptr->next = next_ptr;
    next_ptr->prev = prev_ptr;
    KmallocAddToFreePointer(ptr);
  }
  return ret_addr;
}

void kfree(void* address) {
  // Traverse the allocation list to get the entry
  struct KmallocNode* ptr = allocated_list_head_->next;
  while (ptr->start != (int) address && allocated_list_head_ != ptr) {
    ptr = ptr->next;
  }
  if (ptr == allocated_list_head_) {
    // The address was not allocated in the first place!
    return;
  }

  // Remove this entry from ptr, and traverse the free list to find a good place to put it in.
  struct KmallocNode* prev_ptr = ptr->prev;
  struct KmallocNode* next_ptr = ptr->next;
  prev_ptr->next = next_ptr;
  next_ptr->prev = prev_ptr;

  // Find the position where we should place our newly freed memory
  struct KmallocNode* fl_ptr = free_list_head_->next;
  while (fl_ptr != free_list_head_ && fl_ptr->start < ptr->start) {
    fl_ptr = fl_ptr->next;
  }
 
  // fl_ptr now points to the first node in which fl_ptr->start > ptr->start.
  // move the pointer one place back
  fl_ptr = fl_ptr->prev;

  // Put it in
  struct KmallocNode* next_fl_ptr = fl_ptr->next;
 
  fl_ptr->next = ptr;
  ptr->prev = fl_ptr;
  ptr->next = next_fl_ptr;
  next_fl_ptr->prev = ptr;


  // See if we can compact to the left
  if (fl_ptr != free_list_head_ && fl_ptr->start+fl_ptr->size == ptr->start) {
    ptr->start = fl_ptr->start;
    ptr->size += fl_ptr->size;
    ptr->prev = fl_ptr->prev;
    ptr->prev->next = ptr;
    KmallocAddToFreePointer(fl_ptr);
  }

  // See if we can compact to the right
  if (next_fl_ptr != free_list_head_ && ptr->start + ptr->size == next_fl_ptr->start) {
    ptr->size += next_fl_ptr->size;
    ptr->next = next_fl_ptr->next;
    ptr->next->prev = ptr;
    KmallocAddToFreePointer(next_fl_ptr);
  }
}

void KmallocDisplayLists() {
  {
    struct KmallocNode* testptr = free_list_head_->next;
    while (testptr != free_list_head_) {
      PrintString("[");
      PrintHex((int)testptr->start);
      PrintString(", ");
      PrintInt(testptr->size);
      PrintString(", ");
      PrintHex((int)testptr->prev);
      PrintString(", ");
      PrintHex((int)testptr->next);
      PrintString("] -> ");
      testptr = testptr->next;
    }
  }
  PrintString("TAIL\n");

  {
    struct KmallocNode* testptr = free_pointer_->next;
    while (testptr != free_pointer_) {
      PrintString("[");
      PrintHex((int)testptr->prev);
      PrintString(", ");
      PrintHex((int)testptr->next);
      PrintString("]");

      testptr = testptr->next;
    }
  }
  PrintString("\n");

}

static void KmallocTest() {
  void* d1 = kmalloc(1);
  PrintString("VAS allocated: ");
  PrintHex((int)d1);
  PrintString("\n");

  void* d2 = kmalloc(1);
  PrintString("VAS allocated: ");
  PrintHex((int)d2);
  PrintString("\n");

  void* d3 = kmalloc(7);
  PrintString("VAS allocated: ");
  PrintHex((int)d3);
  PrintString("\n");

  void* d4 = kmalloc(105);
  PrintString("VAS allocated: ");
  PrintHex((int)d4);
  PrintString("\n");


  kfree(d2);
  PrintString("VAS deallocated: ");
  PrintHex((int)d2);
  PrintString("\n");


  void* d5 = kmalloc(55);
  PrintString("VAS allocated: ");
  PrintHex((int)d5);
  PrintString("\n");

  void* d6 = kmalloc(1);
  PrintString("VAS allocated: ");
  PrintHex((int)d6);
  PrintString("\n");

  kfree(d6);
  PrintString("VAS deallocated: ");
  PrintHex((int)d6);
  PrintString("\n");

  kfree(d4);
  PrintString("VAS deallocated: ");
  PrintHex((int)d4);
  PrintString("\n");

  kfree(d1);
  PrintString("VAS deallocated: ");
  PrintHex((int)d3);
  PrintString("\n");

  kfree(d1);
  PrintString("VAS deallocated: ");
  PrintHex((int)d1);
  PrintString("\n");

  void* d7 = kmalloc(2);
  PrintString("VAS allocated: ");
  PrintHex((int)d7);
  PrintString("\n");

  kfree(d3);
  PrintString("VAS deallocated: ");
  PrintHex((int)d3);
  PrintString("\n");

  KmallocDisplayLists();


  kfree(d5);
  PrintString("***VAS deallocated: ");
  PrintHex((int)d5);
  PrintString("\n");

  kfree(d7);
  PrintString("***VAS deallocated: ");
  PrintHex((int)d7);
  PrintString("\n");

  KmallocDisplayLists();
  for(;;);
}