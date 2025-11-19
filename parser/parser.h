#pragma once
#include "tokenizer.h"
#include "expression_utils/expression_utils.h"
#include "type_utils/type_utils.h"

#include "hash_table/type_record.h"
#include "hash_table/function_record.h"
#include "hash_table/variable_record.h"

#include "error_handling/error_manager.h"



expression_t *parse_tokens(parse_manager_t* manager);
