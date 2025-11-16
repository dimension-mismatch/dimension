#pragma once
#include "tokenizer.h"
#include "expression_utils/expression_utils.h"
#include "type_utils/type_utils.h"

#include "hash_table/type_record.h"
#include "hash_table/function_record.h"
#include "hash_table/variable_record.h"




expression_t *parse_tokens(token_array_t *tokens, type_record_t *type_record, variable_record_t *variable_record, function_record_t *function_record);
