#include "expression_builder.h"

#include "tokenizer.h"
#include "constructs.h"
#include "construct_utils.h"
#include "token_cursor.h"
#include "hash_table/pattern_trie.h"
#include "colors.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>



void print_expression_array(expression_array_t* array){
  if(!array){
    printf("~Null Array~");
    return;
  }
  if(!array->next){
    printf("~Empty Array~");
    return;
  }
  array = array->next;
  while(array != NULL){
    print_expression(&array->exp);
    array = array->next;
    printf(" ");
  }
}

int* node_match_expression_array(pattern_trie_node_t* node, expression_array_t* array){
  if(node->match_index != NO_MATCH){
    return &node->match_index;
  }
  if(array == NULL){
    return NULL;
  }

  if(array->exp.type == EXP_RAW_TOKEN){
    int* cip = get_value_from_key(&node->next_identifiers, array->exp.raw_token.content);
    if(cip == NULL){
      //no pattern has this identifier at this position
      return NULL;
    }
    return node_match_expression_array(node->children + *cip, array->next);
  }
  else{
    if(!array->exp.return_type){
      //TODO : handle null typeid as the 'type' type;
      return NULL;
    }
    
    int* type_index = get_value_from_int(&node->next_parameters, array->exp.return_type->type_id);
    if(type_index == NULL){
      //no pattern has any form of this type at this position
      return NULL;
    }
    possible_type_matches_t* matches = node->type_matches + *type_index;
    for(int i = 0; i < matches->num_possibilities; i++){
      pattern_trie_node_t* child = node->children + matches->possible_matches[i];
      if(test_pattern_type(&child->pattern.variable.type, array->exp.return_type)){
        int* result = node_match_expression_array(child, array->next);
        if(result != NULL){
          return result; //TODO: check for highest priority result
        }
      }
    }
    return NULL;
  }
}

typedef struct match{
  expression_array_t* location;
  trie_match_result_t* content;
}match_t;



trie_match_result_t* match_expression_array(pattern_trie_t* trie, expression_array_t* array){
  int* result_index = node_match_expression_array(trie->root, array->next);
  if(!result_index){
    return NULL;
  }
  return trie->matches + *result_index;
}

expression_t* construct_fn_call(match_t match){
  expression_t fn_call = {
    .type = EXP_FUNCTION_CALL, 
    .function_call = {.fn_id = match.content->index, .num_params = 0, .params = NULL}};

  expression_array_t* start = match.location;
  expression_array_t* curr = start;
  expression_array_t* next = start->next;
  printf("Function Def: ");
  print_function_definition(&match.content->fndec);
  printf("\n");
  for(int i = 0; i < match.content->length; i++){
    curr = next;
    switch(match.content->fndec.match.entries[i].type){
      case PATTERN_IDENTIFIER:
        destroy_expression(&curr->exp);
        break;
      case PATTERN_VARIABLE: {
        int new_c = ++fn_call.function_call.num_params;
        fn_call.function_call.params = realloc(fn_call.function_call.params, new_c * sizeof(expression_t));
        fn_call.function_call.params[new_c - 1] = curr->exp;
        break;
      }
      case PATTERN_TYPE: {
        int new_c = fn_call.function_call.num_params + curr->exp.type_literal->num_params;
        fn_call.function_call.params = realloc(fn_call.function_call.params, new_c * sizeof(expression_t));
        for (unsigned int i = fn_call.function_call.num_params, j = 0; i < new_c; i++, j++){
          fn_call.function_call.params[i] = curr->exp.type_literal->params[j];
        }        
        fn_call.function_call.num_params = new_c;
      }
      break;
    }
    next = curr->next;
    free(curr);
  }
  expression_array_t* new = malloc(sizeof(expression_array_t));
  start->next = new;
  new->prev = start;
  new->next = next;
  if(next){
    next->prev = new;
  }
  new->exp = fn_call;
  return &new->exp;
}

expression_t* construct_type_call(match_t match){
  expression_t type_call = {.type = EXP_TYPE_LITERAL};
  type_identifier_t* type_id = malloc(sizeof(type_identifier_t));
  type_call.type_literal = type_id;
  type_id->dimensions.dimension_count = 0;
  type_id->dimensions.dimensions = NULL;
  type_id->num_params = 0;
  type_id->params = NULL;
  type_id->type_id = match.content->index; //TODO generate actual function ids

  expression_array_t* start = match.location;
  expression_array_t* curr = start;
  expression_array_t* next = start->next;
  printf("Type Def: ");
  print_type_declaration(&match.content->typedec);
  printf("\n");
  for(int i = 0; i < match.content->length; i++){
    curr = next;
    switch(match.content->typedec.match_pattern->entries[i].type){
      case PATTERN_IDENTIFIER:
        destroy_expression(&curr->exp);
        break;
      case PATTERN_VARIABLE:{
        int new_c = ++type_id->num_params;
        type_id->params = realloc(type_id->params, new_c * sizeof(expression_t));
        type_id->params[new_c - 1] = curr->exp;
        break;
      }
      case PATTERN_TYPE: {
        int new_c = type_id->num_params + curr->exp.type_literal->num_params;
        type_id->params = realloc(type_id->params, new_c * sizeof(expression_t));
        for (unsigned int i = type_id->num_params, j = 0; i < new_c; i++, j++){
          type_id->params[i] = curr->exp.type_literal->params[j];
        }        
        type_id->num_params = new_c;
      }
      break;
    }
    next = curr->next;
    free(curr);
  }
  expression_array_t* new = malloc(sizeof(expression_array_t));
  start->next = new;
  new->prev = start;
  new->next = next;
  if(next){
    next->prev = new;
  }
  new->exp = type_call;
  return &new->exp;
}

expression_t* construct_variable_read(match_t match){
  expression_t var_read = {.type = EXP_READ_VAR, .read_var_id = match.content->index, .return_type = malloc(sizeof(type_identifier_t))};

  copy_type_identifier(var_read.return_type, &match.content->vardec.type);

  expression_array_t* start = match.location;
  expression_array_t* next = start->next->next;
  printf("Var Read: ");
  print_variable_declaration(&match.content->vardec);
  printf("\n");
  free(start->next);
  expression_array_t* new = malloc(sizeof(expression_array_t));
  start->next = new;
  new->prev = start;
  new->next = next;
  if(next){
    next->prev = new;
  }
  new->exp = var_read;
  return &new->exp;
}

expression_t* construct_match(match_t match){
  switch(match.content->type){
    case MATCH_FUNCTION:
      return construct_fn_call(match);
    case MATCH_TYPE:
      return construct_type_call(match);
    case MATCH_VARIABLE:
      return construct_variable_read(match);
  }
}

expression_array_t* collapse_exp_array(pattern_trie_t* trie, expression_array_t* array){
  printf(RED BOLD "Expression Array: \n" RESET_COLOR);
  print_expression_array(array);
  printf("\n");
  expression_array_t* start = array;
  while(true){
    int step = 0;
    match_t best = {.content = NULL, .location = NULL};
    array = start;
    while(true){
      array = array->next;
      step++;
      if(best.content && step > best.content->length){
        break;
      }
      if(!array){
        printf(RED BOLD "Resulting Expression Array: \n" RESET_COLOR);
        print_expression_array(start);
        printf("\n");
        return start;
      }
    
      trie_match_result_t* contender = match_expression_array(trie, array->prev);
      if(contender && (!best.content || contender->priority > best.content->priority)){
        match_t new = {.content = contender, .location = array->prev};
        best = new;
      }
    }
    construct_match(best);
  }
  return NULL;
}

bool build_expression(pattern_trie_t* trie, expression_array_t* array, expression_t* result){
  if (!array || !array->next){
    return NULL;
  }
  collapse_exp_array(trie, array);
  expression_array_t* ptr = array->next;
  unsigned int count = 0;
  while(ptr){
    count++;
    if(ptr->exp.type == EXP_RAW_TOKEN){
      printf(RED BOLD "Failed to match token \"%s\"" RESET_COLOR, ptr->exp.raw_token.content);
      return false;
    }
    ptr = ptr->next;
  }

  if(count > 1){
    result->type = EXP_VECTOR;
    result->vector.num_params = count;
    result->vector.params = malloc(count * sizeof(expression_t));
    ptr = array->next;
    for(int i = 0; i < count; i++){
      result->vector.params[i] = ptr->exp;
      ptr = ptr->next;
    }
  }
  else{
    *result = array->next->exp;
  }
  return true;
}

//[i] + [f] * [f] ^ [f]
//^^^^^^^^^ 
//      ^^^^^^^^^
//            ^^^^^^^^^ 


//[i] + [f] * [f]
//^^^^^^^^^
//      ^^^^^^^^^


//[i] + [f]
//^^^^^^^^^

