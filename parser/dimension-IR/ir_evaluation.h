#pragma once
#include "ir_constructs.h"

uint8_t* evaluate_instruction(uint8_t opcode, uint16_t outsize, uint16_t op_a_size, uint16_t op_b_size, uint8_t* op_a, uint8_t* op_b);