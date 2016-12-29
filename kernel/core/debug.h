#ifndef __TIO_OS_DEBUG_PRINT_H__
#define __TIO_OS_DEBUG_PRINT_H__

extern void DebugPrintChar(char c);
extern void DebugPrintString(char* str);
extern void DebugPrintHex(unsigned int val);
extern void DebugMoveCursor(unsigned int x, unsigned int y);
extern void DebugPrintInt(int value);
extern int DebugPrintLock();
extern void DebugPrintUnlock();

#endif  //__TIO_OS_DEBUG_PRINT_H__