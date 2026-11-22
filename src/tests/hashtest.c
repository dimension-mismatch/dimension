#include <hash_table/hash_table.h>
#include <stdio.h>

int main(){
    hash_table_t table;
    init_hash_table(&table);

    hash_key_value(&table, "hello", 5);
    hash_key_value(&table, "835", 2);
    hash_key_value(&table, "17", 3);
    hash_key_value(&table, "a", 45);
    hash_key_value(&table, "b", 67);
    hash_key_value(&table, "c", 89);

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

    destroy_hash_table(&table);
}