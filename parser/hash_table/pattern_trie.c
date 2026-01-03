#include "pattern_trie.h"
#include "hash_table.h"
#include "../colors.h"
#include "../constructs.h"
#include "../construct_utils.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
bool type_identifier_compare(type_identifier_t* a, type_identifier_t* b){
  if(a->type_id != b->type_id){
    return false;
  }
  if(a->dimension_count != b->dimension_count){
    return false;
  }
  if(a->num_params != b->num_params){
    return false;
  }
  for(int i = 0; i < a->num_params; i++){
    if(!type_identifier_compare(a->params[i].return_type, b->params[i].return_type)){
      return false;
    }
  }
  return true;
}
bool pattern_entry_compare(pattern_entry_t* a, pattern_entry_t* b){
  if(a->is_identifier != b->is_identifier){
    printf(RED BOLD "is identifier failed" RESET_COLOR);
    return false;
  }

  if(a->is_identifier){
    printf(RED BOLD "strings don't match" RESET_COLOR);
    return !strcmp(a->identifier, b->identifier);
  }
 
  if(a->variable.constant_lvl != b->variable.constant_lvl){
    printf(RED BOLD "const lvls don't match" RESET_COLOR);
    return false;
  }
  if(a->variable.type.is_param != b->variable.type.is_param){
    printf(RED BOLD "is_params don't match" RESET_COLOR);
    return false;
  }
  if(a->variable.type.is_param){
    return true;
  }
  pattern_type_t* a_type = &a->variable.type;
  pattern_type_t* b_type = &b->variable.type;

  if(a_type->base_type_id != b_type->base_type_id){
    printf(RED BOLD "different base types" RESET_COLOR);
    return false;
  }

  if(a_type->dimension_count != b_type->dimension_count){
    printf(RED BOLD "different dimension counts" RESET_COLOR);
    return false;
  }

  if(a_type->param_count != b_type->param_count){
    printf(RED BOLD "different param counts" RESET_COLOR);
    return false;
  }
  for(int i = 0; i < a_type->dimension_count; i++){
    if(a_type->dimensions[i].is_param != b_type->dimensions[i].is_param){
      return false;
    }

    if(!a_type->dimensions[i].is_param){
      if(a_type->dimensions[i].base_value->type == EXP_VALUE_LITERAL && b_type->dimensions[i].base_value->type == EXP_VALUE_LITERAL){
        if(a_type->dimensions[i].base_value->value_literal.u != a_type->dimensions[i].base_value->value_literal.u){
          return false;
        }
      }
    }
  }

  for(int i = 0; i < a_type->param_count; i++){
    if(a_type->parameters[i].is_param != b_type->parameters[i].is_param){
      return false;
    }
    if(a_type->parameters[i].is_param){
      if(!type_identifier_compare(&a_type->parameters[i].param_value, &b_type->parameters[i].param_value)){
        return false;
      }
    }
    else{

    }
    
  }
  return true;
}

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
    int* child_index = NULL;
    if(entry.is_identifier){
      child_index = get_value_from_key(&node->next_identifiers, entry.identifier);
    }
    else{
      if(entry.variable.type.is_param){
        
      }
      else{
        int* match_index = get_value_from_int(&node->next_parameters, entry.variable.type.base_type_id);
        if(match_index == NULL){
          return node;
        }
        possible_type_matches_t possible_indices = node->type_matches[*match_index];
        child_index = NULL;
        printf(GREEN "%i possibilities" RESET_COLOR, possible_indices.num_possibilities);
        for(int i = 0; i < possible_indices.num_possibilities; i++){
          pattern_entry_t* compareto = &node->children[possible_indices.possible_matches[i]].pattern;
          if(pattern_entry_compare(compareto, &entry)){
            child_index = &possible_indices.possible_matches[i];
            break;
          }
        }
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

void pattern_trie_node_push_pattern(pattern_trie_node_t** root, pattern_t* pattern, int result_id){
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
        int* mip = get_value_from_int(&node->next_parameters, entry->variable.type.base_type_id);
        int match_index;
        if(mip == NULL){
          int len = node->next_parameters.key_count;
          node->type_matches = realloc(node->type_matches, (len + 1) * sizeof(possible_type_matches_t));
          possible_type_matches_t new = {.num_possibilities = 0,.possible_matches = NULL};
          node->type_matches[len] = new;
          match_index = len; 
          push_int_value(&node->next_parameters, entry->variable.type.base_type_id, match_index);
        }
        else{
          match_index = *mip;
        }
        possible_type_matches_t* possible_indices = node->type_matches + match_index;
        possible_indices->num_possibilities++;
        possible_indices->possible_matches = realloc(possible_indices->possible_matches, possible_indices->num_possibilities * sizeof(int));
        possible_indices->possible_matches[possible_indices->num_possibilities - 1] = node->children_count;
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
    new_node->type_matches = NULL;
    copy_pattern_entry(&new_node->pattern, pattern->entries + entry_index);
   
    
 
    if(node == NULL){
      *root = new_node;
    }
    node = new_node;
    entry_index++;
  }
  node->match_index = result_id;
}

void pattern_trie_push_type(pattern_trie_t *trie, type_declaration_t *type){
  pattern_trie_node_push_pattern(&trie->root, type->match_pattern, trie->match_count);
  trie->match_count++;
  trie->matches = realloc(trie->matches, trie->match_count * sizeof(trie_match_result_t));
  trie_match_result_t new = {.type = MATCH_TYPE, .priority = 0, .typedec = *type};
  trie->matches[trie->match_count - 1] = new;
}

void print_trie_match_result(trie_match_result_t* result){
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

void print_trie_node(pattern_trie_node_t* node, int indent, int** levels){
  print_pattern_entry(&node->pattern);
  if(node->match_index != NO_MATCH){
    printf(" (MATCH #%d)", node->match_index);
  }
  if(node->children_count == 0){
    return;
  }
  *levels = realloc(*levels, (indent + 1) * sizeof(int));
  (*levels)[indent] = (indent > 0? 3 : 1) + (node->pattern.is_identifier? (node->pattern.identifier? (2 + strlen(node->pattern.identifier)) : 8) : (4 + node->pattern.variable.constant_lvl));
  printf(" ═> ");
  print_trie_node(node->children, indent + 1, levels);
  printf("\n");
  for(int k = 0; k < indent + 1; k++){
    for(int j = 0; j < (*levels)[k]; j++){
      printf(" ");
    }
    printf("║");
  }
  for(int i = 1; i < node->children_count; i++){
    printf("\n");
    for(int k = 0; k < indent + 1; k++){
      for(int j = 0; j < (*levels)[k]; j++){
        printf(" ");
      }
      if(k < indent){
        printf("║");
      }
    }
    printf("╚> ");
    print_trie_node(node->children + i, indent + 1, levels);
  }
}

void print_pattern_trie(pattern_trie_t *trie){
  printf("DEFINITIONS:\n");
  for(int i = 0; i < trie->match_count; i++){
    print_trie_match_result(trie->matches + i);
    printf("\n");
  }
  printf("\n");
  int* levels = NULL;
  print_trie_node(trie->root, 0, &levels);
  free(levels);
}

void destroy_pattern_trie_node(pattern_trie_node_t* node){
  for(int i = 0; i < node->next_parameters.key_count; i++){
    free(node->type_matches->possible_matches);
    node->type_matches->possible_matches = NULL;
    node->type_matches->num_possibilities = 0;
  }
  free(node->type_matches);
  node->type_matches = NULL;
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