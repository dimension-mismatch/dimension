#include "parser.h"
#include "constructs.h"
#include "token_cursor.h"
#include "colors.h"
#include "expression_builder.h"
#include "construct_utils.h"

#include <stdlib.h>
#include <string.h>



bool parse_expression(token_cursor_t* base_tc, token_type_t end_type, expression_t* result);

//* 2
bool parse_type_identifier(token_cursor_t* base_tc, type_identifier_t* result){
  result->dimension_count = 0;
  result->dimensions = NULL;
  token_cursor_t tc = *base_tc;

  while(true){
    if(tc.tk.type == TK_TYPE && tc.tk.is_open){
      // we've reached the end of the dimensions, move on to the type itself
      break;
    }
    else if(tc.tk.type == TK_VECTOR){
      tc_inc(&tc);
      if(!parse_expression(&tc, TK_VECTOR, add_dimension(result))){
        return false;
      }
      tc_inc(&tc);
      continue;
    }
    else if(tc.tk.type == TK_IDENTIFIER){
      expression_t* new = add_dimension(result);
      
      //found a variable
    }
    else if(tc.tk.type == TK_NUMERIC){
      if(tc.tk.number_type == NUM_FLOAT || tc.tk.number_type == NUM_SCI_FLOAT){
        throw_error(tc.error_manager, 3, tc.index);
        return false;
      }
      expression_t* new = add_dimension(result);
      new->type = EXP_VALUE_LITERAL;
      new->value_literal.type = VAL_UNSIGNED;
      new->value_literal.u = atoi(tc.tk.content);
    }
    else{
      throw_error(tc.error_manager, 2, tc.index);
      return false;
    }
    tc_inc(&tc);

    if(tc_is_asterisk(&tc)){
      tc_inc(&tc);
      continue;
    }
    else if(tc.tk.type == TK_TYPE && tc.tk.is_open){
      //we've reached the last dimension, move on to the type itself
      break;
    }
    else{
      throw_error(tc.error_manager, 4, tc.index);
      return false;
    }
  }
  tc_inc(&tc);

  //read the contents of the [square brackets] to get the type expression
  expression_t contents;
  if(!parse_expression(&tc, TK_TYPE, &contents)){
    return false;
  }
  // if(contents.type != EXP_FUNCTION_CALL){
  //   throw_error(tc.error_manager, 2, tc.index);
  //   return false;
  // }
  result->num_params = 0;//contents.function_call.num_params;
  result->params = NULL;//contents.function_call.params;
  result->type_id = 67;
  
  tc_inc(&tc);

  printf(GREEN " (%d-dimensions)" RESET_COLOR, result->dimension_count);
  *base_tc = tc;
  return true;
}

//* 4
bool parse_vardec(token_cursor_t* base_tc, variable_declaration_t* result){
  token_cursor_t tc = *base_tc;

  if(tc.tk.type != TK_IDENTIFIER){
    return false;
  }
  char* name = tc.tk.content;
  tc_inc(&tc);
  if(tc.tk.type != TK_DECL){
    return false;
  }
  result->constant_lvl = tc.tk.decl_const_lvl;
  tc_inc(&tc);
  if(!parse_type_identifier(&tc, &result->type)){
    return false;
  }
  result->var_name = malloc((1 + strlen(name) * sizeof(char)));
  strcpy(result->var_name, name);


  tc_inc(&tc);
  *base_tc = tc;
  return true;

}

//* 5
bool parse_pattern_type(token_cursor_t* base_tc, pattern_type_t* result){
  result->dimension_count = 0;
  result->dimensions = NULL;
  result->param_count = 0;
  result->parameters = NULL;
  result->base_type_id = 69420;
  token_cursor_t tc = *base_tc;
  //read dimensions for this type
  while(true){
    //dimensions may be defined with an expression or declared as a pattern variable
    if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
      variable_declaration_t vardec;
      expression_t exp;
      tc_inc(&tc);
      //try reading a pattern variable
      if(parse_vardec(&tc, &vardec)){
        tc_inc(&tc);
        if(!tc_is_asterisk(&tc)){
          //if we found a variable declaration but no asterisk then this is a dimensionless pattern type
          //e.g. variable :: (t ::: [t])
          *base_tc = tc;
          return true;
        }
      }
      //if that fails, try reading an expression
      else if(parse_expression(&tc, TK_VECTOR, &exp)){

      }
      else{
        //not an expression or pattern variable, throw an error!!
        throw_error(tc.error_manager, 2, tc.index);
        return false;
      }
    }
    //the expression might also just be a single number or variable
    else if(tc.tk.type == TK_NUMERIC){

    }
    else if(tc.tk.type == TK_IDENTIFIER){

    }
    else if(!tc_is_asterisk(&tc)){
      break;
    }
    tc_inc(&tc);
  }

  
  
  if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
    tc_inc(&tc);
    variable_declaration_t vardec;
    parse_vardec(&tc, &vardec);
    //found a variable declaration
  }
  else if(tc.tk.type == TK_TYPE && tc.tk.is_open){
    tc_inc(&tc);
    expression_t exp;
    parse_expression(&tc, TK_TYPE, &exp);

    //found a type literal
  }
  else{
    //syntax error
    throw_error(tc.error_manager, 2, tc.index);
    return false;
  }
  
  *base_tc = tc;
  return true;
}

//* 6
bool parse_pattern_vardec(token_cursor_t* base_tc, pattern_variable_t* result){
  token_cursor_t tc = *base_tc;

  if(tc.tk.type != TK_IDENTIFIER){
    throw_error(tc.error_manager, 0, tc.index);
    return false;
  }
  char* name = tc.tk.content;
  tc_inc(&tc);
  if(tc.tk.type != TK_DECL){
    throw_error(tc.error_manager, 1, tc.index);
    return false;
  }
  result->constant_lvl = tc.tk.decl_const_lvl;
  
  tc_inc(&tc);
  if(!parse_pattern_type(&tc, &result->type)){
    return false;
  }
  // result->name = malloc((1 + strlen(name) * sizeof(char)));
  // strcpy(result->name, name);


  tc_inc(&tc);
  *base_tc = tc;
  return true;
}

//* 7
bool parse_pattern(token_cursor_t* base_tc, token_type_t end_type, pattern_t* result){
  token_cursor_t tc = *base_tc;
  result->entries = NULL;
  result->entry_count = 0;
  while(true){
    

    if(tc.tk.type == TK_IDENTIFIER){
      pattern_entry_t new_entry = {.is_identifier = true, .identifier = malloc((1 + strlen(tc.tk.content)) * sizeof(char))};
      strcpy(new_entry.identifier, tc.tk.content);

      printf(CYAN);
      print_token(&tc.tk);
      printf(RESET_COLOR);
      pattern_push_entry(result, new_entry);
    }
    else if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
      pattern_variable_t variable;
      while(true){
        tc_inc(&tc);
        pattern_variable_t variable;
        if(!parse_pattern_vardec(&tc, &variable)){
          return false;
        }
        pattern_entry_t new_entry = {.is_identifier = false, .variable = variable};
        pattern_push_entry(result, new_entry);
        if(tc.tk.type == TK_FORCE_EXP_END){

        }
        else if(tc.tk.type == TK_VECTOR && !tc.tk.is_open){
          break;
        }
        else{
          throw_error(tc.error_manager, 2, tc.index);
          return false;
        }
      }
    }
    else if(tc.tk.type == end_type){
      *base_tc = tc;
      return true;
    }
    else{
      return false;
    }
    tc_inc(&tc);
  }
}

//* 1
bool parse_expression(token_cursor_t* base_tc, token_type_t end_type, expression_t* result){
  token_cursor_t tc = *base_tc;

  expression_array_t root;
  expression_array_t* prev = &root;
 
  while(true){
    expression_t new;
    if(tc.tk.type == TK_IDENTIFIER || tc.tk.type == TK_NUMERIC || tc.tk.type == TK_CHAR || tc.tk.type == TK_STRING || tc.tk.type == TK_FORCE_EXP_END){
      expression_t exp = {.type = EXP_RAW_TOKEN, .raw_token = tc.tk};
      new = exp;
    }
    else if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
      tc_inc(&tc);
      if(!parse_expression(&tc, TK_VECTOR, &new)){
        return false;
      }
    }
    else if(tc.tk.type == TK_TYPE && tc.tk.is_open){
      tc_inc(&tc);
      if(!parse_expression(&tc, TK_TYPE, &new)){
        return false;
      }
    }
    else if(tc.tk.type == end_type){
      printf("\n");
      printf(RED);
      print_expression_array(&root);
      printf(RESET_COLOR);
      build_expression(tc.type_trie, &root);
      result->type = EXP_VALUE_LITERAL;
      result->value_literal.type = VAL_INT;
      result->value_literal.i = 67;
      *base_tc = tc;
      return true;
    }
    else{
      //unexpected token (Throw an error?)
      throw_error(tc.error_manager, 2, tc.index);
      return false;
    }
    tc_inc(&tc);
    expression_array_t* next = malloc(sizeof(expression_array_t));
    next->prev = prev;
    next->next = NULL;
    next->exp = new;
    prev->next = next;
    prev = next;
  };
  
}

//* 3
bool parse_type_declaration(token_cursor_t* base_tc, type_declaration_t* result){
  token_cursor_t tc = *base_tc;
  if(tc.tk.type != TK_KEYWORD || tc.tk.keyword_id != 0){
    return false;
  }
  tc_inc(&tc);
  if(tc.tk.type != TK_TYPE || !tc.tk.is_open){
    return false;
  }
  tc_inc(&tc);
  result->match_pattern = malloc(sizeof(pattern_t));
  if(!parse_pattern(&tc, TK_TYPE, result->match_pattern)){
    return false;
  }

  tc_inc(&tc);
  if(tc.tk.type == TK_KEYWORD && tc.tk.keyword_id == 9){
    tc_inc(&tc);
    if(tc.tk.type == TK_NUMERIC){
      if(tc.tk.number_type == NUM_FLOAT || tc.tk.number_type == NUM_SCI_FLOAT){
        return false; //TODO: Throw error (non-integer size value)
      }
      result->is_builtin = true;
      result->byte_count = atoi(tc.tk.content);

      return true;
    }
    return false;
  }
  result->is_builtin = false;

  if(tc.tk.type != TK_KEYWORD || !(tc.tk.keyword_id == 1 || tc.tk.keyword_id == 2)){
    return false;
  }
  result->is_is = (tc.tk.keyword_id == 1);
  tc_inc(&tc);

  result->is_enum = (tc.tk.type == TK_KEYWORD && tc.tk.keyword_id == 3);
  if(result->is_enum){
    tc_inc(&tc);
  }

  if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
    while(true){
      tc_inc(&tc);
      variable_declaration_t vardec;
      type_identifier_t typeid;
      if(parse_vardec(&tc, &vardec)){

      }
      else if(!result->is_enum && parse_type_identifier(&tc, &typeid)){
        vardec.var_name = NULL;
        vardec.constant_lvl = 0;
        vardec.type = typeid;
      }
      else if(tc.tk.type == TK_IDENTIFIER){
        vardec.var_name = malloc((1 + strlen(tc.tk.content)) * sizeof(char));
        strcpy(vardec.var_name, tc.tk.content);
      }
      else if(tc.tk.type == TK_FORCE_EXP_END){
        continue;
      }
      else if(tc.tk.type == TK_VECTOR){
        //todo: allow for nesting type declarations here
        *base_tc = tc;
        return true;
      }
      else{
        return false;
      }
    }
  }

  return false;
}

void parse_tokens(token_cursor_t* tc){
  do{
    variable_declaration_t vardec;
    if(parse_vardec(tc, &vardec)){
      print_variable_declaration(&vardec);
    }

    type_declaration_t typedec ;
    if(parse_type_declaration(tc, &typedec)){
      printf("\n");
      print_type_declaration(&typedec);
      type_declaration_t* typedecptr = malloc(sizeof(type_declaration_t));
      *typedecptr = typedec;
      pattern_trie_push_type(tc->type_trie, typedecptr);
    }

    // type_identifier_t typeid;
    // expression_t exp;
    // if(parse_type_identifier(tc, &typeid)){
    //   printf(GREEN BOLD " YES" RESET_COLOR);
    // }
    // if(parse_expression(tc, TK_ENDLINE, &exp)){
    //   printf(GREEN BOLD " EXP" RESET_COLOR);
    // }
  }while(tc_inc(tc));
}