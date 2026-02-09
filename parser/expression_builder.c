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
  array = array->next;
  while(array != NULL){
    print_expression(&array->exp);
    array = array->next;
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

trie_match_result_t* match_expression_array(pattern_trie_t* trie, expression_array_t* array){
  int* result_index = node_match_expression_array(trie->root, array->next);
  if(!result_index){
    return NULL;
  }
  return trie->matches + *result_index;
}

expression_t* build_expression(pattern_trie_t* trie, expression_array_t* array){
  trie_match_result_t* match = match_expression_array(trie, array);
  while(!match){
    array = array->next;
    if(!array){
      return NULL;
    }
    match = match_expression_array(trie, array);
  }

  
  printf(YELLOW "Trie Match Result (Priority %d): " RESET_COLOR, match->priority);
  print_trie_match_result(match);
  bool best_match = true;
  for(int i = 1; i < match->length; i++){
    array = array->next;
    trie_match_result_t* match2 = match_expression_array(trie, array);
    if(match2 && match2->priority > match->priority){
      printf("\n" RED "Overriding Result (Priority %d): " RESET_COLOR, match->priority);
      print_trie_match_result(match);
      best_match = false;
      build_expression(trie, array);
      break;
    }
  }
  if(best_match){
    
  }
  else{

  }
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

