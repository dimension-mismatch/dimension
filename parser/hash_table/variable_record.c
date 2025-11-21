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
int var_record_inc_byte_offset(variable_record_t* record, int byte_count){
  struct scope* current_scope = record->scopes + record->scope_depth - 1;
  current_scope->variable_count++;
  current_scope->local_byte_offset += byte_count;
  return current_scope->local_byte_offset;
}

int var_record_inc_param_byte_offset(variable_record_t* record, int byte_count){
  struct scope* current_scope = record->scopes + record->scope_depth - 1;
  current_scope->variable_count++;
  int ret = current_scope->parameter_byte_offset;
  current_scope->parameter_byte_offset -= byte_count;
  return ret;
}

variable_t variable_record_push_var(variable_record_t* record, char *name, type_identifier_t *type, int byte_offset){
  int idx = record->table.key_count;
  record->variables = realloc(record->variables, (idx + 1) * sizeof(variable_t));
  variable_t new = {typeid_newEmpty(), NULL};
  new.name = malloc((strlen(name) + 1) * sizeof(char));
  strcpy(new.name, name); 
  new.type = *type;
  push_key_value(&(record->table), new.name, idx);
  new.local_byte_offset = byte_offset;
  record->variables[idx] = new;
  return new;
}

variable_t variable_record_push_new(variable_record_t *record, char *name, type_identifier_t *type){
  variable_t new = variable_record_push_var(record, name, type, var_record_inc_byte_offset(record, typeid_bytesize(type)));
  return new;
}

variable_t variable_record_push_param(variable_record_t *record, char *name, type_identifier_t *type){
  variable_t new = variable_record_push_var(record, name, type, var_record_inc_param_byte_offset(record, typeid_bytesize(type)));
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

int* variable_record_get_byte_offset(variable_record_t *record, char *name){
  int* idx = get_value_from_key(&(record->table), name);
  if(!idx){
    return NULL;
  }
  return &(record->variables[*idx].local_byte_offset);
}


void variable_record_scope_in(variable_record_t *record){
  record->scope_depth++;
  record->scopes = realloc(record->scopes, record->scope_depth * sizeof(struct scope));
  struct scope* current_scope = record->scopes + record->scope_depth - 1;
  current_scope->variable_count = 0;
  current_scope->local_byte_offset = 0;
  current_scope->parameter_byte_offset = -8;
}

void variable_record_scope_out(variable_record_t* record){
  record->scope_depth--;
  struct scope* current_scope = record->scopes + record->scope_depth;

  if(current_scope->variable_count != 0){
    for(int i = 0, j = record->table.key_count - 1; i < current_scope->variable_count; i++, j--){
      remove_key_value(&(record->table), record->variables[j].name);
    }
    record->variables = realloc(record->variables, record->table.key_count * sizeof(variable_t));
  }
  record->scopes = realloc(record->scopes, record->scope_depth * sizeof(struct scope));
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
    printf("#%d @ byte %d: ", i, record->variables[i].local_byte_offset);
    print_variable(&(record->variables[i]));
    printf("\n");
  }
}
