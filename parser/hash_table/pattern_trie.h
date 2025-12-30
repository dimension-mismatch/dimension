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

typedef struct{
  trie_match_type_t type;
  int priority;
  union{
    variable_declaration_t vardec;
    type_declaration_t typedec;
    function_definition_t fndec;
  };
}trie_match_result_t;


typedef struct pattern_trie_node{
  hash_table_t next_parameters;
  hash_table_t next_identifiers;
  int children_count;
  struct pattern_trie_node* children;
  int match_index;
  int max_child_priority;
}pattern_trie_node_t;

typedef struct{
  pattern_entry_t pattern;
  pattern_trie_node_t* root;
  int match_count;
  trie_match_result_t* matches;
}pattern_trie_t;

pattern_trie_t pattern_trie_init();


void pattern_trie_push_type(pattern_trie_t* trie,  type_declaration_t* type);

void pattern_trie_push_function(pattern_trie_t* trie, function_definition_t* fn);

void pattern_trie_push_variable(pattern_trie_t* trie, variable_declaration_t* var);


void pattern_trie_destroy(pattern_trie_t* trie);

void print_trie_match_result(trie_match_result_t* result);

void print_trie_node(pattern_trie_node_t* node, int indent);

void print_fn_rec(pattern_trie_t* record);


