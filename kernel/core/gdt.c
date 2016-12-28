#include "gdt.h"
#include "stdint.h"

struct __attribute__ ((packed)) GdtRegister {
  uint16_t limit;   // GDT limit.
  uint32_t base;    // GDT base.
};


static struct GdtDescriptor gdt_[MAX_DESCRIPTORS];
static struct GdtRegister gdtr_;

static void GdtInstall() {
  __asm__("lgdt (%0)" : : "a" (&gdtr_));
}

static void ClearGdtDescriptor(struct GdtDescriptor* gdt) {
  gdt->limit = 0;
  gdt->base_lo = 0;
  gdt->base_mid = 0;
  gdt->base_hi = 0;
  gdt->flags = 0;
  gdt->grand = 0;
}

void GdtSetDescriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand) {
  if (i > MAX_DESCRIPTORS) return;
  ClearGdtDescriptor(&gdt_[i]);
  gdt_[i].base_lo = (uint16_t) (base & 0xffff);
  gdt_[i].base_mid = (uint8_t) ((base >> 16) & 0xff);
  gdt_[i].base_hi = (uint8_t) ((base >> 24) & 0xff);
  gdt_[i].limit = (uint16_t) (limit & 0xffff);
  gdt_[i].flags = access;
  gdt_[i].grand = (uint8_t) ((limit >> 16) & 0x0f);
  gdt_[i].grand |= grand & 0xf0;
}

struct GdtDescriptor* GdtGetDescriptor(int i) {
  if (i > MAX_DESCRIPTORS) return 0;
  return &gdt_[i];
}

int InitializeGdt() {
  gdtr_.limit = (sizeof (struct GdtDescriptor) * MAX_DESCRIPTORS) - 1;
  gdtr_.base = (uint32_t) &gdt_[0];

  GdtSetDescriptor(0, 0, 0, 0, 0);
  GdtSetDescriptor(1, 0, 0xffffffff, GDT_DESC_READWRITE|GDT_DESC_EXEC_CODE|GDT_DESC_CODEDATA|GDT_DESC_MEMORY,
    GDT_GRAND_4K|GDT_GRAND_32BIT|GDT_GRAND_LIMITHI_MASK);
  GdtSetDescriptor(2, 0, 0xffffffff,
    GDT_DESC_READWRITE|GDT_DESC_CODEDATA|GDT_DESC_MEMORY,
    GDT_GRAND_4K|GDT_GRAND_32BIT|GDT_GRAND_LIMITHI_MASK);
  GdtSetDescriptor(3, 0, 0xffffffff,
    GDT_DESC_READWRITE|GDT_DESC_EXEC_CODE|GDT_DESC_CODEDATA|GDT_DESC_MEMORY|GDT_DESC_DPL,
    GDT_GRAND_4K|GDT_GRAND_32BIT|GDT_GRAND_LIMITHI_MASK);
  GdtSetDescriptor(4, 0, 0xffffffff,
    GDT_DESC_READWRITE|GDT_DESC_CODEDATA|GDT_DESC_MEMORY|GDT_DESC_DPL,
    GDT_GRAND_4K|GDT_GRAND_32BIT|GDT_GRAND_LIMITHI_MASK);
  GdtInstall();
}