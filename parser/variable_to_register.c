#include "variable_to_register.h"
#include "dimension-IR/registers.h"

void pattern_to_registers(pattern_t* pattern, register_file_t* rf);

//TODO: not everything is 4 bytes
void pattern_type_to_registers(pattern_type_t* ptype, register_file_t* rf){
  printf("adding a pattern type with %i dimensions and %i parameters\n", ptype->dimensions.dimension_count, ptype->param_count);
  for(int i = 0; i < ptype->dimensions.dimension_count; i++){
    if(ptype->dimensions.dimensions[i].is_param){
      printf("adding a dimension argument\n");
      register_file_push(rf, 4);
    }
  }

  pattern_to_registers(ptype->subpattern, rf);

}
void pattern_variable_to_registers(pattern_variable_t* pvar, register_file_t* rf){
  printf("Adding a pattern variable\n");
  pattern_type_to_registers(&pvar->type, rf);
  register_file_push(rf, 4);
}

void pattern_to_registers(pattern_t* pattern, register_file_t* rf){
  printf("generating argument registers...\n");
  for(int i = 0; i < pattern->entry_count; i++){
    pattern_entry_t* entry = pattern->entries + i; 
    switch(entry->type){
      case PATTERN_IDENTIFIER:
      case PATTERN_EXP:
        break;
      case PATTERN_TYPE:
        pattern_type_to_registers(&entry->pattern_type, rf);
        break;
      case PATTERN_VARIABLE:
        pattern_variable_to_registers(&entry->variable, rf);
        break;
    }
  }
}