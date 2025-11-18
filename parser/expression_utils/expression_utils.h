#pragma once
#include "../type_utils/type_utils.h"
#include "../hash_table/variable_record.h"
typedef enum{
  EXP_NONE,
  EXP_IDENTIFIER,
  EXP_GROUPING,
  EXP_DECLARE_VAR,
  EXP_READ_VAR,
  EXP_CALL_FN,
  EXP_WRITE_VAR,
  EXP_VECTOR_LITERAL,
  EXP_SINGLE_LITERAL,
  EXP_BLOCK
}expression_type_t;

typedef struct expression{
  expression_type_t type;
  type_identifier_t return_type;
  char* text;
  union{
    struct{
      int var_id;
    } read;
    struct{
      int fn_id;
      int arg_c;
      struct expression** arg_v;
      dimension_t multiplicity;
    } function_call;
    struct{
      int stack_depth;
      int arg_c;
      struct expression** arg_v;
    } block;
    struct{
      int var_id;
      struct expression** value;
    } write; 
    struct{
      int component_count;
      struct expression** components;
    } vector_literal;
    int enter_group;
    int numeric_literal;
  };
}expression_t;


typedef struct linked_expression{
  struct linked_expression* next;
  expression_t* expression;
}exp_array_t;

expression_t *exp_init(expression_type_t type, type_identifier_t returnType);

expression_t *exp_create_identifier(char *content);

expression_t *exp_create_var_declaration(type_identifier_t returnType, char *name);

expression_t *exp_create_var_read(type_identifier_t returnType, int varid);

expression_t *exp_create_var_write(type_identifier_t returnType, int varid, expression_t *value);

expression_t *exp_create_grouping(int enter_group);

expression_t *exp_create_numeric_literal(int value);

expression_t *exp_create_block();

void exp_block_push_line(expression_t *block, expression_t *line);

void exp_array_push_expression(exp_array_t **root, exp_array_t **current_node, expression_t *expression);

void print_expression(expression_t *exp);

void print_exp_array(exp_array_t *array);

void exp_destroy(expression_t *exp);

void exp_array_destroy(exp_array_t *array);
