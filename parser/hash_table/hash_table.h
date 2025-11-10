#pragma once



struct hash_entry{
  struct hash_entry* next;
  struct hash_entry* prev;
  int value;
  union{
    char* name;
    int id;
  };
};

typedef struct{
  int array_size;
  int key_count;
  float max_fill_factor;
  struct hash_entry** array;
} hash_table_t;

unsigned long long hash_fn(char *input);

hash_table_t init_hash_table(int array_size, float max_fill_factor);

void push_key_value(hash_table_t *record, char *key, int value);

void push_int_value(hash_table_t *record, int key, int value);

void remove_key_value(hash_table_t *record, char* key);

void remove_int_value(hash_table_t *record, int key);

int *get_value_from_key(hash_table_t *record, char *key);

int *get_value_from_int(hash_table_t *record, int key);

void destroy_hash_table(hash_table_t *record);