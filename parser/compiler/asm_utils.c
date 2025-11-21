#include "asm_utils.h"
#include "../colors.h"

#include "../hash_table/type_record.h"
#include "../hash_table/function_record.h"
#include "../hash_table/variable_record.h"

#include <stdlib.h>
#include <stdio.h>

reg_allocator_t rega_init(int stack_depth){
  reg_allocator_t new = {
    {RAX, RBX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    0,
    stack_depth,
    0
  };
  return new;
}

void put_text(FILE* file, char* txt){
  while(*txt != '\0'){
    putc(*txt, file);
    txt++;
  }
}

void put_number(FILE* file, unsigned int number){
  fprintf(file, "%d", number);
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
      if(address.byte_offset > 0){
        put_text(file, " ptr [rbp - ");
        put_number(file, address.byte_offset);
      }
      else{
        put_text(file, " ptr [rbp + ");
        put_number(file, 8 - address.byte_offset);
      }
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
              put_text(file, "sp");
              break;
            case RBP:
              put_text(file, "bp");
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

void put_new_stack_frame(FILE* file){
  put_text(file, "  push rbp\n  mov rbp, rsp\n");
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

address_t Areg_at_index(int index, address_size_t size, reg_allocator_t* rega){
  return Areg(rega->priority[index], size);
}

address_t Aget_next_free(address_size_t size, reg_allocator_t* rega){
  return Areg_at_index(rega->next_free_register, size, rega);
}

address_t Aconsume_next_free(address_size_t size, reg_allocator_t* rega){
  int reg_i = rega->next_free_register;
  rega->active_reg_count++;
  int i = reg_i + 1;
  while(rega->usage[reg_i]){
    reg_i++;
    if(i > 14){
      printf(RED BOLD "OUT OF REGISTERS, PLEASE IMPLEMENT STACK STORING" RESET_COLOR);
      exit(14);
    }
  }
  address_t new = Areg_at_index(reg_i, size, rega);
  rega->usage[reg_i] = 1;
  return new;
}

int index_of(x86_register_t reg){
  if(reg < 6){
    return (int)reg;
  }
  return (int)reg - 2;
}

void put_instruction(FILE* file, char* instruction, address_t dest, address_t src){
  put_text(file, "  ");
  put_text(file, instruction);
  put_text(file, " ");
  put_address(file, dest);
  put_text(file, ", ");
  put_address(file, src);
  put_text(file, "\n");
}

void mov_instruction(FILE* file, reg_allocator_t* rega, address_t dest, address_t src){
  if(dest.type == ADDR_STACK && src.type == ADDR_STACK){
    address_t addr = Aget_next_free(src.size, rega);
    put_instruction(file, "mov", addr, src);
    put_instruction(file, "mov", dest, addr);
  }
  else{
    put_instruction(file, "mov", dest, src);
  }
}

void rega_free(reg_allocator_t* rega, x86_register_t reg){
  int reg_i = index_of(reg);
  rega->active_reg_count--;
  rega->usage[reg_i] = 0;
  if(reg_i < rega->next_free_register){
    rega->next_free_register = reg_i;
  }
}

//Save the state of all active registers to the stack so that they can be restored later
void rega_regsave(FILE* file, reg_allocator_t* rega, int stack_ptr){
  for(int i = 0; i < NUM_REGISTERS; i++){
    if(rega->usage[i]){
      stack_ptr += 8;

      mov_instruction(file, rega, Astack(stack_ptr, QWORD), Areg(rega->priority[i], QWORD));
    }
  }
}

//restore the saved state of all active registers
void rega_regrestore(FILE* file, reg_allocator_t* rega, int stack_ptr){
  for(int i = 0; i < NUM_REGISTERS; i++){
    if(rega->usage[i]){
      stack_ptr += 8;
      mov_instruction(file, rega, Areg(rega->priority[i], QWORD), Astack(stack_ptr, QWORD));
    }
  }
}