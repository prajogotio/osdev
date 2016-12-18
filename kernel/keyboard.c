#include "keyboard.h"
#include "hal.h"
#include "print.h"

#define KEYBOARD_READ_PORT    0x60

static void KeyboardIrq();

static void KeyboardIrq() {
  __asm__("pusha\n\t");
  char c = ReadFromIoPort(KEYBOARD_READ_PORT);
  PrintHex(c);
  PrintString(" ");
  InterruptDone(1);
  __asm__("popa\n\t"
          "leave\n\t"
          "iret\n\t");
}

void InitializeKeyboard() {
  SetInterruptVector(33, KeyboardIrq);
}