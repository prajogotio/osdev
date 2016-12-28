#include "pit.h"
#include "hal.h"
#include "stdint.h"
#include "task.h"

#define PIT_REG_COUNTER0    0x40
#define PIT_REG_COUNTER1    0x41
#define PIT_REG_COUNTER2    0x42
#define PIT_REG_COMMAND     0x43


static uint32_t pit_ticks_ = 0;


void PitIncreaseTickCount() {
  pit_ticks_++;
}

uint32_t PitSetTickCount(uint32_t i) {
  uint32_t ret = pit_ticks_;
  pit_ticks_ = i;
  return ret;
}

uint32_t PitGetTickCount() {
  return pit_ticks_;
}

void PitSendCommand(uint8_t cmd) {
  WriteToIoPort(PIT_REG_COMMAND, cmd);
}

void PitSendData(uint16_t data, uint8_t counter) {
  uint8_t port = (counter == PIT_OCW_COUNTER_0) ? PIT_REG_COUNTER0 : (
    (counter == PIT_OCW_COUNTER_1) ? PIT_REG_COUNTER1 : PIT_REG_COUNTER2);
  WriteToIoPort(port, (uint8_t) data);
}

uint8_t PitReadData(uint16_t counter) {
  uint8_t port = (counter == PIT_OCW_COUNTER_0) ? PIT_REG_COUNTER0 : (
    (counter == PIT_OCW_COUNTER_1) ? PIT_REG_COUNTER1 : PIT_REG_COUNTER2);
  return ReadFromIoPort(port);
}

void PitStartCounter(uint32_t freq, uint8_t counter, uint8_t mode) {
  if (freq == 0) return;
  uint16_t divisor = (uint16_t) (1193181 / (uint16_t) freq);
  uint8_t ocw = 0;
  ocw = (ocw & ~PIT_OCW_MASK_MODE) | mode;
  ocw = (ocw & ~PIT_OCW_MASK_RL) | PIT_OCW_RL_DATA;
  ocw = (ocw & ~PIT_OCW_MASK_COUNTER) | counter;
  PitSendCommand(ocw);

  PitSendData(divisor & 0xff, 0);
  PitSendData((divisor >> 8) & 0xff, 0);

  pit_ticks_ = 0;
}

void InitializePit() {
  SetInterruptVector(32, PitIrq);
}