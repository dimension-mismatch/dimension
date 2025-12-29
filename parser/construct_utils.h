#pragma once
#include "constructs.h"

void print_block(block_t* block);
void print_expression(expression_t* exp);
void print_type_identifier(type_identifier_t* type);
void print_type_declaration(type_declaration_t* typedec);
void print_variable_declaration(variable_declaration_t* vardec);
void print_pattern_type(pattern_type_t* ptype);
void print_pattern_variable(pattern_variable_t* pvar);
void print_pattern_entry(pattern_entry_t* pentry);
void print_pattern(pattern_t* pattern);
void print_function_definition(function_definition_t* fn_def);


void destroy_block(block_t* block);
void destroy_expression(expression_t* exp);
void destroy_type_identifier(type_identifier_t* type);
void destroy_type_declaration(type_declaration_t* typedec);
void destroy_variable_declaration(variable_declaration_t* vardec);
void destroy_pattern_type(pattern_type_t* ptype);
void destroy_pattern_variable(pattern_variable_t* pvar);
void destroy_pattern_entry(pattern_entry_t* pentry);
void destroy_pattern(pattern_t* pattern);
void destroy_function_definition(function_definition_t* fn_def);