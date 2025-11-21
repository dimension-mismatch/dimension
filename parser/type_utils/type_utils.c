#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "type_utils.h"
#include "../colors.h"

int bits_needed_to_count_to(int n){
  int b = n;
  int i = 1;
  while(n > 1){
    n = n >> 1;
    i++;
  }
  return i;
}

type_identifier_t typeid_newEmpty(){
  type_identifier_t new = {-1, 0, NULL, 0};
  return new;
}

void typeid_pushDimension(type_identifier_t *typeId, unsigned int dimension){
  typeId->dimension_count++;
  typeId->dimensions = realloc(typeId->dimensions, typeId->dimension_count * sizeof(unsigned int));
  typeId->dimensions[typeId->dimension_count - 1] = dimension;
}

void typeid_multiply(type_identifier_t* typeId, dimension_t multiplicity){
  typeId->dimensions = realloc(typeId->dimensions, (typeId->dimension_count + multiplicity.dimension_count) * sizeof(unsigned int));
  for(int i = 0 ; i < multiplicity.dimension_count; i++){
    typeId->dimensions[typeId->dimension_count + i] = multiplicity.dimensions[i];
  }
  typeId->dimension_count += multiplicity.dimension_count;
}

dimension_t dmsn_newEmpty(){
  dimension_t new = {0, NULL};
  return new;
}

void dmsn_pushDimension(dimension_t* dimension, unsigned int d){
  dimension->dimension_count++;
  dimension->dimensions = realloc(dimension->dimensions, dimension->dimension_count * sizeof(unsigned int));
  dimension->dimensions[dimension->dimension_count - 1] = d;
}

dimension_t dmsn_copy(dimension_t* dimension){
  dimension_t new = dmsn_newEmpty();
  if(dimension->dimension_count == 0) return new;
  new.dimensions = malloc(dimension->dimension_count * sizeof(unsigned int));
  for(int i = 0; i < dimension->dimension_count; i++){
    new.dimensions[i] = dimension->dimensions[i];
  }
  new.dimension_count = dimension->dimension_count;
  return new;
}

void dmsn_destroy(dimension_t* dimension){
  if(dimension->dimension_count > 0){
    free(dimension->dimensions);
    dimension->dimensions = NULL;
    dimension->dimension_count = 0;
  }
}


unsigned int typeid_totalDimensions(type_identifier_t *typeId){
  int dim = 1;
  for(int i = 0; i < typeId->dimension_count; i++){
    dim *= typeId->dimensions[i];
  }
  return dim;
}

int typeid_bytesize(type_identifier_t* typeid){
  return (typeid->bit_count + 7) / 8 * typeid_totalDimensions(typeid);
}

void typeid_destroy(type_identifier_t *typeId){
  free(typeId->dimensions);
  typeId->dimension_count = 0;
  typeId->dimensions = NULL;
}

type_identifier_t typeid_copy(type_identifier_t *typeId){
  type_identifier_t new = typeid_newEmpty();
  for(int i = 0; i < typeId->dimension_count; i++){
    typeid_pushDimension(&new, typeId->dimensions[i]);
  }
  new.type_number = typeId->type_number;
  new.bit_count = typeId->bit_count;
  return new;
}

type_declaration_t typedec_newEmpty(){
  type_declaration_t new = {.type_name = NULL, .type_type = NO_TYPE, .bit_count = 0};
  return new;
}

void typedec_setName(type_declaration_t *typedec, type_type_t type, char *name){
  int length = 1 + strlen(name);
  typedec->type_name = realloc(typedec->type_name, length * sizeof(char));
  strcpy(typedec->type_name, name);
  typedec->type_type = type;
}

void addEntry(type_declaration_t *typedec, char *name, int length){
  char* newOption = malloc(length * sizeof(char));
  for(int i = 0; i < length; i++){
    newOption[i] = name[i];
  }
  typedec->enumeration.option_count++;
  typedec->enumeration.options = realloc(typedec->enumeration.options, typedec->enumeration.option_count * sizeof(char*));
  typedec->enumeration.options[typedec->enumeration.option_count - 1] = newOption;
}
void typedec_pushEnumOption(type_declaration_t *typedec, char *name, int length){
  if(typedec->type_type != ENUM_TYPE){
    return;
  }
  addEntry(typedec, name, length);
  typedec->bit_count = bits_needed_to_count_to(typedec->enumeration.option_count);
}

void typedec_pushMember(type_declaration_t *typedec, type_identifier_t* member, char *name, int length){
  if(typedec->type_type != COMP_TYPE){
    return;
  }
  addEntry(typedec, name, length);
  typedec->composition.members = realloc(typedec->composition.members, typedec->composition.member_count * sizeof(type_identifier_t));
  typedec->composition.members[typedec->composition.member_count - 1] = *member;
  typedec->bit_count += typeid_bytesize(member) * 8;

}

void typedec_destroy(type_declaration_t *typedec){
  if(typedec->type_name != NULL){
    free(typedec->type_name);
    typedec->type_name = NULL;
  }
  
  if(typedec->enumeration.option_count > 0){
    for(int i = 0; i < typedec->enumeration.option_count; i++){
      free(typedec->enumeration.options[i]);
    }
    free(typedec->enumeration.options);
    typedec->enumeration.options = NULL;
    if(typedec->type_type == COMP_TYPE){
      for(int i = 0; i < typedec->composition.member_count; i++){
        typeid_destroy(typedec->composition.members + i);
      }
      free(typedec->composition.members);
      typedec->composition.members = NULL;
    }
  }
  typedec->enumeration.option_count = 0;
  typedec->type_type = NO_TYPE;
  typedec->bit_count = 0;
}

void print_type(type_declaration_t* type){
  printf("Type " CYAN "[" BOLD "%s" UNBOLD "]" RESET_COLOR " is a %d-bit ", type->type_name, type->bit_count);
  switch(type->type_type){
    case NO_TYPE:
      printf("nonexistent type\n");
      break;
    case BASE_TYPE:
      printf("Base Type\n");
      break;
    case ENUM_TYPE:
      printf("Enum Type with the following options: \n");
      for(int i = 0; i < type->enumeration.option_count; i++){
        printf("  *  " YELLOW "%s" RESET_COLOR "\n", type->enumeration.options[i]);
      }
      break;
    case COMP_TYPE:
      printf("Composition Type composed of: \n");
      for(int i = 0; i < type->composition.member_count; i++){
        printf(" *  ");
        if(type->composition.member_names[i][0] != '\0'){
          printf("\"%s\" : ", type->composition.member_names[i]);
        }
        print_type_id(&(type->composition.members[i]));
        printf("\n");
      }
  }
}

void print_type_id(type_identifier_t* typeid){
  if(typeid == NULL){
    printf(CYAN "[NULL]" RESET_COLOR);
    return;
  }
  if(typeid->dimension_count > 0){
    printf(MAGENTA);
    int i = 0;
    while(i < typeid->dimension_count - 1){
      printf("%d*",typeid->dimensions[i]);
      i++;
    }
    printf("%d" CYAN "[%d]",typeid->dimensions[i], typeid->type_number);
  }
  else{
    printf(CYAN);
    printf("[%d]", typeid->type_number);
  }
  printf(RESET_COLOR);
}