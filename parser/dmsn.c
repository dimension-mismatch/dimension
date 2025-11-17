#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"
#include "string_utils.h"
#include "colors.h"
#include "compiler/compiler.h"

#include "hash_table/type_record.h"
#include "hash_table/function_record.h"
#include "hash_table/variable_record.h"


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
  
  print_token_array(all_tokens);

  printf("Searching for type definitions\n");

  variable_record_t variable_record = variable_record_init();
  type_record_t type_record = init_type_record();
  function_record_t function_record = fn_rec_init();

  
  expression_t* ast = parse_tokens(all_tokens, &type_record, &variable_record, &function_record);

  print_expression(ast);
  destroy_token_array(all_tokens);
  fclose(file);

  if(argc == 3){
    compile_program(argv[2], ast, &function_record, &variable_record, &type_record);
  }
  exp_destroy(ast);
  destroy_type_record(&type_record);
  variable_record_destroy(&variable_record);
  fn_rec_destroy(&function_record);

}
