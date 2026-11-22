#include <hash_table/hash_table.h>
#include <stdio.h>

int main(){
    hash_table_t table;
    const char* strings[6] = {"hello", "835", "17", "a", "b", "the quick brown fox jumps over the lazy dog"};
    init_hash_table_from_array(&table, strings, 6);

    remove_key_value(&table, "835");
    remove_key_value(&table, "hello");
    remove_key_value(&table, "9");

    int* b = get_value_from_key(&table, "b");
    if(b){
        printf("b : %i\n", *b);
    }
    else{
        printf("\"b\" not in hash table. \n");
    }

    int* k = get_value_from_key(&table, "k");
    if(k){
        printf("k : %i\n", *b);
    }
    else{
        printf("\"k\" not in hash table. \n");
    }

    print_hash_table(&table);

    int_hash_table_t int_table;
    init_hash_table(&int_table);

    hash_int_value(&int_table, 7, 67);
    hash_int_value(&int_table, 8, 68);

    int* i = get_value_from_int(&int_table, 8);
    if(i){
        printf("%i : %i\n", 8, *i);
    }
    else{
        printf("8 not in hash table \n");
    }

    remove_int_value(&int_table, 8);

    i = get_value_from_int(&int_table, 8);
    if(i){
        printf("%i : %i\n", 8, *i);
    }
    else{
        printf("8 not in hash table \n");
    }

    destroy_hash_table(&table);
    destroy_int_hash_table(&int_table);
}