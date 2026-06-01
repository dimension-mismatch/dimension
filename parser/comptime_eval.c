#include "comptime_eval.h"
#include <stdlib.h>

datum_t evaluate_expression(expression_t* exp){
  datum_t ret = {.size = 1, .data = malloc(1)};
  ret.data[0] = 67;
  return ret;
}

uint16_t get_dimension(datum_t datum){
  return (datum.data[0] << 8) | datum.data[1];
}

expression_t create_exp_from_datum(datum_t datum){
  expression_t result = {
    .type = EXP_VALUE_LITERAL, 
    .const_lvl = 3, 
    .value_literal.type = VAL_DATUM,
    .value_literal.datum = datum};

  return result;
}
