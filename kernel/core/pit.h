#ifndef __TIO_OS_PIT_H__
#define __TIO_OS_PIT_H__

#include "stdint.h"

#define PIT_OCW_MASK_BINCOUNT     1
#define PIT_OCW_MASK_MODE         0xE
#define PIT_OCW_MASK_RL           0x30
#define PIT_OCW_MASK_COUNTER      0xC0

#define PIT_OCW_BINCOUNT_BINARY      0
#define PIT_OCW_BINCOUNT_BCD         1
#define PIT_OCW_MODE_TERMINALCOUNT   0
#define PIT_OCW_MODE_ONESHOT         0x2
#define PIT_OCW_MODE_RATEGEN         0x4
#define PIT_OCW_MODE_SQUAREWAVEGEN   0x6
#define PIT_OCW_MODE_SOFTWARETRIG    0x8
#define PIT_OCW_MODE_HARDWARETRIG    0xA
#define PIT_OCW_RL_LATCH             0
#define PIT_OCW_RL_LSBONLY           0x10
#define PIT_OCW_RL_MSBONLY           0x20
#define PIT_OCW_RL_DATA              0x30
#define PIT_OCW_COUNTER_0            0
#define PIT_OCW_COUNTER_1            0x40
#define PIT_OCW_COUNTER_2            0x80


extern void PitIrq();
extern void PitIncreaseTickCount();
extern uint32_t PitSetTickCount(uint32_t i);
extern uint32_t PitGetTickCount();
extern void PitSendCommand(uint8_t cmd);
extern void PitSendData(uint16_t data, uint8_t counter);
extern uint8_t PitReadData(uint16_t counter);
extern void PitStartCounter(uint32_t freq, uint8_t counter, uint8_t mode);
extern void InitializePit();

#endif // __TIO_OS_PIT_H__