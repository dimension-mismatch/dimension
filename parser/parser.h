#pragma once
#include "tokenizer.h"
#include "token_cursor.h"


typedef enum parse_result{
  PRS_SUCCESS,
  PRS_NOT_FOUND,
  PRS_ERROR,
  PRS_ERROR_UNCAUGHT,
}parse_result_t;

void parse_tokens(token_cursor_t* tc);