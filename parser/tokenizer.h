#include <stdio.h>
#pragma once

typedef enum{
  NONE,
  NUMERIC,
  IDENTIFIER,
  SYMBOLIC,
  PROGRAM,
  ASSEMBLY,
}token_type_t;

typedef struct token{
  int line_number;
  int start_pos;
  int length;
  token_type_t type;
  char* content;
}token_t;

token_t new_empty_token();

void push_char(token_t* token, char next);

void destroy_token(token_t* token);

typedef struct{
  int token_count;
  token_t* tokens;
} token_array_t;



token_array_t* init_token_array();

void push_token_to_array(token_array_t* array, token_t token);

void destroy_token_array(token_array_t* array);

void finish_token_and_push_to_array(token_array_t* array, token_t* token, int col, int line);

void print_token(token_t* token);

void print_token_array(token_array_t* tokens);

token_array_t* tokenize_file(FILE* file);