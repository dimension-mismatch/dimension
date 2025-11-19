#include "compiler.h"
#include "asm_utils.h"

#include "../hash_table/type_record.h"
#include "../hash_table/function_record.h"
#include "../hash_table/variable_record.h"

#include "../colors.h"

#include <stdlib.h>
#include <stdio.h>

char* header = "#DIMENSION v0.0.1 compiled\n.global start\n.intel_syntax noprefix\n\nstart:\n";

//puts instructions to compute the value of an expression into the assembly file, and returns an address_t containing the location of the output expression.
address_t compile_expression(FILE* file, reg_allocator_t* rega, expression_t* expression, function_record_t* fn_rec, variable_record_t* var_rec, type_record_t* type_rec){
  if(expression->type == EXP_SINGLE_LITERAL){
    return Aliteral(expression->numeric_literal);
  }
  else if(expression->type == EXP_READ_VAR){
    return Astack(expression->read.var_id, QWORD);
  }
  else if(expression->type == EXP_WRITE_VAR){
    address_t addr = Astack(expression->write.var_id, QWORD);
    expression_t* value = expression->write.value[0];
    mov_instruction(file, rega, addr, compile_expression(file, rega, expression->write.value[0], fn_rec, var_rec, type_rec));
    return addr;
  }
  else if(expression->type == EXP_BLOCK){
    int depth = expression->block.stack_depth;
    reg_allocator_t child_rega = rega_init(depth);
    put_new_stack_frame(file);
    if(depth > 0){
      put_instruction(file, "sub", Areg(RSP, QWORD), Aliteral(depth));
    }
    for(int i = 0; i < expression->block.arg_c; i++){
      compile_expression(file, &child_rega, expression->block.arg_v[i], fn_rec, var_rec, type_rec);
      putc('\n', file);
    }
  }
  else if(expression->type == EXP_CALL_FN){
    struct function_def definition = fn_rec_get_by_index(fn_rec, expression->function_call.fn_id);
    int arg_c = expression->function_call.arg_c;
    address_t arg_locations[arg_c];

    

    if(definition.impl_type == FN_IMPL_ASM){
      for(int i = 0; i < arg_c; i++){
        arg_locations[i] = compile_expression(file, rega, expression->function_call.arg_v[i], fn_rec, var_rec, type_rec);
      }
      char* text = definition.assembly;
      if(*text == 'I'){
        text++;
      }
      else{
        return Aliteral(0); //TODO : add support for true functions in assembly
      }

      while (*text != '\0'){
        if(*text == '@'){
          text++;
          int arg_i = 0;
          while(*text >= '0' && *text <= '9' && *text != '\0'){
            arg_i *= 10;
            arg_i += *text - '0';
            text++;
          }
          if(arg_locations[arg_i].type != ADDR_REGISTER){
            address_t reg = Aconsume_next_free(QWORD, rega);
            putc('#', file);
            for(int i = 0; i < 15; i++){
              put_number(file, rega->usage[i]);
              putc(',', file);
            }
            putc('\n', file);
            mov_instruction(file, rega, reg,  arg_locations[0]);
            putc('\n', file);
            arg_locations[arg_i] = reg;
          }
        }else{
          text++;
        }
      }

      int has_return_reg = 0;
      for(int i = 0; i < arg_c; i++){
        if(arg_locations[i].type == ADDR_REGISTER){
          if(has_return_reg){
            rega_free(rega, arg_locations[i].reg);
          }
          has_return_reg = 1;
        }
      }
   
      
      text = definition.assembly;
      text++;
      int arg_i = 0;
      while (*text != '\0'){
        if(*text == '?' || *text == '@'){
          text++;
          int arg_i = 0;
          while(*text >= '0' && *text <= '9' && *text != '\0'){
            arg_i *= 10;
            arg_i += *text - '0';
            text++;
          }
          put_address(file, arg_locations[arg_i]);
        }
        else{
          putc(*text, file);
          text++;
        }
      }
      return arg_locations[0];
    }
    else{
      int stack_ptr = -8;
      for(int i = arg_c - 1; i >= 0; i--){
        //evaluate each argument of the expression
        address_t arg = compile_expression(file, rega, expression->function_call.arg_v[i], fn_rec, var_rec, type_rec);

        //move the result into the correct location on the stack
        mov_instruction(file, rega, Astack(stack_ptr, QWORD), arg);

        //Free up the register we used for this computation (if we actually used one)
        if(arg.type == ADDR_REGISTER){
          rega_free(rega, arg.reg);
        }

        stack_ptr -= typeid_bytesize(&(expression->function_call.arg_v[i]->return_type));
      }

      put_text(file, "call function_body_");
      put_number(file, expression->function_call.fn_id);
      put_text(file, "\n");
    }
  }
  return Aliteral(0);
}
void compile_program(char* filename, expression_t* expression, function_record_t* fn_rec, variable_record_t* var_rec, type_record_t* type_rec){
  FILE* file = fopen(filename, "w");
  put_text(file, header);
  
  //put_text(file, "sub rsp, 16\n");

  compile_expression(file, NULL, expression, fn_rec, var_rec, type_rec);

  for(int i = 0; i < fn_rec->def_count; i++){
    if(fn_rec->definitions[i].impl_type == FN_IMPL_DMSN){

      put_text(file, "function_body_");
      put_number(file, i);
      put_text(file, ":\n");
      
      compile_expression(file, NULL, fn_rec->definitions[i].dmsn, fn_rec, var_rec, type_rec);
    }
    
  }

  fclose(file);
  file = NULL;
}