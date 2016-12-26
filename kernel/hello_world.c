#include "core/print.h"
#include "core/debug.h"
#include "core/hal.h"
#include "core/kmalloc.h"
#include "lib/string_tokenizer.h"

#define BUFFER_SIZE 4096

static char* CLI_PREFIX = "tio-os$ ";

static char command_buffer_[BUFFER_SIZE];
static int command_size_ = 0;
static void HandleCommand();
static void UpdateCommandBuffer(char c);
static bool DetectMultiwordCommand();

void kernel_main() {
  __asm__("movw $0x10, %ax\n\t"
          "movw %ax, %ds\n\t"
          "movw %ax, %es\n\t"
          "movw %ax, %fs\n\t"
          "movw %ax, %gs\n\t");

  Hal_memory_information = 0;
  __asm__("movl %%ebx, %0" : "=r"(Hal_memory_information));

  ClearScreen();

  PrintString("Initializing HAL...\n");

  HalInitialize();
  ClearScreen();

  __asm__("sti");

  PrintString("\n"
              "Welcome to Tio OS! The best OS ever!\n"
              "Timer and keyboard kinda works!\n");
  PrintString(CLI_PREFIX);

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
  bool command_found = 1;
  // One word command
  if (strcmp("hello", command_buffer_) == 0) {
    PrintString("Hello world!\n");
  } else if (strcmp("meminfo", command_buffer_) == 0) {
    MmapMemoryInformation();
  } else if (strcmp("ls", command_buffer_) == 0) {
    ListDirectoryContent();
  } else {
    command_found = DetectMultiwordCommand();
  }

  if (!command_found) {
    PrintString("Command not recognized: ");
    PrintString(command_buffer_);
    PrintString("\n");
  }
  command_size_ = 0;
  command_buffer_[command_size_] = 0;
  PrintString(CLI_PREFIX);
}

static bool DetectMultiwordCommand() {
  struct StringTokenizer* tokenizer = (struct StringTokenizer*) kmalloc(sizeof(struct StringTokenizer));
  memset(tokenizer, 0, sizeof(struct StringTokenizer));
  char* token = (char*) kmalloc(sizeof(4096));
  memset(token, 0, 4096);

  bool command_found = 1;
  StringTokenizer_Initialize(tokenizer, command_buffer_, ' ');
  { // First phase: check if command is recognized
    // Second phase: use additional input arguments to fulfill the command 
    StringTokenizer_GetNext(tokenizer, token);
    if (strcmp(token, "mkdir") == 0) {
      if (StringTokenizer_GetNext(tokenizer, token) && strlen(token) != 0) {
        CreateDir(token);
        PrintString("Directory created: ");
        PrintString(token);
        PrintString("\n");
      } else {
        PrintString("Error: mkdir: no directory name supplied\n");
      }
    } else if (strcmp(token, "cd") == 0) {
      if (StringTokenizer_GetNext(tokenizer, token) && strlen(token) != 0 && ChangeDir(token)) {
        PrintString("Moved to directory: ");
        PrintString(token);
        PrintString("\n");
      } else {
        PrintString("Error: cd: specify a valid directory name\n");
      }
    } else if (strcmp(token, "diskutil") == 0) {
      if (StringTokenizer_GetNext(tokenizer, token) && strcmp(token, "fmt") == 0) {
        PrintString("Formatting disk... ");
        DiskFormat();
        PrintString("Done.\n");
      } else {
        PrintString("Error: diskutil: invalid option");
      }
    } else {
      command_found = 0;
    }
  }
  // Deallocate all memory
  kfree(tokenizer);
  kfree(token);
  return command_found;
}
