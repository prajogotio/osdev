#include "debug.h"
#include "hal.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_MEMORY 0xB8000

static int cursorX_ = 0;
static int cursorY_ = 0;

static void MoveToNextRow();

void DebugPrintChar(char c) {
  if (c == '\n') {
    MoveToNextRow();
  } else {
    // VGA uses 2 bytes per character
    char* vga_pointer = (char*) (VIDEO_MEMORY + cursorY_ * SCREEN_WIDTH * 2 + cursorX_ * 2);
    *vga_pointer = c;
    *(vga_pointer + 1) = 7;   // white on black
    ++cursorX_;
  }
  if (cursorX_ == SCREEN_WIDTH) {
    MoveToNextRow();
  }
  if (cursorY_ == SCREEN_HEIGHT) {
    cursorY_ = SCREEN_HEIGHT-1;
    cursorX_ = 0;
  }
}

static void MoveToNextRow() {
  cursorX_ = 0;
  ++cursorY_;
}


void DebugPrintString(char* str) {
  while (*str) {
    DebugPrintChar(*str);
    ++str;
  }
}

void DebugPrintHex(unsigned int val) {
  char hex[9] = {0};
  for (int i = 0; i < 8; ++i) {
    int k = val & 0xf;
    if (k <= 9) {
      hex[7-i] = '0' + k;
    } else {
      hex[7-i] = 'a' + k-10;
    }
    val = val >> 4;
  }
  DebugPrintString("0x");
  DebugPrintString(hex);
}


void DebugPrintInt(int value) {
  if (value < 0) {
    DebugPrintString("-");
    DebugPrintInt(-value);
    return;
  }
  char* buffer = "????????????????";
  int position = 15;
  if (value == 0) {
    buffer[position--] = '0';
  } else {
    while (value) {
      buffer[position--] = '0' + (value % 10);
      value /= 10;
    }
  }
  DebugPrintString((char*) &buffer[position+1]);
}

void DebugMoveCursor(unsigned int x, unsigned int y) {
  cursorX_ = x;
  cursorY_ = y;
}