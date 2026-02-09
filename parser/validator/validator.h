#pragma once

#include "../expression_utils/expression_utils.h"
#include "../error_handling/error_manager.h"

void validate_program(expression_t *program, error_manager_t *manager);