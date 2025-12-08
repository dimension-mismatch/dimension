#include "validator.h"

void validate_expression(expression_t* exp, parse_manager_t* manager, type_identifier_t* return_type);



void validate_vector_literal(expression_t* exp, parse_manager_t* manager){
  serialized_type_array_t component_array = {0, NULL};
  serialized_type_array_t return_array = {0, NULL};
  type_identifier_t empty = typeid_newEmpty();
  for(int i = 0; i < exp->vector_literal.component_count; i++){
    expression_t* component = exp->vector_literal.components[i];
    validate_expression(component, manager, &empty);
    serialize_type(&component->return_type, &component_array);
  }
  serialize_type(&exp->return_type, &return_array);
  if(!serial_type_array_compare(&component_array, &return_array)){
    throw_error(manager, 24, exp->token_start);
  }
}

void validate_assignment(expression_t* exp, parse_manager_t* manager){
  type_identifier_t empty = typeid_newEmpty();
  validate_expression(exp->write.value[0], manager, &empty);
  if(!typeid_compare(&exp->return_type, &exp->write.value[0]->return_type)){
    throw_error(manager, 24, exp->write.value[0]->token_start);
    err_type_arg(manager, &exp->return_type);
    err_type_arg(manager, &exp->write.value[0]->return_type);
  }
}

void validate_block(expression_t* exp, parse_manager_t* manager, type_identifier_t* return_type){
  for(int i = 0; i < exp->block.arg_c; i++){
    validate_expression(exp->block.arg_v[i], manager, return_type);
  }
}

void validate_function_call(expression_t* exp, parse_manager_t* manager){
  type_identifier_t empty = typeid_newEmpty();
  for(int i = 0; i < exp->function_call.arg_c; i++){
    validate_expression(exp->function_call.arg_v[i], manager, &empty);
  }
}

void validate_return(expression_t* exp, parse_manager_t* manager, type_identifier_t* return_type){
  if(return_type->type_number == -1){
    throw_error(manager, 26, exp->token_start);
  }
  else if(!typeid_compare(&exp->return_type, return_type)){
    throw_error(manager, 25, exp->token_start);
  }
}

void validate_expression(expression_t* exp, parse_manager_t* manager, type_identifier_t* return_type){
  if(exp == NULL){
    throw_error(manager, 27, 0);
  }
  switch(exp->type){
    case EXP_BLOCK:
      validate_block(exp, manager, return_type);
      break;
    case EXP_CALL_FN:
      validate_function_call(exp, manager);
      break;
    case EXP_IDENTIFIER:
      throw_error(manager, 22, exp->token_start);
      break;
    case EXP_ERROR:
    case EXP_GROUPING:
    case EXP_NONE:
      throw_error(manager, 22, exp->token_start);
      break;
    case EXP_RETURN:
      validate_return(exp, manager, return_type);
      break;
    case EXP_WRITE_VAR:
      validate_assignment(exp, manager);
      break;
    case EXP_VECTOR_LITERAL:
      validate_vector_literal(exp, manager);
      break;
    case EXP_DECLARE_VAR:
    case EXP_READ_VAR:
    case EXP_SINGLE_LITERAL:
      break;
  }
}

void validate_program(expression_t* program, parse_manager_t* manager){
  type_identifier_t empty = typeid_newEmpty();
  validate_expression(program, manager, &empty);
  for(int i = 0; i < manager->fn_rec->def_count; i++){
    struct function_def def = manager->fn_rec->definitions[i];
    if(def.impl_type == FN_IMPL_DMSN){
      validate_expression(def.dmsn, manager, &def.return_type);
    }
  }
}