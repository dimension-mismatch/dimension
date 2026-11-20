#pragma once

#include "../dimension-IR/ir_construct_utils.h"
#include "../construct_utils.h"

#include "../dimension-IR/program_builder.h"

#include "../hash_table/pattern_trie.h"



program_t compile_program(block_t ast, pattern_trie_t* fn_trie, pattern_trie_t* type_trie);
ir_value_t compile_expression(expression_t* expr, program_t* program, hash_table_t* var_table, pattern_trie_t* fn_trie);