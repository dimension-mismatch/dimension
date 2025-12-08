#pragma once
#include "hash_table.h"
#include "../type_utils/type_utils.h"

typedef struct{
  hash_table_t table;
  type_declaration_t** array;
}type_record_t;

type_record_t init_type_record();

void type_record_push_type(type_record_t* record, type_declaration_t typedec);

type_declaration_t* type_record_get_type(type_record_t* record, char* typename);

int type_record_get_type_number(type_record_t* record, char* typename);

type_identifier_t type_record_get_type_id(type_record_t *record, char *typename);

void print_type_id_named(type_identifier_t *typeid, type_record_t *type_rec);

void destroy_type_record(type_record_t* record);
