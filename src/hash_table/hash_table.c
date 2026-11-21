#include <hash_table/hash_table.h>
#include <colors.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define FNV_PRIME 1099511628211
#define FNV_OFFSET 1469598103934665603

#define FILL_FACTOR 0.65

#define TOMBSTONE (char*)1

bool is_occupied(struct hash_entry entry){
  return (bool)entry.key;
}

bool has_value(struct hash_entry entry){
  return !(entry.key == 0 || entry.key == TOMBSTONE);
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
    h *= FNV_PRIME;
    h = h ^ next_chunk;
  }
  return h;
}

uint64_t hash_int(int input){
  return FNV_OFFSET + input * FNV_PRIME;
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
    i++;
    if(i == length){i = 0;}
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
  printf("removing hash %lx\n", hash);
  int i = hash % table->array_size;
  struct hash_entry* entry;

  while(is_occupied(*(entry = table->array + i))){
    printf("comparing to hash %lx\n", entry->hash);
    if(hash == entry->hash && !strcmp(key, entry->key)){
      free(entry->key);
      int next = (i == table->array_size)? 0 : i + 1;
      if(is_occupied(table->array[next])){
        entry->key = TOMBSTONE;
        
      }
      else{
        entry->key = 0;
        int j = (i == 0 ? table->array_size : i) - 1;
        while(table->array[j].key == TOMBSTONE){
          table->array[j].key = 0;
          j = (j == 0 ? table->array_size : j) - 1;
        }
      }
      
      entry->value = 0;
      entry->hash = 0;
    }
    i++;
    if(i == table->array_size){i = 0;}
  }
}



int* get_value_from_key(hash_table_t* table, const char* key){
  hash_t hash =  hash_string(key);
  int i = hash % table->array_size;
  struct hash_entry entry;
  while(is_occupied(entry = table->array[i])){
    if(hash == entry.hash && strcmp(key, entry.key)){
      return &(table->array[i].value);
    }
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

