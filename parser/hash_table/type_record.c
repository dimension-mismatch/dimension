#include "type_record.h"
#include "../type_utils/type_utils.h"
#include <stdlib.h>

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

int type_record_get_type_id(type_record_t *record, char *typename){
  int* idx = get_value_from_key(&(record->table), typename);
  if(idx == NULL){
    return -1;
  }
  return *idx;
}

void destroy_type_record(type_record_t *record){
  for(int i = 0; i < record->table.key_count; i++){
    typedec_destroy(record->array[i]);
  }
  free(record->array);
  destroy_hash_table(&record->table);
}
