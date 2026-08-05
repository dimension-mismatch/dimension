#include "ir_evaluation.h"
#include <stdlib.h>

uint64_t evaluate_instruction(uint8_t opcode, uint64_t op_a, uint64_t op_b){
  switch(opcode){
    case 0:
    case 10: {
      return op_a + op_b;
    }
    case 12: {
      return op_a * op_b;
    }
    case 19: {
      return op_a > op_b ? op_a : op_b;
    }
    default:
      return 0xed;
  }
}
