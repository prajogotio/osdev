#ifndef __TIO_OS_STDIN_BUFFER_H__
#define __TIO_OS_STDIN_BUFFER_H__

#include "stdint.h"

void StdinBufferWriteByte(char c);
char StdinBufferReadByte();
bool StdinBufferIsEmpty();


#endif  //__TIO_OS_STDIN_BUFFER_H__