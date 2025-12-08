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

#include "error_handling/error_manager.h"

#include "validator/validator.h"


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
  
  variable_record_t variable_record = variable_record_init();
  type_record_t type_record = init_type_record();
  function_record_t function_record = fn_rec_init();
  
  token_array_t* all_tokens = tokenize_file(file);
  parse_manager_t errors = parse_manager_init(all_tokens, &function_record, &variable_record, &type_record);

  expression_t* ast = parse_tokens(&errors);
  validate_program(ast, &errors);

  // print_variable_record(&variable_record);
  // print_expression(ast);
  
  fclose(file);

  error_printout(&errors);
  if(errors.error_count > 0){
    exit(1);
  }
  if(argc == 3 && errors.error_count == 0){
    compile_program(argv[2], ast, &function_record, &variable_record, &type_record);
  }

  destroy_token_array(all_tokens);
  exp_destroy(ast);
  destroy_type_record(&type_record);
  variable_record_destroy(&variable_record);
  fn_rec_destroy(&function_record); 

}
