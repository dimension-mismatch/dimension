#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../hash_table/hash_table.h"


#define BLOCK_OPCODE 255
typedef enum ir_value_type{
  VAL_REGISTER,
  VAL_LITERAL,
}ir_value_type_t;

typedef struct ir_value{
  ir_value_type_t type;
  union{
    uint16_t register_id;
    struct{
      uint16_t byte_count;
      uint8_t* data;
    }literal;
  };
}ir_value_t;

struct ir_block;
typedef struct instruction{
  uint8_t opcode;
  union{
    struct{
      uint16_t dest_register;
      ir_value_t a1;
      ir_value_t a2;
    };
    struct ir_block* block;
  };
  
}instruction_t;


typedef struct ir_block{
  uint64_t length;
  instruction_t* instructions;
  ir_value_t multiplier;
}ir_block_t;

typedef struct register_array{
  uint16_t register_count;
  uint16_t* registers;
}register_array_t;
typedef struct program{
  ir_block_t root;
  register_array_t array;
}program_t;