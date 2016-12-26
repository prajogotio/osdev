#include "string_tokenizer.h"
#include "core/kmalloc.h"
#include "core/string.h"

void StringTokenizer_Initialize(struct StringTokenizer* tokenizer, char* str, char delim) {
  tokenizer->str = str;
  tokenizer->delim = delim;
  tokenizer->cursor = 0;
}

bool StringTokenizer_GetNext(struct StringTokenizer* tokenizer, char* token) {
  if (!tokenizer->str[tokenizer->cursor]) {
    // cursor already points to end of string. No more token can be returned.
    token[0] = 0;
    return 0;
  }
  int start_index = tokenizer->cursor;
  while (tokenizer->str[tokenizer->cursor] != 0 &&
         tokenizer->str[tokenizer->cursor] != tokenizer->delim) {
    ++tokenizer->cursor;
  }
  int token_size = tokenizer->cursor - start_index;
  memcpy(&tokenizer->str[start_index], token, token_size);
  token[token_size] = 0;

  if (tokenizer->str[tokenizer->cursor] == tokenizer->delim) {
    ++tokenizer->cursor;
  }
  return 1;
}
