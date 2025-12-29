#include "construct_utils.h"
#include "constructs.h"
#include "colors.h"

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


void destroy_block(block_t* block);
void destroy_expression(expression_t* exp);
void destroy_type_identifier(type_identifier_t* type);
void destroy_type_declaration(type_declaration_t* typedec);
void destroy_variable_declaration(variable_declaration_t* vardec);
void destroy_pattern_type(pattern_type_t* ptype);
void destroy_pattern_variable(pattern_variable_t* pvar);
void destroy_pattern_entry(pattern_entry_t* pentry);
void destroy_pattern(pattern_t* pattern);
void destroy_function_definition(function_definition_t* fn_def);