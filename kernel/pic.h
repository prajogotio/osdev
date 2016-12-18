#ifndef __TIO_OS_PIC_H__
#define __TIO_OS_PIC_H__

#include "stdint.h"

#define PIC_IRQ_TIMER           0
#define PIC_IRQ_KEYBOARD        1
#define PIC_IRQ_SERIAL2         3
#define PIC_IRQ_SERIAL1         4
#define PIC_IRQ_PARALLEL2       5
#define PIC_IRQ_DISKETTE        6
#define PIC_IRQ_PARALLEL1       7

#define PIC_IRQ_CMOSTIMER       0
#define PIC_IRQ_CGARETRACE      1
#define PIC_IRQ_AUXILIARY       4
#define PIC_IRQ_FPU             5
#define PIC_IRQ_HDC             6

#define PIC_OCW2_MASK_L1        1
#define PIC_OCW2_MASK_L2        2
#define PIC_OCW2_MASK_L3        4
#define PIC_OCW2_MASK_EOI       0x20
#define PIC_OCW2_MASK_SL        0x40
#define PIC_OCW2_MASK_ROTATE    0x80

#define PIC_OCW3_MASK_RIS       1
#define PIC_OCW3_MASK_RIR       2
#define PIC_OCW3_MASK_MODE      4
#define PIC_OCW3_MASK_SMM       0x20
#define PIC_OCW3_MASK_ESMM      0x40
#define PIC_OCW3_MASK_D7        0x80

extern void PicSendCommand(uint8_t command, uint8_t pic_number);
extern void PicSendData(uint8_t data, uint8_t pic_number);
extern uint8_t PicReadData(uint8_t pic_number);
extern void InitializePic(uint8_t base0, uint8_t base1);

#endif  //__TIO_OS_PIC_H__