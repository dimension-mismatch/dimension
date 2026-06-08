#pragma once

#include "../dimension-IR/ir_construct_utils.h"
#include "../construct_utils.h"

#include "../dimension-IR/program_builder.h"

#include "../hash_table/pattern_trie.h"

program_t compile_program(block_t ast, pattern_trie_t* fn_trie, pattern_trie_t* type_trie);