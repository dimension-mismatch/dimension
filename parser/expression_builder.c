#include "expression_builder.h"
#include "type_utils/type_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include "colors.h"


typedef struct{
  exp_array_t* begin;
  exp_array_t* end;
  dimension_t multiplicity;
  int id;
  int priority;
  int length;
  int index;
} expression_span_t;


typedef struct{
  int span_count;
  int best_id;
  int best_priority;
  expression_span_t* array;
}exp_span_array_t;


exp_span_array_t exp_span_array_init(){
  exp_span_array_t new = {0,0,-1, NULL};
  return new;
}

void print_exp_span_array(exp_span_array_t *array){
  for(int i = 0; i < array->span_count; i++){
    printf("%d :", i);
    expression_span_t span = array->array[i];
    exp_array_t* match = span.begin;
    for(int j = 0; j < span.length; j++){
      if(match == NULL){
        break;
      }
      if(match->expression->type == EXP_IDENTIFIER){
        printf("\"%s\"", match->expression->text);
      }
      else{
        print_type_id(&(match->expression->return_type));
      }
      printf("  ");
      match = match->next;
    }
    for(int d = 0; d < span.multiplicity.dimension_count; d++){
      printf(MAGENTA "x%d" RESET_COLOR, span.multiplicity.dimensions[d]);
    }
    if(i == array->best_id){
      printf(YELLOW BOLD "<- THIS ONE NEXT (#%d)" RESET_COLOR, array->best_id);
    }
    printf("\n");
  }
}

void exp_span_array_push_span(exp_span_array_t* array, expression_span_t span){
  if(span.priority > array->best_priority){
    array->best_id = array->span_count;
    array->best_priority = span.priority;
  }
  array->span_count++;
  array->array = realloc(array->array, array->span_count * sizeof(expression_span_t));
  array->array[array->span_count - 1] = span;
}



expression_span_t exp_create_span(int index, exp_array_t* array, function_record_t *fn_record){
  
  expression_span_t new = {array, NULL, dmsn_newEmpty(), -1, 0, 0, index};
  
  
  struct fn_tree* current_node = fn_record->root;
  while(array != NULL){
    
    int* id = NULL;
    new.length++;
    switch(array->expression->type){
      case EXP_IDENTIFIER:
        id = get_value_from_key(&(current_node->name_table), array->expression->text);
        break;
      case EXP_GROUPING: 
        break;
      default:
        id = get_value_from_int(&(current_node->type_table), array->expression->return_type.type_number);
        break;
    }
    if(id == NULL){
      return new;
    }
    
    current_node = current_node->children + *id;
    if(current_node->id_number != -1){
      //printf(GREEN "*" RESET_COLOR);
      // printf(GREEN "SUCCESSFULLY FOUND FUNCTION (#%d, consumes %d tokens): " RESET_COLOR, current_node->id_number, new.length);

      // print_fn_def(fn_rec_get_by_index(fn_record, current_node->id_number));
      // printf("\n");
      struct function_def def = fn_rec_get_by_index(fn_record, current_node->id_number);
      exp_array_t* match = new.begin;
      int arg_i = 0;
      for(int i = 0; i < new.length; i++){
        if(match->expression->type != EXP_IDENTIFIER && match->expression->type != EXP_GROUPING){
          if(def.parameters == NULL){
            printf(RED "SOmething is Wrong!!\n");
          }
          type_identifier_t expected_type = def.parameters[arg_i].type;
          
          type_identifier_t input_type = match->expression->return_type;
          
          for(int d = 0; d < input_type.dimension_count; d++){
            if(d < expected_type.dimension_count){
              if(input_type.dimensions[d] != expected_type.dimensions[d]){
                printf(RED BOLD "Dimension Mismatch in parameter \"%s\": " RESET_COLOR "expected ", def.parameters[arg_i].name);
                print_type_id(&expected_type);
                printf(", but got ");
                print_type_id(&input_type);
                printf("\n");
              }
            }
            else if(d - expected_type.dimension_count < new.multiplicity.dimension_count){
              if(input_type.dimensions[d] != new.multiplicity.dimensions[d - expected_type.dimension_count]){
                printf(RED BOLD "Dimension Mismatch in parameter \"%s\": " RESET_COLOR "value of type ", def.parameters[arg_i].name);
                print_type_id(&input_type);
                printf("does not match multiplicity. ");
                printf("\n");
              }
            }
            else{
              dmsn_pushDimension(&(new.multiplicity), input_type.dimensions[d]);
            }
          }
          arg_i++;
        }
        match = match->next;
      }
      new.id = current_node->id_number;
      new.end = array;
      new.priority = fn_rec_get_by_index(fn_record, current_node->id_number).priority;
      return new;
    }
    array = array->next;
  }
  return new;
}

void exp_span_array_find_best(exp_span_array_t* array){
  array->best_id = -1;
  array->best_priority = -1;
  for(int i = 0; i < array->span_count; i++){
    if(array->array[i].priority > array->best_priority){
      array->best_priority = array->array[i].priority;
      array->best_id = i;
    }
  }
}

void exp_span_array_destroy(exp_span_array_t *array){
  for(int i = 0; i < array->span_count; i++){
    free(array->array[i].multiplicity.dimensions);
  }
  free(array->array);
  array->array = NULL;
  array->span_count = 0;
}

void exp_span_array_consume_best_span(exp_span_array_t *array, exp_array_t* matches, function_record_t* fn_record){
  expression_span_t best = array->array[array->best_id];
  exp_array_t* begin = best.begin;
  exp_array_t* end = best.end->next;

  exp_array_t* match = best.begin;

  expression_t* new_expression = malloc(sizeof(expression_t));
  new_expression->type = EXP_CALL_FN;
  new_expression->function_call.fn_id = best.id;
  type_identifier_t return_type = fn_rec_get_by_index(fn_record, best.id).return_type;
  new_expression->return_type = typeid_copy(&return_type);
  typeid_multiply(&(new_expression->return_type), best.multiplicity);

  new_expression->function_call.arg_c = 0;
  for(int i = 0; i < best.length; i++){
    if(match->expression->type != EXP_IDENTIFIER){
      new_expression->function_call.arg_c++;
    }
    match = match->next;
  }
  new_expression->function_call.arg_v = malloc(new_expression->function_call.arg_c * sizeof(expression_t));
  match = best.begin;
  int arg_i = 0;
  for(int i = 0; i < best.length; i++){
    if(match->expression->type != EXP_IDENTIFIER){
      new_expression->function_call.arg_v[arg_i] = match->expression;
      arg_i++;
    }
    exp_array_t* next = match->next;
    if(i > 0){
      free(match);
    }
    match = next;
  }

  begin->expression = new_expression;
  begin->next = end;

  exp_span_array_t new_spans = exp_span_array_init();

  int start = array->best_id - 1;
  while(start >= 0 && array->array[start].length + array->array[start].index > best.index){
    start--;
  }
  int save_after = array->best_id + 1;
  while(array->array[save_after].index < best.index + best.length){
    
    save_after++;
    if(save_after >= array->span_count){
      save_after = array->span_count;
      break;
    }
  }

  int start_index;
  if(start < 0){
    //start scanning for matches at the beginning of the line
    start_index = 0;
    start = 0;
    match = matches;
  }
  else{
    //start scanning for expressions at the furthest left that could have been affected by the deletion
    start_index = array->array[start].index + 1;
    match = array->array[start].begin->next;
    start++;
  }

  for(int i = start; i < save_after; i++){
    free(array->array[i].multiplicity.dimensions);
    array->array[i].multiplicity = dmsn_newEmpty();
    array->array[i].begin = NULL;
  }
  for(int i = start_index; i <= best.index; i++){
    expression_span_t span = exp_create_span(i, match, fn_record);
    if(span.id != -1){
      exp_span_array_push_span(&new_spans, span);
    }
    match = match->next;

    if(match == NULL || match->expression->type == EXP_GROUPING) break;
  }

  
  int og_span_count = array->span_count;
  int deleted_span_count = save_after - start;

  
  array->span_count += new_spans.span_count - deleted_span_count;
  int resize_first = array->span_count > og_span_count; //If the array is growing, realloc before shifting elements around
  int resize_second = og_span_count > array->span_count; //If the array is shrinking, move elements to new positions, then realloc.
  //if array size is not changing, we don't need to realloc

  if(resize_first){
    array->array = realloc(array->array, array->span_count * sizeof(expression_span_t));
  }
  
  // printf("new matching generated %d new spans\n", new_spans.span_count);
  // printf("consuming range %d to %d\n", start, save_after);
  for(int i = save_after; i < og_span_count; i++){
    array->array[i].index -= (best.length - 1);
    array->array[i - deleted_span_count + new_spans.span_count] = array->array[i];
    //printf("moving #%d to position #%d\n", i,i - deleted_span_count + new_spans.span_count);
  }
  for(int i = 0; i < new_spans.span_count; i++){
    array->array[i + start] = new_spans.array[i];
    array->array[i + start].multiplicity = dmsn_copy(&(new_spans.array[i].multiplicity));
    //printf("moving new #%d to position #%d\n", i,i + start);
  }
  if(resize_second){
    array->array = realloc(array->array, array->span_count * sizeof(expression_span_t));
  }
  exp_span_array_destroy(&new_spans);

  exp_span_array_find_best(array);
  
}



exp_array_t* parse_expression(exp_array_t** start, function_record_t* fn_record, int depth){
  
  exp_array_t* array = *start;
  exp_span_array_t spans = exp_span_array_init();

  
  exp_array_t* current = array;
  exp_array_t* prev = NULL;

  exp_array_t* group_entry = NULL;
  exp_array_t* group_prev = NULL;
  while(current != NULL){
    if(current->expression->type == EXP_GROUPING){
      if(current->expression->enter_group){
        group_entry = current->next;
        
        
        printf(YELLOW "recursively parsing: " RESET_COLOR);
        print_exp_array(group_entry);
        printf("\n");
        exp_array_t* result = parse_expression(&group_entry, fn_record, depth + 1);
        

        if(prev){
          prev->next = result;
        }
        else{
          *start = result;
          array = *start;
        }
        exp_destroy(current->expression);
        free(current);
        current = result;
      }
      else{
        
        break;
      }
    } 
    prev = current;
    current = prev->next;
  }

  current = array;
  int i = 0;
  while(current != NULL){
    if(current->expression->type == EXP_GROUPING && !current->expression->enter_group){
      break;
    }
    expression_span_t span = exp_create_span(i, current, fn_record);
    
    if(span.id != -1){
      exp_span_array_push_span(&spans, span);
    }
    prev = current;
    current = current->next;
    i++;
  }

  while(spans.span_count > 0){
    // printf(RED BOLD "Expression is now: " RESET_COLOR);
    // print_exp_array(array);
    // printf("\n");
    // print_exp_span_array(&spans);
    exp_span_array_consume_best_span(&spans, array, fn_record);
  }
  

  expression_t* new;
  

  
  unsigned int exp_count = 0;
  exp_array_t* counter = array;
  while(counter){
    if(counter->expression->type == EXP_GROUPING){
      break;
    }
    exp_count++;
    counter = counter->next;
  }
  exp_array_t* end = counter? counter->next: NULL;

  if(exp_count > 1){
    type_identifier_t returnType = typeid_copy(&(array->expression->return_type));
    typeid_pushDimension(&returnType, exp_count);
    new = exp_init(EXP_VECTOR_LITERAL, returnType);
    new->vector_literal.component_count = exp_count;
    new->vector_literal.components = malloc(exp_count * sizeof(expression_t*));
    int exp_i = 0;
    counter = array;
    while(counter){
      if(counter->expression->type == EXP_GROUPING){
        break;
      }
      new->vector_literal.components[exp_i] = counter->expression;
      exp_i++;
      counter = counter->next;
    }

    end = counter? counter->next: NULL;
  }
  else{
    new = array->expression;
  }

  
  
  exp_span_array_destroy(&spans);
  exp_array_t* new_arr = malloc(sizeof(exp_array_t));
  new_arr->expression = new;

  new_arr->next = end;

  printf("resulting expression array: ");
  print_exp_array(new_arr);
  printf("\n");

  return new_arr;
}
