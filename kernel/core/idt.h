#ifndef __TIO_OS_IDT_H__
#define __TIO_OS_IDT_H__

#include "stdint.h"

#define MAX_INTERRUPTS         256
#define IDT_DESC_BIT16        0x06
#define IDT_DESC_BIT32        0X0E
#define IDT_DESC_RING1        0x40
#define IDT_DESC_RING2        0x20
#define IDT_DESC_RING3        0x60
#define IDT_DESC_PRESENT      0x80

typedef void (*IRQ_HANDLER)(void);

struct __attribute__ ((packed)) IdtDescriptor {
  uint16_t base_lo;
  uint16_t selector;
  uint8_t reserved;
  uint8_t flags;
  uint16_t base_hi;
};

extern struct IdtDescriptor* GetInterruptDescriptor(uint32_t i);

extern int InstallInterruptHandler(uint32_t i, uint16_t flags, uint16_t selective, IRQ_HANDLER);

extern int InitializeIdt(uint16_t code_selector);

#endif  // __TIO_OS_IDT_H__