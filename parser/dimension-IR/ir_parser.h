#pragma once

#include "ir_constructs.h"
#include "../token_cursor.h"
#include <stdint.h>

parse_result_t parse_ir(token_cursor_t* base_tc, program_t* result, register_file_t rf);