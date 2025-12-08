#include "tokenizer.h"
#include "parser.h"
#include "string_utils.h"
#include "type_utils/type_utils.h"
#include "hash_table/type_record.h"
#include "type_utils/parse_tools.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash_table/variable_record.h"
#include "hash_table/function_record.h"
#include "colors.h"
#include "expression_builder.h"
#include "error_handling/error_manager.h"

expression_t* attempt_read_block(int* index, parse_manager_t* manager);

type_identifier_t attempt_read_type_id(int* index, parse_manager_t* manager, int allow_dimensions){
  type_identifier_t type = typeid_newEmpty();

  int i = *index;
  int* ip = &i;
  token_t* current_token;
  int has_prefix = 0;
  
  if(allow_dimensions){
    while(!match_next_content(ip, &current_token, manager, PROGRAM, "[")){
      if(match_current_type(current_token, NUMERIC)){
        typeid_pushDimension(&type, string_to_int(current_token->content));
        has_prefix = 1;
      }
      else{
        if(has_prefix) throw_error(manager, 19,i );
        typeid_destroy(&type);
        return type;
      }
      
      if(cond_peek_next_content(ip, &current_token, manager, PROGRAM, "[")){
        break;
      }
      if(!match_next_content(ip, &current_token, manager, SYMBOLIC, "*")){
        throw_error(manager, 20,i );
        typeid_destroy(&type);
        return type;
      }
    }
  }
  else{
    if(!match_next_content(ip, &current_token, manager, PROGRAM, "[")){
      return type;
    }
  }
  if(!match_next_type(ip, &current_token, manager, IDENTIFIER)){
    typeid_destroy(&type);
    throw_error(manager, 0, i);
    return type;
  }
  type_declaration_t* ref = type_record_get_type(manager->type_rec, current_token->content);
  if(ref == NULL){
    throw_error(manager, 1, i);
    typeid_destroy(&type);
    return type;
  }
  type.type_number = type_record_get_type_number(manager->type_rec, current_token->content);
  type.bit_count = ref->bit_count;

  
  if(!match_next_content(ip, &current_token, manager, PROGRAM, "]")){
    throw_error(manager, 2, i);
    typeid_destroy(&type);
    return type;
  }
  *index = i;
  return type;
}
void attempt_read_type_declaration(int* index, parse_manager_t* manager){
  type_declaration_t dec = typedec_newEmpty();
  int i = *index;
  int* ip = &i;

  token_t* current_token;
  if(!match_next_content(ip, &current_token, manager, IDENTIFIER, "type")) return;

  if(!match_next_content(ip, &current_token, manager, PROGRAM, "[")){
    throw_error(manager, 3, i);
    return;
  }
  if(!match_next_type(ip, &current_token, manager, IDENTIFIER)){
    throw_error(manager, 0, i);
    return;
  }
  
  char* type_name = current_token->content;

  if(!match_next_content(ip, &current_token, manager, PROGRAM, "]")){
    throw_error(manager, 2, i);
    return;
  }

  if(!match_next_content(ip, &current_token, manager, IDENTIFIER, "is")){
    throw_error(manager, 4, i);
    return;
  }
  

  if(match_next_content(ip, &current_token, manager, IDENTIFIER, "oneof")){
    typedec_setName(&dec, ENUM_TYPE, type_name);
    if(!match_next_content(ip, &current_token, manager, PROGRAM, "(")){
      throw_error(manager, 5, i);
      typedec_destroy(&dec);
      return;
    }
    while(cond_peek_next_type(ip, &current_token, manager, IDENTIFIER) || cond_peek_next_type(ip, &current_token, manager, NUMERIC)){
      typedec_pushEnumOption(&dec, current_token->content, current_token->length);
    }
    if(!match_next_content(ip, &current_token, manager, PROGRAM, ")")){
      throw_error(manager, 6, i);
      typedec_destroy(&dec);
      return;
    }
  }
  else{
    typedec_setName(&dec, COMP_TYPE, type_name);
    
    int single_type_only = !match_current_content(current_token,  PROGRAM, "(");
    if(single_type_only){
      i--;
    }
    while(!cond_peek_next_content(ip, &current_token, manager, PROGRAM, ")")){
      char* name = NULL;
      int length = 0;
      if(cond_peek_next_type(ip, &current_token, manager, IDENTIFIER)){
        name = current_token->content;
        length = current_token->length;
        if(!match_next_content(ip, &current_token, manager, PROGRAM, ":")){
          throw_error(manager, 7, i);
          typedec_destroy(&dec);
          return;
        }
      }
      type_identifier_t member;
      if((member = attempt_read_type_id(ip, manager, 1)).type_number != -1){
        typedec_pushMember(&dec, &member, name, length);
      }
      else{
        //No need to throw an error here, since attempt_read_type_id will have already thrown one.
        //This way, we avoid logging two errors for the same issue.
        typedec_destroy(&dec);
        return;
      }
      if(single_type_only) break;
    }
  }
  if(!match_next_content(ip, &current_token, manager, PROGRAM, ";")){
      throw_error(manager, 8, i);
      typedec_destroy(&dec);
      return;
    }
  i--;
  type_record_push_type(manager->type_rec, dec);
  if(dec.type_type == ENUM_TYPE){
    type_identifier_t enum_type_id = typeid_newEmpty();
    enum_type_id.type_number = manager->type_rec->table.key_count - 1;
    enum_type_id.bit_count = manager->type_rec->array[enum_type_id.type_number]->bit_count;

    for(int i = 0; i < dec.enumeration.option_count; i++){
      type_identifier_t local_copy = typeid_copy(&enum_type_id);
      //variable_record_push_enum(manager->var_rec, dec.enumeration.options[i], &local_copy);
    }
  }
  *index = i;
  return;
}

variable_t* attempt_read_variable_declaration(int* index, parse_manager_t* manager, int is_parameter){
  int i = *index;
  int* ip = &i;
  token_t* current_token;

  if(!match_next_type(ip, &current_token, manager, IDENTIFIER)){
    return NULL;
  }
  char* name = current_token->content;
  
  if(!match_next_content(ip, &current_token, manager, PROGRAM, ":")){
    return NULL;
  }
  
  type_identifier_t type = attempt_read_type_id(ip, manager, 1);
  if(type.type_number == -1){
    return NULL;
  }

  
  *index = i;

  variable_t var = is_parameter ? 
    variable_record_push_param(manager->var_rec, name, &type) : 
    variable_record_push_new(manager->var_rec, name, &type);
  
  return variable_record_get_newest(manager->var_rec);
}

void attempt_read_function_declaration(int* index, parse_manager_t* manager){
  int i = *index;
  int* ip = &i;
  token_t* current_token;
  if(!match_next_content(ip, &current_token, manager, IDENTIFIER, "fn")){
    return;
  }
  //if(allow_only_next_content("{", PROGRAM, "Function Declaration", ip, tokens, &current_token)) return;
  exp_array_t* matches = NULL;
  exp_array_t* root = NULL;
  variable_record_scope_in(manager->var_rec);
  while(!(peek_next_content(ip, &current_token, manager, IDENTIFIER, "makes") ||
          peek_next_content(ip, &current_token, manager, IDENTIFIER, "does") ||
          peek_next_content(ip, &current_token, manager, IDENTIFIER, "priority"))){
    if(cond_peek_next_content(ip, &current_token, manager, PROGRAM, "(")){
      while(!cond_peek_next_content(ip, &current_token, manager, PROGRAM, ")")){
        variable_t* param = attempt_read_variable_declaration(ip, manager, 1);
        if(param == NULL){
          throw_error(manager, 9, i);
          exp_array_destroy(root);
          variable_record_scope_out(manager->var_rec);
          return;
        }
        expression_t* exp = exp_create_var_declaration(param->type, param->name, i);
        exp_array_push_expression(&root, &matches, exp);
      }
    }
    else if(cond_peek_next_type(ip, &current_token, manager, IDENTIFIER) || cond_peek_next_type(ip, &current_token, manager, SYMBOLIC)){
      expression_t* exp = exp_create_identifier(current_token->content, i);
      exp_array_push_expression(&root, &matches, exp);
    }
    else{
      throw_error(manager, 10, i);
      exp_array_destroy(root);
      variable_record_scope_out(manager->var_rec);
      return;
    }
  }
  if(root == NULL){
    throw_error(manager, 21, i);
  }
  
  int has_body = 0;
  int has_return = 0;
  int has_priority = 0;

  int priority = 0;
  type_identifier_t ret_type = typeid_newEmpty();

  char* assembly = NULL;
  expression_t* dimension = NULL;

  while(!peek_next_content(ip, &current_token, manager, PROGRAM, ";")){
    if(cond_peek_next_content(ip, &current_token, manager, IDENTIFIER, "does")){
      if(has_body){
        throw_error(manager, 11, i);
        exp_array_destroy(root);
        variable_record_scope_out(manager->var_rec);
        return;
      }
      has_body = 1;
      if(!match_next_content(ip, &current_token, manager, PROGRAM, "{")){
        throw_error(manager, 12, i);
        exp_array_destroy(root);
        variable_record_scope_out(manager->var_rec);
        return;
      }
      if(cond_peek_next_type(ip, &current_token, manager, ASSEMBLY)){
        assembly = current_token->content;
        if(!match_next_content(ip, &current_token, manager, PROGRAM, "}")){
          throw_error(manager, 13, i);
          exp_array_destroy(root);
          variable_record_scope_out(manager->var_rec);
          return;
        }
      }
      else{
        dimension = attempt_read_block(ip, manager);
        print_expression(dimension);
      }
    }
    else if(cond_peek_next_content(ip, &current_token, manager, IDENTIFIER, "makes")){
      if(has_return){
        throw_error(manager, 14, i);
        exp_array_destroy(root);
        variable_record_scope_out(manager->var_rec);
        return;
      }
      has_return = 1;
      ret_type = attempt_read_type_id(ip, manager, 1);
      if(ret_type.type_number == -1){
        exp_array_destroy(root);
        variable_record_scope_out(manager->var_rec);
        return;
      }
    }
    else if(cond_peek_next_content(ip, &current_token, manager, IDENTIFIER, "priority")){
      if(has_priority){
        throw_error(manager, 15, i);
        exp_array_destroy(root);
        variable_record_scope_out(manager->var_rec);
        return;
      }
      has_priority = 1;
      if(match_next_type(ip, &current_token, manager, NUMERIC)){
        priority = string_to_int(current_token->content);
      }
      else{
        throw_error(manager, 16, i);
        exp_array_destroy(root);
        variable_record_scope_out(manager->var_rec);
        return;
      }
    }
    else{
      throw_error(manager, 17, i);
      exp_array_destroy(root);
      variable_record_scope_out(manager->var_rec);
      return;
    }
  }
  if(!has_body){
    throw_error(manager, 23, i);
  }
  fn_rec_push_definition(manager->fn_rec, root, &ret_type, priority, assembly, dimension);
  *index = i;
  variable_record_scope_out(manager->var_rec);
}

exp_array_t* attempt_isolate_expression(int* index, parse_manager_t* manager){
  exp_array_t* matches = NULL;
  exp_array_t* root = NULL;

  // if(!match_current_content(tokens->tokens + *index, PROGRAM, ";")){
  //   return matches;
  // }
  int i = *index;
  int* ip = &i;
  token_t* current_token;
  while(1){
    if(peek_next_content(ip, &current_token, manager, PROGRAM, ";")){
      *index = i;
      return root;
    }
    else if(cond_peek_next_content(ip, &current_token, manager, PROGRAM, ")")){
      exp_array_push_expression(&root, &matches, exp_create_grouping(0, i));
    }
    else if(cond_peek_next_content(ip, &current_token, manager, PROGRAM, "(")){
      exp_array_push_expression(&root, &matches, exp_create_grouping(1, i));
    }
    else if(cond_peek_next_type(ip, &current_token, manager, IDENTIFIER) || cond_peek_next_type(ip, &current_token, manager, SYMBOLIC)){
      exp_array_push_expression(&root, &matches, exp_create_identifier(current_token->content, i));
    }
    else if(cond_peek_next_type(ip, &current_token, manager, NUMERIC)){
      type_identifier_t int_type = type_record_get_type_id(manager->type_rec, "i");
      exp_array_push_expression(&root, &matches, exp_create_numeric_literal(string_to_int(current_token->content), int_type, i));
      typeid_destroy(&int_type);
    }
    else{
      exp_array_destroy(root);
      return NULL;
    }
  }
}
expression_t* attempt_read_return_statement(int* index, parse_manager_t* manager){
  int i = *index;
  int* ip = &i;
  token_t* current_token;
  if(!match_next_content(ip, &current_token, manager, IDENTIFIER, "return")){
    return NULL;
  }
  exp_array_t* expc = attempt_isolate_expression(ip, manager);
    if(expc != NULL){
      parse_expression(&expc, manager, 0);
      expression_t* final = exp_create_return(0, expc->expression, *index);
      *index = i;
      return final;
    }
    else{
      expression_t* final = exp_create_return(0, NULL, *index);
      return final;
    }
}

expression_t* attempt_read_variable_assignment(int* index, parse_manager_t* manager){
  int i = *index;
  int* ip = &i;
  token_t* current_token;
  int id = manager->var_rec->var_count;
  
  variable_t* declare = attempt_read_variable_declaration(ip, manager, 0);
  if(declare == NULL){
    if(match_next_type(ip, &current_token, manager, IDENTIFIER)){
      declare = variable_record_get(manager->var_rec, current_token->content);
      if(declare == NULL){
        return NULL;
      }
      id = declare->var_id;
    }
    else{
      return NULL;
    }
  }
  if(match_next_content(ip, &current_token, manager, SYMBOLIC, "=")){
    exp_array_t* expc = attempt_isolate_expression(ip, manager);
    if(expc != NULL){
      parse_expression(&expc, manager, 0);
      expression_t* final = exp_create_var_write(declare->type, declare->local_byte_offset, expc->expression, i);
      *index = i;
      return final;
    }
    else{
      throw_error(manager, 18, i);
      return NULL;
    }
  }
  return NULL;
}

expression_t* attempt_read_block(int* index, parse_manager_t* manager){
  int i = *index;
  int* ip = &i;
  token_t* current_token;

  expression_t* program = exp_create_block();

  while(!peek_next_type(ip, &current_token, manager, NONE)){
    if(cond_peek_next_content(ip, &current_token, manager, PROGRAM, "}")){
      break;
    }
    attempt_read_type_declaration(&i, manager);

    exp_block_push_line(program, attempt_read_variable_assignment(&i, manager));
    exp_block_push_line(program, attempt_read_return_statement(&i, manager));

    attempt_read_function_declaration(&i, manager);
    exp_array_t* expc = attempt_isolate_expression(&i, manager);
    if(expc != NULL){
      // printf(GREEN BOLD "FOUND EXPRESSION: " RESET_COLOR);
      // print_exp_array(expc);
      // printf("\n");
      parse_expression(&expc, manager, 0);
      exp_block_push_line(program, expc->expression);
    }
    i++;
  }
  *index = i;
  program->block.stack_depth = manager->var_rec->scopes[manager->var_rec->scope_depth - 1].local_byte_offset;
  return program;
}


expression_t* parse_tokens(parse_manager_t* manager){
  type_record_t* type_record = manager->type_rec; 

  type_declaration_t Integer = typedec_newEmpty();
  typedec_setName(&Integer, BASE_TYPE, "i");
  Integer.bit_count = 64;

  type_declaration_t Unsigned = typedec_newEmpty();
  typedec_setName(&Unsigned, BASE_TYPE, "u");

  Unsigned.bit_count = 64;

  type_declaration_t Float = typedec_newEmpty();
  typedec_setName(&Float, BASE_TYPE, "f");

  Float.bit_count = 64;

  type_declaration_t Char = typedec_newEmpty();
  typedec_setName(&Char, BASE_TYPE, "c");

  Char.bit_count = 8;

  type_record_push_type(type_record, Integer);
  type_record_push_type(type_record, Unsigned);
  type_record_push_type(type_record, Float);
  type_record_push_type(type_record, Char);

  int i = -1;
  expression_t* program = attempt_read_block(&i, manager);
 
  // print_fn_rec(manager->fn_rec);
  // printf(YELLOW BOLD "VARIABLES: \n" RESET_COLOR);
  // print_variable_record(manager->var_rec);
  
  return program;
}

