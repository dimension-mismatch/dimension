#pragma once
#include "ir_constructs.h"

void print_program(program_t* program);


void destroy_program(program_t* program);

ir_value_t literal_ir_value(uint64_t value);

ir_value_t register_ir_value(uint64_t register_id);
