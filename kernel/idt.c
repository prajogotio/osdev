#include "idt.h"
#include "hal.h"
#include "print.h"
#include "stdint.h"

struct __attribute__ ((packed)) IdtRegister {
  uint16_t limit;
  uint32_t base;
};

static struct IdtDescriptor idt_[MAX_INTERRUPTS];
static struct IdtRegister idtr_;

static void InstallIdt();
static void DefaultHandler();
static int ClearIdt(struct IdtDescriptor* idt);

static void InstallIdt() {
  __asm__("lidt (%0)" : : "a" (&idtr_));
}

static void DefaultHandler() {
  PrintString("*** Interrupt Default Handler by JOGOG\n");
  for (;;);
}

struct IdtDescriptor* GetInterruptDescriptor(uint32_t i) {
  if (i > MAX_INTERRUPTS) return 0;
  return &idt_[i];
}

int InstallInterruptHandler(uint32_t i, uint16_t flags, uint16_t selector, IRQ_HANDLER irq_handler) {
  if (i > MAX_INTERRUPTS) return 0;
  if (!irq_handler) return 0;
  uint64_t interrupt_handler_base = (uint64_t) irq_handler;
  idt_[i].base_lo = (uint16_t) (interrupt_handler_base & 0xffff);
  idt_[i].base_hi = (uint16_t) ((interrupt_handler_base >> 16) & 0xffff);
  idt_[i].reserved = 0;
  idt_[i].flags = (uint8_t) flags;
  idt_[i].selector = selector;
  return 0;
}

int InitializeIdt(uint16_t code_selector) {
  idtr_.limit = sizeof (struct IdtDescriptor) * MAX_INTERRUPTS - 1;
  idtr_.base = (uint32_t) &idt_[0];
  for (int i = 0; i < MAX_INTERRUPTS; ++i) {
    ClearIdt(&idt_[i]);
  }
  for (int i = 0; i < MAX_INTERRUPTS; ++i) {
    InstallInterruptHandler(i, IDT_DESC_PRESENT|IDT_DESC_BIT32,
      code_selector, (IRQ_HANDLER) DefaultHandler);
  }
  InstallIdt();
}

static int ClearIdt(struct IdtDescriptor* idt) {
  idt->base_lo = 0;
  idt->base_hi = 0;
  idt->reserved = 0;
  idt->flags = 0;
  idt->selector = 0;
}
