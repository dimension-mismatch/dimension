#pragma once
#include "../tokenizer.h"
#include "../constructs.h"

typedef enum{
  ERR_EXPRESSION,
  ERR_FUNCTION,
  ERR_TYPE,
  ERR_TOKEN,
} error_arg_type_t;

typedef struct{
  error_arg_type_t type;
  union{
    expression_t* exp;
    int function_id;
    type_identifier_t* type_id;
    token_t* token;
  };
}error_arg_t;

typedef struct{
  int err_num;
  int arg_c;
  error_arg_t* arg_v;
  int token_idx;
} error_t;

typedef struct{
  int error_count;
  error_t* errors;

  int err_id_count;
  int* index;

  token_array_t* tokens;
} error_manager_t;

error_manager_t error_manager_init(token_array_t* tokens);

void throw_error(error_manager_t *errors, int error_num, int token_idx);

void err_expression_arg(error_manager_t *errors, expression_t *exp);

void err_token_arg(error_manager_t *errors, token_t *token);

void err_type_arg(error_manager_t *errors, type_identifier_t *type);

void err_function_arg(error_manager_t *errors, int fn_id);

void error_printout(error_manager_t* manager);
