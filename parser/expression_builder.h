#pragma once
#include "hash_table/function_record.h"
#include "expression_utils/expression_utils.h"





exp_array_t* parse_expression(exp_array_t** array, function_record_t* fn_record, int depth);