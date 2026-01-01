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
  if(node->match_index != NO_MATCH){
    for(int i = 0; i < indent; i++) printf(" ");
    printf("(MATCH #%d)\n", node->match_index);
    return;
  }
  if(node->next_identifiers.key_count > 0){
    for(int i = 0; i < indent; i++) printf(" ");
    printf("MATCH TEXT:\n");
    for(int i = 0; i < node->next_identifiers.array_size; i++){
      struct hash_entry* entry = node->next_identifiers.array[i];
      while(entry != NULL){
        for(int i = 0; i < indent; i++) printf(" ");
        printf("  * text: %s\n", entry->name);
        print_trie_node(node->children  + entry->value, indent + 3);
        entry = entry->next;
      }
      
    }
  }
  if(node->next_parameters.key_count > 0){
    for(int i = 0; i < indent; i++) printf(" ");
    printf("MATCH TYPES:\n");
    for(int i = 0; i < node->next_parameters.array_size; i++){
      struct hash_entry* entry = node->next_parameters.array[i];
      if(entry == NULL){
        continue;
      }
      while(entry != NULL){
        pattern_trie_node_t* child = node->children + entry->value;
        for(int i = 0; i < indent; i++) printf(" ");
        print_pattern_variable(&node->pattern.variable);
        printf("  :");
        print_trie_node(child, indent + 3);
        entry = entry->next;
      }
    }
  }
}

void print_pattern_trie(pattern_trie_t *trie){
  printf("DEFINITIONS:\n");
  for(int i = 0; i < trie->match_count; i++){
    print_trie_match_result(trie->matches + i);
  }
  
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