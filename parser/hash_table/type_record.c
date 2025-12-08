#include "type_record.h"
#include "../type_utils/type_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include "../colors.h"

type_record_t init_type_record(){
  type_record_t new;
  new.table = init_hash_table(67, 0.9);
  new.array = NULL;
  return new;
}


void type_record_push_type(type_record_t *record, type_declaration_t typedec){
  int idx = record->table.key_count;
  record->array = realloc(record->array, (idx + 1) * sizeof(type_declaration_t*));
  record->array[idx] = malloc(sizeof(type_declaration_t));
  *record->array[idx] = typedec;
  push_key_value(&(record->table), typedec.type_name, idx);
}

type_declaration_t* type_record_get_type(type_record_t *record, char* typename){
  int* idx = get_value_from_key(&(record->table), typename);
  if(idx == NULL){
    return NULL;
  }
  return record->array[*idx];
}

int type_record_get_type_number(type_record_t *record, char *typename){
  int* idx = get_value_from_key(&(record->table), typename);
  if(idx == NULL){
    return -1;
  }
  return *idx;
}

type_identifier_t type_record_get_type_id(type_record_t *record, char *typename){
  type_declaration_t* ref = type_record_get_type(record, typename);
  type_identifier_t typeid = typeid_newEmpty();
  if(ref == NULL){
    return typeid;
  }
  typeid.type_number = type_record_get_type_number(record, typename);
  typeid.bit_count = ref->bit_count;
  return typeid;
}

void print_type_id_named(type_identifier_t* typeid, type_record_t* type_rec){
  char* typename = type_rec->array[typeid->type_number]->type_name;
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
    printf("%d" CYAN "[%s]",typeid->dimensions[i], typename);
  }
  else{
    printf(CYAN);
    printf("[%s]", typename);
  }
  printf(RESET_COLOR);
}

void destroy_type_record(type_record_t *record){
  for(int i = 0; i < record->table.key_count; i++){
    typedec_destroy(record->array[i]);
  }
  free(record->array);
  destroy_hash_table(&record->table);
}
