#include <comparison/return_type.h>
#include <construct_utils.h>



void get_return_type(type_identifier_t* return_type, expression_t* expression, context_t* context){
    switch(expression->type){
        case EXP_FUNCTION_CALL:{
            type_identifier_t* fn_return_type = &function_lookup(context, expression->function_call.fn_id)->return_type;
            copy_type_identifier(return_type, fn_return_type);
            //TODO: fill in parameters to return type
            break;
        }
        case EXP_READ_VAR: {
            type_identifier_t* var_type = &variable_lookup(context, expression->read_var_id)->type;
            copy_type_identifier(return_type, var_type);
            break;
        }
        case EXP_VALUE_LITERAL: {
            //TODO: handle builtin types ([i], [u], [c], etc.)
            return NULL;
        }
        case EXP_VECTOR: {
            return_type->dimensions.dimension_count = 0;
            return_type->dimensions.dimensions = NULL;
            return_type->body.type = TYPE_ID_VECTOR;
            int c = expression->vector.num_params;
            return_type->body.component_count = c;
            return_type->body.components = malloc(c * sizeof(type_identifier_t));
            for(int i = 0; i < c; i++){
                get_return_type(return_type->body.components + i, expression->vector.params + i, context);
            }
        }
    }
}

