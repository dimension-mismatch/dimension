#include "registers.h"
#include <stdlib.h>

register_file_t register_file_init(){
  register_file_t rf = {.count = 0, .name_table = init_hash_table(67, 0.9), .argument_count = 0};
  return rf;
}

void register_file_push(register_file_t *rf)
{
  rf->count++;
}

void register_file_push_named(register_file_t *rf, char *name){
  push_key_value(&rf->name_table, name, rf->count);
  register_file_push(rf);
}

void destroy_register_file(register_file_t* rf){
  destroy_hash_table(&rf->name_table);
  rf->count = 0;
}