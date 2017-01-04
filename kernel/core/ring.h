#ifndef __TIO_OS_RING_H__
#define __TIO_OS_RING_H__

#include "stdint.h"

struct __attribute__((packed)) TssEntry {
  uint32_t prev_tss, esp0, ss0;
  uint32_t esp1, ss1, esp2, ss2, cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi, es, cs, ss, ds, fs, gs, ldt, trap, iomap_base;
};

extern void TssFlush();
extern void TssInstall(uint32_t index, uint16_t kernel_segment_selector, uint32_t kernel_esp);

extern void RingEnterUserMode(uint32_t directory, uint32_t cr3, uint32_t esp);
extern void RingTestUserMode();
extern void RingTestUserFunction();

#endif  //__TIO_OS_RING_H__