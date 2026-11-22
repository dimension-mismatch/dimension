#pragma once
#include <constructs.h>
#include <context.h>

bool test_pattern_type(pattern_type_t *test, type_identifier_t *subject, context_t* context);

bool test_pattern_variable(pattern_variable_t* test, expression_t* subject, context_t* context);


bool compatible_const_levels(const_lvl_t slot, const_lvl_t input);