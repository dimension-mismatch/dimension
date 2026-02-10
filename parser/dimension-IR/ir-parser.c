#include "ir-parser.h"
#include "../colors.h"
#include <stdio.h>
#include <stdlib.h>
#include "../hash_table/hash_table.h"

unsigned int instruction_count = 15;
char* instruction_names[] = {"move", "deref", "+", "-", "i*", "u*", "i/", "u/", "f+", "f-", "and", "or", "not", "jump", "jumpif"};
#define JUMP 14

void print_value(value_t val){
  if(val.is_fixed){
    printf(RED BOLD "@" RESET_COLOR);
  }
  else{
    printf(BLUE BOLD "~" RESET_COLOR);
  }
  switch(val.type){
    case LITERAL:
      printf(WHITE BOLD "%llu" RESET_COLOR, val.data);
      break;
    case VARIABLE:
      printf(YELLOW BOLD "v%llu" RESET_COLOR, val.data);
      break;
    case PARAMETER:
      printf(BLUE BOLD "p%llu" RESET_COLOR, val.data);
      break;
    case RESULT:
      printf(CYAN BOLD "~%llu" RESET_COLOR, val.data);
      break;
    case ARGUMENT:
      printf(MAGENTA BOLD "a%llu" RESET_COLOR, val.data);
      break;
  }
}

void print_instruction(instruction_t* ins){  
  printf(GREEN BOLD "%s " RESET_COLOR "(%ub) ", instruction_names[ins->opcode - 1], ins->datasize);
  print_value(ins->arg1);
  printf(" ");
  if(ins->opcode != JUMP){
    print_value(ins->arg2);
  }
  if(ins->has_label){
    printf(CYAN " : ~%i" RESET_COLOR, ins->label);
  }
}

void print_program(program_t* program, int indent){
  for(int i = 0; i < program->length; i++){
    instruction_t* ins = program->instructions + i;
    if(ins->opcode == 0){
      for(int j = 0; j < indent ; j++){ printf("  "); }
      printf("%i {\n", i);

      print_program(ins->block, indent + 1);

      for(int j = 0; j < indent; j++){ printf("   "); }
      printf("  } " MAGENTA "* " RESET_COLOR);
      print_value(ins->block->multiplier);
      printf("\n");
    }
    else{
      for(int j = 0; j < indent; j++){ printf("   "); }
      printf("%i ", i);
      print_instruction(ins);
      printf("\n");
    }
  }
}

bool read_text(FILE* file, char** result){
  char ch;
  *result = NULL;
  unsigned int length = 0;
  while(((ch = fgetc(file)) >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-'){
    length++;
    *result = realloc(*result, length * sizeof(char));
    (*result)[length - 1] = ch;
  }
  if(length == 0){
    return false;
  }
  *result = realloc(*result, (length + 1) * sizeof(char));
  (*result)[length] = '\0';
  return true;
}


bool read_label(FILE* file, char** result){
  char ch;
  *result = NULL;
  if((ch = fgetc(file)) != '~' && ch != '@'){
    ungetc(ch, file);
    return false;
  }
  if(!read_text(file, result)){
    return false;
  }
  return true;
}


bool read_marker(FILE* file, char** result){
  char ch;
  *result = NULL;
  unsigned int length = 0;
  if((ch = fgetc(file)) != '.'){
    ungetc(ch, file);
    return false;
  }
  if(!read_text(file, result)){
    return false;
  }
  return true;
}

void consume_whitespace(FILE* file){
  char ch;
  while((ch = fgetc(file)) == ' ' || ch == '\n'){}
  ungetc(ch, file);
}

bool read_value(FILE* file, value_t* result, hash_table_t* reg_table){
  char ch = fgetc(file);
  result->data = 0;
  result->is_fixed = false;
  if(ch >= '0' && ch <= '9'){
    result->type = LITERAL;
    result->data = ch - '0';
    result->is_fixed = true;
  }
  else if(ch == 'v' || ch == 'V'){
    result->is_fixed = (ch == 'V');
    result->type = VARIABLE;
  }
  else if(ch == 'p' || ch == 'P'){
    result->is_fixed = (ch == 'P');
    result->type = PARAMETER;
  }
  else if(ch == '~' || ch == '@'){
    result->is_fixed = (ch == '@');
    result->type = RESULT;
    char* label = NULL;
    if(!read_text(file, &label)){
      printf("could not read a register name");
      exit(0);
      return false;
    }
    int* reg = get_value_from_key(reg_table, label);
    if(!reg){
      printf("register '%s' does not exist in this scope", label);
      exit(0);
      return false;
    }
    result->data = *reg;
    return true;
  }
  else if(ch == 'a' || ch == 'A'){
    result->type = ARGUMENT;
    result->is_fixed = (ch == 'A');
  }
  else{
    return false;
  }
  while((ch = fgetc(file)) >= '0' && ch <= '9'){
    result->data *= 10;
    result->data += ch - '0';
  }
  if(ch == ' ' || ch == '\n'){
    ungetc(ch, file);
  }
  else{
    return false;
  }
  return true;
}

bool read_instruction(FILE* file, hash_table_t* instruction_table, unsigned int* result){
  char ch; 
  char* name = NULL;
  int len = 0;
  while((ch = fgetc(file)) != ' ' && ch != '\n' && ch != EOF && ch != '('){
    len++;
    name = realloc(name, (len) * sizeof(char));
    name[len - 1] = ch;
  }
  name = realloc(name, (len + 1) * sizeof(char));
    name[len] = '\0';

  int* match = get_value_from_key(instruction_table, name);
  free(name);
  if(!match){
    return false;
  }
  *result = *match;
  return true;
}

bool read_size_info(FILE* file, instruction_t* result){
  result->datasize = 0;
  char ch = fgetc(file);
  if(ch != '('){
    ungetc(ch, file);
    return false;
  }
  while((ch = fgetc(file)) >= '0' && ch <= '9'){
    result->datasize *= 10;
    result->datasize += ch - '0';
  }
  if(ch == ')'){
    return true;
  }
  return false;
}

bool parse_program(FILE* file, program_t* result, hash_table_t* instruction_table, hash_table_t* reg_table, unsigned int* reg_counter){
  consume_whitespace(file);
  hash_table_t marker_table = init_hash_table(67, 0.9);
  program_t program = {.instructions = NULL, .length = 0, .multiplier = {.type = LITERAL, .data = 1}};
  char ch;
  while(true){
    ch = fgetc(file);
    if(ch == EOF){
      break;
    }
    if(ch == '}'){
      consume_whitespace(file);
      if((ch = fgetc(file)) == '*'){
        consume_whitespace(file);
        if(!read_value(file, &program.multiplier, reg_table)){
          printf("failed to read multiplier value");
          exit(0);
        }
      }
      else{
        ungetc(ch, file);
      }
      break;
    }
    if(ch == '{'){
      instruction_t ins = {.opcode = 0, .block = malloc(sizeof(program_t))};
      parse_program(file, ins.block, instruction_table, reg_table, reg_counter);
      program.instructions = realloc(program.instructions, ++program.length * sizeof(instruction_t));
      program.instructions[program.length - 1] = ins;
      consume_whitespace(file);
      continue;
    }
    ungetc(ch, file);
    char* marker = NULL;
    if(read_marker(file, &marker)){
      push_key_value(&marker_table, marker, program.length);
      consume_whitespace(file);
      continue;
    }
    
    instruction_t ins = {};
    char* label;
    if(read_label(file, &label)){
      ins.has_label = true;
      if(get_value_from_key(reg_table, label)){
        printf("Register " CYAN "~%s" RESET_COLOR " is already used in this scope", label);
        exit(0);
        return false;
      }
      push_key_value(reg_table, label, (*reg_counter)++);
      ins.label = program.length;
    }
    consume_whitespace(file);
    if(!read_instruction(file, instruction_table, &ins.opcode)){
      printf("could not read instruction");
      exit(0);
    }
    consume_whitespace(file);
    if(ins.opcode == JUMP){
      char* marker = NULL;
      if(!read_marker(file, &marker)){
        printf("Jump instruction requires a label to jump to");
        exit(0);
        return false;
      }
      value_t arg = {.type = LITERAL, .text = marker, .is_fixed = true};
      ins.arg1 = arg;
    }
    else{
      if(!read_size_info(file, &ins)){
        printf("missing size info");
        exit(0);
        return false;
      }

      consume_whitespace(file);
      if(!read_value(file, &ins.arg1, reg_table)){
        printf("could not read a value");
        exit(0);
        return false;
      }
      consume_whitespace(file);
      if(!read_value(file, &ins.arg2, reg_table)){
        printf("could not read a value");
        exit(0);
        return false;
      }
    }

    
    consume_whitespace(file);
    program.instructions = realloc(program.instructions, ++program.length * sizeof(instruction_t));
    program.instructions[program.length - 1] = ins;
  }

  for(int i = 0; i < program.length; i++){
    instruction_t* ins = program.instructions + i;
    if(ins->opcode == JUMP){
      int* match = get_value_from_key(&marker_table,  ins->arg1.text);
      if(!match){
        printf("Cannot jump to marker \"%s\" because it doesn't exist", ins->arg1.text);
        exit(0);
      }
      free(ins->arg1.text);
      ins->arg1.text = NULL;
      ins->arg1.data = *match;
    }
    
  }

  destroy_hash_table(&marker_table);
  

  *result = program;
  return true;
}

program_t parse_ir_file(FILE* file){
  hash_table_t instruction_table = init_hash_table(67, 0.9);
  for(int i = 0; i < instruction_count; i++){
    push_key_value(&instruction_table, instruction_names[i], i + 1);
  }
  hash_table_t reg_table = init_hash_table(67, 0.9); 
  unsigned int reg_counter = 0;
  program_t out;
  parse_program(file, &out, &instruction_table, &reg_table, &reg_counter);
  destroy_hash_table(&reg_table);
  destroy_hash_table(&instruction_table);
  print_program(&out, 0);
  return out;
}