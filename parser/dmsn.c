#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tokenizer.h"
#include "colors.h"
#include "parser.h"
#include "hash_table/pattern_trie.h"
#include "construct_utils.h"




char* message = "  ╔═╗             DIMENSION [v0.0.1]\n╔═╝ ║╔═╗╔═════╗\n║ ║ ║║ ║║ ║ ║ ║\n╠═══╣╠═╩╩╦╬═╩═╣\n║ ═ ║║ ║ ║║ ══╣\n║ ══╣║ ║ ║╠══ ║\n╠═╦╦╩╩═╩═╣╠═══╣ \n║ ║║ ═══ ║║ ║ ║\n╚═╝╚═════╝╚═╩═╝\n";
int main(int argc, char* argv[]){
  if(argc != 2 && argc != 3){
    printf(GREEN BOLD "%s" RESET_COLOR, message);
    printf("To compile a .dmsn file, run " WHITE BOLD  "dmsn <filename>.dmsn" RESET_COLOR);
    exit(1);
  }
  
  FILE* file;
  file = fopen(argv[1], "r");
  if(file == NULL){
    printf("Could not read file %s", argv[1]);
    exit(1);
  }
  
  
  token_array_t* all_tokens = tokenize_file(file);
  //print_token_array(all_tokens);
  //parse_tokens(all_tokens);

  pattern_trie_t trie = pattern_trie_init();
  pattern_entry_t test_entry = {.is_identifier = true, .identifier = "test"};
  pattern_entry_t test_entry2 = {.is_identifier = false, .variable = {.constant_lvl = 2, .name = "my_var", .type = {.base_type_id = 0, .dimension_count = 0, .dimensions = NULL, .is_param = false}}};
  pattern_entry_t test_entry3 = {.is_identifier = true, .identifier = "*.a"};
  pattern_entry_t test_entries[] = {test_entry, test_entry2, test_entry3};
  pattern_entry_t test_entriesII[] = {test_entry, test_entry3};
  pattern_t test_pattern = {.entry_count = 3, .entries = test_entries};
  pattern_t test_patternII = {.entry_count = 2, .entries = test_entriesII};
  type_declaration_t test_type = {.component_count = 0, .components = NULL, .is_enum = false, .is_is = false, .match_pattern = &test_pattern};
  type_declaration_t test_typeII = {.component_count = 0, .components = NULL, .is_enum = false, .is_is = false, .match_pattern = &test_patternII};
  print_type_declaration(&test_type);
  pattern_trie_push_type(&trie, &test_type);
  pattern_trie_push_type(&trie, &test_typeII);
  printf("\n");
  print_pattern_trie(&trie);

  // parse_manager_t errors = parse_manager_init(all_tokens, &function_record, &variable_record, &type_record);

  // expression_t* ast = parse_tokens(&errors);
  // validate_program(ast, &errors);

  // print_variable_record(&variable_record);
  // print_expression(ast);
  
  fclose(file);

  // error_printout(&errors);
  // if(errors.error_count > 0){
  //   exit(1);
  // }
  // if(argc == 3 && errors.error_count == 0){
  //   compile_program(argv[2], ast, &function_record, &variable_record, &type_record);
  // }

  // destroy_token_array(all_tokens);
  // exp_destroy(ast);

}
