#include <stdio.h>
#include <stdlib.h>

#include "tokenizer.h"

token_t new_empty_token(){
  token_t new = {.line_number = 0, .start_pos = 0, .length = 0, .content = NULL, .type = NONE};
  return new;
}

int is_numeric(char c){
  return '0' <= c && c <= '9';
}

int is_alphanumeric(char c){
  return 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_' || c == '-' || is_numeric(c);
}

void push_char(token_t* token, char next){
  if(token->length == 0){
    if(is_numeric(next)){
      token->type = NUMERIC;
    }
    else if(is_alphanumeric(next)){
      token->type = IDENTIFIER;
    }
    else{
      token->type = SYMBOLIC;
    }
  }
  token->length++;
  token->content = realloc(token->content, token->length * sizeof(char));
  token->content[token->length - 1] = next;
}

void destroy_token(token_t* token){
  if(token->length > 0){
    token->length = 0;
    free(token->content);
    token->content = NULL;
  }
}


token_array_t* init_token_array(){
  return calloc(1, sizeof(token_array_t));
}

void push_token_to_array(token_array_t* array, token_t token){
  array->token_count++;
  array->tokens = realloc(array->tokens, array->token_count * sizeof(token_t));
  array->tokens[array->token_count - 1] = token;
}

void destroy_token_array(token_array_t* array){
  for(int i = 0; i < array->token_count; i++){
    destroy_token(array->tokens + i);
  }
  if(array->token_count > 0){
    free(array->tokens);
    array->tokens = NULL;
    array->token_count = 0;
  }
  free(array);
}

void finish_token_and_push_to_array(token_array_t* array, token_t* token, int col, int line){
  token->start_pos = col;
  token->line_number = line;
  if(token->length <= 0){
    return;
  }
  push_char(token, '\0');
  push_token_to_array(array, *token);
  *token = new_empty_token();
}

void print_token(token_t* token){
  printf("%s", token->content);
}

void print_token_array(token_array_t* tokens){
  for(int i = 0; i < tokens->token_count; i++){
    printf("Token #%d @(%d, %d): ", i, tokens->tokens[i].line_number, tokens->tokens[i].start_pos);
    switch(tokens->tokens[i].type){
      case NUMERIC:
        printf("NUMERIC    ");
        break;
      case IDENTIFIER:
        printf("IDENTIFIER ");
        break;
      case SYMBOLIC:
        printf("SYMBOLIC   ");
        break;
      case PROGRAM:
        printf("PROGRAM    ");
        break;
      default:
        printf("NONE       ");
    }
    printf(" content: ");
    print_token(&(tokens->tokens[i]));
    printf("\n");
  }
}




token_array_t* tokenize_file(FILE* file){
  token_array_t* all_tokens = init_token_array();

  token_t current_token = new_empty_token();


  int line = 1;
  int col = 0;

  int in_comment = 0;

  char ch;
  while((ch = fgetc(file)) != EOF){
    col++;
    
    if(in_comment == 2){
      if(ch == '\n'){
        in_comment = 0;
        destroy_token(&current_token);
      }
      continue;
    }
    if(ch == '/'){
      in_comment++;
    }

    if(current_token.type == NUMERIC && !is_numeric(ch)){
      finish_token_and_push_to_array(all_tokens, &current_token, col, line);
    }
    else if(current_token.type == IDENTIFIER && !is_alphanumeric(ch)){
      finish_token_and_push_to_array(all_tokens, &current_token, col, line);
    }
    else if(current_token.type == SYMBOLIC && is_alphanumeric(ch)){
      finish_token_and_push_to_array(all_tokens, &current_token, col, line);
    }

    if(ch == '\n' || ch == ' ' || ch == ' ' || ch == ','){
      finish_token_and_push_to_array(all_tokens, &current_token, col, line);
      if(ch == '\n'){
        col = 0;
        line++;
      }
    }
    else if(ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '<' || ch == '>' || ch == '{' || ch == '}' || ch == ':' || ch == ';' || ch == '.'){
      finish_token_and_push_to_array(all_tokens, &current_token, col, line);
      push_char(&current_token, ch);
      current_token.type = PROGRAM;
      finish_token_and_push_to_array(all_tokens, &current_token, col, line);
    }
    else{
      push_char(&current_token, ch);
    }
  }

  finish_token_and_push_to_array(all_tokens, &current_token, col, line);
  push_char(&current_token, '\0');
  current_token.type = NONE;
  finish_token_and_push_to_array(all_tokens, &current_token, col, line);
  return all_tokens;
}