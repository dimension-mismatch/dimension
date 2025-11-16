#include "hash_table.h"
#include <stdlib.h>
#include <string.h>
#include<stdio.h>

#define FNV_PRIME 1099511628211
#define FNV_OFFSET 1469598103934665603


unsigned long long hash_fn(char* input){
  unsigned long long h = FNV_OFFSET;
  int done = 0;
  while(!done){
    unsigned long long next_chunk = 0;
    for(int i = 0; i < 8; i++){
      if(*input == '\0'){
        done = 1;
        break;
      }
      next_chunk = (next_chunk << 8) | *input;
      input++;
    }
    h *= FNV_PRIME;
    h = h ^ next_chunk;
  }
  return h;
}

unsigned long long hash_int(int input){
  return FNV_OFFSET + input * FNV_PRIME;
}

hash_table_t init_hash_table(int array_size, float max_fill_factor){
  hash_table_t record = {.array_size = array_size, .key_count = 0, .max_fill_factor = max_fill_factor};
  record.array = calloc(array_size, sizeof(struct hash_entry*));
  return record;
}

void destroy_hash_table(hash_table_t *record){
  for(int i = 0; i < record->array_size; i++){
    if(record->array[i] != NULL){
      struct hash_entry* ptr = record->array[i];
      while(ptr->next != NULL){
        ptr = ptr->next;
      }
      while(ptr->prev != NULL){
        struct hash_entry* prev = ptr->prev;
        free(ptr->name);
        free(ptr);
        ptr = prev;
      }
    }
  }
  if(record->array != NULL){
    free(record->array);
  }
  record->array = NULL;
  record->key_count = 0;
}



void push_hash_node(hash_table_t* record, unsigned long long hash, struct hash_entry* new_node, char* name, int* id){
  int idx = hash % record->array_size;
  struct hash_entry* ptr = record->array[idx];
  if(ptr == NULL){
    record->array[idx] = new_node;
  }
  else{
    if(name != NULL && strcmp(name, ptr->name)){
      return;
    }
    if(id != NULL && *id == ptr->id){
      return;
    }
    while(ptr->next != NULL){
      ptr = ptr->next;
    }
    new_node->prev = ptr;
    ptr->next = new_node;
  }
  record->key_count++;
}

void push_key_value(hash_table_t* record, char* key, int value){
  struct hash_entry* new_node = malloc(sizeof(struct hash_entry));
  new_node->next = NULL;
  new_node->prev = NULL;
  new_node->value = value;
  new_node->name = malloc((strlen(key) + 1) * sizeof(char));
  strcpy(new_node->name, key);
  push_hash_node(record, hash_fn(key), new_node, key, NULL);
}

void push_int_value(hash_table_t* record, int key, int value){
  struct hash_entry* new_node = malloc(sizeof(struct hash_entry));
  new_node->next = NULL;
  new_node->prev = NULL;
  new_node->value = value;
  new_node->id = key;
  push_hash_node(record, hash_int(key), new_node, NULL, &key);
}


void remove_key_value(hash_table_t *record, char *key){
  if(record == NULL || key == NULL){
    return;
  }
  int idx = hash_fn(key) % record->array_size;
  struct hash_entry* ptr = record->array[idx];
  if(ptr == NULL){
    return;
  }
  while(strcmp(key, ptr->name)){
    ptr = ptr->next;
    if(ptr == NULL){
      return;
    }
  }
  if(ptr->prev){
    ptr->prev->next = ptr->next;
  }
  else{
    record->array[idx] = ptr->next;
  }
  free(ptr->name);
  free(ptr);
  ptr = NULL;
  record->key_count--;
}

void remove_int_value(hash_table_t* record, int key){
  int idx = hash_int(key) % record->array_size;
  struct hash_entry* ptr = record->array[idx];
  if(ptr == NULL){
    return;
  }
  while(ptr->id != key){
    ptr = ptr->next;
    if(ptr == NULL){
      return;
    }
  }
  ptr->prev->next = ptr->next;
  free(ptr->name);
  free(ptr);
  ptr = NULL;
  record->key_count--;
}

int* get_value_from_key(hash_table_t* record, char* key){
  int idx = hash_fn(key) % record->array_size;
  struct hash_entry* ptr = record->array[idx];
  if(ptr == NULL){
    return NULL;
  }
  while(strcmp(key, ptr->name)){
    ptr = ptr->next;
    if(ptr == NULL){
      return NULL;
    }
  }
  return &(ptr->value);
}

int* get_value_from_int(hash_table_t* record, int key){
  int idx = hash_int(key) % record->array_size;
  struct hash_entry* ptr = record->array[idx];
  if(ptr == NULL){
    return NULL;
  }
  while(ptr->id != key){
    ptr = ptr->next;
    if(ptr == NULL){
      return NULL;
    }
  }
  return &(ptr->value);
}


