#include "compute_size.h"
#include "compiler.h"
#include <stdlib.h>

#define UMULTIPLY_INSTR 12
#define UADD_INSTR 10
#define UMAX_INSTR 19

ir_value_t compute_type_identifier_base_size(type_identifier_t* typeid, program_t* program, pattern_trie_t* fn_trie, pattern_trie_t* type_trie, hash_table_t* var_table){
  if(typeid->type_id == -1){
    return literal_ir_value(sizeof(uint64_t), &typeid->size);
  }
  printf("accessing type #%i\n", typeid->type_id);
  type_declaration_t typedec = type_trie->matches[typeid->type_id].typedec;
  if(typedec.is_static_size){
    return literal_ir_value(sizeof(uint64_t), &typedec.size);
  }
  ir_value_t* args = malloc(typeid->num_params * sizeof(ir_value_t));
  for(int i = 0; i < typeid->num_params; i++){
    if(typeid->params[i].type == TYPEARG_SUBTYPE){
      args[i] = single_byte_ir_value(0);//compute_type_identifier_base_size(typeid->params[i].subtype, program, fn_trie, type_trie, var_table);
    }
    else{
      args[i] = compile_expression(typeid->params[i].exp, program, var_table, fn_trie);
    }
  }
  uint16_t return_reg;
  program_call_fn_inline(program, &typedec.compute_size, args, &return_reg);
  free(args);
  return register_ir_value(return_reg);
}

ir_value_t compute_dimension_array_multiplier(dimension_array_t* array, program_t* program, pattern_trie_t* fn_trie, hash_table_t* var_table){
  if(array->dimension_count == 0){
    return single_byte_ir_value(1);
  }
  if(array->dimension_count == 1){
    return compile_expression(array->dimensions, program, var_table, fn_trie);
  }
  ir_value_t result = compile_expression(array->dimensions, program, var_table, fn_trie);
  register_file_push(&program->registers, 4);
  for(int i = 1; i < array->dimension_count; i++){
    ir_value_t a = compile_expression(array->dimensions + i, program, var_table, fn_trie);
    result = program_append_instruction_new_reg(program, UMULTIPLY_INSTR, result, a);
  }
  return result;
}

ir_value_t compute_type_identifier_size(type_identifier_t* typeid, program_t* program, pattern_trie_t* fn_trie, pattern_trie_t* type_trie, hash_table_t* var_table){
  ir_value_t base_size = compute_type_identifier_base_size(typeid, program, fn_trie, type_trie, var_table);
  if(typeid->dimensions.dimension_count == 0){
    return base_size;
  }
  ir_value_t dimension_multiplier = compute_dimension_array_multiplier(&typeid->dimensions, program, fn_trie, var_table);
  return program_append_instruction_new_reg(program, UMULTIPLY_INSTR, base_size, dimension_multiplier);
}

ir_value_t compute_type_entry_size(type_entry_t* entry, program_t* program, pattern_trie_t* type_trie, pattern_trie_t* fn_trie, hash_table_t* var_table){
  printf("computing size of: ");
  print_type_entry(entry);
  printf("\n");
  if(!entry->is_vector){
    if(entry->base.type.type_id == -1){
      return literal_ir_value(sizeof(uint64_t), &entry->base.type.size);
    }
    return compute_type_identifier_size(&entry->base.type, program, fn_trie, type_trie, var_table);
  }

  if(entry->subvector.component_count == 0){
    return single_byte_ir_value(0);
  }
  uint64_t enum_data_size = 1;
  ir_value_t enum_offset = literal_ir_value(sizeof(uint64_t), &enum_data_size);
  uint8_t opcode = entry->subvector.is_enum? UMAX_INSTR : UADD_INSTR;

  ir_value_t sum = compute_type_entry_size(entry->subvector.components, program, type_trie, fn_trie, var_table);
  for(int i = 1; i < entry->subvector.component_count; i++){
    ir_value_t e_n = compute_type_entry_size(entry->subvector.components + i, program, type_trie, fn_trie, var_table);
    sum = program_append_instruction_new_reg(program, opcode, sum, e_n);
  }
  return entry->subvector.is_enum? program_append_instruction_new_reg(program, UADD_INSTR, enum_offset, sum) : sum;
}


void compute_size(type_declaration_t *typedec, pattern_trie_t *type_trie, pattern_trie_t* fn_trie){
  
  typedec->is_static_size = false;
  typedec->compute_size = program_init();
  hash_table_t var_table = init_hash_table(67, 0.9);
  register_file_push(&typedec->compute_size.registers, 8);
  ir_value_t result = compute_type_entry_size(&typedec->entry, &typedec->compute_size, type_trie, fn_trie, &var_table);
  if(result.type == VAL_LITERAL){

  }
  else{
    typedec->compute_size.root.instructions[typedec->compute_size.root.length - 1].dest_register = 0;
  }
}