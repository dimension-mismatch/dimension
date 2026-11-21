#include <hash_table/hash_table.h>


int main(){
    hash_table_t table;
    init_hash_table(&table);

    hash_key_value(&table, "hello", 5);
    hash_key_value(&table, "835", 2);
    hash_key_value(&table, "17", 3);
    hash_key_value(&table, "a", 45);
    hash_key_value(&table, "b", 67);
    hash_key_value(&table, "c", 89);

    remove_key_value(&table, "c");
    remove_key_value(&table, "b");
    remove_key_value(&table, "835");

    print_hash_table(&table);

    destroy_hash_table(&table);
}