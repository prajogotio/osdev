#include "hal.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "print.h"
#include "keyboard.h"

int HalInitialize() {
  InitializeGdt();
  InitializeIdt(0x8);
  InitializePic(0x20, 0x28);
  InitializePit();
  InitializeKeyboard();
  PitStartCounter(100, PIT_OCW_COUNTER_0, PIT_OCW_MODE_SQUAREWAVEGEN);
  PrintString("Done\n");
  return 0;
}

int HalShutdown() {
  return 0;
}

void GenerateInterrupt(int n) {
  // broken.
  __asm__(
    "movl generate_interrupt, %%ebx\n\t"
    "movb %0, 1(%%ebx)\n\t"
    "generate_interrupt:\n\t"
    "int $0\n\t"
    : : "a" (n));
}


void WriteToIoPort(unsigned int port, char value) {
  __asm__("outb %%al, %%dx": : "a"(value), "d"(port));
}

uint8_t ReadFromIoPort(unsigned int port) {
  char value = 0;
  __asm__("inb %%dx, %%al" : "=a"(value) : "d"(port));
  return value;
}

void InterruptDone(unsigned intno) {
  if (intno > 16) return;
  if (intno >= 8) PicSendCommand(PIC_OCW2_MASK_EOI, 1);
  PicSendCommand(PIC_OCW2_MASK_EOI, 0);
}

void SetInterruptVector(int intno, IRQ_HANDLER handler) {
  InstallInterruptHandler(intno, IDT_DESC_PRESENT|IDT_DESC_BIT32, 0x8, handler);
}