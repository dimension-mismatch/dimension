#include "ir_construct_utils.h"
#include "ir_constructs.h"
#include "../colors.h"


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void print_value(ir_value_t* value){
  switch(value->type){
    case VAL_REGISTER:
      printf(CYAN BOLD "R%llu" RESET_COLOR, value->data);
      break;
    case VAL_LITERAL:
      printf(MAGENTA BOLD);
      printf("%llx", value->data);
      printf(RESET_COLOR);
      break;
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
  printf(CYAN BOLD "R%llu " RESET_COLOR " = ", instruction->dest_register);
  printf(GREEN BOLD "op_%i " RESET_COLOR, instruction->opcode);
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


void destroy_ir_block(ir_block_t* block);
void destroy_instruction(instruction_t* instr){
  if(instr->opcode == BLOCK_OPCODE){
    destroy_ir_block(instr->block);
  }
}

void destroy_ir_block(ir_block_t* block){
  for(int i = 0; i < block->length; i++){
    destroy_instruction(block->instructions + i);
  }
  free(block->instructions);
  block->instructions = NULL;
  block->length = 0;
}

void destroy_program(program_t* program){
  destroy_ir_block(&program->root);
  destroy_register_file(&program->registers);
}

ir_value_t literal_ir_value(uint64_t value){
  ir_value_t result = {.type = VAL_LITERAL, .data = value};
  return result;
}

ir_value_t register_ir_value(uint64_t register_id){
  ir_value_t result = {.type = VAL_REGISTER, .data = register_id};
  return result;
}