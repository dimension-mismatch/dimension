#pragma once
#include "hash_table.h"
#include "../type_utils/type_utils.h"

typedef struct{
  type_identifier_t type;
  char* name;
  int local_byte_offset;
  int var_id;
}variable_t;

struct scope{
  int variable_count;
  int local_byte_offset;
  int parameter_byte_offset;
};

typedef struct{
  hash_table_t table;
  int scope_depth;
  struct scope* scopes;
  variable_t* variables;
  int var_count;
}variable_record_t;

variable_record_t variable_record_init();

variable_t variable_record_push_new(variable_record_t* record, char* name, type_identifier_t* type);

variable_t variable_record_push_param(variable_record_t *record, char *name, type_identifier_t *type);

variable_t variable_record_push_enum(variable_record_t *record, char *name, type_identifier_t *type);

variable_t* variable_record_get(variable_record_t* record, char* name);

variable_t* variable_record_get_by_index(variable_record_t* record, int index);

variable_t *variable_record_get_newest(variable_record_t *record);

int* variable_record_get_index(variable_record_t* record, char* name);

int *variable_record_get_byte_offset(variable_record_t *record, char *name);

void variable_record_scope_in(variable_record_t* record);

void variable_record_scope_out(variable_record_t* record);

void variable_record_destroy(variable_record_t* record);

void print_variable(variable_t *var);

void print_variable_record(variable_record_t *record);
