#pragma once
#include "tokenizer.h"
#include "dimension-IR/ir_constructs.h"
#include <stdbool.h>

struct type_identifier;
struct variable_declaration;
struct pattern;
struct pattern_type;
struct type_argument;
struct pattern_variable;

typedef enum expression_type{
  EXP_TYPE_LITERAL,
  EXP_FUNCTION_CALL,
  EXP_VALUE_LITERAL,
  EXP_VECTOR,
  EXP_READ_VAR,
  EXP_RAW_TOKEN,
}expression_type_t;

typedef enum value_type{
  VAL_INT,
  VAL_UNSIGNED,
  VAL_FLOAT,
  VAL_CHAR,
  VAL_STRING,
}value_type_t;

typedef enum constant_level{
  CL_MUTABLE,
  CL_CONST,
  CL_SUPERCONST,
}const_lvl_t;

typedef struct datum{
  uint16_t size;
  uint8_t* data;
}datum_t;

typedef struct expression{
  expression_type_t type;
  struct type_identifier* return_type;
  const_lvl_t const_lvl;
  union{
    struct {
      int num_params;
      struct expression* params;
      int fn_id;
    } function_call;
    struct type_identifier* type_literal;
    struct{
      value_type_t type;
      union{
        int i;
        unsigned int u;
        float f;
        char c;
        char* s;
        datum_t datum;
      };
    } value_literal;
    struct{
      int num_params;
      struct expression* params;
    } vector;
    int read_var_id;
    token_t raw_token;
  };

}expression_t;

typedef struct block{
  int line_count;
  expression_t* lines;
}block_t;

typedef struct dimension_array{
  unsigned int dimension_count;
  expression_t* dimensions;
}dimension_array_t;

typedef enum type_argument_type{
  TYPEARG_SUBTYPE,
  TYPEARG_PARAM_EXP,
}type_argument_type_t;

typedef struct type_argument{
  type_argument_type_t type;
  union{
    struct type_identifier* subtype;
    expression_t* exp;
  };
}type_argument_t;

typedef struct type_identifier{
  int type_id;
  dimension_array_t dimensions;
  union{
    struct{
      int num_params;
      type_argument_t* params;
    };
    uint64_t size;
  };
  
}type_identifier_t;

typedef struct variable_declaration{
  char* var_name;
  const_lvl_t constant_lvl;
  type_identifier_t 
  type;
}variable_declaration_t;

typedef struct type_entry{
  bool is_vector;
  union{
    variable_declaration_t base;
    struct{
      char* name;
      const_lvl_t const_lvl;
      uint16_t component_count;
      struct type_entry* components;
      bool is_enum;
    } subvector;
  };
}type_entry_t;

typedef struct type_declaration{
  struct pattern* match_pattern;
  bool is_builtin;
  bool is_is;
  type_entry_t entry;

  bool is_static_size;
  union{
    uint64_t size;
    program_t compute_size;
  };
} type_declaration_t;




typedef struct pattern_value{
  bool is_param;
  union{
    expression_t* base_value;
    struct pattern_variable* param;
  };
}pattern_value_t;

typedef struct pattern_dimension_array{
  unsigned int dimension_count;
  pattern_value_t* dimensions;
}pattern_dimension_array_t;

typedef struct pattern_type{
  bool is_param;
  union{
    struct{
      int base_type_id;
      struct pattern* subpattern;
    };
    type_identifier_t param_type;
  };
  pattern_dimension_array_t dimensions;
  int param_count;
}pattern_type_t;


typedef struct pattern_variable{
  const_lvl_t constant_lvl;
  pattern_type_t type;
  char* name;
  int var_id;
}pattern_variable_t;

typedef enum pattern_entry_type{
  PATTERN_VARIABLE,
  PATTERN_IDENTIFIER,
  PATTERN_TYPE,
  PATTERN_EXP
} pattern_entry_type_t;

typedef struct pattern_entry{
  pattern_entry_type_t type;
  union{
    pattern_variable_t variable;
    char* identifier;
    pattern_type_t pattern_type;
    expression_t exp;
  };
}pattern_entry_t;


typedef struct pattern{
  int entry_count;
  int param_count;
  pattern_entry_t* entries;
}pattern_t;

typedef struct function_definition{
  pattern_t match;
  type_identifier_t* return_type;
  int priority;
  bool is_IR;
  union{
    block_t body;
    program_t ir;
  };
}function_definition_t;