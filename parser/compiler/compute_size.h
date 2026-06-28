#pragma once
#include "../dimension-IR/ir_construct_utils.h"
#include "../construct_utils.h"

#include "../dimension-IR/program_builder.h"

#include "../hash_table/pattern_trie.h"
#include "../hash_table/hash_table.h"

void compute_size(type_declaration_t *typedec, pattern_trie_t *type_trie, pattern_trie_t* fn_trie);