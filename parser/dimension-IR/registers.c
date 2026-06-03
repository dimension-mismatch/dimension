#include "registers.h"
#include <stdlib.h>

register_file_t register_file_init(){
  register_file_t rf = {.count = 0, .registers = NULL, .name_table = init_hash_table(67, 0.9)};
  return rf;
}

void register_file_push(register_file_t *rf, uint16_t size)
{
  rf->registers = realloc(rf->registers, (rf->count + 1) * sizeof(uint16_t));
  rf->registers[rf->count] = size;
  rf->count++;
}

void register_file_push_named(register_file_t *rf, uint16_t size, char *name){
  push_key_value(&rf->name_table, name, rf->count);
  register_file_push(rf, size);
}

void destroy_register_file(register_file_t* rf){
  free(rf->registers);
  rf->registers = NULL;
  rf->count = 0;
}