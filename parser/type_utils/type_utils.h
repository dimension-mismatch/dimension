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

typedef struct{
  int type_number;
  int count;   
}serialized_type_entry_t;

typedef struct{
  int num_entries;
  serialized_type_entry_t* entries;
}serialized_type_array_t;



type_identifier_t typeid_newEmpty();

void typeid_pushDimension(type_identifier_t* typeId, unsigned int dimension);

void typeid_multiply(type_identifier_t *typeId, dimension_t multiplicity);

dimension_t dmsn_newEmpty();

void dmsn_pushDimension(dimension_t *dimension, unsigned int d);

dimension_t dmsn_copy(dimension_t *dimension);

void dmsn_destroy(dimension_t *dimension);

int typeid_bytesize(type_identifier_t *typeid);

void typeid_destroy(type_identifier_t *typeId);

type_identifier_t typeid_copy(type_identifier_t* typeId);


type_declaration_t typedec_newEmpty();

void typedec_setName(type_declaration_t* typedec, type_type_t type, char* name);
void typedec_pushEnumOption(type_declaration_t* typedec, char* name, int length);

void typedec_pushMember(type_declaration_t *typedec, type_identifier_t* member, char *name, int length);

void typedec_destroy(type_declaration_t* typedec);

int typeid_compare(type_identifier_t *a, type_identifier_t *b);

void serialize_type(type_identifier_t *typeid, serialized_type_array_t *array);

int serial_type_array_compare(serialized_type_array_t *a, serialized_type_array_t *b);

void serial_type_array_destroy(serialized_type_array_t *array);

void print_type(type_declaration_t *type);

void print_type_id(type_identifier_t *typeid);
