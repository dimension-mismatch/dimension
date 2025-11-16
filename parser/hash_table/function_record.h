#pragma once
#include "../type_utils/type_utils.h"
#include "hash_table.h"
#include "variable_record.h"
#include "../expression_utils/expression_utils.h"

//Functions can be either implemented in Dimension or Assembly
enum fn_impl_type{
  FN_IMPL_DMSN,
  FN_IMPL_ASM 
};


struct function_def{
  int num_parameters;
  variable_t* parameters;
  type_identifier_t return_type;
  int priority;
  enum fn_impl_type impl_type;
  union{
    char* assembly;
    expression_t* dmsn;
  };
};

struct fn_tree{
  hash_table_t type_table;
  hash_table_t name_table;
  int children_count;
  struct fn_tree* children;
  int id_number;
};

typedef struct{
  struct fn_tree* root;
  int scope_depth;
  int def_count;
  int* scopes;
  struct function_def* definitions;
}function_record_t;

function_record_t fn_rec_init();

void fn_rec_scope_in(function_record_t* record);

void fn_rec_scope_out(function_record_t* record);

void fn_rec_push_definition(function_record_t *record, exp_array_t *array, type_identifier_t *returntype, int priority, char *assembly, expression_t *dmsn);

struct function_def fn_rec_get_by_index(function_record_t *record, int index);

void fn_rec_destroy(function_record_t* record);

void print_fn_def(struct function_def def);

void print_fn_rec(function_record_t* record);


