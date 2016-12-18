#include "print.h"
#include "debug.h"
#include "hal.h"
#include "pit.h"
#include "stdint.h"

void main() {
  uint32_t* memory_information = 0;
  __asm__("movl %%ebx, %0" : "=r"(memory_information));

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
