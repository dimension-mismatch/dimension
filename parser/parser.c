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



type_identifier_t attempt_read_type_id(token_array_t* tokens, int* index, int allow_dimensions, type_record_t* type_record){
  type_identifier_t type = typeid_newEmpty();

  int i = *index;
  int* ip = &i;
  token_t* current_token;
  
  if(allow_dimensions){
    while(!match_next_content(ip, &current_token, tokens, PROGRAM, "[")){
      if(match_current_type(current_token, NUMERIC)){
        typeid_pushDimension(&type, string_to_int(current_token->content));
      }
      else{
        typeid_destroy(&type);
        return type;
      }
      
      if(cond_peek_next_content(ip, &current_token, tokens, PROGRAM, "[")){
        break;
      }
      if(!match_next_content(ip, &current_token, tokens, SYMBOLIC, "*")){
        typeid_destroy(&type);
        return type;
      }
    }
  }
  else{
    if(!match_next_content(ip, &current_token, tokens, PROGRAM, "[")){
      typeid_destroy(&type);
      return type;
    }
  }
  if(!match_next_type(ip, &current_token, tokens, IDENTIFIER)){
    typeid_destroy(&type);
    error_msg("Type Identifier", "Invalid type name",current_token);
    return type;
  }
  type_declaration_t* ref = type_record_get_type(type_record, current_token->content);
  if(ref == NULL){
    error_msg("Composite Type Member", "Type is not defined", current_token);
    typeid_destroy(&type);
    return type;
  }
  type.type_number = type_record_get_type_id(type_record, current_token->content);
  type.bit_count = ref->bit_count;

  
  if(allow_only_next_content("]", PROGRAM,"Type Identifier", ip, tokens, &current_token)){
    typeid_destroy(&type);
    return type;
  }
  *index = i;
  return type;
}
void attempt_read_type_declaration(token_array_t* tokens, int* index, type_record_t* type_record){
  type_declaration_t dec = typedec_newEmpty();
  int i = *index;
  int* ip = &i;

  token_t* current_token;
  if(!match_next_content(ip, &current_token, tokens, IDENTIFIER, "type")) return;

  if(allow_only_next_content("[", PROGRAM,"Type Declaration", ip, tokens, &current_token)) return;

  if(!match_next_type(ip, &current_token, tokens, IDENTIFIER)){
    error_msg("Type Declaration", "could not find a name for this type", current_token);
    return;
  }
  
  char* type_name = current_token->content;

  if(allow_only_next_content("]", PROGRAM,"Type Declaration", ip, tokens, &current_token)) return;

  if(allow_only_next_content("is", IDENTIFIER,"Type Declaration", ip, tokens, &current_token)) return;
  

  if(match_next_content(ip, &current_token, tokens, IDENTIFIER, "oneof")){
    typedec_setName(&dec, ENUM_TYPE, type_name);
    if(allow_only_next_content("<", PROGRAM, "Enum Type Declaration", ip, tokens, &current_token)){typedec_destroy(&dec); return;}
    while(match_next_type(ip, &current_token, tokens, IDENTIFIER)){
      typedec_pushEnumOption(&dec, current_token->content, current_token->length);
    }
    if(allow_only_current_content(">", PROGRAM, "Enum Type Declaration", current_token)){ typedec_destroy(&dec); return;}
  }
  else{
    typedec_setName(&dec, COMP_TYPE, type_name);
    
    int single_type_only = !match_current_content(current_token,  PROGRAM, "<");
    if(single_type_only){
      i--;
    }
    while(1){
      char* name = NULL;
      int length = 0;
      if(cond_peek_next_type(ip, &current_token, tokens, IDENTIFIER)){
        name = current_token->content;
        length = current_token->length;
        if(allow_only_next_content(":", PROGRAM, "Composite Type Member",ip, tokens, &current_token)){typedec_destroy(&dec); return;}
      }
      type_identifier_t member;
      if((member = attempt_read_type_id(tokens, ip, 1, type_record)).type_number != -1){
        typedec_pushMember(&dec, &member, name, length);
      }
      else{
        break;
      }
      if(single_type_only) break;
    }

    if(!single_type_only && allow_only_next_content(">", PROGRAM, "Composite Type Declaration",ip, tokens, &current_token)){typedec_destroy(&dec); return;}
  }
  if(allow_only_next_content(";", PROGRAM, "Type Declaration", ip, tokens, &current_token)){typedec_destroy(&dec); return;}
  i--;
  print_type(&dec);
  type_record_push_type(type_record, dec);
  *index = i;
  return;
}

variable_t attempt_read_variable_declaration(token_array_t* tokens, int* index, type_record_t* type_record, variable_record_t* var_record){
  int i = *index;
  int* ip = &i;
  token_t* current_token;
  variable_t empty = {typeid_newEmpty(), NULL};

  if(!match_next_type(ip, &current_token, tokens, IDENTIFIER)){
    return empty;
  }
  char* name = current_token->content;
  
  if(!match_next_content(ip, &current_token, tokens, PROGRAM, ":")){
    return empty;
  }
  
  type_identifier_t type = attempt_read_type_id(tokens, ip, 1, type_record);
  if(type.type_number == -1){
    error_msg("Variable Declaration", "Could not find a [type] for this variable", current_token);
  }

  
  *index = i;

  variable_t var = variable_record_push_new(var_record, name, &type);
  return var;
}

void attempt_read_function_declaration(token_array_t* tokens, int* index, type_record_t* type_record, variable_record_t* var_record, function_record_t* fn_record){
  int i = *index;
  int* ip = &i;
  token_t* current_token;
  if(!match_next_content(ip, &current_token, tokens, IDENTIFIER, "fn")){
    return;
  }
  //if(allow_only_next_content("{", PROGRAM, "Function Declaration", ip, tokens, &current_token)) return;
  exp_array_t* matches = NULL;
  exp_array_t* root = NULL;

  while(/*!cond_peek_next_content(ip, &current_token, tokens, PROGRAM, "}")*/ 
    !(
      peek_next_content(ip, &current_token, tokens, IDENTIFIER, "makes") ||
      peek_next_content(ip, &current_token, tokens, IDENTIFIER, "does") ||
      peek_next_content(ip, &current_token, tokens, IDENTIFIER, "priority")
    )
    ){
    if(cond_peek_next_content(ip, &current_token, tokens, PROGRAM, "(")){
      if(cond_peek_next_content(ip, &current_token, tokens, PROGRAM, "(")){
        exp_array_push_expression(&root, &matches, exp_create_identifier("("));
      }
      while(!cond_peek_next_content(ip, &current_token, tokens, PROGRAM, ")")){
        variable_t param = attempt_read_variable_declaration(tokens, ip, type_record, var_record);
        if(param.name == NULL){
          error_msg("Function Parameter Declaration","Parameter Name Expected", current_token);
          exp_array_destroy(root);
          return;
        }
        expression_t* exp = exp_create_var_declaration(param.type, param.name);
        exp_array_push_expression(&root, &matches, exp);
      }

      if(cond_peek_next_content(ip, &current_token, tokens, PROGRAM, ")")){
        exp_array_push_expression(&root, &matches, exp_create_identifier(")"));
      }
    }
    else if(cond_peek_next_type(ip, &current_token, tokens, IDENTIFIER) || cond_peek_next_type(ip, &current_token, tokens, SYMBOLIC)){
      expression_t* exp = exp_create_identifier(current_token->content);
      exp_array_push_expression(&root, &matches, exp);
    }
    else{
      error_msg("Function Declaration", "Found Unexpected Token, Looking for parameters or identifiers", current_token);
      exp_array_destroy(root);
      return;
    }
  }
  printf("found function definition!\n");

  int has_body = 0;
  int has_return = 0;
  int has_priority = 0;

  int priority = 0;
  type_identifier_t ret_type;

  while(!peek_next_content(ip, &current_token, tokens, PROGRAM, ";")){
    if(cond_peek_next_content(ip, &current_token, tokens, IDENTIFIER, "does")){
      if(has_body){
        error_msg("Function Body", "Function can only have one function body", current_token);
        exp_array_destroy(root);
        return;
      }
      has_body = 1;
      if(allow_only_next_content("{", PROGRAM, "Function Body", ip, tokens, &current_token)){
        exp_array_destroy(root);
        return;
      }
      while(!match_next_content(ip, &current_token, tokens, PROGRAM, "}")){
        print_token(current_token);
        printf("\n");
      }
    }
    else if(cond_peek_next_content(ip, &current_token, tokens, IDENTIFIER, "makes")){
      if(has_return){
        error_msg("Function Return Type", "Function can only have one return type", current_token);
        exp_array_destroy(root);
        return;
      }
      has_return = 1;
      ret_type = attempt_read_type_id(tokens, ip, 1, type_record);
      if(ret_type.type_number == -1){
        exp_array_destroy(root);
        error_msg("Function Return Type", "Could not find a return [type] for this function", current_token);
        return;
      }
    }
    else if(cond_peek_next_content(ip, &current_token, tokens, IDENTIFIER, "priority")){
      if(has_priority){
        error_msg("Function Priority", "Function can only have one priority value", current_token);
        exp_array_destroy(root);
        return;
      }
      has_priority = 1;
      if(match_next_type(ip, &current_token, tokens, NUMERIC)){
        priority = string_to_int(current_token->content);
      }
      else{
        error_msg("Function Priority", "Priority must be an integer", current_token);
        exp_array_destroy(root);
        return;
      }
    }
    else{
      error_msg("Function Declaration", "Unexpected token. Looking for a \"makes\", \"does\", or \"priority\"", current_token);
      exp_array_destroy(root);
      return;
    }
  }
  fn_rec_push_definition(fn_record, root, &ret_type, priority);
  *index = i;
}

exp_array_t* attempt_isolate_expression(token_array_t* tokens, int* index, variable_record_t* var_record){
  exp_array_t* matches = NULL;
  exp_array_t* root = NULL;

  // if(!match_current_content(tokens->tokens + *index, PROGRAM, ";")){
  //   return matches;
  // }
  int i = *index;
  int* ip = &i;
  token_t* current_token;
  while(1){
    if(peek_next_content(ip, &current_token, tokens, PROGRAM, ";")){
      *index = i;
      return root;
    }
    else if(cond_peek_next_content(ip, &current_token, tokens, PROGRAM, ")")){
      exp_array_push_expression(&root, &matches, exp_create_grouping(0));
    }
    else if(cond_peek_next_content(ip, &current_token, tokens, PROGRAM, "(")){
      exp_array_push_expression(&root, &matches, exp_create_grouping(1));
    }
    else if(cond_peek_next_type(ip, &current_token, tokens, IDENTIFIER) || cond_peek_next_type(ip, &current_token, tokens, SYMBOLIC)){
      
      int* varid = variable_record_get_index(var_record, current_token->content);
      if(varid != NULL){
        variable_t* var = variable_record_get_by_index(var_record, *varid);
        exp_array_push_expression(&root, &matches, exp_create_var_read(var->type, *varid));
      }
      else{
        exp_array_push_expression(&root, &matches, exp_create_identifier(current_token->content));
      }
    }
    else{
      exp_array_destroy(root);
      return NULL;
    }
  }
}

void attempt_read_variable_assignment(token_array_t* tokens, int* index, type_record_t* type_record, variable_record_t* var_record, function_record_t* fn_record){
  int i = *index;
  int* ip = &i;
  token_t* current_token;
  int id = var_record->table.key_count;
  variable_t declare = attempt_read_variable_declaration(tokens, ip, type_record, var_record);
  int exists = declare.type.type_number == -1;
  if(exists){
    if(match_next_type(ip, &current_token, tokens, IDENTIFIER)){
      int* var_id = variable_record_get_index(var_record, current_token->content);
      if(var_id == NULL){
        return;
      }
      id = *var_id;
      declare = *variable_record_get_by_index(var_record, *var_id);
    }
    else{
      return;
    }
  }
  if(match_next_content(ip, &current_token, tokens, SYMBOLIC, "=")){
    exp_array_t* expc = attempt_isolate_expression(tokens, ip, var_record);
    if(expc != NULL){
      parse_expression(expc, fn_record, 0);
      expression_t* final = exp_create_var_write(declare.type, id, expc->expression);
      printf("\n\n");
      print_expression(final);
      printf("\n\n");
      *index = i;
      return;
    }
    else{
      error_msg("Expression", "Expession Expected", current_token);
      return;
    }
  }
}



void parse_tokens(token_array_t* tokens){
  type_record_t type_record = init_type_record();
  variable_record_t variable_record = variable_record_init();
  function_record_t function_record = fn_rec_init();

  type_declaration_t Integer = typedec_newEmpty();
  typedec_setName(&Integer, BASE_TYPE, "i");

  type_declaration_t Unsigned = typedec_newEmpty();
  typedec_setName(&Unsigned, BASE_TYPE, "u");

  type_declaration_t Float = typedec_newEmpty();
  typedec_setName(&Float, BASE_TYPE, "f");

  type_declaration_t Char = typedec_newEmpty();
  typedec_setName(&Char, BASE_TYPE, "c");

  type_record_push_type(&type_record, Integer);
  type_record_push_type(&type_record, Unsigned);
  type_record_push_type(&type_record, Float);
  type_record_push_type(&type_record, Char);


  for(int i = -1; i < tokens->token_count; i++){
    attempt_read_type_declaration(tokens, &i, &type_record);
    attempt_read_variable_assignment(tokens, &i, &type_record, &variable_record, &function_record);


    attempt_read_function_declaration(tokens, &i, &type_record, &variable_record, &function_record);
    exp_array_t* expc = attempt_isolate_expression(tokens, &i, &variable_record);
    if(expc != NULL){
      printf(GREEN BOLD "FOUND EXPRESSION: " RESET_COLOR);
      print_exp_array(expc);
        

      printf("\n");
      parse_expression(expc, &function_record, 0);
    }
  }
  print_fn_rec(&function_record);
  printf(YELLOW BOLD "VARIABLES: \n" RESET_COLOR);
  print_variable_record(&variable_record);
  
  
  destroy_type_record(&type_record);
  variable_record_destroy(&variable_record);
  fn_rec_destroy(&function_record);
}

