#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"
#include "string_utils.h"
#include "colors.h"
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
  
  print_token_array(all_tokens);

  printf("Searching for type definitions\n");
  parse_tokens(all_tokens);

  destroy_token_array(all_tokens);
  fclose(file);

  if(argc == 3){
    compile_expression(argv[2], NULL);
  }

}
