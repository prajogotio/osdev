#ifndef __TIO_OS_STRING_TOKENIZER_H__
#define __TIO_OS_STRING_TOKENIZER_H__

struct __attribute__((packed)) StringTokenizer {
  char* str;
  char delim;
  int cursor;
};

// Initialize a tokenizer for str
extern void StringTokenizer_Initialize(struct StringTokenizer* tokenizer, char* str, char delim);

// The returned pointers to string have to be freed by the caller.
extern char* StringTokenizer_GetNext(struct StringTokenizer* tokenizer);

#endif  //__TIO_OS_STRING_TOKENIZER_H__