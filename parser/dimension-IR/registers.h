#pragma once
#include "../hash_table/hash_table.h"
#include <stdint.h>

typedef struct{
  uint16_t* registers;
  uint16_t count;
  hash_table_t name_table;
  int argument_count;
}register_file_t;

register_file_t register_file_init();

void register_file_push(register_file_t *rf, uint16_t size);

void register_file_push_named(register_file_t *rf, uint16_t size, char* name);

void destroy_register_file(register_file_t* rf);