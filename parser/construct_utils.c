#include "construct_utils.h"
#include "constructs.h"
#include "colors.h"
#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


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
void print_pattern_value(pattern_value_t* pval){
  printf("(");
  if(pval->is_param){
    
    print_variable_declaration(&pval->param_value);
  }
  else{
    print_expression(pval->base_value);
  }
  printf(")");
}

void print_pattern_type(pattern_type_t* ptype){
  if(!ptype) return;
  for(int i = 0; i < ptype->dimension_count; i++){
    print_pattern_value(ptype->dimensions + i);
    printf(MAGENTA "*" RESET_COLOR);
  }
  if(ptype->is_param){
    print_variable_declaration(&ptype->param_type);
  }
  else{
    printf("[" GREEN BOLD "#%i" RESET_COLOR, ptype->base_type_id);
    if(ptype->param_count > 0){
      printf("(");
      for(int i = 0; i < ptype->param_count; i++){
        if(i > 0){
          printf(",");
        }
        print_pattern_value(ptype->parameters + i);
      }
      printf(")");
    } 
    printf("]");
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
  print_type_identifier(fn_def->return_type);
  if(fn_def->is_IR){
    printf("\n   DOES (IR): %s", fn_def->ir);
  }
  else{
    print_block(&fn_def->body);
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
void destroy_pattern_value(pattern_value_t* pval){
  if(pval->is_param){
    destroy_variable_declaration(&pval->param_value);
  }
  else{
    destroy_expression(pval->base_value);
    free(pval->base_value);
    pval->base_value = NULL;
  }
}
void destroy_pattern_type(pattern_type_t* ptype){
  if(!ptype) return;
  for(int i = 0; i < ptype->dimension_count; i++){
    destroy_pattern_value(ptype->dimensions + i);
  }

  if(ptype->is_param){
    destroy_variable_declaration(&ptype->param_type);
  }
  else{
    for(int i = 0; i < ptype->param_count; i++){
      destroy_pattern_value(ptype->parameters + i);
    }
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
  destroy_type_identifier(fn_def->return_type);
  free(fn_def->return_type);
  fn_def->return_type = NULL;

  if(fn_def->is_IR){
    free(fn_def->ir);
    fn_def->ir = NULL;
  }
  else{
    destroy_block(&fn_def->body);
  }
}

void copy_block(block_t *new, block_t *block){
  new->line_count = block->line_count;
  new->lines = malloc(block->line_count * sizeof(expression_t));
  for(int i = 0; i < new->line_count; i++){
    copy_expression(new->lines + i, block->lines + i);
  }
}

void copy_expression(expression_t *new, expression_t *exp){
  new->type = exp->type;
  switch(new->type){
    case EXP_FUNCTION_CALL:
      new->function_call.num_params = exp->function_call.num_params;
      new->function_call.params = malloc(new->function_call.num_params * sizeof(expression_t));
      for(int i = 0; i < new->function_call.num_params; i++){
        copy_expression(new->function_call.params + i, exp->function_call.params + i);
      }
      break;
    case EXP_RAW_TOKEN:
      new->raw_token = exp->raw_token;
      break;
    case EXP_READ_VAR:
      new->read_var_id = exp->read_var_id;
      break;
    case EXP_TYPE_LITERAL:
      new->type_literal = malloc(sizeof(type_identifier_t));
      *new->type_literal = *exp->type_literal;
      break;
    case EXP_VALUE_LITERAL:
      if(exp->value_literal.type == VAL_STRING){
        new->value_literal.type = VAL_STRING;
        new->value_literal.s = malloc((1 + strlen(exp->value_literal.s)) * sizeof(char));
        strcpy(new->value_literal.s, exp->value_literal.s);
      }
      else{
        new->value_literal = exp->value_literal;
      }
      break;
    case EXP_VECTOR:
      new->vector.num_params = exp->vector.num_params;
      new->vector.params = malloc(exp->vector.num_params * sizeof(expression_t));
      for(int i = 0; i < exp->vector.num_params; i++){
        copy_expression(new->vector.params + i, exp->vector.params + i);
      }
      break;
  }
}
void copy_type_identifier(type_identifier_t *new, type_identifier_t *type){
  new->dimension_count = type->dimension_count;
  new->dimensions = malloc(type->dimension_count * sizeof(expression_t));
  new->num_params = type->num_params;
  new->params = malloc(type->num_params * sizeof(expression_t));
  new->type_id = type->type_id;

  for(int i = 0; i < new->dimension_count; i++){
    copy_expression(new->dimensions + i, type->dimensions + i);
  }
  for(int i = 0; i < new->num_params; i++){
    copy_expression(new->params + i, type->params + i);
  }
}

void copy_type_declaration(type_declaration_t *new, type_declaration_t *typedec){
    new->component_count = typedec->component_count;
    new->components = malloc(typedec->component_count * sizeof(type_declaration_t));
    new->is_enum = typedec->is_enum;
    new->is_is = typedec->is_is;
    new->match_pattern = malloc(sizeof(expression_t));
  copy_pattern(new->match_pattern, typedec->match_pattern);
  for(int i = 0; i < typedec->component_count; i++){
    copy_variable_declaration(new->components + i, typedec->components + i);
  }
}
void copy_variable_declaration(variable_declaration_t *new, variable_declaration_t *vardec){
  new->constant_lvl = vardec->constant_lvl;
  new->var_name = malloc((1 + strlen(vardec->var_name)) * sizeof(char));
  copy_type_identifier(&new->type, &vardec->type);
  strcpy(new->var_name, vardec->var_name);
}
void copy_pattern_value(pattern_value_t* new, pattern_value_t* pval){
  new->is_param = pval->is_param;
  if(new->is_param){
    copy_variable_declaration(&new->param_value, &pval->param_value);
  }
  else{
    new->base_value = malloc(sizeof(expression_t));
    copy_expression(new->base_value, pval->base_value);
  }
}

void copy_pattern_type(pattern_type_t* new, pattern_type_t* ptype){
  new->is_param = ptype->is_param;
  new->dimension_count = ptype->dimension_count;
  new->dimensions = malloc(ptype->dimension_count * sizeof(pattern_value_t));
  for(int i = 0; i < new->dimension_count; i++){
    copy_pattern_value(ptype->dimensions + i, new->dimensions + i);
  }
  if(new->is_param){
    copy_variable_declaration(&new->param_type, &ptype->param_type);
  }
  else{
    new->param_count = ptype->param_count;
    new->parameters = malloc(ptype->param_count * sizeof(pattern_value_t));
    new->base_type_id = ptype->base_type_id;
    for(int i = 0; i < new->param_count; i++){
      copy_pattern_value(new->parameters + i, ptype->parameters + i);
    }
  }
}

void copy_pattern_variable(pattern_variable_t* new, pattern_variable_t* pvar){
  new->constant_lvl = pvar->constant_lvl;
  new->name = malloc((1 + strlen(pvar->name)) * sizeof(char));
  copy_pattern_type(&new->type, &pvar->type);
  strcpy(new->name, pvar->name);
}

void copy_pattern_entry(pattern_entry_t* new, pattern_entry_t* pattern){
  new->is_identifier = pattern->is_identifier;
  if(new->is_identifier){
    new->identifier = malloc((1 + strlen(pattern->identifier)) * sizeof(char));
    strcpy(new->identifier, pattern->identifier);
  }
  else{
    copy_pattern_variable(&new->variable, &pattern->variable);
  }
}

void copy_pattern(pattern_t *new, pattern_t *pattern){
  new->entry_count = pattern->entry_count;
  for(int i = 0; i < new->entry_count; i++){
    copy_pattern_entry(new->entries + i, pattern->entries + i);
  }
}

void copy_function_definition(function_definition_t *new, function_definition_t *fn_def){
  new->is_IR = fn_def->is_IR;
  new->priority = fn_def->priority;
  new->return_type = malloc(sizeof(type_identifier_t));
  copy_type_identifier(new->return_type, fn_def->return_type);
  if(new->is_IR){
    new->ir = malloc((1 + strlen(fn_def->ir)) * sizeof(char));
    strcpy(new->ir, fn_def->ir);
  }
  else{
    copy_block(&new->body, &fn_def->body);
  }
}
