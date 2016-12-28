#include "core/print.h"
#include "core/debug.h"
#include "core/hal.h"
#include "core/kmalloc.h"
#include "core/task.h"
#include "lib/string_tokenizer.h"

#define BUFFER_SIZE 4096

static char* CLI_PREFIX = "\ntio-os$ ";

static char command_buffer_[BUFFER_SIZE];
static int command_size_ = 0;

static void TimeDisplayer();
static void CommandLineInterface();

static void CliHandleCommand();
static void CliUpdateCommandBuffer(char c);
static bool CliDetectMultiwordCommand();
static void CliHandleWriteFile(struct StringTokenizer* tokenizer);
static void CliHandleReadFile(struct StringTokenizer* tokenizer);

void kernel_main() {
  __asm__("movw $0x10, %ax\n\t"
          "movw %ax, %ds\n\t"
          "movw %ax, %es\n\t"
          "movw %ax, %fs\n\t"
          "movw %ax, %gs\n\t");

  __asm__("movl $0xc0090000, %esp\n\t");
  __asm__("movl %esp, %ebp");
  Hal_memory_information = 0;
  __asm__("movl %%ebx, %0" : "=r"(Hal_memory_information));

  ClearScreen();

  PrintString("Initializing HAL...\n");

  HalInitialize();

  // Create 2 tasks for command line
  struct Task* timer_task = (struct Task*) kmalloc(sizeof(struct Task));
  TaskCreate(timer_task, TimeDisplayer, main_task.registers.eflags, (uint32_t*) main_task.registers.cr3);
  TaskSchedule(timer_task);


  struct Task* cli_task = (struct Task*) kmalloc(sizeof(struct Task));
  TaskCreate(cli_task, CommandLineInterface, main_task.registers.eflags, main_task.registers.cr3);
  TaskSchedule(cli_task);


  // For now, always reformat the disk first
  DiskFormat(); 

  for (;;);
}

static void TimeDisplayer() {
  for (;;) {
    DebugMoveCursor(0, 0);
    DebugPrintString("Uptime: ");
    DebugPrintInt(PitGetTickCount()/100);
    DebugPrintString("s");
  }
}

static void CommandLineInterface() {
  ClearScreen();
  PrintString("\n"
              "Welcome to Tio OS! The best OS ever!\n"
              "Timer and keyboard kinda works!\n");
  PrintString(CLI_PREFIX);
  for(;;) {
    while (!StdinBufferIsEmpty()) {
      char curkey = StdinBufferReadByte();
      if (curkey == '\n') {
        PrintChar(curkey);
        CliHandleCommand();
      } else if (curkey == 8) {
        // Backspace only if buffer is not empty
        if (command_size_ > 0) {
          command_buffer_[command_size_--] = 0;
          command_buffer_[command_size_] = 0;
          PrintChar(8);
        }
      } else {
        PrintChar(curkey);
        CliUpdateCommandBuffer(curkey);
      }
    }
  }
}

static void CliUpdateCommandBuffer(char curkey) {
  command_buffer_[command_size_++] = curkey;
  command_buffer_[command_size_] = 0;
}

static void CliHandleCommand() {
  bool command_found = 1;
  // One word command
  if (strcmp("hello", command_buffer_) == 0) {
    PrintString("Hello world!\n");
  } else if (strcmp("meminfo", command_buffer_) == 0) {
    MmapMemoryInformation();
  } else if (strcmp("ls", command_buffer_) == 0) {
    ListDirectoryContent();
  } else {
    command_found = CliDetectMultiwordCommand();
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

static bool CliDetectMultiwordCommand() {
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
    } else if (strcmp(token, "touch") == 0) {
      if (StringTokenizer_GetNext(tokenizer, token)) {
        CreateFile(token);
      } else {
        PrintString("Error: touch: no filename given");
      }
    } else if (strcmp(token, "write") == 0) {
      CliHandleWriteFile(tokenizer);
    } else if (strcmp(token, "read") == 0) {
      CliHandleReadFile(tokenizer);
    } else {
      command_found = 0;
    }
  }
  // Deallocate all memory
  kfree(tokenizer);
  kfree(token);
  return command_found;
}

static void CliHandleWriteFile(struct StringTokenizer* tokenizer) {
  char* filename = (char*) kmalloc(4096);
  if (!StringTokenizer_GetNext(tokenizer, filename)) {
    PrintString("Error: write: file name not supplied.\n");
    goto CleanupInitialCheck;
  }

  struct File* file;
  if (!OpenFile(&file, filename)) {
    PrintString("Error: write: file not found: ");
    PrintString(filename);
    PrintString("\n");
    goto CleanupOpenFilePhase;
  }

  char c = tokenizer->delim;
  StringTokenizer_SetDelimiter(tokenizer, 0);
  char* data = (char*) kmalloc(4096);
  if (!StringTokenizer_GetNext(tokenizer, data)) {
    PrintString("Error: write: no input supplied.\n");
    goto CleanupWritePhase;
  }
  int written = WriteFile(file, data, strlen(data));
  PrintInt(written);
  PrintString(" bytes is written to ");
  PrintString(filename);
  PrintString("\n");

CleanupWritePhase:
  StringTokenizer_SetDelimiter(tokenizer, c);
  kfree(data);

CleanupOpenFilePhase:
  CloseFile(&file);

CleanupInitialCheck:
  kfree(filename);
}

static void CliHandleReadFile(struct StringTokenizer* tokenizer) {
  char* filename = (char*) kmalloc(4096);
  if (!StringTokenizer_GetNext(tokenizer, filename)) {
    PrintString("Error: read: file name not supplied.\n");
    goto CleanupInitialCheck;
  }

  struct File* file;
  if (!OpenFile(&file, filename)) {
    PrintString("Error: read: file not found: ");
    PrintString(filename);
    PrintString("\n");
    goto CleanupOpenFilePhase;
  }

  char* data = (char*) kmalloc(4096);
  int written_len = ReadFile(file, data, strlen(data));
  PrintInt(written_len);
  PrintString(" bytes is read from ");
  PrintString(filename);
  PrintString("\n");

  for (int i = 0; i < written_len; ++i) {
    PrintChar(data[i]);
  }

CleanupReadPhase:
  kfree(data);

CleanupOpenFilePhase:
  CloseFile(&file);

CleanupInitialCheck:
  kfree(filename);
}
