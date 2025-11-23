#pragma once
#include "../expression_utils/expression_utils.h"
#include <stdio.h>

#define NUM_REGISTERS 15
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
  int is_pointer;
  union{
    x86_register_t reg;
    int byte_offset;
  };
}address_t;

typedef struct{
  x86_register_t priority[NUM_REGISTERS];
  int usage[NUM_REGISTERS];
  int next_free_register;
  int stack_depth;
  int active_reg_count;
}reg_allocator_t;

reg_allocator_t rega_init(int stack_depth);

address_size_t expression_size(expression_t *exp);

void put_text(FILE *file, char *txt);

void put_number(FILE *file, unsigned int number);

void put_address(FILE *file, address_t address);

void put_new_stack_frame(FILE *file);

address_t Aliteral(int value);

address_t Astack(int byte_offset, address_size_t size);

address_t Areg(x86_register_t reg, address_size_t size);

address_t Areg_at_index(int index, address_size_t size, reg_allocator_t *rega);

address_t Aget_next_free(address_size_t size, reg_allocator_t *rega);

address_t Aderef(address_t pointer);

address_t Aconsume_next_free(address_size_t size, reg_allocator_t *rega);

int index_of(x86_register_t reg);

void put_instruction(FILE *file, char *instruction, address_t dest, address_t src);

void mov_instruction(FILE *file, reg_allocator_t *rega, address_t dest, address_t src);

void return_instruction(FILE *file);

void lea_instruction(FILE *file, reg_allocator_t *rega, address_t dest, x86_register_t base, int offset, address_size_t size);

void rega_free(reg_allocator_t *rega, x86_register_t reg);

void rega_regsave(FILE *file, reg_allocator_t *rega, int stack_ptr);

void rega_regrestore(FILE *file, reg_allocator_t *rega, int stack_ptr);
