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
static void DeleteOneChar();
static void SetCharacter(char c);

int GetCursorX() {
  return cursorX_;
}

int GetCursorY() {
  return cursorY_;
}

void PrintChar(char c) {
  if (c == '\n') {
    MoveToNextRow();
  } else if (c == 8) {
    DeleteOneChar();
  } else {
    SetCharacter(c);
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

static void SetCharacter(char c) {
  // VGA uses 2 bytes per character
    char* vga_pointer = (char*) (VIDEO_MEMORY + cursorY_ * SCREEN_WIDTH * 2 + cursorX_ * 2);
    *vga_pointer = c;
    *(vga_pointer + 1) = 7;   // white on black
}

static void DeleteOneChar() {
  --cursorX_;
  if (cursorX_ == -1) {
    cursorX_ = SCREEN_WIDTH-1;
    --cursorY_;
  }
  if (cursorY_ == -1) {
    cursorY_ = 0;
    cursorX_ = 0;
  }
  SetCharacter(0);
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
  UpdateCursorPosition();
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
  char hex[9] = "00000000";
  for (int i = 0; i < 8; ++i) {
    unsigned int k = val & 0xf;
    if (0 <= k && k <= 9) {
      hex[7-i] = '0' + k;
    } else if (10 <= k && k <= 15) {
      hex[7-i] = 'a' + k-10;
    } else {
      hex[7-i] = '?';
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

void PrintInt(int value) {
  if (value < 0) {
    PrintString("-");
    PrintInt(-value);
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
  PrintString((char*) &buffer[position+1]);
}