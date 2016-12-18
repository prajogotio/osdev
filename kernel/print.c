#include "print.h"
#include "hal.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_MEMORY 0xB8000

static int cursorX_ = 0;
static int cursorY_ = 0;

static void MoveToNextRow();
static void UpdateCursorPosition();
static void ScrollDown();

int GetCursorX() {
  return cursorX_;
}

int GetCursorY() {
  return cursorY_;
}

void PrintChar(char c) {
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
    ScrollDown();
    cursorY_ = SCREEN_HEIGHT-1;
    cursorX_ = 0;
  }
  UpdateCursorPosition();
}

static void MoveToNextRow() {
  cursorX_ = 0;
  ++cursorY_;
}

static void UpdateCursorPosition() {
  int position = cursorY_ * SCREEN_WIDTH + cursorX_;
  char low_byte = 0xFF & position;
  char high_byte = (0xFF00 & position) >> 8;
  WriteToIoPort(0x03d4, 0x0f);
  WriteToIoPort(0x03d5, low_byte);
  WriteToIoPort(0x03d4, 0x0e);
  WriteToIoPort(0x03d5, high_byte);
}

void ClearScreen() {
  char* vga_pointer = (char*) VIDEO_MEMORY;
  for (int i = 0; i < SCREEN_HEIGHT; ++i) {
    for (int j = 0; j < SCREEN_WIDTH; ++j) {
      *vga_pointer = 0;
      ++vga_pointer;
      *vga_pointer = 7;
      ++vga_pointer;
    }
  }
  cursorX_ = cursorY_ = 0;
}


static void ScrollDown() {
  // Copy all letters up by one row
  for (int i = 1; i < 25; ++i) {
    for (int j = 0; j < 80; ++j) {
      char* current_letter = (char *) (i * SCREEN_WIDTH * 2 + j * 2 + VIDEO_MEMORY);
      char* one_letter_up = (char *) (current_letter - SCREEN_WIDTH * 2);
      *one_letter_up = *current_letter;
      *(one_letter_up+1) = *(current_letter+1);
      *current_letter = 0;
      *(current_letter+1) = 7;
    }
  }
}

void PrintString(char* str) {
  while (*str) {
    PrintChar(*str);
    ++str;
  }
}

void PrintHex(unsigned int val) {
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
  PrintString("0x");
  PrintString(hex);
}

void MoveCursor(unsigned int x, unsigned int y) {
  cursorX_ = x;
  cursorY_ = y;
  UpdateCursorPosition();
}