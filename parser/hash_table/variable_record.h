#pragma once
#include "hash_table.h"
#include "../type_utils/type_utils.h"

typedef struct{
  type_identifier_t type;
  char* name;
}variable_t;

typedef struct{
  hash_table_t table;
  int scope_depth;
  int* scopes;
  variable_t* variables;
}variable_record_t;

variable_record_t variable_record_init();

variable_t variable_record_push_new(variable_record_t* record, char* name, type_identifier_t* type);

variable_t* variable_record_get(variable_record_t* record, char* name);

variable_t* variable_record_get_by_index(variable_record_t* record, int index);

int* variable_record_get_index(variable_record_t* record, char* name);

void variable_record_scope_in(variable_record_t* record);

void variable_record_scope_out(variable_record_t* record);

void variable_record_destroy(variable_record_t* record);

void print_variable(variable_t *var);

void print_variable_record(variable_record_t *record);
