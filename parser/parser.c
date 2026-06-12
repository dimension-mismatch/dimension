#include "parser.h"
#include "constructs.h"
#include "token_cursor.h"
#include "colors.h"
#include "expression_builder.h"
#include "construct_utils.h"
#include "comptime_eval.h"
#include "hash_table/pattern_trie.h"
#include "dimension-IR/ir_parser.h"
#include "variable_to_register.h"

#include <stdlib.h>
#include <string.h>

bool parse_expression(token_cursor_t* base_tc, token_type_t end_type, expression_t* result);
bool parse_pattern(token_cursor_t* base_tc, token_type_t end_type, pattern_t* result, bool allow_exp);
parse_result_t parse_pattern_vardec(token_cursor_t* base_tc, pattern_variable_t* result, bool required);
//* 2
bool parse_type_identifier(token_cursor_t* base_tc, type_identifier_t* result){
  token_cursor_t tc = *base_tc;
  dimension_array_t dimensions = {.dimension_count = 0, .dimensions = NULL};
  while(true){
    if(tc.tk.type == TK_TYPE && tc.tk.is_open){
      // we've reached the end of the dimensions, move on to the type itself
      break;
    }
    else if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
      tc_inc(&tc);
      expression_t exp;
      if(!parse_expression(&tc, TK_VECTOR, &exp)){
        destroy_dimension_array(&dimensions);
        return false;
      }
      add_dimension(&dimensions, exp);
      tc_inc(&tc);
      continue;
    }
    else if(tc.tk.type == TK_NUMERIC){
      if(tc.tk.number_type == NUM_FLOAT || tc.tk.number_type == NUM_SCI_FLOAT){
        destroy_dimension_array(&dimensions);
        tc_throw_error(&tc, 3);
        return false;
      }
      expression_t exp = {
        .type = EXP_VALUE_LITERAL,
        .const_lvl = 3,
        .value_literal = {.type = VAL_UNSIGNED, .u = atoi(tc.tk.content)}
      };
      add_dimension(&dimensions, exp);
    }
    else{
      destroy_dimension_array(&dimensions);
      tc_throw_error(&tc, 2);

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
      tc_throw_error(&tc, 4);
      destroy_dimension_array(&dimensions);
      return false;
    }
  }
  tc_inc(&tc);
  result->dimensions = dimensions;
  token_cursor_t btc = tc;
  if(btc.tk.type == TK_NUMERIC && btc.tk.number_type < NUM_FLOAT){
    uint64_t size = atoi(btc.tk.content);
    tc_inc(&btc);
    if(btc.tk.type == TK_TYPE && !btc.tk.is_open){
      result->type_id = -1;
      result->size = size;
      tc_inc(&btc);
      *base_tc = btc;
      return true;    
    }
  }
  
  //read the contents of the [square brackets] to get the type expression
  expression_t contents;
  if(!parse_expression(&tc, TK_TYPE, &contents)){
    return false;
  }
  
  if(contents.type != EXP_TYPE_LITERAL){
    tc_throw_error(&tc, 2);
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

expression_t resolve_pattern_value(pattern_value_t* pval){
  expression_t result;
  if(pval->is_param){
    result.type = EXP_READ_VAR;
    result.read_var_id = pval->param->var_id;
  }
  else{
    result = *pval->base_value;//create_exp_from_datum(pval->base_value);
  }
  return result;
}

type_identifier_t resolve_pattern_type(pattern_type_t* ptype){
  type_identifier_t result = {
    .dimensions = {
      .dimension_count = ptype->dimensions.dimension_count,
      .dimensions = malloc(ptype->dimensions.dimension_count * sizeof(expression_t))},
    .num_params = 0,
    .params = NULL,
    .type_id = ptype->base_type_id
    };
  for(int i = 0; i < ptype->dimensions.dimension_count; i++){
    if(ptype->dimensions.dimensions[i].is_param){
      expression_t exp = {.type = EXP_READ_VAR, .const_lvl = 2, .read_var_id = ptype->dimensions.dimensions[i].param->var_id};
      result.dimensions.dimensions[i] = exp;
    }else{
      copy_expression(result.dimensions.dimensions + i, ptype->dimensions.dimensions[i].base_value);
    }
     resolve_pattern_value(ptype->dimensions.dimensions + i);
  }
  if(ptype->is_param){
    result.type_id = -1; //TODO: Figure out how to handle fully parameterized types?
    return result;
  }
  for(int i = 0; i < ptype->subpattern->entry_count; i++){
    pattern_entry_t* entry = ptype->subpattern->entries + i;
    type_argument_t arg;
    switch(entry->type){
      case PATTERN_IDENTIFIER:
        continue;
      case PATTERN_TYPE:
        arg.type = TYPEARG_SUBTYPE;
        arg.subtype = malloc(sizeof(type_identifier_t));
        *arg.subtype = resolve_pattern_type(&entry->pattern_type);
        break;
      case PATTERN_VARIABLE:
        arg.type = TYPEARG_PARAM_EXP;
        expression_t exp = {.type = EXP_READ_VAR, .const_lvl = 2, .read_var_id = entry->variable.var_id};
        arg.exp = malloc(sizeof(expression_t));
        *arg.exp = exp;
        break;
      case PATTERN_EXP:
        arg.type = TYPEARG_PARAM_EXP;
        arg.exp = malloc(sizeof(expression_t));
        copy_expression(arg.exp, &entry->exp);
        break;
    }
    result.num_params++;
    result.params = realloc(result.params, result.num_params * sizeof(type_argument_t));
    result.params[result.num_params - 1] = arg;
  }
  return result;
}

//* 4
parse_result_t parse_vardec(token_cursor_t* base_tc, variable_declaration_t* result){
  token_cursor_t tc = *base_tc;

  if(tc.tk.type != TK_IDENTIFIER){
    return PRS_NOT_FOUND;
  }
  char* name = tc.tk.content;
  tc_inc(&tc);
  if(tc.tk.type != TK_DECL){
    return PRS_NOT_FOUND;
  }
  result->constant_lvl = tc.tk.decl_const_lvl;
  tc_inc(&tc);
  if(!parse_type_identifier(&tc, &result->type)){
    return PRS_ERROR;
  }
  result->var_name = name;//malloc((1 + strlen(name) * sizeof(char)));
  strcpy(result->var_name, name);

  *base_tc = tc;
  return PRS_SUCCESS;

}

//* 5
bool parse_pattern_type(token_cursor_t* base_tc, pattern_type_t* result){
  result->dimensions.dimension_count = 0;
  result->dimensions.dimensions = NULL;
  result->subpattern = NULL;
  result->base_type_id = 0;
  result->is_param = true;
  result->param_count = 0;
  token_cursor_t tc = *base_tc;
  pattern_dimension_array_t dimensions = {.dimension_count = 0, .dimensions = NULL};
  //read dimensions for this type
  while(true){
    
    //dimensions may be defined with an expression or declared as a pattern variable
    if(tc.tk.type == TK_VECTOR && tc.tk.is_open){
      pattern_variable_t pvar;
      expression_t* exp = malloc(sizeof(expression_t));
      tc_inc(&tc);
      
      parse_result_t var_result = parse_pattern_vardec(&tc, &pvar, false);
      //try reading a pattern variable
      if(var_result == PRS_ERROR){
        return false;
      }
      else if(var_result == PRS_SUCCESS){
        printf(BLUE BOLD);
        print_token(&tc.tk);
        printf(RESET_COLOR);
        tc_inc(&tc);
        if(!tc_is_asterisk(&tc)){
          //if we found a variable declaration but no asterisk then this is a dimensionless pattern type
          //e.g. variable :: (t ::: [t])
          result->is_param = true;
          result->param_count = 1;
          result->dimensions = dimensions;
          *base_tc = tc;
          return true;
        }
        //parameterized dimensions must be unsigned integer [u] types 
        if(pvar.type.dimensions.dimension_count > 0){
          tc_throw_error(&tc, 10);
          return false;
        }
        if(pvar.type.param_count > 0){
          tc_throw_error(&tc, 10); 
          return false;
        }
        //parameterized dimensions must be superconst :::
        if(pvar.constant_lvl != CL_SUPERCONST){
          tc_throw_error(&tc, 11);
          return false;
        }

        pattern_value_t* value = add_pattern_dimension(&dimensions);
        value->is_param = true;
        value->param = malloc(sizeof(pattern_variable_t));
        *value->param = pvar;
        result->param_count++;
      }
      //if that fails, try reading an expression
      else if(parse_expression(&tc, TK_VECTOR, exp)){
        pattern_value_t* value = add_pattern_dimension(&dimensions);
        value->is_param = false;
        value->base_value = exp;
      }
      else{
        //not an expression or pattern variable, throw an error!!
        tc_throw_error(&tc, 2);
        return false;
      }
    }
    //the expression might also just be a single number
    else if(tc.tk.type == TK_NUMERIC){
      if(tc.tk.number_type == NUM_FLOAT || tc.tk.number_type == NUM_SCI_FLOAT){
        destroy_pattern_dimension_array(&dimensions);
        tc_throw_error(&tc, 2);
        return false;
      }
      pattern_value_t* new = add_pattern_dimension(&dimensions);
      new->is_param = false;
      new->base_value = malloc(sizeof(expression_t));
      new->base_value->type = EXP_VALUE_LITERAL;
      new->base_value->const_lvl = 3;
      new->base_value->value_literal.type = VAL_UNSIGNED;
      new->base_value->value_literal.u = atoi(tc.tk.content);
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
    parse_result_t var_result = parse_vardec(&tc, &vardec);
    if(!(var_result == PRS_SUCCESS)){
      return false;
    }
    result->param_type = vardec.type;
    result->is_param = true;
    result->param_count++;
    //found a variable declaration
  }
  else if(tc.tk.type == TK_TYPE && tc.tk.is_open){
    tc_inc(&tc);
    pattern_t pattern;
    if(!parse_pattern(&tc, TK_TYPE, &pattern, true)){
      return false;
    }
    trie_match_result_t* match = pattern_trie_validate_pattern(tc.type_trie, &pattern);
    if(!match){
      return false;
    }
    result->subpattern = malloc(sizeof(pattern_t));
    *result->subpattern = pattern;
    result->param_count += pattern.param_count;

    result->is_param = false;
    result->base_type_id = match->index;
  }
  else{
    //syntax error
    tc_throw_error(&tc, 2);
    return false;
  }
  
  *base_tc = tc;
  return true;
}

//* 6
parse_result_t parse_pattern_vardec(token_cursor_t* base_tc, pattern_variable_t* result, bool required){
  token_cursor_t tc = *base_tc;

  if(tc.tk.type != TK_IDENTIFIER){
    if(required){
      tc_throw_error(&tc, 0);
      return PRS_ERROR;
    }
    return PRS_NOT_FOUND;
  }
  char* name = tc.tk.content;
  tc_inc(&tc);
  if(tc.tk.type != TK_DECL){
    if(required){
      tc_throw_error(&tc, 1);
      return PRS_ERROR;
    }
    return PRS_NOT_FOUND;
  }
  result->constant_lvl = tc.tk.decl_const_lvl;
  
  tc_inc(&tc);
  if(!parse_pattern_type(&tc, &result->type)){
    return PRS_ERROR;
  }

  result->name = name;
  variable_declaration_t vardec = {
    .constant_lvl = result->constant_lvl, 
    .type = resolve_pattern_type(&result->type), 
    .var_name = name};

  result->var_id = tc.fn_trie->match_count;
  pattern_trie_push_variable(tc.fn_trie, &vardec);
  
  tc_inc(&tc);
  *base_tc = tc;
  return PRS_SUCCESS;
}

//* 7
bool parse_pattern(token_cursor_t* base_tc, token_type_t end_type, pattern_t* result, bool allow_exp){
  token_cursor_t tc = *base_tc;
  result->entries = NULL;
  result->entry_count = 0;
  result->param_count = 0;
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
        expression_t exp;
        if(parse_pattern_vardec(&tc, &variable, true) == PRS_SUCCESS){
          pattern_entry_t new_entry = {.type = PATTERN_VARIABLE, .variable = variable};
          pattern_push_entry(result, new_entry);
          result->param_count += (1 + variable.type.param_count);
        }
        else if(allow_exp && parse_expression(&tc, TK_VECTOR, &exp)){
          pattern_entry_t new_entry = {.type = PATTERN_EXP, .exp = exp};
          pattern_push_entry(result, new_entry);
        }
        else{
          return false;
        }
        
        if(tc.tk.type == TK_FORCE_EXP_END){

        }
        else if(tc.tk.type == TK_VECTOR && !tc.tk.is_open){
          break;
        }
        else{
          tc_throw_error(&tc, 2);
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
        tc_throw_error(&tc, 6);
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
      printf(" Building at Line %i, Column %i", tc.tk.line_number, tc.tk.start_pos);
      if(!build_expression(trie, &root, result, end_type == TK_VECTOR, tc.error_manager)){
        return false;
      }
      *base_tc = tc;
      return true;
    }
    else{
      //unexpected token (Throw an error?)
      tc_throw_error(&tc, 2);
      return false;
    }
    
    expression_array_t* next = malloc(sizeof(expression_array_t));
    next->prev = prev;
    next->next = NULL;
    next->exp = new;
    next->tk_begin = tc.index;
    next->tk_end = tc.index;
    prev->next = next;
    prev = next;
    tc_inc(&tc);
  };
  
}
bool parse_type_entry(token_cursor_t* base_tc, type_entry_t* result, bool check_name, bool require_name){
  token_cursor_t tc = *base_tc;
  variable_declaration_t vardec = {.constant_lvl = CL_MUTABLE, .var_name = NULL, .type.type_id = -1};
  if(check_name){
    if(tc.tk.type != TK_IDENTIFIER){
      if(require_name){
        printf("expected identifier on: %s\n", tc.tk.content);
        tc_throw_error(&tc, 0);
        return false;
      } 
    }
    else{
      printf("found identifier on: %s\n", tc.tk.content);
      vardec.var_name = tc.tk.content;
      tc_inc(&tc);
      if(tc.tk.type != TK_DECL){
        if(require_name){
          result->is_vector = false;
          result->base = vardec;
          *base_tc = tc;
          return true;
        }

        tc_throw_error(&tc, 1);
        return false;
      }
      vardec.constant_lvl = tc.tk.decl_const_lvl;
      tc_inc(&tc);
    }
  }
  token_cursor_t vectc = tc;
  result->is_vector = true;
  result->subvector.component_count = 0;
  result->subvector.components = NULL;
  result->subvector.name = vardec.var_name;
  result->subvector.const_lvl = vardec.constant_lvl;
  result->subvector.is_enum = false;
  if(vectc.tk.type == TK_KEYWORD && tc.tk.keyword_id == 3){
    result->subvector.is_enum = true;
    tc_inc(&vectc);
  }
  if(vectc.tk.type == TK_VECTOR && tc.tk.is_open){
    printf("reading a new vector\n");
    tc_inc(&vectc);
    while(true){
      if(vectc.tk.type == TK_VECTOR && !vectc.tk.is_open){
        printf("exiting a vector\n");
        tc_inc(&vectc);
        *base_tc = vectc;
        return true;
      }
      if(vectc.tk.type == TK_FORCE_EXP_END){
        tc_inc(&vectc);
        continue;
      }
      type_entry_t entry;
      if(!parse_type_entry(&vectc, &entry, true, result->subvector.is_enum)){
        break;
      }
      result->subvector.component_count++;
      result->subvector.components = realloc(result->subvector.components, result->subvector.component_count * sizeof(type_entry_t));
      result->subvector.components[result->subvector.component_count - 1] = entry;
    }
  }
  printf("didn't read a vector\n");
  if(parse_type_identifier(&tc, &vardec.type)){
    result->is_vector = false;
    result->base = vardec;
    *base_tc = tc;
    return true;
  }
  tc_throw_error(&tc, 2);
  return false;
}

//* 3
parse_result_t parse_type_declaration(token_cursor_t* base_tc, type_declaration_t* result){
  token_cursor_t tc = *base_tc;
  if(tc.tk.type != TK_KEYWORD || tc.tk.keyword_id != 0){
    return PRS_NOT_FOUND;
  }
  pattern_trie_scope_in(tc.fn_trie);
  tc_inc(&tc);
  if(tc.tk.type != TK_TYPE || !tc.tk.is_open){
    tc_throw_error(&tc, 5);
    return PRS_ERROR;
  }
  tc_inc(&tc);
  result->match_pattern = malloc(sizeof(pattern_t));
  if(!parse_pattern(&tc, TK_TYPE, result->match_pattern, false)){
    return PRS_ERROR;
  }

  tc_inc(&tc);
  result->is_builtin = false;

  //Handle "is" and "has" keywords
  if(tc.tk.type != TK_KEYWORD || !(tc.tk.keyword_id == 1 || tc.tk.keyword_id == 2)){
    tc_throw_error(&tc, 7);
    return PRS_ERROR;
  }
  result->is_is = (tc.tk.keyword_id == 1); 
  tc_inc(&tc);
  if(!parse_type_entry(&tc, &result->entry, false, false)) return PRS_ERROR;
    
  if(tc.tk.type == TK_ENDLINE){
    pattern_trie_scope_out(tc.fn_trie);
    tc_inc(&tc);
    *base_tc = tc;
    return PRS_SUCCESS;
  }
  tc_throw_error(&tc, 17);
  return PRS_ERROR;
}


parse_result_t parse_fn_declaration(token_cursor_t* base_tc, function_definition_t* result){
  token_cursor_t tc = *base_tc;
  if(tc.tk.type != TK_KEYWORD || tc.tk.keyword_id != 4){
    return PRS_NOT_FOUND;
  }
  tc_inc(&tc);
  pattern_trie_scope_in(tc.fn_trie);
  pattern_trie_scope_in(tc.type_trie);
  pattern_t pattern;
  if(!parse_pattern(&tc, TK_KEYWORD, &pattern, false)){
    return PRS_ERROR;
  }
  result->match = pattern;
  result->is_IR = false;
  result->priority = 0;
  bool has_makes = false;
  bool has_priority = false;
  bool has_does = false;
  while(tc.tk.type != TK_ENDLINE){
    
    if(tc.tk.type != TK_KEYWORD){
      tc_throw_error(&tc, 12);
      return PRS_ERROR;
    }
    if(tc.tk.keyword_id == 5){
      //makes clause
      if(has_makes){
        tc_throw_error(&tc, 13);
        return PRS_ERROR;
      }
      has_makes = true;
      tc_inc(&tc);
      result->return_type = malloc(sizeof(type_identifier_t));
      if(!parse_type_identifier(&tc, result->return_type)){
        return PRS_ERROR;
      }
    }
    else if(tc.tk.keyword_id == 7){
      //priority clause
      if(has_priority){
        tc_throw_error(&tc, 14);
        return PRS_ERROR;
      }
      has_priority = true;
      tc_inc(&tc);
      expression_t priority;
      if(!parse_expression(&tc, TK_KEYWORD, &priority)){
        return PRS_ERROR;
      }
      destroy_expression(&priority);
    }
    else if(tc.tk.keyword_id == 6){
      //does clause
      if(has_does){
        tc_throw_error(&tc, 15);
        return PRS_ERROR;
      }
      has_does = true;
      tc_inc(&tc);
      block_t fn_body = {.line_count = 0, .lines = NULL};
      expression_t one_liner;
      result->is_IR = false;
      result->body = fn_body;
      if(tc.tk.type == TK_BLOCK && tc.tk.is_open){
        program_t ir;
        tc_inc(&tc);
        
        register_file_t registers = register_file_init();
        pattern_to_registers(&result->match, &registers);
        register_file_push_named(&registers, 4, "result");
        if(parse_ir(&tc, &ir, registers) == PRS_SUCCESS){
          result->is_IR = true;
          result->ir = ir;
          tc_inc(&tc);
          if(tc.tk.type != TK_BLOCK || tc.tk.is_open){
            tc_throw_error(&tc, 2);
            return PRS_ERROR;
          }
        }
        else{     
          while(tc.tk.type != TK_BLOCK){
            tc_inc(&tc);
          }
        }
        tc_inc(&tc);
      }
      else if(parse_expression(&tc, TK_KEYWORD, &one_liner)){

      }
      else{
        
        tc_throw_error(&tc, 2);
        return PRS_ERROR;
      }
    }
    else{
      
      tc_throw_error(&tc, 12);
      return PRS_ERROR;
    }
  }
  pattern_trie_scope_out(tc.fn_trie);
  pattern_trie_scope_out(tc.type_trie);
  tc_inc(&tc);

  *base_tc = tc;
  return PRS_SUCCESS;
}

void recover_from_error(token_cursor_t* tc){
  while(tc->tk.type != TK_ENDLINE && tc->tk.type != TK_NONE){
    tc_inc(tc);
    if(tc->tk.type == TK_KEYWORD && (tc->tk.keyword_id == 0 || tc->tk.keyword_id == 4)){
      tc_throw_error(tc, 17);
      tc_dec(tc);
      return;
    }
  }
  tc_inc(tc);

}

void parse_block(token_cursor_t* tc, token_type_t end_type, block_t* result){
  result->line_count = 0;
  result->lines = NULL;
  int iters = 0;
  while(tc->tk.type != end_type && tc->tk.type != TK_NONE){
    type_declaration_t typedec;
    parse_result_t type_result = parse_type_declaration(tc, &typedec);
    if(type_result == PRS_SUCCESS){
      type_declaration_t* typedecptr = malloc(sizeof(type_declaration_t));
      *typedecptr = typedec;
      pattern_trie_push_type(tc->type_trie, typedecptr);
      printf("successful typedec! we are now at line %i, column %i\n", tc->tk.line_number, tc->tk.start_pos);
      //continue;
    }
    else if(type_result == PRS_ERROR){
      printf("error in type declaration, recovering now\n");
      printf("current position: line %i, column %i\n", tc->tk.line_number, tc->tk.start_pos);
      pattern_trie_scope_out(tc->fn_trie);
      recover_from_error(tc);
      printf("recovered to: line %i, column %i\n", tc->tk.line_number, tc->tk.start_pos);
      //continue;
    }
    else{
      variable_declaration_t vardec;
      parse_result_t var_result = parse_vardec(tc, &vardec);
      if(var_result == PRS_SUCCESS){
        pattern_trie_push_variable(tc->fn_trie, &vardec);
        if(tc->tk.type != TK_ENDLINE){
          tc_throw_error(tc, 17);
        }
        tc_inc(tc);
      }
      else if(var_result == PRS_ERROR){
        recover_from_error(tc);
      }
      else{
        function_definition_t fn_dec;
        parse_result_t fn_result = parse_fn_declaration(tc, &fn_dec);
        if(fn_result == PRS_SUCCESS){
          print_function_definition(&fn_dec);
          pattern_trie_push_function(tc->fn_trie, &fn_dec);
        }
        else if(fn_result == PRS_ERROR){
          recover_from_error(tc);
          pattern_trie_scope_out(tc->fn_trie);
          pattern_trie_scope_out(tc->type_trie);
        }
        else{

          expression_t line;
          if(parse_expression(tc, TK_ENDLINE, &line)){
            result->line_count++;
            result->lines = realloc(result->lines, result->line_count * sizeof(expression_t));
            result->lines[result->line_count - 1] = line;
            tc_inc(tc);
          }
          else{
            recover_from_error(tc);
          }
          //tc_inc(tc);
        }
      }
    }
  }
}

block_t parse_tokens(token_cursor_t* tc){
  block_t program;
  parse_block(tc, TK_BLOCK, &program);
  return program;
}