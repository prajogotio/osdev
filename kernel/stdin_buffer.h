#ifndef __TIO_OS_STDIN_BUFFER_H__
#define __TIO_OS_STDIN_BUFFER_H__

#include "stdint.h"

void PushToStdinBuffer(char c);
char ReadFromStdin();
bool IsStdinBufferEmpty();


#endif  //__TIO_OS_STDIN_BUFFER_H__