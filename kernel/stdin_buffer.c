#include "stdin_buffer.h"

#define BUFFER_SIZE 1024

// circular buffer
static int start_index_ = 0;
static int end_index_ = 0;
static char buffer_[BUFFER_SIZE];

void PushToStdinBuffer(char c) {
  buffer_[end_index_++] = c;
  if (end_index_ >= BUFFER_SIZE) {
    end_index_ = 0;
  }
}

char ReadFromStdin() {
  char ret = buffer_[start_index_++];
  if (start_index_ >= BUFFER_SIZE) {
    start_index_ = 0;
  }
  return ret;
}

bool IsStdinBufferEmpty() {
  return start_index_ == end_index_;
}