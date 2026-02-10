#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ir-parser.h"
#include "ir-constructs.h"
#include "dmsn-vm.h"
#include "../colors.h"

#include "../hash_table/hash_table.h"




int main(int argc, char* argv[]){
  
  if(argc != 2){
    printf(GREEN BOLD "%s" RESET_COLOR, "DIMENSION IR v0.0.1");
    printf("Usage:  " WHITE BOLD  "ir <filename>.dmsnir" RESET_COLOR);
    exit(1);
  }
  FILE* file;
  file = fopen(argv[1], "r");
  if(file == NULL){
    printf("Could not read file %s", argv[1]);
    exit(1);
  }
  program_t program = parse_ir_file(file);
  execute_program(program);
  

  fclose(file);
}