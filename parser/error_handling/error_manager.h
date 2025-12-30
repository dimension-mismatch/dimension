#pragma once
#include "../expression_utils/expression_utils.h"
#include "../tokenizer.h"
#include "../hash_table/function_record.h"
#include "../hash_table/type_record.h"
#include "../hash_table/variable_record.h"


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

  pattern_trie_t* fn_rec;
  type_record_t* type_rec;
  variable_record_t* var_rec;

  token_array_t* tokens;
} parse_manager_t;

parse_manager_t parse_manager_init(token_array_t* tokens, pattern_trie_t *fn_rec, variable_record_t *var_rec, type_record_t *type_rec);

void throw_error(parse_manager_t *errors, int error_num, int token_idx);

void err_expression_arg(parse_manager_t *errors, expression_t *exp);

void err_token_arg(parse_manager_t *errors, token_t *token);

void err_type_arg(parse_manager_t *errors, type_identifier_t *type);

void err_function_arg(parse_manager_t *errors, int fn_id);

void error_printout(parse_manager_t* manager);
