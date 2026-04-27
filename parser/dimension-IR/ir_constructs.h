#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../hash_table/hash_table.h"

int instruction_count = 20;
char* instruction_names[] = {"+", "-", "*", "/", ">", "<", ">=", "<=", "==", "deref", ">>", "<<", "&", "|", "^", "!", "&&", "||", "^^", "!!", "printchar"};


#define BLOCK_OPCODE 255
typedef enum value_type{
  REGISTER,
  LITERAL,
}value_type_t;

typedef struct value{
  value_type_t type;
  union{
    uint64_t register_id;
    struct{
      uint64_t byte_count;
      uint8_t* data;
    }literal;
  };
}value_t;

struct block;
typedef struct instruction{
  uint8_t opcode;
  union{
    struct{
      uint64_t dest_register;
      value_t a1;
      value_t a2;
    };
    struct block* block;
  };
  
}instruction_t;


typedef struct block{
  uint64_t length;
  instruction_t* instructions;
  value_t multiplier;
}ir_block_t;


typedef struct program{
  ir_block_t root;
  uint64_t register_count;
  uint64_t* registers;
}program_t;