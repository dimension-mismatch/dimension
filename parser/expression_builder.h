#pragma once

#include "constructs.h"
#include "tokenizer.h"
#include "token_cursor.h"

typedef struct expression_array{
  struct expression_array* prev;
  expression_t exp;
  int tk_begin;
  int tk_end;
  struct expression_array* next;
}expression_array_t;

void print_expression_array(expression_array_t *array);

bool build_expression(pattern_trie_t *trie, expression_array_t *array, expression_t* result, bool allow_vectors, error_manager_t* errors);
