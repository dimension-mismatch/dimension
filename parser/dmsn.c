#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tokenizer.h"
#include "colors.h"
#include "parser.h"
#include "hash_table/pattern_trie.h"
#include "construct_utils.h"
#include "error_handling/error_manager.h"
#include "token_cursor.h"

#include "compiler/compiler.h"



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
  

  pattern_trie_t type_trie = pattern_trie_init();
  pattern_trie_t fn_trie = pattern_trie_init();

  print_token_array(all_tokens);
  error_manager_t errors = error_manager_init(all_tokens);

  token_cursor_t tc = tc_init(all_tokens, &fn_trie, &type_trie, &errors);
  block_t ast = parse_tokens(&tc);

  printf(MAGENTA BOLD "\nType Trie:\n" RESET_COLOR);
  print_pattern_trie(&type_trie);
  printf(MAGENTA BOLD "\nFunction Trie:\n" RESET_COLOR);
  print_pattern_trie(&fn_trie);


  printf("\n");

  
  fclose(file);

  error_printout(&errors);

  if(errors.error_count != 0){
    return 0;
  }

  program_t ir = compile_program(ast, &fn_trie, &type_trie);
  print_program(&ir);
  printf("end of main");
  return 0;
}
