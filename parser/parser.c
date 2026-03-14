#include "parser.h"
#include "constructs.h"
#include "token_cursor.h"
#include "colors.h"
#include "expression_builder.h"
#include "construct_utils.h"

#include <stdlib.h>
#include <string.h>



bool parse_expression(token_cursor_t* base_tc, token_type_t end_type, expression_t* result);
bool parse_pattern(token_cursor_t* base_tc, token_type_t end_type, pattern_t* result);
//* 2
bool parse_type_identifier(token_cursor_t* base_tc, type_identifier_t* result){
  token_cursor_t tc = *base_tc;
  dimension_array_t dimensions = {.dimension_count = 0, .dimensions = NULL};
  while(true){
    if(tc.tk.type == TK_TYPE && tc.tk.is_open){
      // we've reached the end of the dimensions, move on to the type itself
      break;
    }
    else if(tc.tk.type == TK_VECTOR){
      tc_inc(&tc);
      if(!parse_expression(&tc, TK_VECTOR, add_dimension(&dimensions))){
        destroy_dimension_array(&dimensions);
        return false;
      }
      tc_inc(&tc);
      continue;
    }
    else if(tc.tk.type == TK_IDENTIFIER){
      expression_t* new = add_dimension(&dimensions);
      
      //found a variable
    }
    else if(tc.tk.type == TK_NUMERIC){
      if(tc.tk.number_type == NUM_FLOAT || tc.tk.number_type == NUM_SCI_FLOAT){
        destroy_dimension_array(&dimensions);
        throw_error(tc.error_manager, 3, tc.index);
        return false;
      }
      expression_t* new = add_dimension(&dimensions);
      new->type = EXP_VALUE_LITERAL;
      new->value_literal.type = VAL_UNSIGNED;
      new->value_literal.u = atoi(tc.tk.content);
    }
    else{
      destroy_dimension_array(&dimensions);
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
      destroy_dimension_array(&dimensions);
      return false;
    }
  }
  tc_inc(&tc);
  result->dimensions = dimensions;
  //read the contents of the [square brackets] to get the type expression
  expression_t contents;
  if(!parse_expression(&tc, TK_TYPE, &contents)){
    return false;
  }
  if(contents.type != EXP_TYPE_LITERAL){
    throw_error(tc.error_manager, 2, tc.index);
    destroy_expression(&contents);
    destroy_dimension_array(&result->dimensions);
    return false;
  }
  result->type_id = contents.type_literal->type_id;
  result->num_params = contents.type_literal->num_params;
  result->params = contents.type_literal->params;
  tc_inc(&tc);

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

  *base_tc = tc;
  return true;

}

//* 5
bool parse_pattern_type(token_cursor_t* base_tc, pattern_type_t* result){
  result->dimensions.dimension_count = 0;
  result->dimensions.dimensions = NULL;
  result->param_count = 0;
  result->parameters = NULL;
  result->base_type_id = 0;
  result->is_param = true;
  token_cursor_t tc = *base_tc;
  pattern_dimension_array_t dimensions = {.dimension_count = 0, .dimensions = NULL};
  //read dimensions for this type
  while(true){
    
    //dimensions may be defined with an expression or declared as a pattern variable
    if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
      variable_declaration_t vardec;
      expression_t* exp = malloc(sizeof(expression_t));
      tc_inc(&tc);
      
      
      //try reading a pattern variable
      if(parse_vardec(&tc, &vardec)){
        printf(BLUE BOLD);
        print_token(&tc.tk);
        printf(RESET_COLOR);
        tc_inc(&tc);
        if(!tc_is_asterisk(&tc)){
          //if we found a variable declaration but no asterisk then this is a dimensionless pattern type
          //e.g. variable :: (t ::: [t])
          result->is_param = true;
          *base_tc = tc;
          return true;
        }
        if(vardec.type.dimensions.dimension_count > 0){
          throw_error(tc.error_manager, 10, tc.index);
          return false;
        }
        if(vardec.type.num_params > 0){
          throw_error(tc.error_manager, 10, tc.index); 
          return false;
        }
        if(vardec.constant_lvl != 2){
          throw_error(tc.error_manager, 11, tc.index);
          return false;
        }
        pattern_value_t* value = add_pattern_dimension(&dimensions);
        value->is_param = true;
        pattern_type_t ptype = {.is_param = true, .dimensions = {.dimension_count = 0, .dimensions = NULL}, .param_count = 0, .parameters = NULL};
        ptype.base_type_id = vardec.type.type_id;
        
        value->param = malloc(sizeof(pattern_type_t));
        *value->param = ptype;
        
      }
      //if that fails, try reading an expression
      else if(parse_expression(&tc, TK_VECTOR, exp)){
        pattern_value_t* value = add_pattern_dimension(&dimensions);
        value->is_param = false;
        value->base_value = exp;
      }
      else{
        //not an expression or pattern variable, throw an error!!
        throw_error(tc.error_manager, 2, tc.index);
        return false;
      }
    }
    //the expression might also just be a single number or variable
    else if(tc.tk.type == TK_NUMERIC){
      if(tc.tk.number_type == NUM_FLOAT || tc.tk.number_type == NUM_SCI_FLOAT){
        destroy_pattern_dimension_array(&dimensions);
        throw_error(tc.error_manager, 3, tc.index);
        return false;
      }
      pattern_value_t* new = add_pattern_dimension(&dimensions);
      new->is_param = false;
      new->base_value = malloc(sizeof(expression_t));
      expression_t* new_exp = new->base_value;
      new_exp->type = EXP_VALUE_LITERAL;
      new_exp->value_literal.type = VAL_UNSIGNED;
      new_exp->value_literal.u = atoi(tc.tk.content);
    }
    else if(tc.tk.type == TK_IDENTIFIER){

    }
    else if(!tc_is_asterisk(&tc)){
      break;
    }
    tc_inc(&tc);
  }

  result->dimensions = dimensions;
  if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
    tc_inc(&tc);
    variable_declaration_t vardec;
    if(!parse_vardec(&tc, &vardec)){
      return false;
    }
    result->param_type = vardec.type;
    result->is_param = true;
    //found a variable declaration
  }
  else if(tc.tk.type == TK_TYPE && tc.tk.is_open){
    tc_inc(&tc);
    pattern_t pattern;
    if(!parse_pattern(&tc, TK_TYPE, &pattern)){
      return false;
    }
    trie_match_result_t* match = pattern_trie_validate_pattern(tc.type_trie, &pattern);
    if(!match){
      return false;
    }


    result->is_param = false;
    result->base_type_id = match->index;
    result->param_count = 0;//pattern_count_parameters(&pattern);
    result->parameters = malloc(result->param_count * sizeof(pattern_value_t));
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
      pattern_entry_t new_entry = {.type = PATTERN_IDENTIFIER, .identifier = malloc((1 + strlen(tc.tk.content)) * sizeof(char))};
      strcpy(new_entry.identifier, tc.tk.content);
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
        pattern_entry_t new_entry = {.type = PATTERN_VARIABLE, .variable = variable};
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
    else if(tc.tk.type == TK_TYPE && tc.tk.is_open){
      pattern_type_t subpattern;
      if(!parse_pattern_type(&tc, &subpattern)){
        return false;
      }
      pattern_entry_t new_entry = {.type = PATTERN_TYPE, .pattern_type = subpattern};
      pattern_push_entry(result, new_entry);
    }
    else if(tc.tk.type == end_type){
      
      if(result->entry_count == 0){
        throw_error(tc.error_manager, 6, tc.index);
        return false;
      }
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
    if(tc.tk.type == TK_IDENTIFIER){
      expression_t exp = {.type = EXP_RAW_TOKEN, .raw_token = tc.tk};
      new = exp;
    }
    else if(tc.tk.type == TK_NUMERIC || tc.tk.type == TK_CHAR || tc.tk.type == TK_STRING){
      new.type = EXP_VALUE_LITERAL;
      new.return_type = NULL;
      switch(tc.tk.type){
        case TK_NUMERIC:
          switch(tc.tk.number_type){
            case NUM_BINARY_INT:
            case NUM_DECIMAL_INT:
            case NUM_HEX_INT:
            case NUM_OCTAL_INT:
              new.value_literal.type = VAL_UNSIGNED;
              new.value_literal.u = 6769420;
              break;
            case NUM_FLOAT:
            case NUM_SCI_FLOAT:
              new.value_literal.type = VAL_FLOAT;
              new.value_literal.f = 6.7;
              break;
          }
        break;
        case TK_CHAR:
          new.value_literal.type = VAL_CHAR;
          new.value_literal.c = tc.tk.content[0];
        break;
        case TK_STRING:
          new.value_literal.type = VAL_STRING;
          strcpy(new.value_literal.s, tc.tk.content);
        break;
        default:
        break;
      }
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
    else if(tc.tk.type == end_type || tc.tk.type == TK_FORCE_EXP_END || tc.tk.type == TK_ENDLINE){
      pattern_trie_t* trie = (end_type == TK_TYPE)? tc.type_trie : tc.fn_trie;
      if(!build_expression(trie, &root, result)){
        return false;
      }
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
    throw_error(tc.error_manager, 5, tc.index);
    return false;
  }
  tc_inc(&tc);
  result->match_pattern = malloc(sizeof(pattern_t));
  if(!parse_pattern(&tc, TK_TYPE, result->match_pattern)){
    return false;
  }

  tc_inc(&tc);
  if(tc.tk.type == TK_KEYWORD && tc.tk.keyword_id == 9){ //Handle "holds" keyword
    tc_inc(&tc);
    if(tc.tk.type == TK_NUMERIC){
      if(tc.tk.number_type != NUM_FLOAT && tc.tk.number_type != NUM_SCI_FLOAT){
        result->is_builtin = true;
        result->byte_count = atoi(tc.tk.content);
        *base_tc = tc;
        return true;
      }
      throw_error(tc.error_manager, 9, tc.index);
      return false;
    }
    throw_error(tc.error_manager, 8, tc.index);
    return false;
  }
  result->is_builtin = false;

  //Handle "is" and "has" keywords
  if(tc.tk.type != TK_KEYWORD || !(tc.tk.keyword_id == 1 || tc.tk.keyword_id == 2)){
    throw_error(tc.error_manager, 7, tc.index);
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
        throw_error(tc.error_manager, 2, tc.index);
        return false;
      }
    }
  }

  return false;
}


bool parse_fn_declaration(token_cursor_t* base_tc, function_definition_t* result){
  token_cursor_t tc = *base_tc;
  if(tc.tk.type != TK_KEYWORD || tc.tk.keyword_id != 4){
    return false;
  }
  tc_inc(&tc);
  pattern_t pattern;
  if(!parse_pattern(&tc, TK_KEYWORD, &pattern)){
    return false;
  }
  result->match = pattern;
  bool has_makes = false;
  bool has_priority = false;
  bool has_does = false;
  while(tc.tk.type != TK_ENDLINE){
    if(tc.tk.type != TK_KEYWORD){
      throw_error(tc.error_manager, 12, tc.index);
      return false;
    }
    if(tc.tk.keyword_id == 5){
      //makes clause
      if(has_makes){
        throw_error(tc.error_manager, 13, tc.index);
        return false;
      }
      has_makes = true;
      tc_inc(&tc);
      result->return_type = malloc(sizeof(type_identifier_t));
      if(!parse_type_identifier(&tc, result->return_type)){
        return false;
      }
    }
    else if(tc.tk.keyword_id == 7){
      //priority clause
      if(has_priority){
        throw_error(tc.error_manager, 14, tc.index);
        return false;
      }
      has_priority = true;
      tc_inc(&tc);
      expression_t priority;
      if(!parse_expression(&tc, TK_KEYWORD, &priority)){
        return false;
      }
      destroy_expression(&priority);
    }
    else if(tc.tk.keyword_id == 6){
      //does clause
      if(has_does){
        throw_error(tc.error_manager, 15, tc.index);
        return false;
      }
      has_does = true;
      tc_inc(&tc);
      block_t fn_body = {.line_count = 0, .lines = NULL};
      expression_t one_liner;
      if(tc.tk.type == TK_BLOCK && tc.tk.is_open){
        do{
          tc_inc(&tc);
        }while(tc.tk.type != TK_BLOCK);
        tc_inc(&tc);
      }
      else if(parse_expression(&tc, TK_KEYWORD, &one_liner)){

      }
      else{
        throw_error(tc.error_manager, 2, tc.index);
        return false;
      }
    }
    else{
      throw_error(tc.error_manager, 12, tc.index);
      return false;
    }
  }
  *base_tc = tc;
  return true;
}

void parse_tokens(token_cursor_t* tc){
  do{
    type_declaration_t typedec;
    variable_declaration_t vardec;
    function_definition_t fn_dec;
    if(parse_type_declaration(tc, &typedec)){
      type_declaration_t* typedecptr = malloc(sizeof(type_declaration_t));
      *typedecptr = typedec;
      print_type_declaration(typedecptr);
      pattern_trie_push_type(tc->type_trie, typedecptr);
    }
    else if(parse_fn_declaration(tc, &fn_dec)){
      print_function_definition(&fn_dec);
      pattern_trie_push_function(tc->fn_trie, &fn_dec);
    }
    else if(parse_vardec(tc, &vardec)){
      pattern_trie_push_variable(tc->fn_trie, &vardec);
      print_variable_declaration(&vardec);
      printf("\n");
    }
    else{
      tc_inc(tc);
    }

    // type_identifier_t typeid;
    // expression_t exp;
    // if(parse_type_identifier(tc, &typeid)){
    //   printf(GREEN BOLD " YES" RESET_COLOR);
    // }
    // if(parse_expression(tc, TK_ENDLINE, &exp)){
    //   printf(GREEN BOLD " EXP" RESET_COLOR);
    // }
  }while(tc->tk.type != TK_NONE);
}