#include "ir_evaluation.h"
#include <stdlib.h>
#include <string.h>

uint64_t d2i(double d){
  uint64_t result;
  memcpy(&result, &d, sizeof(double));
  return result;
}
uint64_t evaluate_instruction(uint8_t opcode, uint64_t op_a, uint64_t op_b){
  switch(opcode){
    case 0:
    case 10:
      return op_a + op_b;
    case 1:
    case 11: 
      return op_a - op_b;
    case 12: 
      return op_a * op_b;
    case 13:
      return op_a / op_b;
    case 14:
      return op_a > op_b ? 1 : 0;
    case 15:
      return op_a < op_b ? 1 : 0;
    case 16: 
      return op_a >= op_b ? 1 : 0;
    case 17:
      return op_a <= op_b ? 1 : 0;
    case 18:
      return op_a < op_b ? op_a : op_b;
    case 19: 
      return op_a > op_b ? op_a : op_b;
    case 30:
      return op_a == op_b;
    case 31: 
      return *(uint64_t*)(op_a + op_b);
    case 32: 
      return op_a >> op_b;
    case 33:
      return op_a << op_b;
    case 34: 
      return op_a & op_b;
    case 35:
      return op_a | op_b;
    case 36: 
      return op_a ^ op_b;
    case 37:
      return op_a && op_b;
    case 38: 
      return op_a || op_b;
    case 39: 
      return (op_a || op_b) && !(op_a && op_b);
    case 40:
      return 0;
    default:
      break;
  }
  int64_t i_a = (int64_t) op_a;
  int64_t i_b = (int64_t) op_b;
  switch(opcode){
    case 2:
      return i_a * i_b;
    case 3:
      return i_a / i_b;
    case 4:
      return i_a > i_b ? 1 : 0;
    case 5:
      return i_a < i_b ? 1 : 0;
    case 6:
      return i_a >= i_b ? 1 : 0;
    case 7:
      return i_a <= i_b ? 1 : 0;
    case 8:
      return i_a < i_b ? i_a : i_b;
    case 9:
      return i_a > i_b ? i_a : i_b;
  }

  double d_a;
  double d_b;
  memcpy(&d_a, &op_a, sizeof(double));
  memcpy(&d_b, &op_a, sizeof(double));
  switch(opcode){
    case 20:
      return d2i(d_a + d_b);
    case 21:
      return d2i(d_a - d_b);
    case 22:
      return d2i(d_a * d_b);
    case 23:
      return d2i(d_a / d_b);
    case 24: 
      return d_a > d_b ? 1 : 0;
    case 25: 
      return d_a < d_b ? 1 : 0;
    case 26:
      return d_a >= d_b ? 1 : 0;
    case 27:
      return d_a <= d_b ? 1 : 0;
    case 28:
      return d2i(d_a < d_b ? d_a : d_b);
    case 29:
      return d2i(d_a > d_b ? d_a : d_b);
  }
  return 0xed;
}
