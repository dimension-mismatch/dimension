#pragma once
#include "parse_tools.h"
#include "../tokenizer.h"
#include "type_utils.h"

#include "../error_handling/error_manager.h"

//Prints an error message that informs the user the Line, Column, and token which caused the error. 
//The error type parameter can provide specific information about the type of error that occurred
void error_msg(char *error_type, char *error_msg, token_t *bad_token);

//Prints an error message that informs the user that a specific token was expected, but was not found
void error_msg_incorrect_token(char *error_type, char *expected_token, token_t *bad_token);

//gets the next token in the array and increments the index
token_t *next_token(parse_manager_t* manager, int *index);

//Returns True (1) if the current token is of the expected type
int match_current_type(token_t *token, token_type_t type);

//Returns True (1) if the current token is of the expected type and contains the expected text
int match_current_content(token_t *token, token_type_t type, char *content);

//Returns True (1) if the next token is of the expected type, and increments the counter
int match_next_type(int *index, token_t **current, parse_manager_t* manager, token_type_t type);

//Returns True (1) if the next token is of the expected type, and increments the counter only if the expected token was found
int cond_peek_next_type(int *index, token_t **current, parse_manager_t* manager, token_type_t type);

int peek_next_type(int *index, token_t **current, parse_manager_t* manager, token_type_t type);

//Returns True (1) if the next token is of the expected type and contains the expected text, and increments the counter
int match_next_content(int *index, token_t **current, parse_manager_t* manager, token_type_t type, char *content);

//Returns True (1) if the next token is of the expected type and contains the expected text, and increments the counter only if the expected token was found
int cond_peek_next_content(int *index, token_t **current, parse_manager_t* manager, token_type_t type, char *content);

int peek_next_content(int *index, token_t **current, parse_manager_t* manager, token_type_t type, char *content);

//returns (1) and throws an error if the next token does not match the specified type and content text 
int allow_only_next_content(char *allowed_token, token_type_t allowed_type, char *error_type, int *index, parse_manager_t* manager, token_t **current);

//returns (1) and throws an error if the next token does not match the specified type
int allow_only_next_type(token_type_t allowed_type, char *error_type, int *index, parse_manager_t* manager, token_t **current);

//returns (1) and throws an error if the current token does not match the specified type and content text 
int allow_only_current_content(char *allowed_token, token_type_t allowed_type, char *error_type, token_t *current);

//returns (1) and throws an error if the current token does not match the specified type
int allow_only_current_type(token_type_t allowed_type, char *error_type, token_t *current);
