#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "hash_table/hash_table.h"
#include "colors.h"

token_t new_empty_token(){
  token_t new = {.line_number = 0, .start_pos = 0, .length = 0, .content = NULL, .type = TK_NONE};
  return new;
}

int is_numeric(char c){
  return '0' <= c && c <= '9';
}

int is_alphanumeric(char c){
  return 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_' || c == '-' || is_numeric(c);
}

void push_char(token_t* token, char next){
  token->length++;
  token->content = realloc(token->content, token->length * sizeof(char));
  token->content[token->length - 1] = next;
}

void pop_char(token_t* token){
  if(token->length> 0){
    token->length--;
    token->content = realloc(token->content, token->length * sizeof(char));
  }
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

void finish_token_and_push_to_array(token_array_t* array, token_t* token, hash_table_t* keyword_table){
  if(token->length <= 0){
    return;
  }
  push_char(token, '\0');
  if(token->type == TK_IDENTIFIER){
    if(token->is_symbolic_identifier){
      if(token->length == 2){
        char c = token->content[0];
        switch(c){
          case '{':
          case '}':
            token->type = TK_BLOCK;
            break;
          case '(':
          case ')':
            token->type = TK_VECTOR;
            break;
          case '[':
          case ']':
            token->type = TK_TYPE;
            break;
          case ':':
            token->type = TK_DECL;
            token->decl_const_lvl = 0;
            break;
          case ';':
            token->type = TK_ENDLINE;
            break;
          case ',':
            token->type = TK_FORCE_EXP_END;
            break;
          default:
            break;
        }
        
      }
      else if(token->content[0] == ':' && token->content[1] == ':'){
        if(token->length == 3){
          token->type = TK_DECL;
          token->decl_const_lvl = 1;
        }
        else if(token->length == 4 && token->content[2] == ':'){
          token->type = TK_DECL;
          token->decl_const_lvl = 2;
        }
      }
    }
    else {
      int* keyword = get_value_from_key(keyword_table, token->content);
      if(keyword){
        token->type = TK_KEYWORD;
        token->keyword_id = *keyword;
      }
    }
  }
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
      case TK_NUMERIC:
        printf(MAGENTA "NUMERIC     " RESET_COLOR);
        break;
      case TK_IDENTIFIER:
        printf(CYAN "IDENTIFIER  "   RESET_COLOR);
        break;
      case TK_BLOCK:
      case TK_TYPE:
      case TK_VECTOR:
        printf(YELLOW "CONTAINER   " RESET_COLOR);
        break;
      case TK_DECL:
        printf(BLUE "DECLARATION " RESET_COLOR);
        break;
      case TK_KEYWORD:
        printf(RED "KEYWORD     " RESET_COLOR);
        break;
      case TK_DMSN_IR:
        printf(BLACK "IR          " RESET_COLOR);
        break;
      case TK_CHAR :
        printf(GREEN "CHAR        " RESET_COLOR);
        break;
      case TK_STRING:
        printf(GREEN "CHAR        " RESET_COLOR);
        break;
      case TK_FORCE_EXP_END:
        printf(BLACK "COMMA       " RESET_COLOR);
        break;
      case TK_ENDLINE:
        printf(BLACK "SEMICOLON       " RESET_COLOR);
        break;
      default:
        printf("NONE        ");
    }
    printf(" content: ");
    print_token(&(tokens->tokens[i]));
    printf("\n");
  }
}




token_array_t* tokenize_file(FILE* file){

  //Creating a table to look up keywords
  int keyword_count = 9;
  char* keywords[] = {"type","is",  "has","oneof", "fn", "makes", "does", "priority", "return"};

  hash_table_t keyword_table = init_hash_table(67, 0.95);
  for(int i = 0; i < keyword_count; i++){
    push_key_value(&keyword_table, keywords[i], i);
  }

  //Creating a table to look up symbols
  int symbol_count = 9;
  char symbols[] = {'{', '}', '[', ']', '(', ')', ':', ';', ','};
  token_type_t symbol_types[] = {TK_BLOCK, TK_BLOCK, TK_TYPE, TK_TYPE, TK_VECTOR, TK_VECTOR, TK_DECL, TK_ENDLINE, TK_FORCE_EXP_END};

  hash_table_t symbol_table = init_hash_table(67, 0.95);
  for(int i = 0; i < symbol_count; i++){
    char str[] = {symbols[i], '\0'};
    push_key_value(&symbol_table, str, symbol_types[i]);
  }

  token_array_t* all_tokens = init_token_array();

  token_t current_token = new_empty_token();


  int line = 1;
  int col = 0;

  int in_comment = 0;
  bool in_asm = false;
  bool in_char = false;
  bool in_string = false;

  char ch;
  while((ch = fgetc(file)) != EOF){
    
    col++;
    if(ch == '\n'){
      col = 0;
      line++;
    }

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
    else{
      in_comment = 0;
    }

    if(current_token.length == 0){
      current_token.line_number = line;
      current_token.start_pos = col;
    }

    if(current_token.type == TK_NUMERIC && !is_numeric(ch)){
      finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
    }
    else if(current_token.type == TK_IDENTIFIER && !current_token.is_symbolic_identifier && !is_alphanumeric(ch)){
      finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
    }
    else if(current_token.type == TK_IDENTIFIER && current_token.is_symbolic_identifier && is_alphanumeric(ch)){
      finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
    }
    else if(current_token.type == TK_DECL && (ch != ':' || current_token.length >= 4)){
      finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
    }



    if(in_asm){
      if(ch == '~'){
        finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
        in_asm = false;
      }
      else{
        push_char(&current_token, ch);
      }
      continue;
    }
    if(in_char){
      if(ch == '\''){
        finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
        in_char = false;
      }
      else{
        push_char(&current_token, ch);
      }
      continue;
    }
    if(in_string){
      if(ch == '"'){
        finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
        in_string = false;
      }
      else{
        push_char(&current_token, ch);
      } 
      continue;
    }

    
    if(ch == '~'){
      finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
      current_token.type = TK_DMSN_IR;
      in_asm = true;
      continue;
    }
    if(ch == '\''){
      finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
      current_token.type = TK_CHAR;
      in_char = true;
      continue;
    }
    if(ch == '"'){
      finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
      current_token.type = TK_STRING;
      in_string = true;
      continue;
    }

    if(ch == '\n' || ch == '\t' || ch == ' '){
      finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
      continue;
    }

    
    
    if(current_token.length == 0){
      if(is_numeric(ch)){
        current_token.type = TK_NUMERIC;
      }
      else{
        current_token.type = TK_IDENTIFIER;
        current_token.is_symbolic_identifier = !is_alphanumeric(ch);
      } 
    }
    push_char(&current_token, ch);
    if(current_token.type == TK_IDENTIFIER && current_token.is_symbolic_identifier){
      push_char(&current_token, '\0');
      int* symbol_type = get_value_from_key(&symbol_table, current_token.content);
      if(symbol_type){
        current_token.type = *symbol_type;
        switch(ch){
          case '(':
          case '{':
          case '[':
            current_token.is_open = true;
            break;
          case ')':
          case '}':
          case ']':
            current_token.is_open = false;
          default:
            break;
        }
        if(current_token.type != TK_DECL){
          finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
        }
        pop_char(&current_token);
      }
    }
  }

  finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
  push_char(&current_token, '\0');
  current_token.type = TK_NONE;
  finish_token_and_push_to_array(all_tokens, &current_token, &keyword_table);
  destroy_hash_table(&keyword_table);
  return all_tokens;
}