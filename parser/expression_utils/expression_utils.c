#include "expression_utils.h"
#include "../type_utils/type_utils.h"
#include "../colors.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

expression_t* exp_init(expression_type_t type, type_identifier_t returnType){
  expression_t* new = calloc(1, sizeof(expression_t));
  new->type = type;
  new->return_type = returnType;
  new->text = NULL;
  return new;
}

expression_t* exp_create_identifier(char* content, int token_id){
  expression_t* new = exp_init(EXP_IDENTIFIER, typeid_newEmpty());
  new->text = malloc((strlen(content) + 1) * sizeof(char));
  new->token_id = token_id;
  strcpy(new->text, content);
  return new;
}

expression_t* exp_create_var_declaration(type_identifier_t returnType, char* name){
  expression_t* new = exp_init(EXP_DECLARE_VAR, typeid_copy(&returnType));
  new->text = malloc((strlen(name) + 1) * sizeof(char));
  strcpy(new->text, name);
  
  return new;
}

expression_t* exp_create_var_read(type_identifier_t returnType, int varid){
  expression_t* new = exp_init(EXP_READ_VAR, typeid_copy(&returnType));
  new->read.var_id = varid;
  return new;
}

expression_t* exp_create_var_write(type_identifier_t returnType, int varid, expression_t* value){
  expression_t* new = exp_init(EXP_WRITE_VAR, typeid_copy(&returnType));
  new->write.var_id = varid;
  new->write.value = malloc(sizeof(expression_t*));
  new->write.value[0] = value;
  return new;
}

expression_t* exp_create_return(int byte_offset, expression_t* value){
  expression_t* new;
  if(value != NULL){
    new = exp_init(EXP_RETURN, typeid_copy(&(value->return_type)));
    new->write.var_id = byte_offset;
    new->write.value = malloc(sizeof(expression_t*));
    new->write.value[0] = value;
  }
  else{
    new = exp_init(EXP_RETURN, typeid_newEmpty());
    new->write.var_id = byte_offset;
  }
 
  return new;
}

expression_t* exp_create_grouping(int enter_group){
  expression_t* new = exp_init(EXP_GROUPING, typeid_newEmpty());
  new->enter_group = enter_group;
  return new;
}

expression_t* exp_create_numeric_literal(int value, type_identifier_t returnType){
  expression_t* new = exp_init(EXP_SINGLE_LITERAL, typeid_copy(&returnType));
  new->numeric_literal = value;
  return new;
}

expression_t* exp_create_block(){
  expression_t* new = exp_init(EXP_BLOCK, typeid_newEmpty());
  new->function_call.arg_c = 0;
  new->function_call.arg_v = NULL;
  return new;
}

void exp_block_push_line(expression_t* block, expression_t* line){
  if(block == NULL || line == NULL){
    return;
  }
  block->block.arg_c++;
  block->block.arg_v = realloc(block->block.arg_v, block->block.arg_c * sizeof(expression_t*));
  block->block.arg_v[block->block.arg_c - 1] = line;
}

expression_t* exp_create_error(){
  expression_t* new = exp_init(EXP_ERROR, typeid_newEmpty());
  return new;
}

void exp_array_push_expression(exp_array_t** root, exp_array_t** current_node, expression_t* expression){
  exp_array_t* new_node = malloc(sizeof(exp_array_t));
  new_node->expression = expression;
  new_node->next = NULL;
  if(*root == NULL){
    *root = new_node;
  }
  else{
    (*current_node)->next = new_node; 
  }
  *current_node = new_node;
}


void print_expression(expression_t* exp){
  int i;
  if(exp == NULL){
    printf("{NULL EXP}");
    return;
  }
  switch(exp->type){
    case EXP_IDENTIFIER:
      printf("%s", exp->text);
      return;
    case EXP_CALL_FN:
      printf("{" MAGENTA "FN" GREEN "#%d" RESET_COLOR "(", exp->function_call.fn_id);
      i = 0;
      while(i < exp->function_call.arg_c - 1){
        print_expression(exp->function_call.arg_v[i]);
        printf(",");
        i++;
      }
      if(exp->function_call.arg_c > 0){
        print_expression(exp->function_call.arg_v[i]);
      }
      printf(") -> ");
      print_type_id(&(exp->return_type));
      //printf(RED BOLD "return type has %d dimensions\n" RESET_COLOR, exp->return_type.dimension_count);
      printf("}");
      return;
    case EXP_BLOCK:
      printf(YELLOW BOLD "BLOCK: \n" RESET_COLOR);
      for(int i = 0; i < exp->block.arg_c; i++){
        printf("%d : ", i);
        print_expression(exp->block.arg_v[i]);
        printf("\n");
      }
      return;
    case EXP_GROUPING:
      if(exp->enter_group){
        printf("(");
      }
      else{
        printf(")");
      }
      break;
    case EXP_WRITE_VAR:
      printf("SET VAR #%d TO : ", exp->write.var_id);
      print_expression(exp->write.value[0]);
      break;
    case EXP_RETURN:
      printf("RETURN: ");
      if(exp->write.value) print_expression(exp->write.value[0]);
      break;
    case EXP_VECTOR_LITERAL:
      printf("⟨");
      i = 0;
      while(i < exp->vector_literal.component_count - 1){
        print_expression(exp->vector_literal.components[i]);
        printf(",");
        i++;
      }
      print_expression(exp->vector_literal.components[i]);
      printf("⟩");
      break;
    case EXP_ERROR:
      printf(RED BOLD "{ERROR EXP}" RESET_COLOR);
    default: 
      if(exp->text != NULL){
        printf("\"%s\" : ", exp->text);
      }
      print_type_id(&(exp->return_type));
  }
}

void print_exp_array(exp_array_t* array){
  while(array != NULL){
    print_expression(array->expression);
    printf("   ");
    array = array->next;
  }
}

void exp_destroy(expression_t* exp){
  if(exp == NULL) return;

  typeid_destroy(&(exp->return_type));

  free(exp->text);
  exp->text = NULL;

  
  switch(exp->type){
    case EXP_NONE:
    case EXP_IDENTIFIER:
    case EXP_DECLARE_VAR:
    case EXP_GROUPING:
    case EXP_ERROR:
      break;
    case EXP_READ_VAR:
      exp->read.var_id = -1;
      break;
    case EXP_BLOCK:
      exp->block.stack_depth = 0;
      for(int i = 0; i < exp->block.arg_c; i++){
        exp_destroy(exp->block.arg_v[i]);
        free(exp->block.arg_v[i]);
      }
      free(exp->block.arg_v);
      exp->block.arg_v = NULL;  
      break;
    case EXP_CALL_FN:
      exp->function_call.fn_id = -1;
      for(int i = 0; i < exp->function_call.arg_c; i++){
        exp_destroy(exp->function_call.arg_v[i]);
        free(exp->function_call.arg_v[i]);
      }
      free(exp->function_call.arg_v);
      exp->function_call.arg_v = NULL;  
      break;
    case EXP_WRITE_VAR:
    case EXP_RETURN:
      exp->write.var_id = -1;
      if(exp->write.value) exp_destroy(exp->write.value[0]); 
      free(exp->write.value);
      exp->write.value = NULL;
      break;
    case EXP_VECTOR_LITERAL:
      for(int i = 0; i < exp->vector_literal.component_count; i++){
        exp_destroy(exp->vector_literal.components[i]);
        free(exp->vector_literal.components[i]);
      }
      free(exp->vector_literal.components);
      exp->vector_literal.components = NULL;
      break;  
    case EXP_SINGLE_LITERAL:
      break;
  }
}

void exp_array_destroy(exp_array_t* array){
  while(array != NULL){
    exp_destroy(array->expression);
    exp_array_t* next = array->next;
    free(array);
    array = next;
  }
}