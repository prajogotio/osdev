#ifndef __TIO_OS_HAL_H__
#define __TIO_OS_HAL_H__

#include "stdint.h"
#include "physical.h"
#include "stdin_buffer.h"
#include "string.h"
#include "ata_pio.h"

#define KERNEL_OFFSET 0xc0000000

extern uint32_t* Hal_memory_information;

extern int HalInitialize();
extern int HalShutdown();
extern void GenerateInterrupt(int n);
extern void WriteToIoPort(unsigned int port, char value);
extern uint8_t ReadFromIoPort(unsigned int port);
extern void InterruptDone(unsigned intno);
extern void SetInterruptVector(int intno, void (*vect)());

#endif // __TIO_OS_HAL_H__