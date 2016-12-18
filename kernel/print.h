#ifndef __TIO_OS_PRINT_H__
#define __TIO_OS_PRINT_H__

extern void PrintChar(char c);
extern void PrintString(char* str);
extern void PrintHex(unsigned int val);
extern void MoveCursor(unsigned int x, unsigned int y);
extern void ClearScreen();
extern int GetCursorX();
extern int GetCursorY();

#endif  //__TIO_OS_PRINT_H__