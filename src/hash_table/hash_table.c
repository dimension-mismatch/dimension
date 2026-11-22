#include <hash_table/hash_table.h>
#include <colors.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define FNV_PRIME 1099511628211
#define FNV_OFFSET 1469598103934665603

#define FILL_FACTOR 0.65

#define TOMBSTONE 1

bool is_occupied(struct hash_entry entry){
  return (bool)entry.hash;
}

bool has_value(struct hash_entry entry){
  return !(entry.hash == 0 || entry.hash == TOMBSTONE);
}

int inc_index(int i, int length){
  return (++i == length)? 0 : i;
}

int dec_index(int i, int length){
  return ((i == 0)? length : i) - 1;
}

hash_t hash_string(const char* input){
  hash_t h = FNV_OFFSET;
  int done = 0;
  while(!done){
    hash_t next_chunk = 0;
    for(int i = 0; i < 8; i++){
      if(*input == '\0'){
        done = 1;
        break;
      }
      next_chunk = (next_chunk << 8) | *input;
      input++;
    }
    h = h ^ next_chunk;
    h *= FNV_PRIME;

  }
  return (h < 2)? 2 : h;
}

uint64_t hash_int(int input){
  hash_t h = FNV_OFFSET + input * FNV_PRIME;
  return (h < 2)? 2 : h;
}

void init_hash_table(hash_table_t* table){
  table->array = NULL;
  table->array_size = 0;
  table->key_count = 0;
}


void destroy_hash_table(hash_table_t *table){
  for(int i = 0; i < table->array_size; i++){
    struct hash_entry entry = table->array[i];
    if(has_value(entry)){
      free(entry.key);
    }
  }
  free(table->array);
  table->array = NULL;
  table->key_count = 0;
}




void insert_hash_entry(struct hash_entry* array, int length, struct hash_entry entry){
  int i = entry.hash % length;
  while(is_occupied(array[i])){
    if(entry.hash == array[i].hash && !strcmp(entry.key, array[i].key)){

    }
    i = inc_index(i, length);
  }
  array[i] = entry;
}

void rehash_table(hash_table_t* table, int size){
  struct hash_entry* new_array = calloc(size, sizeof(struct hash_entry));
  for(int i = 0; i < table->array_size; i++){
    struct hash_entry entry = table->array[i];
    if(has_value(entry)){
      insert_hash_entry(new_array, size, entry);
    }
  }
  free(table->array);
  table->array = new_array;
  table->array_size = size;
}

void hash_key_value(hash_table_t* table, const char* key, int value){
  table->key_count++;
  struct hash_entry new_entry = {.hash = hash_string(key), .key = strdup(key), .value = value};
  if(table->array_size == 0){
    table->array_size = 8;
    table->array = calloc(table->array_size, sizeof(struct hash_entry));
  }
  else if(table->key_count > table->array_size * FILL_FACTOR){
    rehash_table(table, 2 * table->array_size);
  }
  insert_hash_entry(table->array, table->array_size, new_entry);
}


void remove_key_value(hash_table_t *table, const char *key){
  hash_t hash = hash_string(key);
  int i = hash % table->array_size;
  struct hash_entry* entry;

  while(is_occupied(*(entry = table->array + i))){
    if(hash == entry->hash && !strcmp(key, entry->key)){
      free(entry->key);
      int next = inc_index(i, table->array_size);
      if(is_occupied(table->array[next])){
        entry->hash = TOMBSTONE;
        
      }
      else{
        entry->hash = 0;
        int j = dec_index(i, table->array_size);
        while(table->array[j].hash == TOMBSTONE){
          table->array[j].hash = 0;
          j = dec_index(j, table->array_size);
        }
      }
      
      entry->value = 0;
      entry->key = 0;
    }
    i = inc_index(i, table->array_size);
  }
}



int* get_value_from_key(hash_table_t* table, const char* key){
  hash_t hash = hash_string(key);
  int i = hash % table->array_size;
  struct hash_entry entry;
  while(is_occupied(entry = table->array[i])){
    if(hash == entry.hash && !strcmp(key, entry.key)){
      return &(table->array[i].value);
    }
    i = inc_index(i, table->array_size);
  }
  return NULL;
}

void print_hash_table(hash_table_t* table){
  for(int i = 0; i < table->array_size; i++){
    struct hash_entry entry = table->array[i];
    printf("%i | ", i);
    if(is_occupied(entry)){
      if(has_value(entry)){
        printf("%s (%lx) -> %i", entry.key, entry.hash, entry.value);
      }
      else{
        printf(BLACK "[tombstone]" RESET_COLOR);
      }
    }
    else{
      printf(BLACK "[empty]" RESET_COLOR);
    }
    printf("\n");
  }
}

void init_hash_table_from_array(hash_table_t* table, const char* array[], int length){
  table->array_size = length / FILL_FACTOR;
  table->array = calloc(table->array_size, sizeof(struct hash_entry));
  for(int i = 0; i < length; i++){
    hash_key_value(table, array[i], i);
  }
}

void destroy_int_hash_table(hash_table_t *table){
  free(table->array);
  table->array = NULL;
  table->key_count = 0;
}

void hash_int_value(hash_table_t *table, int key, int value){
  table->key_count++;
  struct hash_entry new_entry = {.hash = hash_int(key), .int_key = key, .value = value};
  if(table->array_size == 0){
    table->array_size = 8;
    table->array = calloc(table->array_size, sizeof(struct hash_entry));
  }
  else if(table->key_count > table->array_size * FILL_FACTOR){
    rehash_table(table, 2 * table->array_size);
  }
  insert_hash_entry(table->array, table->array_size, new_entry);
}

void remove_int_value(hash_table_t *table, int key){

}