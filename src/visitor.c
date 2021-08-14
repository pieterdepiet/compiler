#include "include/visitor.h"
#include <string.h>
#include "include/utils.h"
#include "include/errors.h"
#include "include/defs.h"
visitor_T* init_visitor(as_file_T* as_file) {
    visitor_T* visitor = calloc(1, sizeof(struct VISITOR_STRUCT));
    visitor->noop = init_ast(AST_NOOP);

    visitor->int_type = init_data_type(TYPE_INT, "Int");
    visitor->int_type->primitive_size = 4;
    visitor->null_type = init_data_type(TYPE_NULL, "Null");
    visitor->null_type->primitive_size = 0;

    visitor->global_scope = init_scope((void*) 0);

    scope_add_variable(visitor->global_scope, "Int", visitor->int_type);
    scope_add_variable(visitor->global_scope, "Null", visitor->null_type);
    visitor->as_file = as_file;

    defs_define_all(visitor->global_scope);
    
    return visitor;
}
data_type_T* get_data_type(visitor_T* visitor, scope_T* scope, char* name) {
    if (name == (void*) 0) {
        return (void*) 0;
    } else if (utils_strcmp(name, "Int")) {
        return visitor->int_type;
    } else {
        return (void*) 0;
    }
}
int node_to_data_type(visitor_T* visitor, data_type_T* data_type, AST_T* node) {
    if (data_type == visitor->int_type) {
        if (node->type == AST_INT) {
            return 0;
        } else if (node->type == AST_FLOAT) {
            node->type = AST_FLOAT;
            node->float_value = (float) node->int_value;
            return 0;
        } else {
            return -1;
        }
    }
    return -1;
}

int node_equals_data_type(data_type_T* type1, data_type_T* type2) {
    if (type1 == type2) {
        return 1;
    } else {
        return 0;
    }
}
data_type_T* get_variable(scope_T* scope, char* name) {
    scope_T* last_scope = scope;
    while (last_scope->parent) {
        for (size_t i = 0; i < last_scope->variables_size; i++) {
            if (utils_strcmp(last_scope->variable_names[i], name)) {
                return last_scope->variable_types[i];
            }
        }
        last_scope = scope->parent;
    }
    return (void*) 0;
}

char* visitor_data_type_to_arg_name(visitor_T* visitor, scope_T* scope, data_type_T* data_type) {
    if (data_type->primitive_type == TYPE_INT) {
        return "i";
    } else {
        char* arg_name = calloc(1, (3 + strlen(data_type->type_name) + 1) * sizeof(char));
        sprintf(arg_name, "%lu%s", strlen(data_type->type_name), data_type->type_name);
        return arg_name;
    }
}

data_type_T* visitor_visit(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    switch (node->type) {
        case AST_INT: return visitor_visit_int(visitor, scope, node, as_function); break;
        case AST_VARIABLE_DEFINITION: return visitor_visit_variable_definition(visitor, scope, node, as_function); break;
        case AST_VARIABLE: return visitor_visit_variable(visitor, scope, node, as_function); break;
        case AST_FUNCTION_CALL: return visitor_visit_function_call(visitor, scope, node, as_function); break;
        case AST_BINOP: return visitor_visit_binop(visitor, scope, node, as_function); break;
        default: err_unexpected_node(node); break;
    }
    return (void*) 0;
}
as_value_U visitor_visit_data(visitor_T* visitor, data_type_T* data_type, AST_T* node) {
    as_value_U value;
    if (data_type->primitive_type == TYPE_INT) {
        if (node->type != AST_INT) {
            err_node_not_convertable_to_data_type(node, data_type);
        }
        value.int_value = node->int_value;
        return value;
    } else {
        err_bad_global_type(data_type);
    }
    return value;
}
data_type_T* visitor_visit_global(visitor_T* visitor, AST_T* root) {
    for (size_t i = 0; i < root->compound_size; i++) {
        if (root->compound_value[i]->type == AST_VARIABLE_DEFINITION) {
            data_type_T* data_type = get_data_type(visitor, visitor->global_scope, root->compound_value[i]->variable_definition_type);
            if (!data_type) {
                err_unknown_type(root->compound_value[i]->variable_definition_type);
            }
            if (node_to_data_type(visitor, data_type, root->compound_value[i]->variable_definition_value) != 0) {
                err_node_not_convertable_to_data_type(root->compound_value[i]->variable_definition_value, data_type);
            }
            as_data_T* as_data = init_as_data(root->compound_value[i]->variable_definition_name, data_type);
            as_data->value = visitor_visit_data(visitor, data_type, root->compound_value[i]->variable_definition_value);
            if (data_type != visitor->int_type) {
                err_bad_global_type(data_type);
            }
            as_add_data(visitor->as_file, as_data);
            scope_add_variable(visitor->global_scope, root->compound_value[i]->variable_definition_name, data_type);
        } else if (root->compound_value[i]->type == AST_FUNCTION_DEFINITION) {
            visitor_visit_function_definition(visitor, visitor->global_scope, root->compound_value[i]);
        }
    }
    return (void*) 0;
}
data_type_T* visitor_visit_function_definition(visitor_T* visitor, scope_T* scope, AST_T* node) {
    scope_T* function_scope = init_scope(scope);
    data_type_T* return_type;
    if (node->function_definition_return_type == (void*) 0) {
        return_type = visitor->null_type;
    } else {
        return_type = get_data_type(visitor, scope, node->function_definition_return_type);
    }
    fspec_T* fspec = init_fspec(return_type);
    if (utils_strcmp(node->function_definition_name, "start")) {
        fspec->symbol_name = "_start";
    } else {
        char* return_type_name = visitor_data_type_to_arg_name(visitor, scope, return_type);
        fspec->symbol_name = calloc(1, (4 + 3 + strlen(node->function_definition_name) + 3 + strlen(return_type_name) + 1));
        sprintf(fspec->symbol_name, "_MyC%lu%s%lu%s", strlen(node->function_definition_name), node->function_definition_name, strlen(return_type_name), return_type_name);
    }
    as_function_T* as_function = init_as_function(fspec->symbol_name);
    as_add_function(visitor->as_file, as_function);
    for (size_t i = 0; i < node->function_definition_args->unnamed_size; i++) {
        data_type_T* arg_type = get_data_type(visitor, scope, node->function_definition_args->unnamed_types[i]);
        scope_add_variable(function_scope, node->function_definition_args->unnamed_names[i], arg_type);
        fspec_add_unnamed_arg(fspec, arg_type);
        as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
        as_op->argno = i;
        as_op->op_size = arg_type->primitive_size;
        as_op->var_location = scope_get_variable_relative_location(function_scope, node->function_definition_args->unnamed_names[i]);
        if (as_op->var_location < 0) {
            err_no_info();
        }
        as_add_op_to_function(as_function, as_op);
        // char* arg_name = visitor_data_type_to_arg_name(visitor, scope, arg_type);
        // fspec->symbol_name = realloc(fspec->symbol_name, strlen(fspec->symbol_name) + strlen(arg_name) + 1);
        // strcat(fspec->symbol_name, arg_name);
    }
    for (size_t i = 0; i < node->function_definition_args->named_size; i++) {
        data_type_T* arg_type = get_data_type(visitor, scope, node->function_definition_args->named_types[i]);
        scope_add_variable(function_scope, node->function_definition_args->named_inside_names[i], arg_type);
        fspec->named_length++;
        fspec->named_names = realloc(fspec->named_names, fspec->named_length * sizeof(char*));
        fspec->named_names[fspec->named_length-1] = node->function_definition_args->named_public_names[i];
        fspec->named_types = realloc(fspec->named_types, fspec->named_length * sizeof(data_type_T*));
        fspec->named_types[fspec->named_length-1] = arg_type;
        as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
        as_op->argno = node->function_definition_args->unnamed_size + i;
        as_op->op_size = arg_type->primitive_size;
        as_op->var_location = scope_get_variable_relative_location(function_scope, node->function_definition_args->named_inside_names[i]);
        as_add_op_to_function(as_function, as_op);
        // char* arg_name = visitor_data_type_to_arg_name(visitor, scope, arg_type);
        // fspec->symbol_name = realloc(fspec->symbol_name, strlen(fspec->symbol_name) + strlen(arg_name) + 1);
        // strcat(fspec->symbol_name, arg_name);
    }
    scope_add_function(scope, node->function_definition_name, fspec);
    data_type_T* compound_type = visitor_visit_compound(visitor, function_scope, node->function_definition_body, return_type, as_function);
    if (compound_type == visitor->null_type) {
        err_no_return_value(return_type);
    }
    scope_get_scope_size(scope);
    return return_type;
}
data_type_T* visitor_visit_compound(visitor_T* visitor, scope_T* scope, AST_T* node, data_type_T* return_type, as_function_T* as_function) {
    for (size_t i = 0; i < node->compound_size; i++) {
        if (node->compound_value[i]->type == AST_RETURN) {
            visitor_visit_return(visitor, scope, node->compound_value[i], as_function, return_type);
            return return_type;
        } else {
            visitor_visit(visitor, scope, node->compound_value[i], as_function);
        }
    }
    return visitor->null_type;
}
data_type_T* visitor_visit_return(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function, data_type_T* return_type) {
    data_type_T* value_type = visitor_visit(visitor, scope, node->return_value, as_function);
    if (value_type != return_type) {
        err_conflicting_types(value_type, return_type);
    }
    as_op_T* as_op = init_as_op(ASOP_RETURN);
    as_op->op_size = value_type->primitive_size;
    as_add_op_to_function(as_function, as_op);
    return value_type;
}

data_type_T* visitor_visit_binop(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    if (node->binop_type == BINOP_ASSIGN) {
        return visitor_visit_variable_assignment(visitor, scope, node, as_function);
    }
    data_type_T* left_hand_type = visitor_visit(visitor, scope, node->left_hand, as_function);
    as_op_T* as_op = init_as_op(ASOP_NEXTREG);
    as_op->op_size = left_hand_type->primitive_size;
    as_add_op_to_function(as_function, as_op);
    data_type_T* right_hand_type = visitor_visit(visitor, scope, node->right_hand, as_function);
    if (left_hand_type != right_hand_type) {
        err_conflicting_types(left_hand_type, right_hand_type);
    }
    as_op = init_as_op(ASOP_BINOP);
    as_op->op_size = left_hand_type->primitive_size;
    as_op->binop_type = node->binop_type;
    as_op->data_type = left_hand_type;
    as_add_op_to_function(as_function, as_op);
    as_op = init_as_op(ASOP_FREEREG);
    as_add_op_to_function(as_function, as_op);
    return left_hand_type;
}
// data_type_T* visitor_visit_plus(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
//     data_type_T* left_hand_type = visitor_visit(visitor, scope, node->left_hand, as_function);
//     as_op_T* as_op = init_as_op(ASOP_NEXTREG);
//     as_op->op_size = left_hand_type->primitive_size;
//     as_add_op_to_function(as_function, as_op);
//     data_type_T* right_hand_type = visitor_visit(visitor, scope, node->right_hand, as_function);
//     if (left_hand_type != right_hand_type) {
//         err_conflicting_types(left_hand_type, right_hand_type);
//     }
//     as_op = init_as_op(ASOP_ADD);
//     as_op->op_size = left_hand_type->primitive_size;
//     as_add_op_to_function(as_function, as_op);
//     as_op = init_as_op(ASOP_FREEREG);
//     as_add_op_to_function(as_function, as_op);
//     return left_hand_type;
// }
data_type_T* visitor_visit_int(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    as_op_T* asop = init_as_op(ASOP_SETLASTIMM);
    asop->op_size = visitor->int_type->primitive_size;
    asop->data_type = visitor->int_type;
    asop->value.int_value = node->int_value;
    as_add_op_to_function(as_function, asop);
    return visitor->int_type;
}
data_type_T* visitor_visit_variable_definition(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    data_type_T* definition_type = get_data_type(visitor, scope, node->variable_definition_type);
    if (definition_type==(void*) 0) {
        err_unknown_type(node->variable_definition_type);
    }
    data_type_T* value_type = visitor_visit(visitor, scope, node->variable_definition_value, as_function);
    if (definition_type != value_type) {
        err_conflicting_types(definition_type, value_type);
    }
    scope_add_variable(scope, node->variable_definition_name, definition_type);
    as_op_T* as_op = init_as_op(ASOP_VDEF);
    as_op->var_location = scope_get_variable_relative_location(scope, node->variable_definition_name);
    if (as_op->var_location < 0) {
        err_no_info();
    }
    as_op->op_size = definition_type->primitive_size;
    as_add_op_to_function(as_function, as_op);
    return definition_type;
}
data_type_T* visitor_visit_variable_assignment(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    data_type_T* variable_type = scope_get_variable(scope, node->left_hand->variable_name);
    if (variable_type==(void*) 0) {
        err_undefined_variable(node->left_hand->variable_name);
    }
    data_type_T* value_type = visitor_visit(visitor, scope, node->right_hand, as_function);
    if (variable_type != value_type) {
        err_conflicting_types(variable_type, value_type);
    }
    as_op_T* as_op = init_as_op(ASOP_VMOD);
    as_op->var_location = scope_get_variable_relative_location(scope, node->left_hand->variable_name);
    if (as_op->var_location < 0) {
        err_no_info();
    }
    as_op->op_size = variable_type->primitive_size;
    as_add_op_to_function(as_function, as_op);
    return variable_type;
}
data_type_T* visitor_visit_variable(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    data_type_T* data_type = scope_get_variable(scope, node->variable_name);
    as_op_T* as_op = init_as_op(ASOP_VREF);
    as_op->var_location = scope_get_variable_relative_location(scope, node->variable_name);
    as_op->op_size = data_type->primitive_size;
    as_add_op_to_function(as_function, as_op);
    return data_type;
}
data_type_T* visitor_visit_function_call(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    fspec_T* fspec = scope_get_function(visitor->global_scope, node->function_call_function->variable_name);
    for (size_t i = 0; i < node->function_call_unnamed_size; i++) {
        data_type_T* arg_type = visitor_visit(visitor, scope, node->function_call_unnamed_values[i], as_function);
        if (i >= fspec->unnamed_length) {
            err_too_many_args_in_fcall();
        }
        if (arg_type != fspec->unnamed_types[i]) {
            err_conflicting_types(arg_type, fspec->unnamed_types[i]);
        }
        as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
        as_op->op_size = arg_type->primitive_size;
        as_op->argno = i;
        as_add_op_to_function(as_function, as_op);
    }
    for (size_t i = 0; i < node->function_call_named_size; i++) {
        if (i >= fspec->named_length) {
            err_too_many_args_in_fcall();
        }
        if (utils_strcmp(fspec->named_names[i], node->function_call_named_names[i])) {
            err_no_named_arg(node->function_call_named_names[i]);
        }
        data_type_T* arg_type = visitor_visit(visitor, scope, node->function_call_named_values[i], as_function);
        if (arg_type != fspec->named_types[i]) {
            err_conflicting_types(arg_type, fspec->named_types[i]);
        }
        as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
        as_op->op_size = arg_type->primitive_size;
        as_op->argno = i + node->function_call_unnamed_size;
        as_add_op_to_function(as_function, as_op);
    }
    char* as_function_name = fspec->symbol_name;
    as_op_T* call_op = init_as_op(ASOP_FCALL);
    call_op->op_size = 8;
    call_op->name = as_function_name;
    as_add_op_to_function(as_function, call_op);
    if (fspec->return_type->primitive_type != TYPE_NULL) {
        as_op_T* retval_op = init_as_op(ASOP_RETVAL);
        retval_op->op_size = fspec->return_type->primitive_size;
        as_add_op_to_function(as_function, retval_op);
    }
    return fspec->return_type;
}