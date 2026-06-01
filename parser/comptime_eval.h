#pragma once

#include "constructs.h"

datum_t evaluate_expression(expression_t* exp);

uint16_t get_dimension(datum_t datum);

expression_t create_exp_from_datum(datum_t datum);