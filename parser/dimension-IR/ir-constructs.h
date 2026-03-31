#pragma once

#include <stdint.h>
#include <stdbool.h>

char* instruction_names[] = {"+", "-", "*", "/", ">", "<", ">=", "<=", "==", "deref", ">>", "<<", "&", "|", "^", "!", "&&", "||", "^^", "!!"};

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
    };
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
    } instruction;
    struct block* block;
  };
  
}instruction_t;


typedef struct block{
  uint64_t length;
  instruction_t* instructions;
  value_t multiplier;
}block_t;

typedef struct program{
  block_t root;
  
}program_t;