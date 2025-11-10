#pragma once

typedef enum{
  NO_TYPE,
  ENUM_TYPE,
  COMP_TYPE,
  BASE_TYPE,
}type_type_t;

typedef struct{
  int dimension_count;
  unsigned int* dimensions;
} dimension_t;

typedef struct{
  int type_number;
  int dimension_count;
  unsigned int* dimensions;
  int bit_count;
}type_identifier_t;

typedef struct type_declaration{
  char* type_name;
  type_type_t type_type;
  int bit_count;
  union{
    struct{
      int option_count;
      char** options;
    } enumeration;
    struct{
      int member_count;
      char** member_names;
      type_identifier_t* members; 
    } composition;
    struct{

    } builtin;
  };
}type_declaration_t;



type_identifier_t typeid_newEmpty();

void typeid_pushDimension(type_identifier_t* typeId, unsigned int dimension);

void typeid_multiply(type_identifier_t *typeId, dimension_t multiplicity);

dimension_t dmsn_newEmpty();

void dmsn_pushDimension(dimension_t *dimension, unsigned int d);

dimension_t dmsn_copy(dimension_t *dimension);

void typeid_destroy(type_identifier_t* typeId);

type_identifier_t typeid_copy(type_identifier_t* typeId);


type_declaration_t typedec_newEmpty();

void typedec_setName(type_declaration_t* typedec, type_type_t type, char* name);
void typedec_pushEnumOption(type_declaration_t* typedec, char* name, int length);

void typedec_pushMember(type_declaration_t *typedec, type_identifier_t* member, char *name, int length);

void typedec_destroy(type_declaration_t* typedec);

void print_type(type_declaration_t* type);

void print_type_id(type_identifier_t *typeid);
