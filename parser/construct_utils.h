#pragma once
#include "constructs.h"

void print_block(block_t* block);
void print_expression(expression_t* exp);
void print_type_identifier(type_identifier_t* type);
void print_type_declaration(type_declaration_t* typedec);
void print_variable_declaration(variable_declaration_t* vardec);
void print_pattern_value(pattern_value_t* pval);
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
void destroy_pattern_value(pattern_value_t* pval);
void destroy_pattern_type(pattern_type_t* ptype);
void destroy_pattern_variable(pattern_variable_t* pvar);
void destroy_pattern_entry(pattern_entry_t* pentry);
void destroy_pattern(pattern_t* pattern);
void destroy_function_definition(function_definition_t* fn_def);

void copy_block(block_t* new, block_t* block);
void copy_expression(expression_t* new, expression_t* exp);
void copy_type_identifier(type_identifier_t* new, type_identifier_t* type);
void copy_type_declaration(type_declaration_t* new, type_declaration_t* typedec);
void copy_variable_declaration(variable_declaration_t* new, variable_declaration_t* vardec);
void copy_pattern_value(pattern_value_t* new, pattern_value_t* pval);
void copy_pattern_type(pattern_type_t* new, pattern_type_t* ptype);
void copy_pattern_variable(pattern_variable_t* new, pattern_variable_t* pvar);
void copy_pattern_entry(pattern_entry_t* new, pattern_entry_t* pentry);
void copy_pattern(pattern_t* new, pattern_t* pattern);
void copy_function_definition(function_definition_t* new, function_definition_t* fn_def);