#include "string_tokenizer.h"

void StringTokenizer_Initialize(struct StringTokenizer* tokenizer, char* str, char delim) {
  tokenizer->str = str;
  tokenizer->delim = delim;
  tokenizer->cursor = 0;
}

char* StringTokenizer_GetNext(struct StringTokenizer* tokenizer) {
  int start_index = tokenizer->cursor;
  while (tokenizer->str[tokenizer->cursor] != 0 ||
         tokenizer->str[tokenizer->cursor] != delim) {
    ++tokenizer->cursor;
  }
  int token_size = tokenizer->cursor - start_index;
  char* token = (char*) kmalloc(token_size+1);
  memcpy(&tokenizer->str[start_index], token, token_size);
  token[token_size] = 0;

  if (tokenizer->str[tokenizer->cursor] == delim) {
    ++tokenizer->cursor;
  }
  return token;
}
