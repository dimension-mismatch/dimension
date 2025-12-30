#include "expression_builder.h"

#include "tokenizer.h"
#include "constructs.h"
#include "construct_utils.h"
#include "token_cursor.h"

#include <stdlib.h>
#include <stdio.h>



void print_expression_array(expression_array_t* array){
  array = array->next;
  while(array != NULL){
    print_expression(&array->exp);
    array = array->next;
  }
}