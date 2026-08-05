#include "ir_evaluation.h"
#include <stdlib.h>

uint8_t *evaluate_instruction(uint8_t opcode, uint16_t outsize, uint16_t op_a_size, uint16_t op_b_size, uint8_t *op_a, uint8_t *op_b){
  uint8_t* result = malloc(outsize);
  //uint16_t minsize = op_a_size < op_b_size ? op_a_size : op_b_size;
  uint16_t maxsize = op_a_size > op_b_size ? op_a_size : op_b_size;
  switch(opcode){
    case 0:
    case 10: {
      int carry = 0;
      for(int i = 0; i < outsize; i++){
        uint8_t aval = i < op_a_size ? op_a[i] : 0;
        uint8_t bval = i < op_b_size ? op_b[i] : 0;
        uint16_t sum = aval + bval + carry;
        result[i] = sum & 0x00FF;
        carry = sum >> 8;
      }
      break;
    }
    case 19: {
      uint8_t* bestdata;
      uint16_t bestsize;
      for(int i = maxsize - 1; i >= 0; i--){
        uint8_t aval = i < op_a_size ? op_a[i] : 0;
        uint8_t bval = i < op_b_size ? op_b[i] : 0;
        if(aval > bval){
          bestdata = op_a;
          bestsize = op_a_size;
          break;
        }
        if(bval > aval){
          bestdata = op_b;
          bestsize = op_b_size;
          break;
        }
      }
      for(int i = 0; i < outsize; i++){
        result[i] =  i < bestsize ? bestdata[i] : 0;
      }
      break;
    }
    default:
      for(int i = 0; i < outsize; i++){
        result[i] = 0;
      }
      result[0] = 0xed;
  }
  return result;
}
