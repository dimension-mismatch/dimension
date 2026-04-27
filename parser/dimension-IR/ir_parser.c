#include "ir_parser.h"
#include "ir_constructs.h"
#include "../hash_table/hash_table.h"
#include "../token_cursor.h"
#include <stdio.h>
#include <stdbool.h>


parse_instruction(token_cursor_t* base_tc, instruction_t* result){

}

bool parse_ir_block(token_cursor_t* base_tc, ir_block_t* result){

}

program_t* parse_ir(token_cursor_t* base_tc, program_t* result){
  hash_table_t instruction_table = init_hash_table_from_array(67, 0.9, instruction_names, 20);
  hash_table_t register_table = init_hash_table(67, 0.9);
  hash_table_t label_table = init_hash_table(67, 0.9);

}

