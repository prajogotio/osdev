#include "print.h"
#include "debug.h"
#include "hal.h"

#define BUFFER_SIZE 4096

static char* CLI_PREFIX = "tio-os$ ";

static char command_buffer_[BUFFER_SIZE];
static int command_size_ = 0;
static void HandleCommand();
static void UpdateCommandBuffer(char c);

void kernel_main() {
  __asm__("movw $0x10, %ax\n\t"
          "movw %ax, %ds\n\t"
          "movw %ax, %es\n\t"
          "movw %ax, %fs\n\t"
          "movw %ax, %gs\n\t");

  Hal_memory_information = 0;
  __asm__("movl %%ebx, %0" : "=r"(Hal_memory_information));

  ClearScreen();

  char* welcome_message = 
               "SCROLLING TEST: THIS ROW SHOULD NOT BE PRINTED\n"
               "0123456789---------2---------3---------4---------5---------6---------7---------8"
               "1        Hello There, Welcome to Tio OS!\n"
               "2       By Prajogo Tio, 2016\n"
               "3\n"
               "4        This OS is OSomeee\n"
               "5TESTMORETHAN801XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXXTHISSHOULDBEONANEWLINE\n"
               "7\n"
               "8\n"
               "9\n"
               "10\n"
               "11\n"
               "12\n"
               "13\n"
               "14\n"
               "15\n"
               "16\n"
               "17\n"
               "18\n"
               "19\n"
               "20\n"
               "21\n"
               "22\n"
               "23\n"
               ;
  PrintString(welcome_message);
  PrintString("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nScrolling seems to work. Let's initialize HAL...\n");

  HalInitialize();
  ClearScreen();
  __asm__("sti");

  PrintString("\n"
              "Welcome to Tio OS! The best OS ever!\n"
              "Timer and keyboard kinda works!\n");
  PrintString(CLI_PREFIX);
  AtaPioReadFromDisk(ATA_PIO_MASTER, 0x1000, 5, 0x100);
  AtaPioWriteToDisk(ATA_PIO_MASTER, 0x1000, 4, 0x100);
  AtaPioReadFromDisk(ATA_PIO_MASTER, 0x1000, 5, 0x100);
  AtaPioWriteToDisk(ATA_PIO_MASTER, 0x1000, 4, 0x100);




  for (;;) {
    DebugMoveCursor(0, 0);
    DebugPrintString("Uptime: ");
    DebugPrintInt(PitGetTickCount()/100);
    DebugPrintString("s");

    while (!StdinBufferIsEmpty()) {
      char curkey = StdinBufferReadByte();
      if (curkey == '\n') {
        PrintChar(curkey);
        HandleCommand();
      } else if (curkey == 8) {
        // Backspace only if buffer is not empty
        if (command_size_ > 0) {
          command_buffer_[command_size_--] = 0;
          command_buffer_[command_size_] = 0;
          PrintChar(8);
        }
      } else {
        PrintChar(curkey);
        UpdateCommandBuffer(curkey);
      }
    }
  }

  __asm__("cli \n\t"
          "hlt \n\t");
}

static void UpdateCommandBuffer(char curkey) {
  command_buffer_[command_size_++] = curkey;
  command_buffer_[command_size_] = 0;
}

static void HandleCommand() {
  if (strcmp("hello", command_buffer_) == 0) {
    PrintString("Hello world!\n");
  } else if (strcmp("meminfo", command_buffer_) == 0) {
    MmapMemoryInformation();
  } else {
    PrintString("Command not recognized: ");
    PrintString(command_buffer_);
    PrintString("\n");
  }
  command_size_ = 0;
  command_buffer_[command_size_] = 0;
  PrintString(CLI_PREFIX);
}
