#pragma once
#include "ir_constructs.h"

void print_program(program_t* program);

void print_program_registers(program_t* program);

void destroy_program(program_t* program);

ir_value_t single_byte_ir_value(uint8_t byte);

ir_value_t register_ir_value(uint16_t regid);

ir_value_t literal_ir_value(uint16_t size, void* data);