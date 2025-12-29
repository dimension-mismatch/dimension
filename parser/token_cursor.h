#pragma once
#include <stdbool.h>

#include "constructs.h"
#include "parser.h"
#include "tokenizer.h"

typedef struct token_cursor{
  token_array_t* array;
  int index;
  token_t tk;
}token_cursor_t;

token_cursor_t tc_init(token_array_t *array);

void tc_update(token_cursor_t *tc);

bool tc_inc(token_cursor_t *tc);

bool tc_is_asterisk(token_cursor_t *tc);

expression_t* add_dimension(type_identifier_t *typeid);

void pattern_push_entry(pattern_t *pattern, pattern_entry_t entry);
