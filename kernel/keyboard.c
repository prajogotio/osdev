#include "keyboard.h"
#include "hal.h"
#include "print.h"
#include "stdint.h"
#include "stdin_buffer.h"

#define KEYBOARD_READ_PORT              0x60
#define KEYBOARD_CONTROLLER_STATUS_PORT 0x64
#define KEYBOARD_OUTPUT_STATUS_BIT      1    

char qwerty[] = "qwertyuiop"; // 0x10 ..0x19
char qwerty_caps[] = "QWERTYUIOP";

char sym_qwerty[] = "[]"; // 0x1a .. 0x1b
char sym_qwerty_caps[] = "{}";


char asdf[] = "asdfghjkl"; // 0x1e .. 0x26
char asdf_caps[] = "ASDFGHJKL";

char sym_asdf[] = ";'"; // 0x27 .. 0x28
char sym_asdf_caps[] = ":\"";

char zxcv[] = "zxcvbnm"; // 0x2c .. 0x32
char zxcv_caps[] = "ZXCVBNM";

char sym_zxcv[] = ",./"; // 0x33 .. 0x35
char sym_zxcv_caps[] = "<>?";

char numbers[] = "1234567890-="; //0x02 .. 0x0d
char numbers_caps[] = "!@#$%^&*()_+";

static bool is_shift_on_ = 0;
static bool caps_toggle_ = 0;

static void HandleKeyboardEvent(char scan_code);
static void KeyboardIrq();

static void KeyboardIrq() {
  __asm__("pusha\n\t");
  __asm__("cli\n\t");
  if (ReadFromIoPort(KEYBOARD_CONTROLLER_STATUS_PORT) & KEYBOARD_OUTPUT_STATUS_BIT) {
    char scan_code = ReadFromIoPort(KEYBOARD_READ_PORT);
    HandleKeyboardEvent(scan_code);
  }
  InterruptDone(1);
  __asm__("sti\n\t");
  __asm__("popa\n\t"
          "leave\n\t"
          "iret\n\t");
}

void InitializeKeyboard() {
  SetInterruptVector(33, KeyboardIrq);
}

static void HandleKeyboardEvent(char scan_code) {
  bool is_break_code = scan_code & 0x80;
  if (is_break_code) {
    // Compute the corresponding make code.
    scan_code -= 0x80;
  }
  // Handle shift.
  if (scan_code == 0x2a || scan_code == 0x36) {
    is_shift_on_ = !!!is_break_code;
    return;
  }

  if (scan_code == 0x3a) {
    if (!is_break_code) {
      caps_toggle_ ^= 1;
    }
    return;
  }

  char curkey = 0;
  if (!is_break_code) {
    if (0x10 <= scan_code && scan_code <= 0x19) {
      if (is_shift_on_ || caps_toggle_) {
        curkey = qwerty_caps[scan_code - 0x10];
      } else {
        curkey = qwerty[scan_code - 0x10];
      }
    } else if (0x1a <= scan_code && scan_code <= 0x1b) {
      if (!is_shift_on_) {
        curkey = sym_qwerty[scan_code - 0x1a];
      } else {
        curkey = sym_qwerty_caps[scan_code - 0x1a];
      }
    } else if (0x1e <= scan_code && scan_code <= 0x26) {
      if(is_shift_on_ || caps_toggle_) {
        curkey = asdf_caps[scan_code - 0x1e];
      } else {
        curkey = asdf[scan_code - 0x1e];
      }
    } else if (0x27 <= scan_code && scan_code <= 0x28) {
      if(!is_shift_on_) {
        curkey = sym_asdf[scan_code - 0x27];
      } else {
        curkey = sym_asdf_caps[scan_code - 0x27];
      }
    } else if (0x2c <= scan_code && scan_code <= 0x32) {
      if(is_shift_on_ || caps_toggle_) {
        curkey = zxcv_caps[scan_code - 0x2c];
      } else {
        curkey = zxcv[scan_code - 0x2c];
      }
    } else if (0x33 <= scan_code && scan_code <= 0x35) {
      if(!is_shift_on_) {
        curkey = sym_zxcv[scan_code - 0x33];
      } else {
        curkey = sym_zxcv_caps[scan_code - 0x33];
      }
    } else if (scan_code == 0x39) {
      curkey = ' ';
    } else if (0x02 <= scan_code && scan_code <= 0x0d) {
      if (!is_shift_on_) {
        curkey = numbers[scan_code - 0x02];
      } else {
        curkey = numbers_caps[scan_code - 0x02];
      }
    } else if (0x0e == scan_code) {
      curkey = 8;
    } else if (0x1c == scan_code) {
      curkey = '\n';
    } else if (0x2b == scan_code) {
      curkey = is_shift_on_ ? '|' : '\\';
    } else if (0x29 == scan_code) {
      curkey = is_shift_on_ ? '~' : '`';
    } else {
      // Return early since we can't handle this key
      return;
    }
    PushToStdinBuffer(curkey);
    //PrintChar(curkey);
  }
}