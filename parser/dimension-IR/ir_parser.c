#include "ir_parser.h"
#include "ir_constructs.h"
#include "ir_construct_utils.h"
#include "../hash_table/hash_table.h"
#include "../token_cursor.h"
#include "registers.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


typedef enum identifier_meaning{
  ID_REGISTER,
  ID_INSTRUCTION,
  ID_NOT_DEFINED
}identifier_meaning_t;

typedef struct decoded_identifier{
  identifier_meaning_t type;
  uint16_t data;
}decoded_identifier_t;

decoded_identifier_t decode_identifier(char* identifier, hash_table_t* itable, register_file_t* rf){
  int* instruction = get_value_from_key(itable, identifier);
  if(instruction){
    decoded_identifier_t res = {.type = ID_INSTRUCTION, .data = *instruction};
    return res;
  }
  int* reg = get_value_from_key(&rf->name_table, identifier);
  if(reg){
    decoded_identifier_t res = {.type = ID_REGISTER, .data = *reg};
    return res;
  }
  decoded_identifier_t res = {.type = ID_NOT_DEFINED};
  return res;
}

bool parse_value(token_cursor_t* base_tc, ir_value_t* result, hash_table_t* itable, register_file_t* rf){
  token_cursor_t tc = *base_tc;
  if(tc.tk.type == TK_NUMERIC){
    result->type = VAL_LITERAL;
    //TODO: handle different immediate sizes
    result->literal.byte_count = 1;
    result->literal.data = malloc(1);
    int data = atoi(tc.tk.content);
    *result->literal.data = data;
  }
  else if(tc.tk.type == TK_IDENTIFIER){
    decoded_identifier_t id = decode_identifier(tc.tk.content, itable, rf);
    if(id.type == ID_INSTRUCTION){
      tc_throw_error(&tc, 19);
      return false;
    }
    if(id.type == ID_NOT_DEFINED){
      tc_throw_error(&tc, 20);
      return false;
    }
    result->type = VAL_REGISTER;
    result->register_id = id.data;
  }
  *base_tc = tc;
  return true;
}

parse_result_t parse_instruction(token_cursor_t* base_tc, instruction_t* result, hash_table_t* itable, register_file_t* rf){
  token_cursor_t tc = *base_tc;
  if(tc.tk.type != TK_IDENTIFIER){
    tc_throw_error(&tc, 18);
    return PRS_NOT_FOUND;
  }
  decoded_identifier_t id = decode_identifier(tc.tk.content, itable, rf);
  bool register_needs_size = false;
  if(id.type == ID_INSTRUCTION){
    result->dest_register = -1;
    tc_inc(&tc);
  }
  else{
    if(id.type == ID_REGISTER){
      result->dest_register = id.data;
      tc_inc(&tc);
    }
    else{
      char* name = tc.tk.content;
      register_needs_size = true;
      int size = 1;
      tc_inc(&tc);
      if(tc.tk.type == TK_DECL && tc.tk.decl_const_lvl == CL_MUTABLE){
        tc_inc(&tc);
        if(tc.tk.type != TK_NUMERIC){
          tc_throw_error(&tc, 8);
          return PRS_ERROR;
        }
        if(tc.tk.number_type == NUM_SCI_FLOAT || tc.tk.number_type == NUM_FLOAT){
          tc_throw_error(&tc, 9);
          return PRS_ERROR;
        }
        
        size = atoi(tc.tk.content);
        tc_inc(&tc);
      }
      
      result->dest_register = rf->count;
      register_file_push_named(rf, size, name);
     
    }
    if(tc.tk.type != TK_IDENTIFIER || tc.tk.length != 2 || tc.tk.content[0] != '='){
      tc_throw_error(&tc, 21);
      return PRS_ERROR;
    }
    tc_inc(&tc);
    if(tc.tk.type != TK_IDENTIFIER){
      tc_throw_error(&tc, 22);
      return PRS_ERROR;
    }
    decoded_identifier_t instr_id = decode_identifier(tc.tk.content, itable, rf);
    if(instr_id.type != ID_INSTRUCTION){
      tc_throw_error(&tc, 22);
      return PRS_ERROR;
    }
    result->opcode = instr_id.data;
    tc_inc(&tc);
  }
  if(!parse_value(&tc, &result->a1, itable, rf)){
    return PRS_ERROR;
  }
  //if register size was not specified, then we infer size from instruction arguments
  if(register_needs_size){
    rf->registers[result->dest_register] = infer_size_from_args(result->a1, result->a2, rf);
  }
  tc_inc(&tc);
  if(tc.tk.type == TK_FORCE_EXP_END){
    tc_inc(&tc);
  }
  if(!parse_value(&tc, &result->a2, itable, rf)){
    return PRS_ERROR;
  }
  tc_inc(&tc);
  *base_tc = tc;
  return PRS_SUCCESS;
}

void parse_ir_block(token_cursor_t* base_tc, ir_block_t* result, hash_table_t* itable, register_file_t* reg_array){
  token_cursor_t tc = *base_tc;
  result->length = 0;
  result->instructions = NULL;
  result->multiplier.type = VAL_LITERAL;
  result->multiplier.literal.byte_count = 1;
  result->multiplier.literal.data = malloc(1);
  *result->multiplier.literal.data = 1;
  while(tc.tk.type != TK_IR && !(tc.tk.type == TK_BLOCK && !tc.tk.is_open) && tc.tk.type != TK_NONE){
    instruction_t instruction;
    parse_result_t instr_res = parse_instruction(&tc, &instruction, itable, reg_array);
    if(instr_res == PRS_NOT_FOUND){
      tc_inc(&tc);
    }
    else if(instr_res == PRS_ERROR){
      int current_line = tc.tk.line_number;
      while(tc.tk.line_number == current_line && tc.tk.type != TK_IR && !(tc.tk.type == TK_BLOCK && !tc.tk.is_open) && tc.tk.type != TK_NONE){
        tc_inc(&tc);
      }
    }
    else{
      result->length++;
      result->instructions = realloc(result->instructions, result->length * sizeof(instruction_t));
      result->instructions[result->length - 1] = instruction;
    }
  }
  *base_tc = tc;
}

parse_result_t parse_ir(token_cursor_t* base_tc, program_t* result, register_file_t rf){
  token_cursor_t tc = *base_tc;
  if(tc.tk.type != TK_IR){
    return PRS_NOT_FOUND;
  }
  tc_inc(&tc);
  int instruction_count = 42;
  char* instruction_names[] = {"+", "-", "*", "/", ">", "<", ">=", "<=", "min", "max", "u+", "u-", "u*", "u/", "u>", "u<", "u>=", "u<=", "umin", "umax", "f+", "f-", "f*", "f/", "f>", "f<", "f>=", "f<=", "fmin", "fmax", "==", "deref", ">>", "<<", "&", "|", "^", "!", "&&", "||", "^^", "!!", "printchar"};
  hash_table_t instruction_table = init_hash_table_from_array(67, 0.9, instruction_names, instruction_count);
  hash_table_t label_table = init_hash_table(67, 0.9);
  result->registers = rf;
  printf("\n Reading IR with %i provided arguments\n", rf.count);
  
  parse_ir_block(&tc, &result->root, &instruction_table, &result->registers);

  destroy_hash_table(&instruction_table);
  destroy_hash_table(&label_table);
  *base_tc = tc;
  printf("after attempting to read ir, we are now at ln %i, col %i\n", tc.tk.line_number, tc.tk.start_pos);
  return PRS_SUCCESS;
}

