#include "construct_utils.h"
#include "constructs.h"
#include "colors.h"
#include "tokenizer.h"
#include "dimension-IR/ir_construct_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void print_datum(datum_t* datum){
  for(int i = 0; i < datum->size; i++){
    printf("%x", datum->data[i]);
  }
}

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
      printf(")");
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
      printf(GREEN "READ #%i" RESET_COLOR, exp->read_var_id);
      break;
    case EXP_RAW_TOKEN:
      print_token(&exp->raw_token);
      break;
  }
}

void print_dimension_array(dimension_array_t* array){
  for(int i = 0; i < array->dimension_count; i++){
    printf("(");
    print_expression(array->dimensions + i);
    printf(")" MAGENTA "*" RESET_COLOR);
  }
}

void print_type_argument(type_argument_t* arg){
  switch(arg->type){
    case TYPEARG_SUBTYPE:
      print_type_identifier(arg->subtype);
      break;
    case TYPEARG_PARAM_EXP:
      print_expression(arg->exp);
      break;
  }
}

void print_type_identifier(type_identifier_t* type){
  if(!type) return;
  print_dimension_array(&type->dimensions);
  if(type->type_id == -1){
    printf("[%lu]", type->size);
    return;
  }
  printf("[" GREEN BOLD "#%i" RESET_COLOR, type->type_id);
  if(type->num_params > 0){
    printf("(");
    for(int i = 0; i < type->num_params; i++){
      if(i > 0){
        printf(",");
      }
      print_type_argument(&type->params[i]);
    }
    printf(")");
  } 
  printf("]");
}
void print_type_entry(type_entry_t* entry){
  if(entry->is_vector){
    printf(" %i components\n", entry->subvector.component_count);
    if(entry->subvector.name){
      printf(MAGENTA "%s" BLUE, entry->subvector.name);
      for(int i = 0; i < entry->subvector.const_lvl + 1; i++){
        printf(":");
      }
    }
    printf(CYAN);
    printf(entry->subvector.is_enum ? " oneof" : " ");
    printf(RESET_COLOR "(");
    for(int i = 0; i < entry->subvector.component_count; i++){
      print_type_entry(entry->subvector.components + i);
      if(i + 1 < entry->subvector.component_count) printf(", ");
    }
    printf(")");
  }
  else{
    if(entry->base.var_name){
      if(entry->base.type.type_id == -1 && entry->base.type.size == 0){
        printf(MAGENTA "%s" RESET_COLOR, entry->base.var_name);
      }
      else{
        print_variable_declaration(&entry->base);
      }
    }
    else{
      print_type_identifier(&entry->base.type);
    }
  }
}

void print_type_declaration(type_declaration_t* typedec){
  if(!typedec) return;
  printf(CYAN "TYPE [" RESET_COLOR);
  print_pattern(typedec->match_pattern);
  printf(CYAN "] %s ", typedec->is_is ? "is" : "has");
  print_type_entry(&typedec->entry);
  if(typedec->is_static_size){
    printf(" (%lu Bytes)", typedec->size);
    return;
  }
  printf("\n Size Program: \n");
  print_program(&typedec->compute_size);
}
void print_variable_declaration(variable_declaration_t* vardec){
  if(!vardec) return;
  if(vardec->var_name){
    printf(MAGENTA "%s " RESET_COLOR BLUE, vardec->var_name);
    for(int i = 0; i < vardec->constant_lvl + 1; i++){
      printf(":");
    }
    printf(RESET_COLOR);
  }
  print_type_identifier(&vardec->type);
}
void print_pattern_value(pattern_value_t* pval){
  printf("(");
  if(pval->is_param){
    print_pattern_variable(pval->param);
  }
  else{
    print_expression(pval->base_value);
  }
  printf(")");
}
void print_pattern_dimension_array(pattern_dimension_array_t* array){
  for(int i = 0; i < array->dimension_count; i++){
    print_pattern_value(array->dimensions + i);
    printf(MAGENTA "*" RESET_COLOR);
  }
}
void print_pattern_type(pattern_type_t* ptype){
  if(!ptype) return;
  print_pattern_dimension_array(&ptype->dimensions);
  if(ptype->is_param){
    print_type_identifier(&ptype->param_type);
  }
  else{
    printf("[");
    //printf(GREEN BOLD "#%i" RESET_COLOR, ptype->base_type_id);
    print_pattern(ptype->subpattern);
    printf("]");
  }
}
void print_pattern_variable(pattern_variable_t* pvar){
  if(!pvar) return;
  
  for(int i = 0; i < pvar->constant_lvl + 1; i++){
    printf(":");
  }
  printf(RESET_COLOR);
  print_pattern_type(&pvar->type);
}
void print_pattern_entry(pattern_entry_t* pentry){
  if(!pentry) return;
  switch(pentry->type){
    case PATTERN_IDENTIFIER: {
      printf(WHITE "%s" RESET_COLOR, pentry->identifier);
      break;
    }
    case PATTERN_VARIABLE:
      printf("(");
      print_pattern_variable(&pentry->variable);
      printf(")");
      break;
    case PATTERN_TYPE: 
      print_pattern_type(&pentry->pattern_type);
      break;
    case PATTERN_EXP:
      print_expression(&pentry->exp);
      break;
  }
}
void print_pattern(pattern_t* pattern){
  for(int i = 0; i < pattern->entry_count; i++){
    print_pattern_entry(pattern->entries + i);
    printf(" ");
  }
 // printf(" (%i total parameters)", pattern->param_count);
}
void print_function_definition(function_definition_t* fn_def){
  printf(GREEN "FUNCTION: \n" RESET_COLOR);
  printf("   MATCHES: ");
  print_pattern(&fn_def->match);
  printf(" (Priority %i)", fn_def->priority);
  printf("\n   RETURNS: ");
  print_type_identifier(fn_def->return_type);
  if(fn_def->is_IR){
    printf("\n   DOES (IR): \n");
    print_program(&fn_def->ir);
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

void destroy_datum(datum_t* datum){
  free(datum->data);
  datum->data = NULL;
  datum->size = 0;
}
void destroy_type_argument(type_argument_t* arg){
  switch(arg->type){
    case TYPEARG_SUBTYPE:
      destroy_type_identifier(arg->subtype);
      break;
    case TYPEARG_PARAM_EXP:
      destroy_expression(arg->exp);
      break;
  }
}

void destroy_dimension_array(dimension_array_t* array){
  free(array->dimensions);
  array->dimensions = NULL;
  array->dimension_count = 0;
}
void destroy_type_identifier(type_identifier_t* type){
  if(!type) return;
  destroy_dimension_array(&type->dimensions);
  if(type->type_id == -1) return;
  for(int i = 0; i < type->num_params; i++){
    destroy_type_argument(type->params + i);
  }
  free(type->params);
  type->params = NULL;
  type->num_params = 0;
}
void destroy_type_entry(type_entry_t* entry){
  if(entry->is_vector){
    for(int i = 0; i < entry->subvector.component_count; i++){
      destroy_type_entry(entry->subvector.components + i);
    }
    free(entry->subvector.components);
    entry->subvector.component_count = 0;
    entry->subvector.components = NULL;
  }
  else{
    destroy_variable_declaration(&entry->base);
  }
}
void destroy_type_declaration(type_declaration_t* typedec){
  if(!typedec) return;
  destroy_pattern(typedec->match_pattern);
  free(typedec->match_pattern);
  typedec->match_pattern = NULL;
  destroy_type_entry(&typedec->entry);
  if(typedec->is_static_size){

  }
  else{
    destroy_program(&typedec->compute_size);
  }
}

void destroy_variable_declaration(variable_declaration_t* vardec){
  if(!vardec) return;
  vardec->var_name = NULL;
  destroy_type_identifier(&vardec->type);
}
void destroy_pattern_value(pattern_value_t* pval){
  if(pval->is_param){
    destroy_pattern_variable(pval->param);
  }
  else{
    destroy_expression(pval->base_value);
  }
}
void destroy_pattern_dimension_array(pattern_dimension_array_t* array){
  for(int i = 0; i < array->dimension_count; i++){
    destroy_pattern_value(array->dimensions + i);
  }
  free(array->dimensions);
  array->dimensions = NULL;
  array->dimension_count = 0;
}

void destroy_pattern_type(pattern_type_t* ptype){
  if(!ptype) return;
  destroy_pattern_dimension_array(&ptype->dimensions);

  if(ptype->is_param){
    destroy_type_identifier(&ptype->param_type);
  }
  else{
    destroy_pattern(ptype->subpattern);
  }
}
void destroy_pattern_variable(pattern_variable_t* pvar){
  if(!pvar) return;
  // free(pvar->name);
  // pvar->name = NULL;
  destroy_pattern_type(&pvar->type);
}
void destroy_pattern_entry(pattern_entry_t* pentry){
  if(!pentry) return;

  switch(pentry->type){
    case PATTERN_IDENTIFIER:
      free(pentry->identifier);
      pentry->identifier = NULL;
      break;
    case PATTERN_VARIABLE:
      destroy_pattern_variable(&pentry->variable);
      break;
    case PATTERN_TYPE:
      destroy_pattern_type(&pentry->pattern_type);
      break;
    case PATTERN_EXP:
      destroy_expression(&pentry->exp);
      break;
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
    destroy_program(&fn_def->ir);
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
void copy_datum(datum_t* new, datum_t* datum){
  new->size = datum->size;
  new->data = malloc(new->size);
  for(int i = 0; i < new->size; i++){
    new->data[i] = datum->data[i];
  }
}
void copy_type_argument(type_argument_t* new, type_argument_t* arg){
  new->type = arg->type;
  switch(arg->type){
    case TYPEARG_SUBTYPE:
      new->subtype = malloc(sizeof(type_identifier_t));
      copy_type_identifier(new->subtype, arg->subtype);
      break;
    case TYPEARG_PARAM_EXP:
      new->exp = malloc(sizeof(expression_t));
      copy_expression(new->exp, arg->exp);
      break;
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
void copy_dimension_array(dimension_array_t* new, dimension_array_t* array){
  new->dimension_count = array->dimension_count;
  new->dimensions = malloc(array->dimension_count * sizeof(expression_t));
  for(int i = 0; i < new->dimension_count; i++){
    new->dimensions[i] = array->dimensions[i];
  }
}
void copy_type_identifier(type_identifier_t *new, type_identifier_t *type){
  new->type_id = type->type_id;
  copy_dimension_array(&new->dimensions, &type->dimensions);
  if(type->type_id == -1){
    new->size = type->size;
    return;
  }
  new->num_params = type->num_params;
  
  new->params = malloc(type->num_params * sizeof(type_argument_t));

  for(int i = 0; i < new->num_params; i++){
    copy_type_argument(new->params + i, type->params + i);
  }
}
void copy_type_entry(type_entry_t* new, type_entry_t* entry){
  new->is_vector = entry->is_vector;
  if(entry->is_vector){
    new->subvector.name = entry->subvector.name;
    new->subvector.is_enum = entry->subvector.is_enum;
    new->subvector.const_lvl = entry->subvector.component_count;
    new->subvector.component_count = entry->subvector.component_count;
    new->subvector.components = malloc(new->subvector.component_count * sizeof(type_entry_t));
    for(int i = 0; i < new->subvector.component_count; i++){
      copy_type_entry(new->subvector.components + i, entry->subvector.components + i);
    }
  }
  else{
    copy_variable_declaration(&new->base, &entry->base);
  }
}

void copy_type_declaration(type_declaration_t *new, type_declaration_t *typedec){
  new->is_static_size = typedec->is_static_size;
  new->size = typedec->size;
  new->is_is = typedec->is_is;
  new->match_pattern = malloc(sizeof(pattern_t));
  copy_pattern(new->match_pattern, typedec->match_pattern);
  copy_type_entry(&new->entry, &typedec->entry);
}
void copy_variable_declaration(variable_declaration_t *new, variable_declaration_t *vardec){
  new->constant_lvl = vardec->constant_lvl;
  new->var_name = vardec->var_name;
  copy_type_identifier(&new->type, &vardec->type);
  //strcpy(new->var_name, vardec->var_name);
}
void copy_pattern_value(pattern_value_t* new, pattern_value_t* pval){
  new->is_param = pval->is_param;
  if(new->is_param){
    new->param = malloc(sizeof(pattern_variable_t));
    copy_pattern_variable(new->param, pval->param);
  }
  else{
    new->base_value = malloc(sizeof(expression_t));
    copy_expression(new->base_value, pval->base_value);
  }
}

void copy_pattern_dimension_array(pattern_dimension_array_t* new, pattern_dimension_array_t* array){
  
  new->dimension_count = array->dimension_count;
  new->dimensions = malloc(array->dimension_count * sizeof(pattern_value_t));
  for(int i = 0; i < array->dimension_count; i++){
    copy_pattern_value(new->dimensions + i, array->dimensions + i);
  }
}
void copy_pattern_type(pattern_type_t* new, pattern_type_t* ptype){
  new->is_param = ptype->is_param;
  copy_pattern_dimension_array(&new->dimensions, &ptype->dimensions);
  
  if(new->is_param){
    copy_type_identifier(&new->param_type, &ptype->param_type);
  }
  else{
    new->base_type_id = ptype->base_type_id;
    new->subpattern = malloc(sizeof(pattern_t));
    copy_pattern(new->subpattern, ptype->subpattern);
  }
}

void copy_pattern_variable(pattern_variable_t* new, pattern_variable_t* pvar){
  new->constant_lvl = pvar->constant_lvl;
  copy_pattern_type(&new->type, &pvar->type);
  new->name = pvar->name;
}

void copy_pattern_entry(pattern_entry_t* new, pattern_entry_t* pattern){
  new->type = pattern->type;
  switch(new->type){
    case PATTERN_IDENTIFIER:
      new->identifier = malloc((1 + strlen(pattern->identifier)) * sizeof(char));
      strcpy(new->identifier, pattern->identifier);
      break;
    case PATTERN_VARIABLE:
      copy_pattern_variable(&new->variable, &pattern->variable);
      break;
    case PATTERN_TYPE:
      copy_pattern_type(&new->pattern_type, &pattern->pattern_type);
      break;
    case PATTERN_EXP:
      copy_expression(&new->exp, &pattern->exp);
      break;
  }
}

void copy_pattern(pattern_t *new, pattern_t *pattern){
  new->entry_count = pattern->entry_count;
  new->param_count = pattern->param_count;
  new->entries = malloc(new->entry_count * sizeof(pattern_entry_t));
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
    //yeah im not gonna implement this i dont think I ever use this function anyways
  }
  else{
    copy_block(&new->body, &fn_def->body);
  }
}



//expands the array of dimensions by one 
void add_dimension(dimension_array_t* array, expression_t exp){
  array->dimension_count++;
  array->dimensions = realloc(array->dimensions, array->dimension_count * sizeof(expression_t));
  array->dimensions[array->dimension_count - 1] = exp;
}

//expands the array of dimensions by one and returns a pointer to the newly created expression
pattern_value_t* add_pattern_dimension(pattern_dimension_array_t* array){
  array->dimension_count++;
  array->dimensions = realloc(array->dimensions, array->dimension_count * sizeof(pattern_value_t));
  return array->dimensions + array->dimension_count - 1;
}


void pattern_push_entry(pattern_t* pattern, pattern_entry_t entry){
  pattern->entry_count++;
  pattern->entries = realloc(pattern->entries, pattern->entry_count * sizeof(pattern_entry_t));
  pattern->entries[pattern->entry_count - 1] = entry;
}

bool compatible_const_levels(const_lvl_t slot, const_lvl_t input){
  if(slot == CL_CONST) return true;
  return input == slot;
}

