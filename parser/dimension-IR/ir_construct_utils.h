#pragma once
#include "ir_constructs.h"

void print_program(program_t* program);

void print_program_registers(program_t* program);

void destroy_program(program_t* program);

uint16_t sizeof_ir_value(ir_value_t val, register_file_t *registers);

uint16_t infer_size_from_args(ir_value_t a1, ir_value_t a2, register_file_t *registers);

ir_value_t single_byte_ir_value(uint8_t byte);

ir_value_t register_ir_value(uint16_t regid);

ir_value_t literal_ir_value(uint16_t size, void* data);