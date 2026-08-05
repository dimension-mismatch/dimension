#include "program_builder.h"
#include "registers.h"
#include "ir_constructs.h"
#include "ir_construct_utils.h"
#include "ir_evaluation.h"
#include <stdlib.h>
#include <stdio.h>

program_t program_init(){
  program_t program = {.registers = register_file_init(), .root = {.length = 0, .instructions = NULL, .multiplier = literal_ir_value(1)}};
  return program;
}

void program_append_instruction(program_t *program, uint8_t opcode, ir_value_t arg1, ir_value_t arg2, uint16_t dest_reg){
  ir_block_t* block = &program->root;
  block->length++;
  block->instructions = realloc(block->instructions, block->length * sizeof(instruction_t));
  instruction_t new = {.opcode = opcode, .a1 = arg1, .a2 = arg2, .dest_register = dest_reg};
  block->instructions[block->length - 1] = new;
}

ir_value_t program_append_instruction_new_reg(program_t *program, uint8_t opcode, ir_value_t arg1, ir_value_t arg2){
  if(arg1.type == VAL_LITERAL && arg2.type == VAL_LITERAL){
    ir_value_t result = {.type = VAL_LITERAL};
    result.data = evaluate_instruction(opcode, arg1.data, arg2.data);
    return result;
  }
  uint16_t dest_reg = program->registers.count;
  program_append_instruction(program, opcode, arg1, arg2, dest_reg);
  ir_value_t result = {.type = VAL_REGISTER, .data = dest_reg};
  return result;
}

void update_operand(ir_value_t* operand, ir_value_t* arg_registers, int reg_offset, int arg_count){
  if(operand->type == VAL_REGISTER){
    if(operand->data < arg_count){
      *operand = arg_registers[operand->data];
    }
    else{
      operand->data += reg_offset;
    }
  }
}
instruction_t translate_instruction(instruction_t instr, ir_value_t* arg_registers, int reg_offset, int arg_count){
  instruction_t new = instr;
  update_operand(&new.a1, arg_registers, reg_offset, arg_count);
  update_operand(&new.a2, arg_registers, reg_offset, arg_count);
  new.dest_register += reg_offset;
  return new;
}

void program_call_fn_inline(program_t *program, program_t *function, ir_value_t *arg_registers, uint16_t* return_register){
  ir_block_t* block = &program->root;
  ir_block_t* fn_block = &function->root;
  
  block->instructions = realloc(block->instructions, (block->length + fn_block->length) * sizeof(instruction_t));
  int register_offset = program->registers.count - function->registers.argument_count;
  *return_register = program->registers.count;
  for(int i = block->length, j = 0; j < fn_block->length; i++, j++){
    block->instructions[i] = translate_instruction(fn_block->instructions[j], arg_registers, register_offset, function->registers.argument_count);
  }
  block->length += fn_block->length;
  uint64_t new_count = register_offset + function->registers.count;
  program->registers.count = new_count;
}