#ifndef __TIO_OS_STRING_TOKENIZER_H__
#define __TIO_OS_STRING_TOKENIZER_H__

#include "core/stdint.h"

struct __attribute__((packed)) StringTokenizer {
  char* str;
  char delim;
  int cursor;
};

// Initialize a tokenizer for str
extern void StringTokenizer_Initialize(struct StringTokenizer* tokenizer, char* str, char delim);

// Set token to the next token returned by tokenizer. Returns 1 if a token is returned, 0 no more token is available.
extern bool StringTokenizer_GetNext(struct StringTokenizer* tokenizer, char* token);

#endif  //__TIO_OS_STRING_TOKENIZER_H__