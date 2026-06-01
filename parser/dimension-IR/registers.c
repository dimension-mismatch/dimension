#include "registers.h"
#include <stdlib.h>

void register_file_push(register_file_t* rf, uint16_t size){
  rf->registers = realloc(rf->registers, (rf->count + 1) * sizeof(uint16_t));
  rf->registers[rf->count] = size;
  rf->count++;
}

void destroy_register_file(register_file_t* rf){
  free(rf->registers);
  rf->registers = NULL;
  rf->count = 0;
}