#pragma once
#include <stdint.h>

typedef struct{
  uint16_t* registers;
  uint16_t count;
}register_file_t;

void register_file_push(register_file_t *rf, uint16_t size);

void destroy_register_file(register_file_t* rf);