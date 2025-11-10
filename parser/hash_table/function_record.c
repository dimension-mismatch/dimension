#include "function_record.h"
#include "hash_table.h"
#include "variable_record.h"
#include "../type_utils/type_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fn_tree fn_tree_init(){
  struct fn_tree new = {init_hash_table(67, 0.9), init_hash_table(67, 0.9), 0, NULL, -1};
  return new;
}

function_record_t fn_rec_init(){
  function_record_t new = {NULL, 0,0,NULL,NULL};
  new.root = malloc(sizeof(struct fn_tree));
  *(new.root) = fn_tree_init();
  return new;
}

int fn_tree_push_definition(struct fn_tree* tree, int fn_id, exp_array_t* array, type_identifier_t *returntype, int priority){
  if(array == NULL){
    if(tree->id_number != -1){ //don't allow redefining existing functions
      return 0;
    }
    tree->id_number = fn_id;
    return 1;
  }

  int* next_idx = NULL;
  switch(array->expression->type){    
    case EXP_IDENTIFIER:
      next_idx = get_value_from_key(&(tree->name_table), array->expression->text);
      break;
    default:
      next_idx = get_value_from_int(&(tree->type_table), array->expression->return_type.type_number);
      break;
  }
  if(next_idx != NULL){
    return fn_tree_push_definition(tree->children + *next_idx, fn_id, array->next, returntype, priority);
  }

  int child_index = tree->children_count;
  switch(array->expression->type){
    case EXP_IDENTIFIER:
      push_key_value(&(tree->name_table), array->expression->text, child_index);
      break;
    default:
      push_int_value(&(tree->type_table), array->expression->return_type.type_number, child_index);
      break;
  }
  tree->children_count++;
  tree->children = realloc(tree->children, tree->children_count * sizeof(struct fn_tree));
  tree->children[child_index] = fn_tree_init();
  return fn_tree_push_definition(tree->children + child_index, fn_id, array->next, returntype, priority);
}


void fn_rec_push_definition(function_record_t *record, exp_array_t* array, type_identifier_t *returntype, int priority){;
  if(array == NULL) return;
  int child_index = record->def_count;
  int pushed = fn_tree_push_definition(record->root, child_index, array, returntype, priority);
  if(!pushed){
    return;
  }
  struct function_def def = {.return_type = *returntype, .num_parameters = 0, .parameters = NULL, .priority = priority};
  while(array != NULL){
    if(array->expression->type == EXP_DECLARE_VAR){
      def.num_parameters++;
      def.parameters = realloc(def.parameters, def.num_parameters * sizeof(variable_t));
      variable_t new_param = {typeid_copy(&(array->expression->return_type)), NULL};
      new_param.name = malloc(strlen(array->expression->text) + 1);
      strcpy(new_param.name, array->expression->text);
      def.parameters[def.num_parameters - 1] = new_param;
      //printf("adding parameter %s\n", new_param.name);
    }
    array = array->next;
  }
  record->def_count++;
  record->definitions = realloc(record->definitions, record->def_count * sizeof(struct function_def));
  record->definitions[child_index] = def;
}

struct function_def fn_rec_get_by_index(function_record_t *record, int index){
  return record->definitions[index];
}
void fn_tree_destroy(struct fn_tree* tree){
  for(int i = 0; i < tree->children_count; i++){
    fn_tree_destroy(tree->children + i);
  }
  destroy_hash_table(&(tree->name_table));
  destroy_hash_table(&(tree->type_table));
  free(tree->children);
  tree->children = NULL;
  tree->children_count = 0;
}

void fn_rec_destroy(function_record_t *record){
  fn_tree_destroy(record->root);
  free(record->root);
  record->root = NULL;
  for(int i = 0; i < record->def_count; i++){
    struct function_def def = record->definitions[i];
    for(int j = 0; j < def.num_parameters; j++){
      free(def.parameters[j].name);
      typeid_destroy(&(def.parameters[j].type));
    }
    typeid_destroy(&(def.return_type));
  }
  free(record->definitions);
  record->definitions = NULL;
  free(record->scopes);
  record->scopes = NULL;
  record->scope_depth = 0;
  record->def_count = 0;
}

void print_fn_def(struct function_def def)
{
  printf("( ");
  for(int i = 0; i < def.num_parameters; i++){
    print_type_id((&(def.parameters + i)->type));
    printf(", ");
  }
  printf(") makes ");
  print_type_id(&(def.return_type));
  printf(" with priority %d\n", def.priority);
}

void print_fn_tree(struct fn_tree* tree, int indent){
  if(tree->id_number != -1){
    for(int i = 0; i < indent; i++) printf(" ");
    printf("(FN #%d)\n", tree->id_number);
    return;
  }
  if(tree->name_table.key_count > 0){
    for(int i = 0; i < indent; i++) printf(" ");
    printf("MATCH TEXT:\n");
    for(int i = 0; i < tree->name_table.array_size; i++){
      struct hash_entry* entry = tree->name_table.array[i];
      while(entry != NULL){
        for(int i = 0; i < indent; i++) printf(" ");
        printf("  * text: %s\n", entry->name);
        print_fn_tree(tree->children  + entry->value, indent + 3);
        entry = entry->next;
      }
      
    }
  }
  if(tree->type_table.key_count > 0){
    for(int i = 0; i < indent; i++) printf(" ");
    printf("MATCH TYPES:\n");
    for(int i = 0; i < tree->type_table.array_size; i++){
      struct hash_entry* entry = tree->type_table.array[i];
      if(entry == NULL){
        continue;
      }
      while(entry != NULL){
        for(int i = 0; i < indent; i++) printf(" ");
        printf("  * type: [%d]\n", entry->id);
        print_fn_tree(tree->children + entry->value, indent + 3);
        entry = entry->next;
      }
    }
  }
}

void print_fn_rec(function_record_t *record){
  printf("DEFINITIONS:\n");
  for(int i = 0; i < record->def_count; i++){
    print_fn_def(record->definitions[i]);
  }
  
  print_fn_tree(record->root, 0);
}

