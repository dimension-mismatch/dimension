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
    if(match.content->fndec.match.entries[i].is_identifier){
      destroy_expression(&curr->exp);
    }
    else{
      int new_c = ++fn_call.function_call.num_params;
      fn_call.function_call.params = realloc(fn_call.function_call.params, new_c * sizeof(expression_t));
      fn_call.function_call.params[new_c - 1] = curr->exp;
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
  type_id->dimension_count = 0;
  type_id->dimensions = NULL;
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
    if(match.content->typedec.match_pattern->entries[i].is_identifier){
      destroy_expression(&curr->exp);
    }
    else{
      int new_c = ++type_id->num_params;
      type_id->params = realloc(type_id->params, new_c * sizeof(expression_t));
      type_id->params[new_c - 1] = curr->exp;
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
  expression_t var_read = {.type = EXP_READ_VAR, .read_var_id = match.content->index};

  expression_array_t* start = match.location;
  expression_array_t* next = start->next;
  printf("Var Read: ");
  print_variable_declaration(&match.content->vardec);
  printf("\n");
  destroy_expression(&match.location->exp);
  free(match.location);
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
    printf(YELLOW BOLD "Current Expression Array: \n" RESET_COLOR);
    print_expression_array(start);
    printf("\n");
  }
  return NULL;
}

expression_t* build_expression(pattern_trie_t* trie, expression_array_t* array){
  collapse_exp_array(trie, array);
  return NULL;
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

