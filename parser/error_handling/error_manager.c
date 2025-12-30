#include "error_manager.h"

#include "../colors.h"

#include <stdio.h>
#include <stdlib.h>

parse_manager_t parse_manager_init(token_array_t* tokens, pattern_trie_t* fn_rec, variable_record_t* var_rec, type_record_t* type_rec){
  parse_manager_t new = {0, NULL, 0, NULL, fn_rec, type_rec, var_rec, tokens};
  FILE* file = fopen("error_handling/dmsn_errors.txt", "r");
  if(file == NULL){
    printf(RED BOLD "Failed to load error handling\n" RESET_COLOR);
    exit(15);
  }

  char c;
  int i = 0;
  int newline = 1;
  while((c = fgetc(file)) != EOF){
    i++;
    if(newline && c == '~'){
      new.err_id_count++;
      new.index = realloc(new.index, new.err_id_count * sizeof(int));
      new.index[new.err_id_count - 1] = i;
    }
    newline = (c == '\n');
  }
  return new;
}

void throw_error(parse_manager_t* errors, int error_num, int token_idx){
  errors->error_count++;
  errors->errors = realloc(errors->errors, errors->error_count * sizeof(error_t));
  error_t err = {error_num, 0, NULL, token_idx};
  errors->errors[errors->error_count - 1] = err;
}

void err_arg(parse_manager_t* errors, error_arg_t arg){
  error_t* err = errors->errors + errors->error_count - 1;

  err->arg_c++;
  err->arg_v = realloc(err->arg_v, err->arg_c * sizeof(error_arg_t));
  err->arg_v[err->arg_c - 1] = arg;
}

void err_expression_arg(parse_manager_t* errors, expression_t* exp){
  error_arg_t arg = {.type = ERR_EXPRESSION, .exp = exp};
  err_arg(errors, arg);
}

void err_token_arg(parse_manager_t* errors, token_t* token){
  error_arg_t arg = {.type = ERR_TOKEN, .token = token};
  err_arg(errors, arg);
}

void err_type_arg(parse_manager_t* errors, type_identifier_t* type){
  error_arg_t arg = {.type = ERR_TYPE, .type_id = type};
  err_arg(errors, arg);
}

void err_function_arg(parse_manager_t* errors, int fn_id){
  error_arg_t arg = {.type = ERR_FUNCTION, .function_id = fn_id};
  err_arg(errors, arg);
}


void print_error(FILE* error_src_txt, parse_manager_t* manager, int error){
  error_t err = manager->errors[error];
  token_t token = manager->tokens->tokens[err.token_idx];
  fseek(error_src_txt, manager->index[err.err_num], SEEK_SET);
  char c;
  printf(RED BOLD);
  while((c = fgetc(error_src_txt)) != '|'){
    putc(c, stdout);
  }
  printf(RESET_COLOR);
  int in_quote = 0;
  int arg_i = 0;
  while((c = fgetc(error_src_txt)) != '\n' && c != EOF){
    if(c == '@'){
      printf("at " BLUE "(Line " BOLD "%d" UNBOLD ", Col" BOLD " %d" UNBOLD "):\n        " RESET_COLOR, token.line_number, token.start_pos - token.length + 1);
    }
    else if(c == '\''){
      in_quote = 1 - in_quote;
      printf(in_quote? YELLOW BOLD : RESET_COLOR);
    }
    else if(c == '\"'){
      in_quote = 1 - in_quote;
      if(in_quote) printf(YELLOW BOLD);
      printf("\'");
      if(!in_quote) printf(RESET_COLOR);
    }
    else if(c == '#'){
      c = fgetc(error_src_txt);
      if(c == 'T'){
        printf("%s", token.content);
      }
      if (c == 't'){
        error_arg_t arg = err.arg_v[arg_i];
        if(arg.type == ERR_TYPE){
          print_type_id_named(arg.type_id, manager->type_rec);
        }
        arg_i++;
      }
      
    }
    else{
      putc(c, stdout);
    }
  }
  printf(GREEN " (#%d)" RESET_COLOR, manager->errors[error].err_num + 100);
  printf("\n\n");
}

void error_printout(parse_manager_t* manager){
  if(manager->error_count == 0){
    printf(GREEN BOLD "Compile Successful! :)\n" RESET_COLOR);
    return;
  }
  FILE* error_src_txt = fopen("error_handling/dmsn_errors.txt", "r");
  printf(RED BOLD "\nCompile Failed\n" RESET_COLOR);
  if(manager->error_count == 1){
    printf("Found 1 Error:\n\n");
  }
  else{
    printf("Found %d Errors:\n\n", manager->error_count);
  }
  
  for(int i = 0; i < manager->error_count; i++){
    printf(WHITE BOLD "(%d/%d) : " RESET_COLOR, i + 1, manager->error_count);
    print_error(error_src_txt, manager, i);
  }
}