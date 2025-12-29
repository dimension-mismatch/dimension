#pragma once
#include <stdbool.h>

struct type_identifier;
struct variable_declaration;
struct pattern;

typedef enum expression_type{
  EXP_TYPE_LITERAL,
  EXP_FUNCTION_CALL,
  EXP_VALUE_LITERAL,
  EXP_VECTOR,
  EXP_READ_VAR,
}expression_type_t;

typedef enum value_type{
  VAL_INT,
  VAL_UNSIGNED,
  VAL_FLOAT,
  VAL_CHAR,
  VAL_STRING
}value_type_t;



typedef struct expression{
  expression_type_t type;
  struct type_identifier* return_type;
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
      };
    } value_literal;
    struct{
      int num_params;
      struct expression* params;
    } vector;
    int read_var_id;
  };

}expression_t;

typedef struct block{
  int line_count;
  expression_t* lines;
}block_t;


typedef struct type_identifier{
  int type_id;
  int num_params;
  expression_t* params;
  int dimension_count;
  expression_t* dimensions;
}type_identifier_t;


typedef struct type_declaration{
  struct pattern* match_pattern;
  bool is_is;
  bool is_enum;
  int component_count;
  struct variable_declaration* components;
 
}type_declaration_t;


typedef struct variable_declaration{
  char* var_name;
  int constant_lvl;
  type_identifier_t 
  type;
}variable_declaration_t;



typedef struct pattern_type{
  bool is_param;
  union{
    type_identifier_t* base_type;
    variable_declaration_t param_type;
  };

  int dimension_count;
  struct{
    bool is_param;
    union{
      expression_t* base_dimension;
      variable_declaration_t param_dimension;
    };
  }* dimensions;
}pattern_type_t;


typedef struct pattern_variable{
  char* name;
  int constant_lvl;
  pattern_type_t type;
}pattern_variable_t;

typedef struct pattern_entry{
  bool is_identifier;
  union{
    pattern_variable_t variable;
    char* identifier;
  };
}pattern_entry_t;

typedef struct pattern{
  int entry_count;
  pattern_entry_t* entries;
}pattern_t;

typedef struct function_definition{
  pattern_t match;
  expression_t* return_type;
  int priority;
  bool is_IR;
  union{
    block_t* body;
    char* ir;
  };
}function_definition_t;