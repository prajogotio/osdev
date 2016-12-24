#include "ata_pio.h"
#include "hal.h"
#include "stdint.h"
#include "print.h"

#define ATA_PIO_DATA_PORT            0x1F0
#define ATA_PIO_ERROR_PORT           0x1F1
#define ATA_PIO_SECTOR_COUNT_PORT    0x1F2
#define ATA_PIO_SECTOR_LBA_28_1_PORT 0x1F3
#define ATA_PIO_SECTOR_LBA_28_2_PORT 0x1F4
#define ATA_PIO_SECTOR_LBA_28_3_PORT 0x1F5
#define ATA_PIO_DRIVE_PORT           0x1F6
#define ATA_PIO_CONTROLLER_PORT      0x1F7

#define ATA_PIO_READ_SECTORS_COMMAND 0x20

static uint32_t current_read_buffer_ = -1;

static void AtaPioIrqPrimary();
static void AtaPioIrqSecondary();

static void AtaPioIrqPrimary() {
  __asm__("pusha");
  PrintString("*** PRIMARY IRQ\n");
  __asm__("cld");   // REP INSW set to auto increment.
  __asm__("movl %0, %%edi" : : "r" (current_read_buffer_));
  // Specify port on which the data string will be read.
  __asm__("movl %0, %%dx" : : "i" (ATA_PIO_DATA_PORT));
  // Repeat 256 times (1 sector at a time)
  __asm__("movl $256, %cx");
  __asm__("rep insw");
  AtaPioDelay400ns();
  InterruptDone(14);
  __asm__("popa\n\t"
          "leave\n\t"
          "iret\n\t");
}

static void AtaPioIrqSecondary() {
  __asm__("cli\n\t");
  PrintString("*** From PIO Secondary bus: Unhandled");
  for(;;);
}

void AtaPioInitialize() {
  // IRQ 14 is from Primary bus.
  SetInterruptVector(0x20 + 14, AtaPioIrqPrimary);
  // IRQ 15 is from Secondary bus.
  SetInterruptVector(0x20 + 15, AtaPioIrqSecondary);
}

void AtaPioReadFromDisk(uint8_t ata_pio_target, uint32_t lba_28bit, uint8_t num_of_sectors, uint32_t read_buffer) {
  current_read_buffer_ = read_buffer;

  uint8_t cmd = ata_pio_target | ((lba_28bit >> 24) & 0xf);
  WriteToIoPort(ATA_PIO_DRIVE_PORT, cmd);
  WriteToIoPort(ATA_PIO_ERROR_PORT, 0x00);
  // Set number of sectors to read
  WriteToIoPort(ATA_PIO_SECTOR_COUNT_PORT, num_of_sectors);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_1_PORT, lba_28bit & 0xff);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_2_PORT, (lba_28bit >> 8) & 0xff);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_3_PORT, (lba_28bit >> 16) & 0xff);
  WriteToIoPort(ATA_PIO_CONTROLLER_PORT, ATA_PIO_READ_SECTORS_COMMAND);
}

void AtaPioDelay400ns() {
  __asm__("mov $0x1f7, %dx\n\t"
          "inb %dx, %al\n\t"
          "inb %dx, %al\n\t"
          "inb %dx, %al\n\t"
          "inb %dx, %al\n\t");
}