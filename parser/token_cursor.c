#include "token_cursor.h"
#include "tokenizer.h"
#include "constructs.h"
#include "hash_table/pattern_trie.h"
#include "error_handling/error_manager.h"
#include <stdbool.h>
#include <stdlib.h>

token_cursor_t tc_init(token_array_t* array, pattern_trie_t* fn_trie, pattern_trie_t* type_trie, error_manager_t* error_manager){
  token_cursor_t cursor = {.array = array,.index = 0, .tk = array->tokens[0], .fn_trie = fn_trie, .type_trie = type_trie, .error_manager = error_manager};
  return cursor;
}

void tc_update(token_cursor_t* tc){
  tc->tk = tc->array->tokens[tc->index];
}

bool tc_inc(token_cursor_t* tc){
  bool not_at_end = tc->index < tc->array->token_count - 1;
  if(not_at_end){
    tc->index++;
    tc_update(tc);
  }
  return not_at_end;
}

void tc_dec(token_cursor_t* tc){
  if(tc->index > 0){
    tc->index--;
    tc_update(tc);
  }
}

void tc_throw_error(token_cursor_t* tc, int error_num){
  throw_error(tc->error_manager, error_num, tc->index);
}

bool tc_is_asterisk(token_cursor_t* tc){
  return tc->tk.type == TK_IDENTIFIER && tc->tk.is_symbolic_identifier && tc->tk.length == 2 && tc->tk.content[0] == '*';
}



