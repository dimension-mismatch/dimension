#pragma once
#include "hash_table/function_record.h"
#include "expression_utils/expression_utils.h"

#include "error_handling/error_manager.h"



exp_array_t* parse_expression(exp_array_t** array, parse_manager_t* manager, int depth);