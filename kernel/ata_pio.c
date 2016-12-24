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

#define ATA_PIO_READ_SECTORS_COMMAND    0x20
#define ATA_PIO_WRITE_SECTORS_COMMAND   0x30
#define ATA_PIO_CACHE_FLUSH_COMMAND     0xE7

#define ATA_PIO_READ_MODE   1
#define ATA_PIO_WRITE_MODE  2

static uint32_t current_read_buffer_pointer_ = -1;
static uint32_t current_write_buffer_pointer_ = -1;
static uint32_t number_of_sectors_to_process_ = 0;

static int current_mode_ = -1;
static char ata_lock_ = 0;

static void AtaPioIrqPrimary();
static void AtaPioIrqSecondary();
static void AtaPioIrqHandleRead();
static void AtaPioIrqHandleWrite();
static void AtaPioDelay400ns();

static void AtaPioIrqPrimary() {
  __asm__("pusha");
  if (current_mode_ == ATA_PIO_READ_MODE) {
    PrintString("*** PRIMARY READ IRQ\n");
    AtaPioIrqHandleRead();
  } else {
    PrintString("*** PRIMARY WRITE IRQ\n");
    AtaPioIrqHandleWrite();
  }
  InterruptDone(14);
  __asm__("popa\n\t"
          "leave\n\t"
          "iret\n\t");
}

static void AtaPioIrqHandleRead() {
  while (ReadFromIoPort(ATA_PIO_CONTROLLER_PORT) & 0x80);
  __asm__("cld");   // REP INSW set to auto increment.
  __asm__("movl %0, %%edi" : : "r" (current_read_buffer_pointer_));
  // Specify port on which the data string will be read.
  __asm__("movl %0, %%dx" : : "i" (ATA_PIO_DATA_PORT));
  // Repeat 256 times (1 sector at a time)
  __asm__("movl $256, %cx");
  __asm__("rep insw");
  // Advance buffer pointer by 512 bytes.
  current_read_buffer_pointer_ += 128;
  --number_of_sectors_to_process_;
  // Allow for 400ns delay to allow the controller to update its status.
  AtaPioDelay400ns();
  if (number_of_sectors_to_process_ == 0) {
    AtaPioUnlock();
  }
}

static void AtaPioIrqHandleWrite() {
  while (ReadFromIoPort(ATA_PIO_CONTROLLER_PORT) & 0x80);
  if (number_of_sectors_to_process_ == 0) {
    // Writing done!
    AtaPioUnlock();
    return;
  }

  __asm__("cld");   // OUTSW set to auto increment.  
  __asm__("movl %0, %%esi" : : "r" (current_write_buffer_pointer_));  
  // Specify port on which the data string will be read.
  __asm__("movw %0, %%dx" : : "i" (ATA_PIO_DATA_PORT));  
  // Repeat 256 times
  for (int i = 0; i < 256; ++i) {
    __asm__("outsw");
  }
  // Advance buffer pointer by 512 bytes.
  current_write_buffer_pointer_ += 128;
  --number_of_sectors_to_process_;
  // Allow for 400ns delay to allow the controller to update its status.
  AtaPioDelay400ns();
  // Send clear cache command
  WriteToIoPort(ATA_PIO_CONTROLLER_PORT, ATA_PIO_CACHE_FLUSH_COMMAND);
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
  while (!AtaPioLock());

  current_read_buffer_pointer_ = read_buffer;
  current_mode_ = ATA_PIO_READ_MODE;
  number_of_sectors_to_process_ = num_of_sectors;

  uint8_t cmd = ata_pio_target | ((lba_28bit >> 24) & 0xf);
  WriteToIoPort(ATA_PIO_DRIVE_PORT, cmd);
  WriteToIoPort(ATA_PIO_ERROR_PORT, 0x00);
  // Set number of sectors to read
  WriteToIoPort(ATA_PIO_SECTOR_COUNT_PORT, num_of_sectors);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_1_PORT, lba_28bit & 0xff);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_2_PORT, (lba_28bit >> 8) & 0xff);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_3_PORT, (lba_28bit >> 16) & 0xff);
  WriteToIoPort(ATA_PIO_CONTROLLER_PORT, ATA_PIO_READ_SECTORS_COMMAND);

  AtaPioDelay400ns();
}

void AtaPioWriteToDisk(uint8_t ata_pio_target, uint32_t lba_28bit, uint8_t num_of_sectors, uint32_t write_buffer) {
  while (!AtaPioLock());

  current_write_buffer_pointer_ = write_buffer;
  current_mode_ = ATA_PIO_WRITE_MODE;
  number_of_sectors_to_process_ = num_of_sectors;

  uint8_t cmd = ata_pio_target | ((lba_28bit >> 24) & 0xf);
  WriteToIoPort(ATA_PIO_DRIVE_PORT, cmd);
  WriteToIoPort(ATA_PIO_ERROR_PORT, 0x00);
  // Set number of sectors to write
  WriteToIoPort(ATA_PIO_SECTOR_COUNT_PORT, num_of_sectors);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_1_PORT, lba_28bit & 0xff);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_2_PORT, (lba_28bit >> 8) & 0xff);
  WriteToIoPort(ATA_PIO_SECTOR_LBA_28_3_PORT, (lba_28bit >> 16) & 0xff);
  WriteToIoPort(ATA_PIO_CONTROLLER_PORT, ATA_PIO_WRITE_SECTORS_COMMAND);
  AtaPioIrqHandleWrite();
}

void AtaPioDelay400ns() {
  __asm__("mov $0x1f7, %dx\n\t"
          "inb %dx, %al\n\t"
          "inb %dx, %al\n\t"
          "inb %dx, %al\n\t"
          "inb %dx, %al\n\t");
}

bool AtaPioLock() {
  char test_var = 1;
  __asm__ __volatile__(
    "xchgb %0, (%1)" : "=d"(test_var) : "a"(&ata_lock_), "d"(test_var)
    );
  return !test_var;
}

void AtaPioUnlock() {
  ata_lock_ = 0;
}