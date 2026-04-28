#include "ir_construct_utils.h"
#include "ir_constructs.h"
#include "../colors.h"


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void print_value(ir_value_t* value){
  switch(value->type){
    case VAL_REGISTER:
      printf(CYAN BOLD "R%hu" RESET_COLOR, value->register_id);
      break;
    case VAL_LITERAL:
      printf(MAGENTA BOLD);
      uint8_t* data_p = value->literal.data;
      for(int i = 0; i < value->literal.byte_count; i++){
        printf("0x%x", *data_p);
        data_p++;
      }
      printf(RESET_COLOR);
  }
}
void print_ir_block(ir_block_t* block, int indent);

void print_instruction(instruction_t* instruction, int indent){
  if(instruction->opcode == 255){
    print_ir_block(instruction->block, indent);
    return;
  }
  for(int i = 0; i < indent; i++){
    printf(" ");
  }
  printf(CYAN BOLD "R%hu " RESET_COLOR " = ", instruction->dest_register);
  printf(GREEN BOLD "op %i " RESET_COLOR, instruction->opcode);
  print_value(&instruction->a1);
  printf(", ");
  print_value(&instruction->a2);
  
}
void print_ir_block(ir_block_t* block, int indent){
  for(int i = 0; i < indent; i++){
    printf(" ");
  }
  printf("{\n");
  for(int i = 0; i < block->length ; i++){
    print_instruction(block->instructions + i, indent + 1);
    printf("\n");
  }
  for(int i = 0; i < indent; i++){
    printf(" ");
  }
  printf("{\n");
}

void print_program(program_t* program){
  print_ir_block(&program->root, 0);
}

void destroy_value(ir_value_t* value){
  if(value->type == VAL_LITERAL){
    free(value->literal.data);
    value->literal.data = NULL;
    value->literal.byte_count = 0;
  }
}

void destroy_ir_block(ir_block_t* block);
void destroy_instruction(instruction_t* instr){
  if(instr->opcode == BLOCK_OPCODE){
      destroy_ir_block(instr->block);
    }
    else{
      destroy_value(&instr->a1);
      destroy_value(&instr->a2);
    }
}

void destroy_ir_block(ir_block_t* block){
  for(int i = 0; i < block->length; i++){
    destroy_instruction(block->instructions + i);
  }
  free(block->instructions);
  block->instructions = NULL;
  block->length = 0;
  destroy_value(&block->multiplier);
}

void destroy_program(program_t* program){
  destroy_ir_block(&program->root);
  free(program->array.registers);
  program->array.register_count = 0;
}