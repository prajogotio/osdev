#include "core/print.h"
#include "core/debug.h"
#include "core/hal.h"
#include "core/kmalloc.h"
#include "core/task.h"
#include "core/ring.h"
#include "lib/string_tokenizer.h"

#define BUFFER_SIZE 4096

static char* CLI_PREFIX = "\ntio-os$ ";

static char command_buffer_[BUFFER_SIZE];
static int command_size_ = 0;

static void TimeDisplayer();
static void CommandLineInterface();
static void TioOsMarquee();

static void CliHandleCommand();
static void CliUpdateCommandBuffer(char c);
static bool CliDetectMultiwordCommand();
static void CliHandleWriteFile(struct StringTokenizer* tokenizer);
static void CliHandleReadFile(struct StringTokenizer* tokenizer);

void kernel_main() {
  __asm__("movl %%ebx, %0" : "=r"(Hal_memory_information));
  HalInitialize();
  ClearScreen();

  struct Task* timer_task = (struct Task*) kmalloc(sizeof(struct Task));
  memset(timer_task, 0, sizeof(struct Task));
  TaskCreate(timer_task, TimeDisplayer, main_task.registers.eflags, (uint32_t*) main_task.registers.cr3);  
  TaskSchedule(timer_task);

  struct Task* banner_task = (struct Task*) kmalloc(sizeof(struct Task));
  memset(banner_task, 0, sizeof(struct Task));
  TaskCreate(banner_task, TioOsMarquee, main_task.registers.eflags, (uint32_t*) main_task.registers.cr3);
  TaskSchedule(banner_task);  

  struct Task* cli_task = (struct Task*) kmalloc(sizeof(struct Task));
  memset(cli_task, 0, sizeof(struct Task));
  TaskCreate(cli_task, CommandLineInterface, main_task.registers.eflags, (uint32_t*) main_task.registers.cr3);
  TaskSchedule(cli_task);

  // For now, always reformat the disk first
  DiskFormat();
  
  struct Task* user_task = (struct Task*) kmalloc(sizeof(struct Task));
  memset(user_task, 0, sizeof(struct Task));
  TaskCreateUserProcess(user_task, RingTestUserFunction, main_task.registers.eflags);
  TaskSchedule(user_task);

  for(;;);
}

static void TioOsMarquee() {
  // Just used to test multi tasking
  int last_moved = PitGetTickCount()-100;
  char* marquee_buffer = (char*) kmalloc(15);
  memset(marquee_buffer, 0, 15);
  char* banner = ":Tio OS:";
  int cursor = 0;
  for (;;) {
    int now = PitGetTickCount();
    if (now - last_moved >= 100) {
      last_moved = now;
      memset(marquee_buffer, '=', 14);
      cursor = (cursor+1)%14;
      for (int i = 0; i < 8; ++i) {
        marquee_buffer[(cursor+i)%14] = banner[i];
      }
      while (!DebugPrintLock());
      DebugMoveCursor((80-15)/2, 0);
      DebugPrintString(marquee_buffer);
      DebugPrintUnlock();
    }
  }
}

static void TimeDisplayer() {
  int last_updated = PitGetTickCount() - 100;
  for (;;) {
    int now = PitGetTickCount();
    if (now - last_updated < 100) {
      continue;
    }
    last_updated = now;
    while (!DebugPrintLock());
    DebugMoveCursor(0, 0);
    DebugPrintString("Uptime: ");
    DebugPrintInt(PitGetTickCount()/100);
    DebugPrintString("s");
    DebugPrintUnlock();
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

  char* token = (char*) kmalloc(4096);
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
  memset(filename, 0, 4096);
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
  memset(data, 0, 4096);
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
  memset(filename, 0, 4096);
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
  memset(data, 0, 4096);
  int read_len = ReadFile(file, data, 4096);
  PrintInt(read_len);
  PrintString(" bytes is read from ");
  PrintString(filename);
  PrintString("\n");

  for (int i = 0; i < read_len; ++i) {
    PrintChar(data[i]);
  }

CleanupReadPhase:
  kfree(data);

CleanupOpenFilePhase:
  CloseFile(&file);

CleanupInitialCheck:
  kfree(filename);
}
