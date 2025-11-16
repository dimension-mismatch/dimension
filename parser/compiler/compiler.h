#pragma once

#include "../expression_utils/expression_utils.h"
#include "../hash_table/type_record.h"
#include "../hash_table/function_record.h"
#include "../hash_table/variable_record.h"

void compile_program(char *filename, expression_t *expression, function_record_t *fn_rec, variable_record_t *var_rec, type_record_t *type_rec);
