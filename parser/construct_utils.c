#include "construct_utils.h"
#include "constructs.h"
#include "colors.h"
#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


void print_block(block_t* block){
  if(!block) return;
  for(int i = 0; i < block->line_count; i++){
    print_expression(&block->lines[i]);
  }
}
void print_expression(expression_t* exp){
  if(!exp) return;
  switch(exp->type){
    case EXP_FUNCTION_CALL:
      printf(GREEN "CALL" BOLD "#%i" RESET_COLOR ": (" , exp->function_call.fn_id);
      for(int i = 0; i < exp->function_call.num_params; i++){
        if(i > 0){
          printf(",");
        }
        print_expression(&exp->function_call.params[i]);
      }
      break;
    case EXP_TYPE_LITERAL:
      print_type_identifier(exp->type_literal);
      break;
    case EXP_VALUE_LITERAL:
      switch(exp->value_literal.type){
        case VAL_INT:
          printf(YELLOW "%i" RESET_COLOR, exp->value_literal.i);
          break;
        case VAL_UNSIGNED:
          printf(YELLOW "%u" RESET_COLOR, exp->value_literal.i);
          break;
        case VAL_FLOAT:
          printf(YELLOW "%f" RESET_COLOR, exp->value_literal.f);
          break;
        case VAL_CHAR:
          printf(WHITE "'%c'" RESET_COLOR, exp->value_literal.c);
          break;
        case VAL_STRING:
          printf(WHITE "\"%s\"" RESET_COLOR, exp->value_literal.s);
          break;
      }
      break;
    case EXP_VECTOR:
      printf("(");
      for(int i = 0; i < exp->vector.num_params; i++){
        if(i > 0){
          printf(",");
        }
        print_expression(&exp->vector.params[i]);
      }
      printf(")");
      break;
    case EXP_READ_VAR:
      printf(GREEN "READ #%i", exp->read_var_id);
      break;
    case EXP_RAW_TOKEN:
      print_token(&exp->raw_token);
      break;
  }
}

void print_type_identifier(type_identifier_t* type){
  if(!type) return;
  printf(GREEN "TYPE" RESET_COLOR);
  for(int i = 0; i < type->dimension_count; i++){
    printf("(");
    print_expression(&type->dimensions[i]);
    printf(")" MAGENTA "*" RESET_COLOR);
  }
  printf("[" GREEN BOLD "#%i" RESET_COLOR, type->type_id);
  if(type->num_params > 0){
    printf("(");
    for(int i = 0; i < type->num_params; i++){
      if(i > 0){
        printf(",");
      }
      print_expression(&type->params[i]);
    }
    printf(")");
  } 
  printf("]");
}

void print_type_declaration(type_declaration_t* typedec){
  if(!typedec) return;
  printf(CYAN "TYPE [" RESET_COLOR);
  print_pattern(typedec->match_pattern);
  printf(CYAN "] %s %s", typedec->is_is ? "is" : "has", typedec->is_enum ? "oneof" : "");
  printf("(");
  for(int i = 0; i < typedec->component_count; i++){
    if(i > 0){
      printf(",");
    }
    print_variable_declaration(typedec->components + i);
  }
  printf(")");
}
void print_variable_declaration(variable_declaration_t* vardec){
  if(!vardec) return;
  if(vardec->var_name){
    printf(MAGENTA "%s " RESET_COLOR BLUE, vardec->var_name);
    for(int i = 0; i < vardec->constant_lvl; i++){
      printf(":");
    }
    printf(RESET_COLOR);
  }
  print_type_identifier(&vardec->type);
}

void print_pattern_type(pattern_type_t* ptype){
  if(!ptype) return;
  for(int i = 0; i < ptype->dimension_count; i++){
    printf("(");
    if(ptype->dimensions[i].is_param){
      
      print_variable_declaration(&ptype->dimensions[i].param_dimension);
    }
    else{
      print_expression(ptype->dimensions[i].base_dimension);
    }
    printf(")");
    printf(MAGENTA "*" RESET_COLOR);
  }
  if(ptype->is_param){
    print_variable_declaration(&ptype->param_type);
  }
  else{
    print_type_identifier(ptype->base_type);
  }
}
void print_pattern_variable(pattern_variable_t* pvar){
  if(!pvar) return;
  if(pvar->name){
    printf(MAGENTA "%s " BLUE, pvar->name);
    for(int i = 0; i < pvar->constant_lvl; i++){
      printf(":");
    }
    printf(RESET_COLOR " ");
  }
  print_pattern_type(&pvar->type);
}
void print_pattern_entry(pattern_entry_t* pentry){
  if(!pentry) return;
  if(pentry->is_identifier){
    printf(WHITE "\"%s\"" RESET_COLOR, pentry->identifier);
  }
  else{
    print_pattern_variable(&pentry->variable);
  }
}
void print_pattern(pattern_t* pattern){
  for(int i = 0; i < pattern->entry_count; i++){
    print_pattern_entry(pattern->entries + i);
    printf(" ");
  }
}
void print_function_definition(function_definition_t* fn_def){
  printf(GREEN "FUNCTION: \n" RESET_COLOR);
  printf("   MATCHES: ");
  print_pattern(&fn_def->match);
  printf(" (Priority %i)", fn_def->priority);
  printf("\n   RETURNS: ");
  print_expression(fn_def->return_type);
  if(fn_def->is_IR){
    printf("\n   DOES (IR): %s", fn_def->ir);
  }
  else{
    print_block(fn_def->body);
  }
  
} 


void destroy_block(block_t* block){
  if(!block) return;
  for(int i = 0; i < block->line_count; i++){
    destroy_expression(block->lines + i);
  }
  free(block->lines);
  block->lines = NULL;
  block->line_count = 0;
}
void destroy_expression(expression_t* exp){
  if(!exp) return;
  switch(exp->type){
    case EXP_FUNCTION_CALL:
      for(int i = 0; i < exp->function_call.num_params; i++){
        destroy_expression(exp->function_call.params + i);
      }
      free(exp->function_call.params);
      exp->function_call.params = NULL;
      exp->function_call.num_params = 0;
      break;
    case EXP_TYPE_LITERAL:
      destroy_type_identifier(exp->type_literal);
      free(exp->type_literal);
      exp->type_literal = NULL;
      break;
    case EXP_VALUE_LITERAL:
      switch(exp->value_literal.type){
        case VAL_STRING:
          free(exp->value_literal.s);
          exp->value_literal.s = NULL;
          break;
        default:
          break;
      }
      break;
    case EXP_VECTOR:
      for(int i = 0; i < exp->vector.num_params; i++){
        destroy_expression(exp->vector.params + i);
        free(exp->vector.params);
        exp->vector.params = NULL;
        exp->vector.num_params = 0;
      }
      break;
    case EXP_RAW_TOKEN:
    case EXP_READ_VAR:
      break;
  }
  
}
void destroy_type_identifier(type_identifier_t* type){
  if(!type) return;
  for(int i = 0; i < type->dimension_count; i++){
    destroy_expression(type->dimensions + i);
  }
  free(type->dimensions);
  type->dimensions = NULL;
  type->dimension_count = 0;
  for(int i = 0; i < type->num_params; i++){
    destroy_expression(type->params + i);
  }
  free(type->params);
  type->params = NULL;
  type->num_params = 0;
}

void destroy_type_declaration(type_declaration_t* typedec){
  if(!typedec) return;
  destroy_pattern(typedec->match_pattern);
  free(typedec->match_pattern);
  typedec->match_pattern = NULL;

  for(int i = 0; i < typedec->component_count; i++){
    destroy_variable_declaration(typedec->components + i);
  }
  free(typedec->components);
  typedec->components = NULL;
  typedec->component_count = 0;
}

void destroy_variable_declaration(variable_declaration_t* vardec){
  if(!vardec) return;
  free(vardec->var_name);
  vardec->var_name = NULL;

  destroy_type_identifier(&vardec->type);
}
void destroy_pattern_type(pattern_type_t* ptype){
  if(!ptype) return;
  for(int i = 0; i < ptype->dimension_count; i++){
    if(ptype->dimensions[i].is_param){
      destroy_variable_declaration(&ptype->dimensions[i].param_dimension);
    }
    else{
      destroy_expression(ptype->dimensions[i].base_dimension);
      free(ptype->dimensions[i].base_dimension);
      ptype->dimensions[i].base_dimension = NULL;
    }
  }

  if(ptype->is_param){
    destroy_variable_declaration(&ptype->param_type);
  }
  else{
    destroy_type_identifier(ptype->base_type);
    free(ptype->base_type);
    ptype->base_type = NULL;
  }
}
void destroy_pattern_variable(pattern_variable_t* pvar){
  if(!pvar) return;
  free(pvar->name);
  pvar->name = NULL;
  destroy_pattern_type(&pvar->type);
}
void destroy_pattern_entry(pattern_entry_t* pentry){
  if(!pentry) return;
  if(pentry->is_identifier){
    free(pentry->identifier);
    pentry->identifier = NULL;
  }
  else{
    destroy_pattern_variable(&pentry->variable);
  }
}
void destroy_pattern(pattern_t* pattern){
  for(int i = 0; i < pattern->entry_count; i++){
    destroy_pattern_entry(pattern->entries + i);
  }
  free(pattern->entries);
  pattern->entries = NULL;
  pattern->entry_count = 0;
}
void destroy_function_definition(function_definition_t* fn_def){
  destroy_pattern(&fn_def->match);
  destroy_expression(fn_def->return_type);
  free(fn_def->return_type);
  fn_def->return_type = NULL;

  if(fn_def->is_IR){
    free(fn_def->ir);
    fn_def->ir = NULL;
  }
  else{
    destroy_block(fn_def->body);
    free(fn_def->body);
    fn_def->body = NULL;
  }
}