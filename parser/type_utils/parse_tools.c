#include "parse_tools.h"
#include "../tokenizer.h"
#include "type_utils.h"
#include "../colors.h"
#include <stdio.h>


void error_msg(char* error_type, char* error_msg, token_t* bad_token){
  printf(RED BOLD "Error" UNBOLD " in %s at " BLUE "(Line " BOLD "%d" UNBOLD ", Col" BOLD " %d" UNBOLD ")" RED ":  " BOLD MAGENTA  "\"%s\"" UNBOLD RED" %s\n" RESET_COLOR, error_type, bad_token->line_number, bad_token->start_pos, bad_token->content, error_msg);
}

void error_msg_incorrect_token(char* error_type, char* expected_token, token_t* bad_token){
  printf(RED "Error in %s at (Line %d, Col %d), expected \"%s\", but instead found \"%s\"\n" RESET_COLOR, error_type, bad_token->line_number, bad_token->start_pos, expected_token, bad_token->content);
}

void error_msg_unknown_bad_token(char* error_type, token_t* bad_token){
  printf(RED "Error in %s at (Line %d, Col %d), encountered unexpexted \"%s\"\n" RESET_COLOR, error_type, bad_token->line_number, bad_token->start_pos, bad_token->content);
}

token_t* next_token(token_array_t* tokens, int* index){
  if(*index < tokens->token_count){
    (*index)++;
  }
  token_t* token = &tokens->tokens[*index];

  return token;
}
int match_current_type(token_t* token, token_type_t type){
  return token->type == type;
}
int match_current_content(token_t* token, token_type_t type, char* content){
  if(token->type != type){
    return 0;
  }
  for(int i = 0; i < token->length; i++){
    if(token->content[i] != content[i]){
      return 0;
    }
  }
  return 1;
}
int match_next_type(int* index, token_t** current, token_array_t* tokens, token_type_t type){
  token_t* token = next_token(tokens, index);
  *current = token;
  return match_current_type(token, type);
}

int cond_peek_next_type(int* index, token_t** current, token_array_t* tokens, token_type_t type){
  int idx = *index;
  token_t* token = next_token(tokens, &idx);
  int res = match_current_type(token, type);
  if(res){
    (*index)++;
    *current = token;
  }
  return res;
}

int peek_next_type(int* index, token_t** current, token_array_t* tokens, token_type_t type){
  int idx = *index;
  return match_current_type(next_token(tokens, &idx), type);
}

int match_next_content(int* index, token_t** current, token_array_t* tokens, token_type_t type, char* content){
  token_t* token = next_token(tokens, index);
  *current = token;
  return match_current_content(token, type, content);
}

int cond_peek_next_content(int* index, token_t** current, token_array_t* tokens, token_type_t type, char* content){
  int idx = *index;
  token_t* token = next_token(tokens, &idx);
  int res = match_current_content(token, type, content);
  if(res){
    (*index)++;
    *current = token;
  }
  return res;
}

int peek_next_content(int* index, token_t** current, token_array_t* tokens, token_type_t type, char* content){
  int idx = *index;
  return match_current_content(next_token(tokens, &idx), type, content);
}


int allow_only_next_content(char* allowed_token, token_type_t allowed_type, char* error_type, int* index, token_array_t* tokens, token_t** current){
  if(!match_next_content(index, current, tokens, allowed_type, allowed_token)){
    error_msg_incorrect_token(error_type, allowed_token, *current);
    return 1;
  }
  return 0;
}

int allow_only_next_type(token_type_t allowed_type, char* error_type, int* index, token_array_t* tokens, token_t** current){
  if(!match_next_type(index, current, tokens, allowed_type)){
    error_msg_unknown_bad_token(error_type, *current);
    return 1;
  }
  return 0;
}

int allow_only_current_content(char* allowed_token, token_type_t allowed_type, char* error_type, token_t* current){
  if(!match_current_content(current, allowed_type, allowed_token)){
    error_msg_incorrect_token(error_type, allowed_token, current);
    return 1;
  }
  return 0;
}

int allow_only_current_type(token_type_t allowed_type, char* error_type, token_t* current){
  if(!match_current_type(current, allowed_type)){
    error_msg_unknown_bad_token(error_type, current);
    return 1;
  }
  return 0;
}
