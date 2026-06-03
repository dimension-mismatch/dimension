#pragma once
#define NO_MATCH -1
#include "hash_table.h"

#include "../constructs.h"
#include <stdbool.h>

typedef enum{
  MATCH_TYPE,
  MATCH_FUNCTION,
  MATCH_VARIABLE
}trie_match_type_t;




typedef struct possible_type_matches{
  int num_possibilities;
  int* possible_matches;
}possible_type_matches_t;

typedef struct pattern_trie_node{
  pattern_entry_t pattern;
  hash_table_t next_parameters;
  hash_table_t next_identifiers;
  hash_table_t next_pattern_types;
  int children_count;
  struct pattern_trie_node** children;
  struct pattern_trie_node* parent;
  int match_index;
  
  int max_child_priority;

  int num_matches;
  possible_type_matches_t* type_matches;
}pattern_trie_node_t;

typedef struct{
  trie_match_type_t type;
  struct pattern_trie_node* match;
  int index;
  int priority;
  int length;
  union{
    variable_declaration_t vardec;
    type_declaration_t typedec;
    function_definition_t fndec;
  };
}trie_match_result_t;

typedef struct{
  pattern_trie_node_t* root;
  int match_count;
  trie_match_result_t* matches;
  int scope_levels;
  int* scopes;
}pattern_trie_t;

bool test_pattern_type(pattern_type_t *test, type_identifier_t *subject);

trie_match_result_t* pattern_trie_validate_pattern(pattern_trie_t* trie, pattern_t* pattern);

pattern_trie_t pattern_trie_init();

void pattern_trie_push_type(pattern_trie_t* trie,  type_declaration_t* type);

void pattern_trie_push_function(pattern_trie_t* trie, function_definition_t* fn);

void pattern_trie_push_variable(pattern_trie_t* trie, variable_declaration_t* var);


void destroy_pattern_trie(pattern_trie_t* trie);

void print_trie_match_result(trie_match_result_t* result);

void print_pattern_trie(pattern_trie_t* record);

void print_trie_starting_at_node(pattern_trie_node_t *node);

void pattern_trie_scope_in(pattern_trie_t* trie);

void pattern_trie_scope_out(pattern_trie_t* trie);

