#include "pattern_trie.h"
#include "hash_table.h"
#include "../colors.h"
#include "../constructs.h"
#include "../construct_utils.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pattern_trie_node_t trie_node_init(){
  pattern_trie_node_t new = {{.is_identifier = true, .identifier = NULL}, init_hash_table(67, 0.9), init_hash_table(67, 0.9), 0, NULL, NO_MATCH, 0};
  return new;
}

pattern_trie_t pattern_trie_init(){
  pattern_trie_t new = {NULL, 0,NULL};
  new.root = malloc(sizeof(pattern_trie_node_t));
  *(new.root) = trie_node_init();
  return new;
}

pattern_trie_node_t* pattern_trie_node_match_pattern(pattern_trie_node_t* node, pattern_t* pattern, int* entry_index){
  while(*entry_index < pattern->entry_count){
    pattern_entry_t entry = pattern->entries[*entry_index];
    int* child_index;
    if(entry.is_identifier){
      child_index = get_value_from_key(&node->next_identifiers, entry.identifier);
    }
    else{
      if(entry.variable.type.is_param){
        
      }
      else{
        child_index = get_value_from_int(&node->next_parameters, entry.variable.type.base_type_id);
      }
    }
    if(child_index == NULL){
      return node;
    }
    node = node->children + *child_index;
    (*entry_index)++;
  }
  return node;
}

void pattern_trie_node_push_pattern(pattern_trie_node_t** root, pattern_t* pattern){
  int entry_index = 0;
  pattern_trie_node_t* node = *root;
  if(node != NULL){
    node = pattern_trie_node_match_pattern(node, pattern, &entry_index);
  }
  while(entry_index < pattern->entry_count){
    pattern_entry_t* entry = pattern->entries + entry_index;
    if(entry->is_identifier){
      push_key_value(&node->next_identifiers, entry->identifier, node->children_count);
    }
    else{
      if(entry->variable.type.is_param){

      }
      else{
        push_int_value(&node->next_parameters, entry->variable.type.base_type_id, node->children_count);
      }
    }
    node->children_count++;
    node->children = realloc(node->children, node->children_count * sizeof(pattern_trie_node_t));
    pattern_trie_node_t* new_node = node->children + node->children_count - 1;
    new_node->children = NULL;
    new_node->children_count = 0;
    new_node->match_index = NO_MATCH;
    new_node->max_child_priority = 0;
    new_node->next_identifiers = init_hash_table(67, 0.9);
    new_node->next_parameters = init_hash_table(67, 0.9);
    copy_pattern_entry(&new_node->pattern, pattern->entries + entry_index);
   
    
 
    // printf(YELLOW BOLD "\n new output:\n" RESET_COLOR);
    // print_trie_node(node, 0);
    if(node == NULL){
      *root = new_node;
    }
    node = new_node;
    entry_index++;
  }
}

void pattern_trie_push_type(pattern_trie_t *trie, type_declaration_t *type){
  pattern_trie_node_push_pattern(&trie->root, type->match_pattern);
}

void print_trie_match_result(trie_match_result_t* result){
  printf(CYAN "MATCH RESULT: " RESET_COLOR);
  switch(result->type){
    case MATCH_FUNCTION:
      print_function_definition(&result->fndec);
      break;
    case MATCH_TYPE:
      print_type_declaration(&result->typedec);
      break;
    case MATCH_VARIABLE:
      print_variable_declaration(&result->vardec);
      break;
  }
}

void print_trie_node(pattern_trie_node_t* node, int indent){
  print_pattern_entry(&node->pattern);
  if(node->match_index != NO_MATCH){
    printf(" (MATCH #%d)\n", node->match_index);
    return;
  }
  if(node->children_count == 0){
    return;
  }
  for(int i = 0; i < node->children_count; i++){
    printf("\n");
    for(int k = 0; k < indent; k++) printf("  ║  ");
    printf("  ╚═> ");
    print_trie_node(node->children + i, indent + 1);
  }
}

void print_pattern_trie(pattern_trie_t *trie){
  printf("DEFINITIONS:\n");
  for(int i = 0; i < trie->match_count; i++){
    print_trie_match_result(trie->matches + i);
  }
  printf("\n");
  print_trie_node(trie->root, 0);
}

void destroy_pattern_trie_node(pattern_trie_node_t* node){
  destroy_pattern_entry(&node->pattern);
  destroy_hash_table(&node->next_identifiers);
  destroy_hash_table(&node->next_parameters);
  for(int i = 0; i < node->children_count; i++){
    destroy_pattern_trie_node(node->children + i);
  }
  node->children_count = 0;
  free(node->children);
  node->children = NULL;
}

void destroy_trie_match_result(trie_match_result_t* result){
  switch(result->type){
    case MATCH_FUNCTION:
      destroy_function_definition(&result->fndec);
    break;
    case MATCH_TYPE:
      destroy_type_declaration(&result->typedec);
    break;
    case MATCH_VARIABLE:
      destroy_variable_declaration(&result->vardec);
    break;
  }
}
void destroy_pattern_trie(pattern_trie_t* trie){
  destroy_pattern_trie_node(trie->root);
  for(int i = 0; i < trie->match_count; i++){
    destroy_trie_match_result(trie->matches + i);
  }
  free(trie->matches);
  trie->matches = NULL;
  trie->match_count = 0;
}