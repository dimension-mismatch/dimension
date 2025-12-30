#include "pattern_trie.h"
#include "hash_table.h"
#include "../colors.h"
#include "../constructs.h"
#include "../construct_utils.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pattern_trie_node_t trie_node_init(){
  pattern_trie_node_t new = {init_hash_table(67, 0.9), init_hash_table(67, 0.9), 0, NULL, NO_MATCH, 0};
  return new;
}

pattern_trie_t pattern_trie_init(){
  pattern_trie_t new = {NULL, 0,0,NULL,NULL};
  new.root = malloc(sizeof(pattern_trie_node_t));
  *(new.root) = trie_node_init();
  return new;
}



void trie_node_destroy(pattern_trie_node_t* node){
  for(int i = 0; i < node->children_count; i++){
    fn_tree_destroy(node->children + i);
  }
  destroy_hash_table(&(node->next_identifiers));
  destroy_hash_table(&(node->next_parameters));
  free(node->children);
  node->children = NULL;
  node->children_count = 0;
}

void fn_rec_destroy(pattern_trie_t *record){
  fn_tree_destroy(record->root);
  free(record->root);
  record->root = NULL;
  for(int i = 0; i < record->match_count; i++){
    trie_match_result_t res = record->matches[i];
    switch(res.type){
      case MATCH_FUNCTION:
        destroy_function_definition(&res.fndec);
        break;
      case MATCH_TYPE:
        destroy_type_declaration(&res.typedec);
        break;
      case MATCH_VARIABLE:
        destroy_variable_declaration(&res.vardec);
        break;
    }
  }
  free(record->matches);
  record->matches = NULL;
  record->match_count = 0;
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
        print_fn_tree(node->children  + entry->value, indent + 3);
        entry = entry->next;
      }
      
    }
  }
  if(tree->type_table.key_count > 0){
    for(int i = 0; i < indent; i++) printf(" ");
    printf("MATCH TYPES:\n");
    for(int i = 0; i < tree->type_table.array_size; i++){
      struct hash_entry* entry = tree->type_table.array[i];
      if(entry == NULL){
        continue;
      }
      while(entry != NULL){
        for(int i = 0; i < indent; i++) printf(" ");
        printf("  * type: [%d]\n", entry->id);
        print_fn_tree(tree->children + entry->value, indent + 3);
        entry = entry->next;
      }
    }
  }
}

void print_fn_rec(pattern_trie_t *record){
  printf("DEFINITIONS:\n");
  for(int i = 0; i < record->def_count; i++){
    print_fn_def(record->definitions[i]);
  }
  
  print_fn_tree(record->root, 0);
}

