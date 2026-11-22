#pragma once
#include <constructs.h>
#include <context.h>

bool type_identifier_compare(type_identifier_t* a, type_identifier_t* b, context_t* context);
bool pattern_entry_compare(pattern_entry_t* a, pattern_entry_t* b, context_t* context);

bool expression_compare(expression_t* a, expression_t* b, context_t* context);

bool pattern_type_compare(pattern_type_t* a, pattern_type_t* b, context_t* context);

bool pattern_value_compare(pattern_value_t* a, pattern_value_t* b, context_t* context);