#include "dmsn-vm.h"
#include "ir-constructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../colors.h"

typedef struct stack{
  uint8_t* data;
  unsigned int byte_count;
}block_t;

typedef struct register_set{
  block_t* resgisters;
  unsigned int register_count;
}register_set_t;

typedef struct program_state{
  block_t variables;
  block_t inputs;
  register_set_t registers;
  block_t arguments;
}program_state_t;

void print_hex_data(uint8_t* data, size_t byte_count){
  for(size_t i = 0; i < byte_count; i++, data++){
    if(*data < 16){
      printf("0%x", *data);
    }
    else{
      printf("%x", *data);
    }
    if(i % 2 == 1){
      printf(" ");
      if(i % 8 == 7){
        printf("\n");
      }
    }
    
  }
}

void print_stack(block_t* block){
  printf(YELLOW "STACK: \n");
  print_hex_data(block->data, block->byte_count);
  printf("\n" RESET_COLOR);
}

void print_regset(register_set_t* regset){
  printf(CYAN "REGISTERS: \n");
  for(int i = 0; i < regset->register_count; i++){
    printf("~%i : ", i);
    print_hex_data(regset->resgisters[i].data, regset->resgisters[i].byte_count);
  }
}

void destroy_stack(block_t* block){
  free(block->data);
  block->data = NULL;
  block->byte_count = 0;
}

void destroy_regset(register_set_t* regset){
  for(int i = 0; i < regset->register_count; i++){
    destroy_stack(regset->resgisters + i);
  }
  free(regset->resgisters);
  regset->resgisters = NULL;
  regset->register_count = 0;
}

void block_write(block_t* block, unsigned int address, unsigned int bytes, uint8_t* data){
  if(address + bytes > block->byte_count){
    printf("not enough space in the block!!!");
    return;
  }
  memcpy(block->data + address, data, bytes);
}

void register_write(register_set_t* regset, unsigned int register_id, unsigned int bytes, uint8_t* data){
  if(regset->register_count <= register_id){
    unsigned int oc = regset->register_count;
    regset->register_count = register_id + 1;
    regset->resgisters = realloc(regset->resgisters, regset->register_count * sizeof(block_t));
    block_t empty = {.byte_count = 0, .data = NULL};
    for(int i = oc; i < regset->register_count; i++){
      regset->resgisters[i] = empty;
    }
  }
  else if(regset->resgisters[register_id].data){
    destroy_stack(regset->resgisters + register_id);
  }
  block_t new = {.byte_count = bytes, .data = malloc(bytes)};
  memcpy(new.data, data, bytes);
  regset->resgisters[register_id] = new;
}


program_state_t init_program_state(){
  program_state_t state = {
    .inputs = {.byte_count = 16, .data = calloc(1, 16)}, 
    .registers = {.register_count = 0, .resgisters = NULL},
    .variables = {.byte_count = 64, .data = calloc(1, 64)},
    .arguments = {.byte_count = 16, .data = calloc(1, 16)}};
  return state;
}
void print_program_state(program_state_t* state){
  print_regset(&state->registers);
  print_stack(&state->inputs);
  print_stack(&state->variables);
  print_stack(&state->arguments);
}

void destroy_program_state(program_state_t* state){
  destroy_regset(&state->registers);
  destroy_stack(&state->inputs);
  destroy_stack(&state->variables);
  destroy_stack(&state->arguments);
}

uint8_t* read_from_value(program_state_t* state, value_t* value, unsigned int offset){
  if(value->is_fixed){
    offset = 0;
  }
  switch(value->type){
    case LITERAL:
      return (uint8_t*)&value->data;
    case VARIABLE:
      return state->variables.data + value->data + offset;
    case PARAMETER:
      return state->inputs.data + value->data + offset;
    case RESULT:
      return (state->registers.resgisters + value->data)->data + offset;
    case ARGUMENT:
      return state->arguments.data + value->data + offset;
    
  }
}

void write_data_to_value(program_state_t* state, value_t dest, uint8_t* data, unsigned int byte_count, unsigned int offset){
  if(dest.is_fixed){
    offset = 0;
  }
  switch(dest.type){
    case LITERAL:
      printf(RED BOLD "Cannot write to a literal!!!\n" RESET_COLOR);
      break;
    case VARIABLE:
      block_write(&state->variables, dest.data + offset, byte_count, data);
      break;
    case PARAMETER:
      block_write(&state->inputs, dest.data + offset, byte_count, data);
      break;
    case RESULT:
      register_write(&state->registers, dest.data + offset, byte_count, data);
      break;
    case ARGUMENT:
      block_write(&state->arguments, dest.data + offset, byte_count, data);
      break;
  }
}


void execute_subprogram(program_t* program, program_state_t* state, unsigned int offset){
  unsigned int max_steps = 10;
  unsigned int iptr = 0;
  while(iptr < program->length){
    max_steps--;
    if(max_steps <= 0){
      break;
    }
    instruction_t* ins = program->instructions + iptr;
    unsigned int isize = ins->datasize;
    unsigned int byteOffset = isize * offset;
    iptr++;
    switch(ins->opcode){
      case 0:{
        unsigned int stepCount = *read_from_value(state, &ins->block->multiplier, byteOffset);
        printf("stepcount: %u\n", stepCount);

        for(unsigned int o = 0; o < stepCount; o++){
          execute_subprogram(ins->block, state, o);
        }
      
        break;
      }
      case 1: //move
        write_data_to_value(state, ins->arg2, read_from_value(state, &ins->arg1, byteOffset), ins->datasize, byteOffset);
        break;
      case 2: //deref
        break;
      case 3: {//integer addition
        uint8_t* result = malloc(ins->datasize);
        uint8_t* arg1 = read_from_value(state, &ins->arg1, byteOffset);
        uint8_t* arg2 = read_from_value(state, &ins->arg2, byteOffset);
        //printf("a1: %u, a2: %u", *arg1, *arg2);
        bool carry = false;
        for(int i = 0; i < ins->datasize; i++){
          uint16_t sum = *(arg1 + i) + *(arg2 + i) + (carry? 1 : 0);
          carry = (sum > 255);
          *(result + i) = sum & 255;
        }
        if(ins->has_label){
          register_write(&state->registers, ins->label, ins->datasize, result);
        }
        free(result);
        break;
      }
      case 14: //jump
        iptr = ins->arg1.data;
        break;
      default:
        printf(MAGENTA BOLD "not implemented yet!!!\n" RESET_COLOR);

    }
  }
}

void execute_program(program_t program){
  program_state_t state = init_program_state();
  execute_subprogram(&program, &state, 0);
  print_program_state(&state);
  destroy_program_state(&state);
}