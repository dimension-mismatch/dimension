#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef uint64_t hash_t;

struct hash_entry{
  hash_t hash;
  int value;
  union{
    char* key;
    int int_key;
  };
};



typedef struct{
  int array_size;
  int key_count;
  struct hash_entry* array;
} hash_table_t;

typedef hash_table_t int_hash_table_t;



hash_t hash_string(const char* input);

void init_hash_table(hash_table_t* table);

void init_hash_table_from_array(hash_table_t* table, const char* const array[], int length);

void hash_key_value(hash_table_t *table, const char *key, int value);

void remove_key_value(hash_table_t *table, const char* key);



int *get_value_from_key(hash_table_t *table, const char *key);




void destroy_hash_table(hash_table_t *table);



void print_hash_table(hash_table_t *table);



void destroy_int_hash_table(int_hash_table_t* table);

void hash_int_value(int_hash_table_t *table, int key, int value);

void remove_int_value(int_hash_table_t *table, int key);

int *get_value_from_int(int_hash_table_t *table, int key);