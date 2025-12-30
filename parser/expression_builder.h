#pragma once

#include "constructs.h"
#include "tokenizer.h"
#include "token_cursor.h"

typedef struct expression_array{
  struct expression_array* prev;
  expression_t exp;
  struct expression_array* next;
}expression_array_t;

void print_expression_array(expression_array_t *array);

