#include "compiler.h"
#include "../hash_table/hash_table.h"

#include <stdlib.h>


ir_value_t compile_expression(expression_t* expr, program_t* program, hash_table_t* var_table, pattern_trie_t* fn_trie){
  switch(expr->type){
    case EXP_RAW_TOKEN: //shouldn't appear at this point in the process
    case EXP_TYPE_LITERAL: //shouldn't appear at this point in the process
    case EXP_VECTOR: //TODO: handle vector expressions
      return single_byte_ir_value(0);
    case EXP_VALUE_LITERAL:
      switch(expr->value_literal.type){
        case VAL_DATUM:
        case VAL_STRING:
          return single_byte_ir_value(0);
        case VAL_CHAR:
          return literal_ir_value(sizeof(char), &expr->value_literal.c);
        case VAL_FLOAT:
          return literal_ir_value(sizeof(float), &expr->value_literal.f);
        case VAL_INT:
          return literal_ir_value(sizeof(int), &expr->value_literal.i);
        case VAL_UNSIGNED:
          return literal_ir_value(sizeof(unsigned int), &expr->value_literal.u);
      }
    case EXP_READ_VAR: {
      int* val = get_value_from_int(var_table, expr->read_var_id);
      if(!val) return single_byte_ir_value(0);
      return register_ir_value(*val);
    }
    case EXP_FUNCTION_CALL: {
      trie_match_result_t match = fn_trie->matches[expr->function_call.fn_id];

      if(match.type != MATCH_FUNCTION) return single_byte_ir_value(0);

      function_definition_t fn = match.fndec;
      if(!fn.is_IR) return single_byte_ir_value(0); //TODO: compile functions into IR

      ir_value_t* args = malloc(expr->function_call.num_params * sizeof(ir_value_t));
      for(int i = 0; i < expr->function_call.num_params; i++){
        args[i] = compile_expression(expr->function_call.params + i, program, var_table, fn_trie);
      }
      uint16_t return_register_id;
      //TODO: don't always call functions inline
      program_call_fn_inline(program, &fn.ir, args, &return_register_id);
      printf("return register: %i\n", return_register_id );
      return register_ir_value(return_register_id);
    }
  }
}
program_t compile_program(block_t ast, pattern_trie_t* fn_trie, pattern_trie_t* type_trie){
  program_t out = program_init();
  hash_table_t var_table = init_hash_table(67, 0.9);

  for(int i = fn_trie->scopes[fn_trie->scope_levels - 1]; i < fn_trie->match_count; i++){
    trie_match_result_t match = fn_trie->matches[i];
    if(match.type == MATCH_VARIABLE){
      //if(fn_trie->matches[i].vardec.constant_lvl != 3){ //superconsts are not compiled as regular variables
      push_int_value(&var_table, match.index, out.registers.count);  
      register_file_push(&out.registers, 4); //TODO: determine size of variables
        
      //}
    }

  }

  for(int i = 0; i < ast.line_count; i++){
    compile_expression(ast.lines + i, &out, &var_table, fn_trie);
  }
  
  return out;
}