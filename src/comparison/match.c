#include <comparison/match.h>
#include <comparison/comparison.h>
#include <comparison/return_type.h>

#include <construct_utils.h>

bool test_pattern_value(pattern_value_t* test, expression_t* subject, context_t* context){
    if(test->is_param){
        return test_pattern_variable(test->param, subject, context);
    }
    return expression_compare(test->base_value, subject, context);
}

bool test_pattern_dimension_array(pattern_dimension_array_t* test, dimension_array_t* subject, context_t* context){
    //TODO: multiplicity
    if(test->dimension_count != subject->dimension_count) return false;
    for(int i = 0; i < test->dimension_count; i++){
        if(!test_pattern_value(test->dimensions + i, subject->dimensions + i, context)) return false;
    }
    return true;
}
bool test_pattern_type(pattern_type_t *test, type_identifier_t *subject, context_t* context){
    if(test->is_param) return true;
    if(test->body.type != subject->body.type) return false;
    if(!test_pattern_dimension_array(&test->dimensions, &subject->dimensions, context)) return false;

    if(test->body.type == TYPE_ID_RAW_BITS){
        return test_pattern_value(&test->body.size, &subject->body.size, context);
    }

    if(test->body.type == TYPE_ID_VECTOR){
        if(test->body.component_count != subject->body.component_count) return false;
        for(int i = 0; i < test->body.component_count; i++){
            //Do we need to check const level here?
            if(!test_pattern_type(&test->body.components->type, subject->body.components + i, context)) return false;
        }
        return true;
    }
    if(test->body.type_id != subject->body.type_id) return false;
    

    pattern_t* pattern = type_lookup(context, test->body.type_id)->match_pattern;
    int arg_count = pattern->arg_count;
    int type_arg_count = pattern->type_arg_count;

    for(int i = 0; i < arg_count; i++){
        if(!test_pattern_value(test->body.exp_args + i, subject->body.exp_args + i, context)) return false;
    }
    
    for(int i = 0; i < type_arg_count; i++){
        if(!test_pattern_type(test->body.type_args + i, subject->body.type_args + i, context)) return false;
    }

    return true;
} 

bool compatible_const_levels(const_lvl_t slot, const_lvl_t input){
  return (slot == CL_CONST) || input == slot;
}



bool test_pattern_variable(pattern_variable_t* test, expression_t* subject, context_t* context){
    if(!compatible_const_levels(test->constant_lvl, subject->const_lvl)) return false;
    type_identifier_t subject_type;
    get_return_type(&subject_type, subject, context);

    bool result = test_pattern_type(&test->type, &subject_type, context);

    destroy_type_identifier(&subject_type);
    return result;
}