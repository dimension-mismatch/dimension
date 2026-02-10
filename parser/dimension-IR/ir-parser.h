#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "ir-constructs.h"



void print_value(value_t val);

void print_instruction(instruction_t* ins);

program_t parse_ir_file(FILE* file);