#ifndef __TIO_OS_GDT_H__
#define __TIO_OS_GDT_H__

#include "stdint.h"

#define MAX_DESCRIPTORS              6
#define GDT_DESC_ACCESS         0x0001
#define GDT_DESC_READWRITE      0x0002
#define GDT_DESC_EXPANSION      0x0004
#define GDT_DESC_EXEC_CODE      0x0008
#define GDT_DESC_CODEDATA       0x0010
#define GDT_DESC_DPL            0x0060
#define GDT_DESC_MEMORY         0x0080
#define GDT_GRAND_LIMITHI_MASK  0x0f
#define GDT_GRAND_OS            0x10
#define GDT_GRAND_32BIT         0x40
#define GDT_GRAND_4K            0x80

struct __attribute__ ((packed)) GdtDescriptor {
  // 15 bit segment limit.
  uint16_t  limit;

  // 24 bit base address.
  uint16_t  base_lo;
  uint8_t   base_mid;

  uint8_t   flags;
  uint8_t   grand;

  // 24-32 bits base high.
  uint8_t   base_hi;
};

extern void GdtSetDescriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand);
extern struct GdtDescriptor* GdtGetDescriptor(int i);
extern int InitializeGdt();

#endif  // __TIO_OS_GDT_H__