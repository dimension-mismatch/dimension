#include "ir_constructs.h"

program_t program_init();

void program_append_instruction(program_t* program, uint8_t opcode, ir_value_t arg1, ir_value_t arg2, uint16_t dest_reg);

ir_value_t program_append_instruction_new_reg(program_t *program, uint8_t opcode, ir_value_t arg1, ir_value_t arg2);

void program_call_fn_inline(program_t* program, program_t* function, ir_value_t* arg_registers, uint16_t* return_register);

