#include "ir_construct_utils.h"
#include "ir-constructs.h"
#include "../colors.h"


#include <stdio.h>
#include <stdint.h>

void print_value(value_t* value){
  switch(value->type){
    case REGISTER:
      printf(CYAN BOLD "R%i" RESET_COLOR, value->register_id);
      break;
    case LITERAL:
      printf(MAGENTA BOLD);
      uint8_t* data_p = value->data;
      for(int i = 0; i < value->byte_count; i++){
        printf("0x%x", *data_p);
        data_p++;
      }
      printf(RESET_COLOR);
  }
}
void print_block(block_t* block, int indent);

void print_instruction(instruction_t* instruction, int indent){
  if(instruction->opcode == 255){
    print_block(instruction->block, indent);
    return;
  }
  for(int i = 0; i < indent; i++){
    printf(" ");
  }
  printf(CYAN BOLD "R%i " RESET_COLOR " = ", instruction->instruction.dest_register);
  printf(GREEN BOLD "%s " RESET_COLOR, instruction_names[instruction->opcode]);
  print_value(&instruction->instruction.a1);
  printf(", ");
  print_value(&instruction->instruction.a2);
  
}
void print_block(block_t* block, int indent){
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
  print_block(&program->root, 0);
}