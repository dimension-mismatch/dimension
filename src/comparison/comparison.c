
#include <comparison/comparison.h>

#include <string.h>

bool dimension_array_compare(dimension_array_t* a, dimension_array_t* b, context_t* context){
    if(a->dimension_count != b->dimension_count) return false;

    for(int i = 0; i < a->dimension_count; i++){
        if(!expression_compare(a->dimensions + i, b->dimensions + i, context)) return false;
    }
    return true;
}

bool type_identifier_compare(type_identifier_t* a, type_identifier_t* b, context_t* context){
    if(a->hash != b->hash) return false;
    
    if(a->body.type != b->body.type) return false;

    if(!dimension_array_compare(&a->dimensions, &b->dimensions, context));

    if(a->body.type == TYPE_ID_RAW_BITS){
        return expression_compare(&a->body.size, &b->body.size, context);
    }
    if(a->body.type == TYPE_ID_VECTOR){
        if(a->body.component_count != b->body.component_count) return false;
        for(int i = 0; i < a->body.component_count; i++){
            if(!type_identifier_compare(a->body.components + i, b->body.components + i, context)) return false;
        }
        return true;
    }

    if(a->body.type_id != b->body.type_id) return false;

    pattern_t* pattern = type_lookup(context, a->body.type_id)->match_pattern;
    int arg_count = pattern->arg_count;
    int type_arg_count = pattern->type_arg_count;
    for(int i = 0; i < arg_count; i++){
        if(!expression_compare(a->body.exp_args + i, b->body.exp_args + i, context)) return false;
    }
    for(int i = 0; i < type_arg_count; i++){
        if(!type_identifier_compare(a->body.type_args + i, b->body.type_args + i, context)) return false;
    }

    return true;
}

bool expression_compare(expression_t* a, expression_t* b, context_t* context){
    if(a->hash != b->hash) return false;

    if(a->type != b->type) return false;

    switch(a->type){
        case EXP_FUNCTION_CALL: {
            if(a->function_call.fn_id != b->function_call.fn_id) return false;

            pattern_t* pattern = &function_lookup(context, a->function_call.fn_id)->match;
            int arg_count = pattern->arg_count;
            int type_arg_count = pattern->type_arg_count;

            for(int i = 0; i < arg_count; i++){
                if(!expression_compare(a->function_call.params + i, b->function_call.params + i, context)) return false;
            }

            for(int i = 0; i < type_arg_count; i++){
                if(!type_identifier_compare(a->function_call.type_args + i, b->function_call.type_args + i, context)) return false;
            }
            return true;
        }
        case EXP_READ_VAR:
            return a->read_var_id == b->read_var_id;
        case EXP_VALUE_LITERAL:
            if(a->value_literal.type != b->value_literal.type) return false;

            switch(a->value_literal.type){
                case VAL_CHAR:
                    return a->value_literal.c == b->value_literal.c;
                case VAL_FLOAT:
                    return a->value_literal.f == b->value_literal.f;
                case VAL_INT:
                    return a->value_literal.i == b->value_literal.i;
                case VAL_UNSIGNED:
                    return a->value_literal.u == b->value_literal.u;
                case VAL_STRING:
                    return !strcmp(a->value_literal.s, b->value_literal.s);
            }
        case EXP_VECTOR:
            if(a->vector.num_params != b->vector.num_params) return false;
            
            for(int i = 0; i < a->vector.num_params; i++){
                if(!expression_compare(a->vector.params + i, b->vector.params + i, context)) return false;
            }
            return true;
    }
}


bool pattern_variable_compare(pattern_variable_t* a, pattern_variable_t* b, context_t* context){
    if(a->constant_lvl != b->constant_lvl) return false;
    return pattern_type_compare(&a->type, &b->type, context);
    
}
bool pattern_entry_compare(pattern_entry_t* a, pattern_entry_t* b, context_t* context){
    if(a->type != b->type) return false;
    switch(a->type){
        case PATTERN_EXP:
            return expression_compare(&a->exp, &b->exp, context);
        case PATTERN_IDENTIFIER:
            return !strcmp(a->identifier, b->identifier);
        case PATTERN_TYPE:
            return pattern_type_compare(&a->pattern_type, &b->pattern_type, context);
        case PATTERN_VARIABLE:
            return pattern_variable_compare(&a->variable, &b->variable, context);
    }
}

bool pattern_dimension_array_compare(pattern_dimension_array_t* a, pattern_dimension_array_t* b, context_t* context){
    if(a->dimension_count != b->dimension_count) return false;
    for(int i = 0; i < a->dimension_count; i++){
        if(!pattern_value_compare(a->dimensions + i, b->dimensions + i, context)) return false;
    }
    return true;
}

bool pattern_type_compare(pattern_type_t* a, pattern_type_t* b, context_t* context){
    if(a->is_param != b->is_param) return false;
    if(a->body.type != b->body.type) return false;

    if(!pattern_dimension_array_compare(&a->dimensions, &b->dimensions, context)) return false;

    //fully parameterized pattern types need to only have matching dimensions
    if(a->is_param) return true;

    if(a->body.type == TYPE_ID_RAW_BITS){
        return pattern_value_compare(&a->body.size, &b->body.size, context);
    }
    if(a->body.type == TYPE_ID_VECTOR){
        if(a->body.component_count != b->body.component_count) return false;
        for(int i = 0; i < a->body.component_count; i++){
            if(pattern_type_compare(a->body.components + i, b->body.components + i, context)) return false;
        }
        return true;
    }

    pattern_t* pattern = type_lookup(context, a->body.type_id)->match_pattern;
    int arg_count = pattern->arg_count;
    int type_arg_count = pattern->type_arg_count;

    for(int i = 0; i < arg_count; i++){
        if(!pattern_value_compare(a->body.exp_args + i, b->body.exp_args + i, context)) return false;
    }
    for(int i = 0; i < type_arg_count; i++){
        if(!pattern_type_compare(a->body.type_args + i, b->body.type_args + i, context)) return false;
    }

    return true;
}

bool pattern_value_compare(pattern_value_t* a, pattern_value_t* b, context_t* context){
    if(a->is_param != b->is_param) return false;

    if(a->is_param){
        return pattern_variable_compare(&a->param, &b->param, context);
    }
    return expression_compare(&a->base_value, &b->base_value, context);
}