#include <stdio.h>
#include <stdbool.h>
#pragma once

typedef enum{
  TK_NONE,
  TK_NUMERIC,
  TK_IDENTIFIER,

  TK_KEYWORD,

  TK_DECL,

  TK_TYPE,

  TK_VECTOR,

  TK_BLOCK,

  TK_FORCE_EXP_END,
  TK_ENDLINE,

  TK_DMSN_IR,

  TK_CHAR,
  TK_STRING,
}token_type_t;

typedef enum{
  NUM_HEX_INT,
  NUM_DECIMAL_INT,
  NUM_OCTAL_INT,
  NUM_BINARY_INT,
  NUM_FLOAT,
  NUM_SCI_FLOAT

}numeric_literal_type_t;

typedef struct token{
  int line_number;
  int start_pos;
  int length;
  token_type_t type;
  char* content;
  union{
    bool is_open;
    int decl_const_lvl;
    int keyword_id;
    bool is_symbolic_identifier;
    numeric_literal_type_t number_type;
  };
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

void print_token(token_t* token);

void print_token_array(token_array_t* tokens);

token_array_t* tokenize_file(FILE* file);