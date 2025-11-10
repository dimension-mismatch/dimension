#include "variable_record.h"
#include "hash_table.h"
#include "../colors.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

variable_record_t variable_record_init(){
  variable_record_t new;
  new.scope_depth = 0;
  new.scopes = NULL;
  new.variables = NULL;
  new.table = init_hash_table(67, 0.9);
  variable_record_scope_in(&new);
  return new;
}


variable_t variable_record_push_new(variable_record_t *record, char *name, type_identifier_t *type){
  int idx = record->table.key_count;
  record->variables = realloc(record->variables, (idx + 1) * sizeof(variable_t));
  variable_t new = {typeid_newEmpty(), NULL};
  new.name = malloc((strlen(name) + 1) * sizeof(char));
  strcpy(new.name, name); 
  new.type = *type;
  record->scopes[record->scope_depth - 1]++;
  record->variables[idx] = new;
  push_key_value(&(record->table), new.name, idx);
  return new;
}

variable_t *variable_record_get(variable_record_t *record, char *name){
  int* idx = variable_record_get_index(record, name);
  if(idx == NULL){
    return NULL;
  }
  return variable_record_get_by_index(record, *idx);
}

variable_t *variable_record_get_by_index(variable_record_t *record, int index){
  return &(record->variables[index]);
}

int* variable_record_get_index(variable_record_t *record, char *name){
  return get_value_from_key(&(record->table), name);
}

void variable_record_scope_in(variable_record_t *record){
  record->scope_depth++;
  record->scopes = realloc(record->scopes, record->scope_depth * sizeof(int));
}

void variable_record_scope_out(variable_record_t* record){
  record->scope_depth--;
  for(int i = 0, j = record->scope_depth; i < record->scopes[record->scope_depth]; i++, j--){
    remove_key_value(&(record->table), record->variables[j].name);
  }
  record->variables = realloc(record->variables, record->scopes[record->scope_depth] * sizeof(variable_t));
  record->scopes = realloc(record->scopes, record->scope_depth * sizeof(int));
  
}

void variable_record_destroy(variable_record_t *record){
  for(int i = 0; i < record->table.key_count; i++){
    typeid_destroy(&(record->variables[i].type));
    free(record->variables[i].name);
  }
  
  destroy_hash_table(&(record->table));
  free(record->scopes);
  record->scopes = NULL;
  
  free(record->variables);
  record->variables = NULL;
}

void print_variable(variable_t* var){
  if(var == NULL){
    printf(RED "NULL : [NULL]" RESET_COLOR);
    return;
  }
  printf(GREEN "%s" WHITE " : ", var->name);
  print_type_id(&(var->type));
}
void print_variable_record(variable_record_t* record){
  for(int i = 0; i < record->table.key_count; i++){
    printf("#%d : ", i);
    print_variable(&(record->variables[i]));
    printf("\n");
  }
}
