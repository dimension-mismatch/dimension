#pragma once
#include <stdbool.h>
#include "constructs.h"
#include "tokenizer.h"
#include "hash_table/pattern_trie.h"
#include "error_handling/error_manager.h"

typedef struct token_cursor{
  token_array_t* array;
  int index;
  token_t tk;
  pattern_trie_t* fn_trie;
  pattern_trie_t* type_trie;
  error_manager_t* error_manager;
}token_cursor_t;

token_cursor_t tc_init(token_array_t *array, pattern_trie_t *fn_trie, pattern_trie_t *type_trie, error_manager_t *error_manager);

void tc_update(token_cursor_t *tc);

bool tc_inc(token_cursor_t *tc);

bool tc_is_asterisk(token_cursor_t *tc);

expression_t* add_dimension(type_identifier_t *typeid);

void pattern_push_entry(pattern_t *pattern, pattern_entry_t entry);
