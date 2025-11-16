#include "compiler.h"
#include "../hash_table/function_record.h"

#include "../hash_table/type_record.h"
#include "../hash_table/function_record.h"
#include "../hash_table/variable_record.h"

#include <stdlib.h>
#include <stdio.h>

typedef enum{
  RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP, R8, R9, R10, R11, R12, R13, R14, R15
}x86_register_t;

typedef enum{
  ADDR_REGISTER,
  ADDR_STACK,
  ADDR_LITERAL,
}address_type_t;

typedef enum{
  BYTE,
  WORD,
  DWORD,
  QWORD,
}address_size_t;

typedef struct{
  address_type_t type;
  address_size_t size;
  union{
    x86_register_t reg;
    int byte_offset;
  };
}address_t;

x86_register_t register_priority[] = {RAX, RBX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15};
int next_free_register = 0;

char* header = "#DIMENSION v0.0.1 compiled\n.global start\n.intel_syntax noprefix\n\nstart:\n";
char* new_stack_frame = "  push rbp\n  mov rbp, rsp";
void put_text(FILE* file, char* txt){
  while(*txt != '\0'){
    putc(*txt, file);
    txt++;
  }
}

void put_number(FILE* file, unsigned int number){
  if(number == 0){
    put_text(file, "0");
  }
  int exp = 1;
  while(number / exp > 10){
    exp *= 10;
  }
  while(number > 0){
    int digit = number / exp;
    putc('0' + digit, file);
    number -= digit * exp;
    exp /= 10;
  }
}

void put_address(FILE* file, address_t address){
  switch(address.type){
    case ADDR_STACK:
      switch(address.size){
        case QWORD:
          put_text(file, "qword");
          break;
        case DWORD:
          put_text(file, "dword");
          break;
        case WORD:
          put_text(file, "word");
          break;
        case BYTE:
          put_text(file, "byte");
          break;
      }
      put_text(file, " ptr [rsp - ");
      put_number(file, address.byte_offset);
      put_text(file, "]");
      break;
    case ADDR_REGISTER:
      if(address.reg < R8){
        switch(address.size){
          case QWORD: 
            put_text(file, "r");
            break;
          case DWORD:
            put_text(file, "e");
            break;
          default:
            break;
        }
        if(address.reg <= RDX){
          char name = 'a' + address.reg;
          put_text(file, &name);
          if(address.size > BYTE){
            put_text(file, "x");
          }
        }
        else{
          switch(address.reg){
            case RSI:
              put_text(file, "si");
              break;
            case RDI:
              put_text(file, "di");
              break;
            case RSP:
              put_text(file, "rs");
              break;
            case RBP:
              put_text(file, "rb");
              break;
            default:
              break;
          }
        }
        if(address.size == BYTE){
          put_text(file, "l");
        }
      }
      else{
        put_text(file, "r");
        put_number(file, address.reg);
        switch(address.size){
          case BYTE:
            put_text(file, "b");
            break;
          case WORD:
            put_text(file, "w");
            break;
          case DWORD:
            put_text(file, "d");
            break;
          case QWORD:
            break;
        }
      }
      break;
    case ADDR_LITERAL:
      put_number(file, address.byte_offset);
      break;
  }
}

address_t Aliteral(int value){
  address_t new = {.type = ADDR_LITERAL, .byte_offset = value};
  return new;
}

address_t Astack(int byte_offset, address_size_t size){
  address_t new = {.type = ADDR_STACK, .size = size, .byte_offset = byte_offset};
  return new;
}

address_t Areg(x86_register_t reg, address_size_t size){
  address_t new = {.type = ADDR_REGISTER, .size = size, .reg = reg};
  return new;
}

address_t Areg_at_index(int index, address_size_t size){
  return Areg(register_priority[index], size);
}

address_t Aconsume_next_free(address_size_t size){
  address_t new = Areg_at_index(next_free_register, size);
  next_free_register++;
  return new;
}

void put_instruction(FILE* file, char* instruction, address_t dest, address_t src){
  put_text(file, "  ");
  put_text(file, instruction);
  put_text(file, " ");
  put_address(file, dest);
  put_text(file, ", ");
  put_address(file, src);
}

//puts instructions to compute the value of an expression into the assembly file, and returns an address_t containing the location of the output expression.
address_t compile_expression(FILE* file, expression_t* expression, function_record_t* fn_rec, variable_record_t* var_rec, type_record_t* type_rec){
  if(expression->type == EXP_SINGLE_LITERAL){
    printf("literal: %d\n", expression->numeric_literal);
    return Aliteral(expression->numeric_literal);
  }
  else if(expression->type == EXP_READ_VAR){
    return Astack(expression->read.var_id, QWORD);
  }
  else if(expression->type == EXP_WRITE_VAR){
    address_t addr = Astack(expression->write.var_id, QWORD);
    expression_t* value = expression->write.value[0];
    put_instruction(file, "mov", addr, compile_expression(file, expression->write.value[0], fn_rec, var_rec, type_rec));
    return addr;
  }
  else if(expression->type == EXP_BLOCK){
    for(int i = 0; i < expression->function_call.arg_c; i++){
      compile_expression(file, expression->function_call.arg_v[i], fn_rec, var_rec, type_rec);
      putc('\n', file);
    }
  }
  else if(expression->type == EXP_CALL_FN){
    struct function_def definition = fn_rec_get_by_index(fn_rec, expression->function_call.fn_id);
    address_t arg_locations[expression->function_call.arg_c];

    for(int i = 0; i < expression->function_call.arg_c; i++){
      arg_locations[i] = compile_expression(file, expression->function_call.arg_v[i], fn_rec, var_rec, type_rec);
    }

    if(definition.impl_type == FN_IMPL_ASM){
      if(arg_locations[0].type != ADDR_REGISTER){
        address_t reg = Aconsume_next_free(QWORD);
        put_instruction(file, "mov", reg, arg_locations[0]);
        arg_locations[0] = reg;
      }

      char* text = definition.assembly;
      if(*text == 'I'){
        text++;
      }
      else{
        return Aliteral(0); //TODO : add support for true functions in assembly
      }
      int arg_i = 0;
      while (*text != '\0'){
        if(*text == '?'){
          text++;
          // do{
          //   text++;
          // }while(*text >= '0' && *text <= '9' && *text != '\0');
          put_address(file, arg_locations[arg_i]);
          arg_i++;
        }
        else{
          putc(*text, file);
        }
        text++;
      }
      //next_free_register--;
      return arg_locations[0];
    }
  }
  return Aliteral(0);
}
void compile_program(char* filename, expression_t* expression, function_record_t* fn_rec, variable_record_t* var_rec, type_record_t* type_rec){
  FILE* file = fopen(filename, "w");
  put_text(file, header);
  put_text(file, new_stack_frame);
  put_text(file, "\n");
  //put_text(file, "sub rsp, 16\n");
  compile_expression(file, expression, fn_rec, var_rec, type_rec);


  fclose(file);
  file = NULL;
}