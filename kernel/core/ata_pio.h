#ifndef __TIO_OS_ATA_PIO_H__
#define __TIO_OS_ATA_PIO_H__

#include "stdint.h"

// ATA PIO mode.
#define ATA_PIO_MASTER    0xE0
#define ATA_PIO_SLAVE     0xF0

// 28 bit LBA PIO
// num_of_sectors = 0 wrap around to 256
extern void AtaPioReadFromDisk(uint8_t ata_pio_target, uint32_t lba_28bit, uint8_t num_of_sectors, char* read_buffer);
extern void AtaPioRead(uint32_t buffer_read);
extern void AtaPioWriteToDisk(uint8_t ata_pio_target, uint32_t lba_28bit, uint8_t num_of_sectors, char* write_buffer);
extern void AtaPioInitialize();
#endif  //__TIO_OS_ATA_PIO_H__