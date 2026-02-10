#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum value_type{
  RESULT,
  VARIABLE,
  PARAMETER,
  LITERAL,
  ARGUMENT,
}value_type_t;

typedef struct value{
  value_type_t type;
  union{
    uint64_t data;
    char* text;
  };
  bool is_fixed;
}value_t;

struct program;

typedef struct instruction{
  unsigned int opcode;
  union{
    struct{
      bool has_label;
      unsigned int label;
      unsigned int datasize;
      value_t arg1;
      value_t arg2;
    };
    struct program* block;
  };

}instruction_t;

typedef struct program{
  unsigned int length;
  instruction_t* instructions;
  
  value_t multiplier;
}program_t;