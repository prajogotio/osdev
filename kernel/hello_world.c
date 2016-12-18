#include "print.h"
#include "debug.h"
#include "hal.h"
#include "pit.h"
#include "stdint.h"


void kernel_main() {
  __asm__("movw $0x10, %ax\n\t"
          "movw %ax, %ds\n\t"
          "movw %ax, %es\n\t"
          "movw %ax, %fs\n\t"
          "movw %ax, %gs\n\t");
  uint32_t* memory_information = 0;
  uint32_t* memory_map_table = 0;
  __asm__("movl %%ebx, %0" : "=r"(memory_information));
  __asm__("movl %%ecx, %0" : "=r"(memory_map_table));

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
  PrintString("\n"
              "Welcome to Tio OS! The best OS ever!\n"
              "Timer and keyboard kinda works!\n");

  __asm__("sti");
  PrintString("Extended Memory between 1MB to 16MB (in KB): ");
  PrintHex((int) *memory_information);
  PrintString("\nExtended Memory between > 16MB (in 64KB): ");
  PrintHex((int) *(memory_information+1));
  PrintString("\nConfigured Memory between 1MB to 16MB (in KB): ");
  PrintHex((int) *(memory_information+2));
  PrintString("\nConfigured Memory between > 16MB (in 64KB): ");
  PrintHex((int) *(memory_information+3));
  PrintString("\n");


  PrintString("MemoryMap entries: ");
  int entry_size = *memory_map_table;
  PrintHex(entry_size);
  PrintString("\n");
  for (int index = 0; index <= entry_size; ++index) {
    PrintString("Entry: ");
    int offset = 1+index*6;
    PrintString("\n  Base address: ");
    PrintHex((int) *(memory_map_table+offset));
    PrintString("\n  Length: ");
    PrintHex((int) *(memory_map_table+2+offset));
    PrintString("\n  Type: ");
    PrintHex((int) *(memory_map_table+4+offset));
    PrintString("\n  ACPI_NULL: ");
    PrintHex((int) *(memory_map_table+5+offset));
    PrintString("\n");
  }

  for (;;) {
    DebugMoveCursor(0, 0);
    DebugPrintString("Current tick count: ");
    DebugPrintHex(PitGetTickCount());
    DebugPrintString(" which is ");
    DebugPrintHex(PitGetTickCount()/100);
    DebugPrintString(" s                            ");
  }

  __asm__("cli \n\t"
          "hlt \n\t");
}
