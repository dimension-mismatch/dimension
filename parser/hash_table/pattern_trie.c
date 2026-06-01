#include "pattern_trie.h"
#include "hash_table.h"
#include "../colors.h"
#include "../constructs.h"
#include "../construct_utils.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
bool type_identifier_compare(type_identifier_t* a, type_identifier_t* b);
bool pattern_entry_compare(pattern_entry_t* a, pattern_entry_t* b);

bool compare_data(datum_t* a, datum_t* b){
  if(a->size != b->size){
    return false;
  }
  for(int i = 0; i < a->size; i++){
    if(a->data[i] != b->data[i]){
      return false;
    }
  }
  return true;
}

bool uint16_datum_compare(datum_t* a, uint16_t b){
  if(a->size != 2) return false; //datum must be 2 bytes to match uint16
  uint16_t read = (a->data[0] << 8) | a->data[1]; //(a->data[1] << 8) | a->data[0]; swap endianness
  return read == b;
}

bool type_argument_compare(type_argument_t* a, type_argument_t* b){
  if(a->is_subtype != b->is_subtype){
    return false;
  }
  if(a->is_subtype){
    return type_identifier_compare(a->subtype, b->subtype);
  }
  return compare_data(&a->arg, &b->arg);
}


bool type_identifier_compare(type_identifier_t* a, type_identifier_t* b){
  if(a->type_id != b->type_id){
    return false;
  }
  if(a->dimensions.dimension_count != b->dimensions.dimension_count){
    return false;
  }
  if(a->num_params != b->num_params){
    return false;
  }
  for(int i = 0; i < a->dimensions.dimension_count; i++){
    if(a->dimensions.dimensions[i] != b->dimensions.dimensions[i]){
      return false;
    }
  }
  for(int i = 0; i < a->num_params; i++){
    if(!type_argument_compare(&a->params[i], &b->params[i])){
      return false;
    }
  }
  return true;
}
bool pattern_type_compare(pattern_type_t* a, pattern_type_t* b);

bool pattern_value_compare(pattern_value_t* a, pattern_value_t* b){
  if(a->is_param != b->is_param){
    return false;
  }
  if(a->is_param){
    return pattern_type_compare(a->param.type, b->param.type);
  }
  else{
    return compare_data(&a->base_value, &b->base_value);
  }
}
bool pattern_type_compare(pattern_type_t* a, pattern_type_t* b){
  if(a->is_param != b->is_param){
    return false;
  }

  if(a->dimensions.dimension_count != b->dimensions.dimension_count){
    return false;
  }
  
  if(!a->is_param && a->param_count != b->param_count){
    return false;
  }
  for(int i = 0; i < a->dimensions.dimension_count; i++){
    if(!pattern_value_compare(a->dimensions.dimensions + i, b->dimensions.dimensions + i)){
      return false;
    }
  }
  if(a->is_param){
    return true;
  }
  for(int i = 0; i < a->subpattern->entry_count; i++){
    if(!pattern_entry_compare(a->subpattern->entries + i, b->subpattern->entries + i)){
      return false;
    }
  }
  return true;

}
bool pattern_entry_compare(pattern_entry_t* a, pattern_entry_t* b){
  if(a->type != b->type){
    return false;
  }
  switch(a->type){
    case PATTERN_IDENTIFIER:
      return !strcmp(a->identifier, b->identifier);
    case PATTERN_TYPE:
      return pattern_type_compare(&a->pattern_type, &b->pattern_type);
    case PATTERN_VARIABLE: {
      if(a->variable.constant_lvl != b->variable.constant_lvl){
        return false;
      }
      return pattern_type_compare(&a->variable.type, &b->variable.type);
    }
    case PATTERN_EXP:
      return compare_data(&a->datum, &b->datum);
  }
}


bool test_pattern_type(pattern_type_t* test, type_identifier_t* subject){
  if(test->is_param){
    return false; //TODO: handle matching for parameter types
  }
  if(test->base_type_id != subject->type_id){
    return false;
  }
  if(test->dimensions.dimension_count != subject->dimensions.dimension_count){
    return false; //TODO: handle multiplicity 
  }
  if(test->param_count != subject->num_params){
    return false; //shouldn't ever get here
  }
  for(int i = 0; i < test->dimensions.dimension_count; i++){
    if(!test->dimensions.dimensions[i].is_param){
      if(!uint16_datum_compare(&test->dimensions.dimensions[i].base_value, subject->dimensions.dimensions[i])){
        return false;
      }
    }
  }
  for(int i = 0, j = 0; i < test->subpattern->entry_count; i++){
    pattern_entry_t* entry = test->subpattern->entries + i;
    type_argument_t* arg = subject->params + j;
    switch(entry->type){
      case PATTERN_IDENTIFIER:
        break;
      case PATTERN_VARIABLE: {
        if(arg->is_subtype) return false; //we shouldn't need to check this since we know both structs fit the same pattern
        j++;
      }
      case PATTERN_TYPE: {
        if(!arg->is_subtype) return false;
        if(!test_pattern_type(&entry->pattern_type, arg->subtype)) return false;
        j++;
      }
      case PATTERN_EXP: {
        if(arg->is_subtype) return false;
        if(!compare_data(&entry->datum, &arg->arg)) return false;
        j++;
      }
    } 
  }
  return true;
}

pattern_trie_node_t trie_node_init(){
  pattern_trie_node_t new = {
    .pattern = {.type = PATTERN_IDENTIFIER, .identifier = NULL}, 
    .next_identifiers = init_hash_table(67, 0.9), 
    .next_parameters =  init_hash_table(67, 0.9), 
    .next_pattern_types = init_hash_table(67, 0.9), 
    .children_count = 0, 
    .children = NULL, 
    .parent = NULL,
    .match_index = NO_MATCH, 
    .max_child_priority = 0,
    .num_matches = 0,
    .type_matches = NULL};
  return new;
}

pattern_trie_t pattern_trie_init(){
  pattern_trie_t new = {.match_count = 0, .matches = NULL, .root = NULL, .scope_levels = 0, .scopes = NULL};
  new.root = malloc(sizeof(pattern_trie_node_t));
  *(new.root) = trie_node_init();
  pattern_trie_scope_in(&new);
  return new;
}

pattern_trie_node_t* pattern_trie_node_find_next(pattern_trie_node_t* node, pattern_entry_t* entry){
  int* child_index = NULL;
  switch(entry->type){
    case PATTERN_IDENTIFIER:
      child_index = get_value_from_key(&node->next_identifiers, entry->identifier);
      break;
    case PATTERN_VARIABLE:
      if(entry->variable.type.is_param){
        return NULL;
      }
      //Fall through is intentional here, both pattern types and variables need to compare to existing possibilities
    case PATTERN_TYPE: {
      int* match_index = (entry->type == PATTERN_TYPE)? 
      get_value_from_int(&node->next_pattern_types, entry->pattern_type.base_type_id):
      get_value_from_int(&node->next_parameters, entry->variable.type.base_type_id);

      if(match_index == NULL){
        return NULL;
      }
      possible_type_matches_t possible_indices = node->type_matches[*match_index];
      child_index = NULL;
      for(int i = 0; i < possible_indices.num_possibilities; i++){
        pattern_entry_t* compareto = &node->children[possible_indices.possible_matches[i]]->pattern;
        if(pattern_entry_compare(compareto, entry)){
          child_index = &possible_indices.possible_matches[i];
          break;
        }
      }
    }
    case PATTERN_EXP:
      break; // this case doesn't happen
  }
  if(child_index == NULL){
    return NULL;
  }
  return node->children[*child_index];
}

pattern_trie_node_t* pattern_trie_node_match_pattern(pattern_trie_node_t* node, pattern_t* pattern, int* entry_index){
  while(*entry_index < pattern->entry_count){
    pattern_entry_t entry = pattern->entries[*entry_index];
    pattern_trie_node_t* next = pattern_trie_node_find_next(node, &entry);
    if(!next){
      return node;
    }
    node = next;
    (*entry_index)++;
  }
  return node;
}

trie_match_result_t* pattern_trie_validate_pattern(pattern_trie_t* trie, pattern_t* pattern){
  int entry_index = 0;
  pattern_trie_node_t* node = pattern_trie_node_match_pattern(trie->root, pattern, &entry_index);
  if(entry_index < pattern->entry_count){
    return NULL;
  }
  if(node->match_index == NO_MATCH){
    return NULL;
  }
  return trie->matches + node->match_index;
}



pattern_trie_node_t* pattern_trie_node_push_pattern(pattern_trie_node_t* root, pattern_t* pattern, int result_id){
  int entry_index = 0;
  pattern_trie_node_t* node =  pattern_trie_node_match_pattern(root, pattern, &entry_index);

  while(entry_index < pattern->entry_count){
    pattern_entry_t* entry = pattern->entries + entry_index;
    switch(entry->type){
      case PATTERN_IDENTIFIER:
        push_key_value(&node->next_identifiers, entry->identifier, node->children_count);
        break;
      case PATTERN_VARIABLE:
        if(entry->variable.type.is_param){
          break;
        }
      //Fall through to handle the type of this variable as a pattern type
      case PATTERN_TYPE: {
        
        int* mip = entry->type == PATTERN_TYPE?
        get_value_from_int(&node->next_pattern_types, entry->pattern_type.base_type_id):
        get_value_from_int(&node->next_parameters, entry->variable.type.base_type_id);
        int match_index;
        if(mip == NULL){
          node->num_matches++;
          node->type_matches = realloc(node->type_matches, (node->num_matches) * sizeof(possible_type_matches_t));
          possible_type_matches_t new = {.num_possibilities = 0,.possible_matches = NULL};
          node->type_matches[node->num_matches - 1] = new;
          match_index = node->num_matches - 1; 
          if(entry->type == PATTERN_TYPE){
            push_int_value(&node->next_pattern_types, entry->pattern_type.base_type_id, match_index);
          }
          else{
            push_int_value(&node->next_parameters, entry->variable.type.base_type_id, match_index);
          }
        }
        else{
          match_index = *mip;
        }
        possible_type_matches_t* possible_indices = node->type_matches + match_index;
        possible_indices->num_possibilities++;
        possible_indices->possible_matches = realloc(possible_indices->possible_matches, possible_indices->num_possibilities * sizeof(int));
        possible_indices->possible_matches[possible_indices->num_possibilities - 1] = node->children_count;
        break;
      }
      case PATTERN_EXP:
        //this case doesn't happen
        break;
    }
    node->children_count++;
    node->children = realloc(node->children, node->children_count * sizeof(pattern_trie_node_t*));
    pattern_trie_node_t* new_node = malloc(sizeof(pattern_trie_node_t));
    node->children[node->children_count - 1] = new_node;
    *new_node = trie_node_init();
    new_node->parent = node;
    copy_pattern_entry(&new_node->pattern, pattern->entries + entry_index);

    node = new_node;
    entry_index++;
  }
  node->match_index = result_id;
  return node;
}

void pattern_trie_push_type(pattern_trie_t *trie, type_declaration_t *type){
  
  pattern_trie_node_t* node = pattern_trie_node_push_pattern(trie->root, type->match_pattern, trie->match_count);
  trie->match_count++;
  trie->matches = realloc(trie->matches, trie->match_count * sizeof(trie_match_result_t));
  trie_match_result_t new = {.match = node, .type = MATCH_TYPE, .priority = 0, .typedec = *type, .length = type->match_pattern->entry_count, .index = trie->match_count - 1};
  trie->matches[trie->match_count - 1] = new;
}

void pattern_trie_push_variable(pattern_trie_t* trie, variable_declaration_t* var){
  pattern_entry_t entry = {.type = PATTERN_IDENTIFIER, .identifier = var->var_name};
  pattern_t pattern = {.entries = &entry, .entry_count = 1};

  pattern_trie_node_t* node = pattern_trie_node_push_pattern(trie->root, &pattern, trie->match_count);
  trie->match_count++;
  trie->matches = realloc(trie->matches, trie->match_count * sizeof(trie_match_result_t));
  trie_match_result_t new = {.match = node, .type = MATCH_VARIABLE, .priority = 0, .vardec = *var, .length = 1, .index = trie->match_count - 1};
  trie->matches[trie->match_count - 1] = new;
}

void pattern_trie_push_function(pattern_trie_t* trie, function_definition_t* fn){
  pattern_trie_node_t* node = pattern_trie_node_push_pattern(trie->root, &fn->match, trie->match_count);
  trie->match_count++;
  trie->matches = realloc(trie->matches, trie->match_count * sizeof(trie_match_result_t));
  trie_match_result_t new = {.match = node, .type = MATCH_FUNCTION, .priority = fn->priority, .fndec = *fn, .length = fn->match.entry_count, .index = trie->match_count - 1};
  trie->matches[trie->match_count - 1] = new;
}

void print_trie_match_result(trie_match_result_t* result){
  if(!result) return;
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
void print_single_trie_node(pattern_trie_node_t* node){
  print_pattern_entry(&node->pattern);
  if(node->match_index != NO_MATCH){
    printf(" (MATCH #%d)", node->match_index);
  }
}

void print_trie_node(pattern_trie_node_t* node, int indent, int** levels){
  print_single_trie_node(node);
  //printf(RED "   %p   " RESET_COLOR, node);
  if(node->children_count == 0){
    
    return;
  }
  
  *levels = realloc(*levels, (indent + 1) * sizeof(int));
  (*levels)[indent] = (indent > 0? 3 : 1) + (node->pattern.type == PATTERN_IDENTIFIER? (node->pattern.identifier? (2 + strlen(node->pattern.identifier)) : 8) : (4 + node->pattern.variable.constant_lvl));
  printf(" ═> ");
  print_trie_node(node->children[0], indent + 1, levels);
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
    print_trie_node(node->children[i], indent + 1, levels);
  }
}

void print_pattern_trie(pattern_trie_t *trie){
  printf("DEFINITIONS:\n");
  for(int i = 0; i < trie->match_count; i++){
    if((trie->matches + i)->match){
      // printf("  ->   ");
      // print_single_trie_node((trie->matches + i)->match);
    }
    else{
      printf(BLACK BOLD "  Descoped " RESET_COLOR);
    }
    print_trie_match_result(trie->matches + i);
    //printf(RED "   %p" RESET_COLOR, (trie->matches + i)->match);
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
    destroy_pattern_trie_node(node->children[i]);
    free(node->children[i]);
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


void pattern_trie_scope_in(pattern_trie_t *trie){
  trie->scope_levels++;
  trie->scopes = realloc(trie->scopes, trie->scope_levels * sizeof(int));
  trie->scopes[trie->scope_levels - 1] = trie->match_count;
}

void pattern_trie_pop_pattern(pattern_trie_t* trie, int match_to_remove){
  pattern_trie_node_t* node = (trie->matches + match_to_remove)->match;
  (trie->matches + match_to_remove)->match = NULL;
  if(node->children_count > 0){
    return;
  }

  pattern_entry_t* slice_pattern;
  while(node->children_count < 2 && node->parent){
    
    pattern_trie_node_t* next = node->parent;
    slice_pattern = &node->pattern;
    if(next){
      node = next;
    }
    else{
      break;
    }
    
  }
  
  int* child_index = NULL;
  switch(slice_pattern->type){
    case PATTERN_IDENTIFIER:  
      child_index = get_value_from_key(&node->next_identifiers, slice_pattern->identifier);
      remove_key_value(&node->next_identifiers, slice_pattern->identifier);
      
      break;
    case PATTERN_VARIABLE:
      if(slice_pattern->variable.type.is_param){
        return;
      }
      //Fall through is intentional here, both pattern types and variables need to compare to existing possibilities
    case PATTERN_TYPE: {
      int* match_index = (slice_pattern->type == PATTERN_TYPE)? 
      get_value_from_int(&node->next_pattern_types, slice_pattern->pattern_type.base_type_id):
      get_value_from_int(&node->next_parameters, slice_pattern->variable.type.base_type_id);

      if(match_index == NULL){
        break;
        //We should always have a match so this should never be null
      }
      possible_type_matches_t*  possible_indices = node->type_matches + *match_index;
      child_index = NULL;
      for(int i = possible_indices->num_possibilities - 1; i <= 0; i--){
        pattern_entry_t* compareto = &node->children[possible_indices->possible_matches[i]]->pattern;
        if(pattern_entry_compare(compareto, slice_pattern)){
          *child_index = possible_indices->possible_matches[i];
          if(i == 0){
            if(slice_pattern->type == PATTERN_TYPE){
              remove_int_value(&node->next_pattern_types, slice_pattern->pattern_type.base_type_id);
            }
            else{
              remove_int_value(&node->next_parameters, slice_pattern->variable.type.base_type_id);
            }
            free(possible_indices->possible_matches);
            possible_indices->num_possibilities = 0;
            node->num_matches = *match_index;
            node->type_matches = realloc(node->type_matches, node->num_matches * sizeof(possible_type_matches_t));
            break;
          }
          possible_indices->num_possibilities = *match_index;
          possible_indices->possible_matches = realloc(possible_indices->possible_matches, possible_indices->num_possibilities * sizeof(int));
          break;
        }
      }
    }
    case PATTERN_EXP:
      break; //this case doesn't happen

  }

  destroy_pattern_trie_node(node->children[*child_index]);
  node->children_count = *child_index;
  node->children = realloc(node->children, node->children_count * sizeof(pattern_trie_node_t*));
}
void pattern_trie_scope_out(pattern_trie_t* trie){
  for(int i = trie->match_count - 1; i >= trie->scopes[trie->scope_levels - 1]; i--){
    if(trie->matches[i].match){
      pattern_trie_pop_pattern(trie, i);
    }
  }
  trie->scope_levels--;
  trie->scopes = realloc(trie->scopes, trie->scope_levels * sizeof(int));
}