#include "include/visitor.h"
#include <string.h>
#include "include/utils.h"
#include "include/errors.h"
#include "include/defs.h"
#define prefix "_Pre"
#define prefix_len 4



visitor_T* init_visitor(as_file_T* as_file) {
    visitor_T* visitor = calloc(1, sizeof(struct VISITOR_STRUCT));


    visitor->global_scope = init_global_scope(visitor);
    defs_define_all(visitor->global_scope);


    visitor->null_type = scope_get_typeg(visitor->global_scope, "Null");
    visitor->int_type = scope_get_typeg(visitor->global_scope, "Int");
    visitor->bool_type = scope_get_typeg(visitor->global_scope, "Bool");

    visitor->string_type = scope_get_typeg(visitor->global_scope, "String");
    visitor->as_file = as_file;
    
    return visitor;
}
data_type_T* get_data_type(scope_T* scope, char* name) {
    if (name == (void*) 0) {
        return (void*) 0;
    } else if (utils_strcmp(name, "Int")) {
        return scope->visitor->int_type;
    } else if (utils_strcmp(name, "String")) {
        return scope->visitor->string_type;
    } else {
        data_type_T* data_type = scope_get_type(scope, name);
        if (data_type) {
            return data_type;
        } else {
            return (void*) 0;
        }
    }
}
int node_to_data_type(scope_T* scope, data_type_T* data_type, AST_T* node) {
    if (data_type == scope->visitor->int_type) {
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

char* visitor_data_type_to_arg_name(data_type_T* data_type) {
    if (data_type->primitive_type == TYPE_INT) {
        char* str = calloc(1, 2 * sizeof(char));
        strcpy(str, "i");
        return str;
    } else if (data_type->primitive_type == TYPE_STRING) {
        char* str = calloc(1, 2 * sizeof(char));
        strcpy(str, "s");
        return str;
    } else if (data_type->primitive_type == TYPE_NULL) {
        char* str = calloc(1, 2 * sizeof(char));
        strcpy(str, "v");
        return str;
    } else {
        char* arg_name = calloc(1, (3 + strlen(data_type->type_name) + 1) * sizeof(char));
        sprintf(arg_name, "%zu%s", strlen(data_type->type_name), data_type->type_name);
        return arg_name;
    }
}

data_type_T* visitor_visit(scope_T* scope, AST_T* node, data_type_T* hint) {
    // printf("Visit node type %s\n", ast_node_type_string(node));
    switch (node->type) {
        case AST_INT: return visitor_visit_int_as(scope, node, hint?hint:scope->visitor->int_type); break;
        case AST_STRING: return visitor_visit_string(scope, node); break;
        case AST_VARIABLE_DEFINITION: return visitor_visit_variable_definition(scope, node); break;
        case AST_VARIABLE: return visitor_visit_variable(scope, node); break;
        case AST_FUNCTION_CALL: return visitor_visit_function_call(scope, node); break;
        case AST_BINOP: return visitor_visit_binop(scope, node, hint); break;
        case AST_MEMBER: return visitor_visit_member(scope, node); break;
        case AST_NEW: return visitor_visit_new(scope, node); break;
        case AST_UNOP: return visitor_visit_unop(scope, node, hint); break;
        default: err_unexpected_node(node); break;
    }
    return (void*) 0;
}
data_type_T* visitor_visit_global(global_T* global_scope, AST_T* root) {
    for (size_t i = 0; i < root->compound_size; i++) {
        // printf("Visit global node %s\n", ast_node_type_string(root->compound_value[i]));
        if (root->compound_value[i]->type == AST_VARIABLE_DEFINITION) {
            data_type_T* data_type = scope_get_typeg(global_scope, root->compound_value[i]->variable_definition_type);
            if (!data_type) {
                err_unknown_type(root->compound_value[i]->variable_definition_type, root->compound_value[i]);
            }
            // if (node_to_data_type(global_scope, data_type, root->compound_value[i]->variable_definition_value) != 0) {
                // err_node_not_convertable_to_data_type(root->compound_value[i]->variable_definition_value, data_type);
            // }
            // as_data_T* as_data = init_as_data(root->compound_value[i]->variable_definition_name, data_type);
            // as_data->value = visitor_visit_data(global_scope, data_type, root->compound_value[i]->variable_definition_value);
            // if (data_type != global_scope->visitor->int_type) {
            //     err_bad_global_type(data_type);
            // }
            // as_add_data(global_scope->visitor->as_file, as_data);
            // scope_add_variable(global_scope->visitor->global_scope, root->compound_value[i]->variable_definition_name, data_type);
        } else if (root->compound_value[i]->type == AST_FUNCTION_DEFINITION) {
            as_function_T* as_function = init_as_function(root->compound_value[i]->function_definition_name);
            as_add_function(global_scope->visitor->as_file, as_function);

            scope_T* function_scope = init_function_scope(global_scope, as_function);
            fspec_T* function_definition = visitor_visit_function_definition(function_scope, root->compound_value[i]);
            function_definition->name = root->compound_value[i]->function_definition_name;
            if (utils_strcmp(as_function->name, "start")) {
                as_function->name = "_start";
            } else {
                function_definition->symbol_name = calloc(1, (prefix_len + 4 + strlen(root->compound_value[i]->function_definition_name) + 1));
                sprintf(function_definition->symbol_name, prefix "%zu%s", strlen(root->compound_value[i]->function_definition_name), root->compound_value[i]->function_definition_name);
                for (size_t j = 0; j < function_definition->unnamed_length; j++) {
                    char* type_name = visitor_data_type_to_arg_name(function_definition->unnamed_types[j]);
                    function_definition->symbol_name = realloc(function_definition->symbol_name, (strlen(function_definition->symbol_name) + strlen(type_name) + 2) * sizeof(char));
                    sprintf(function_definition->symbol_name, "%s_%s", function_definition->symbol_name, type_name);
                    free(type_name);
                }
                for (size_t j = 0; j < function_definition->named_length; j++) {
                    char* type_name = visitor_data_type_to_arg_name(function_definition->named_types[j]);
                    function_definition->symbol_name = realloc(function_definition->symbol_name, (strlen(function_definition->symbol_name) + 4 + strlen(function_definition->named_names[j]) + strlen(type_name)) * sizeof(char));
                    sprintf(function_definition->symbol_name, "%s%zu%s%s", function_definition->symbol_name, strlen(function_definition->named_names[j]), function_definition->named_names[j], type_name);
                    free(type_name);
                }
                char* return_type_name = visitor_data_type_to_arg_name(function_definition->return_type);
                function_definition->symbol_name = realloc(function_definition->symbol_name, (strlen(function_definition->symbol_name) + strlen(return_type_name) + 1) * sizeof(char));
                sprintf(function_definition->symbol_name, "%s%s", function_definition->symbol_name, return_type_name);
                free(return_type_name);
                as_function->name = function_definition->symbol_name;
            }
            for (size_t i = 0; i < global_scope->functions_size; i++) {
                if (fspec_check_equals(function_definition, global_scope->function_specs[i])) {
                    err_duplicate_function(root->compound_value[i]);
                }
            }
            scope_add_functiong(global_scope, function_definition);
        } else if (root->compound_value[i]->type == AST_CLASS_DEFINITION) {
            visitor_visit_class_definition(global_scope, root->compound_value[i]);
        } else {
            err_unexpected_node(root->compound_value[i]);
        }
    }
    return (void*) 0;
}
fspec_T* visitor_visit_function_definition(scope_T* scope, AST_T* node) {
    if (node->function_definition_return_type == (void*) 0) {
        scope->return_type = scope->visitor->null_type;
    } else {
        scope->return_type = get_data_type(scope, node->function_definition_return_type);
    }
    fspec_T* fspec = init_fspec(scope->return_type);
    for (size_t i = 0; i < node->function_definition_args->unnamed_size; i++) {
        data_type_T* arg_type = get_data_type(scope, node->function_definition_args->unnamed_types[i]);
        scope_add_variable(scope, node->function_definition_args->unnamed_names[i], arg_type);
        fspec_add_unnamed_arg(fspec, arg_type);
        as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
        as_op->argno = scope->as_function->argc;
        scope->as_function->argc++;
        as_op->op_size = arg_type->primitive_size;
        as_op->var_location = scope_get_variable_relative_location(scope, node->function_definition_args->unnamed_names[i]);
        if (as_op->var_location < 0) {
            err_undefined_variable(node->function_definition_args->unnamed_types[i], node);
        }
        as_add_op_to_function(scope->as_function, as_op);
    }
    for (size_t i = 0; i < node->function_definition_args->named_size; i++) {
        data_type_T* arg_type = get_data_type(scope, node->function_definition_args->named_types[i]);
        scope_add_variable(scope, node->function_definition_args->named_inside_names[i], arg_type);
        list_add(&fspec->named_types, &fspec->named_length, arg_type);
        fspec->named_length--;
        list_add(&fspec->named_names, &fspec->named_length, node->function_definition_args->named_public_names[i]);
        as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
        as_op->argno = scope->as_function->argc;
        scope->as_function->argc++;
        as_op->op_size = arg_type->primitive_size;
        as_op->var_location = scope_get_variable_relative_location(scope, node->function_definition_args->named_inside_names[i]);
        as_add_op_to_function(scope->as_function, as_op);
    }
    data_type_T* compound_type = visitor_visit_compound(scope, node->function_definition_body, scope->return_type);
    if (compound_type == scope->visitor->null_type) {
        if (node->function_definition_return_type) {
            err_no_return_value(scope->return_type);
        } else {
            as_op_T* as_op = init_as_op(ASOP_RETNULL);
            as_add_op_to_function(scope->as_function, as_op);
        }
    }

    scope->as_function->scope_size = scope_get_scope_size(scope);
    return fspec;
}
data_type_T* visitor_visit_compound(scope_T* scope, AST_T* node, data_type_T* return_type) {
    int return_in_all_control_paths = -1;
    for (size_t i = 0; i < node->compound_size; i++) {
        // printf("Visit compound node %s\n", ast_node_type_string(node->compound_value[i]));
        if (node->compound_value[i]->type == AST_RETURN) {
            visitor_visit_return(scope, node->compound_value[i], return_type);
            return return_type;
        } else if (node->compound_value[i]->type == AST_IF) {
            data_type_T* if_return_type = visitor_visit_if(scope, node->compound_value[i]);
            if (if_return_type == scope->visitor->null_type) {
                return_in_all_control_paths = 0;
            } else if (return_in_all_control_paths < 0) {
                return_in_all_control_paths = 1;
            }
        } else {
            visitor_visit(scope, node->compound_value[i], (void*) 0);
        }
    }
    if (return_in_all_control_paths == 1) {
        return return_type;
    }
    return scope->visitor->null_type;
}
data_type_T* visitor_visit_return(scope_T* scope, AST_T* node, data_type_T* return_type) {
    if (node->return_value) {
        data_type_T* value_type = visitor_visit(scope, node->return_value, return_type);
        if (return_type && value_type != return_type) {
            err_conflicting_types(value_type, return_type, node);
        }
        as_op_T* as_op = init_as_op(ASOP_RETURN);
        as_op->op_size = value_type->primitive_size;
        as_add_op_to_function(scope->as_function, as_op);
        return value_type;
    } else {
        if (return_type != scope->visitor->null_type) {
            err_conflicting_types(scope->visitor->null_type, return_type, node);
        }
        as_op_T* as_op = init_as_op(ASOP_RETNULL);
        as_add_op_to_function(scope->as_function, as_op);
        return scope->visitor->null_type;
    }
}

data_type_T* visitor_visit_binop(scope_T* scope, AST_T* node, data_type_T* hint) {
    if (node->binop_type == BINOP_ASSIGN) {
        return visitor_visit_variable_assignment(scope, node);
    }
    data_type_T* left_hand_type = visitor_visit(scope, node->left_hand, hint);
    as_op_T* as_op = init_as_op(ASOP_NEXTREG);
    as_op->op_size = left_hand_type->primitive_size;
    as_add_op_to_function(scope->as_function, as_op);
    data_type_T* right_hand_type = visitor_visit(scope, node->right_hand, left_hand_type);
    if (left_hand_type != right_hand_type) {
        err_conflicting_types(left_hand_type, right_hand_type, node);
    }
    as_op = init_as_op(ASOP_BINOP);
    as_op->op_size = left_hand_type->primitive_size;
    as_op->binop_type = node->binop_type;
    as_op->data_type = left_hand_type;
    as_add_op_to_function(scope->as_function, as_op);
    as_op = init_as_op(ASOP_FREEREG);
    as_add_op_to_function(scope->as_function, as_op);
    if (ast_binop_is_bool(node->binop_type)) {
        return scope->visitor->bool_type;
    }
    return left_hand_type;
}
data_type_T* visitor_visit_unop(scope_T* scope, AST_T* node, data_type_T* hint) {
    data_type_T* valuetype = visitor_visit(scope, node->unop_value, (void*) 0);
    as_op_T* as_op = init_as_op(ASOP_UNOP);
    if (node->unop_type == UNOP_NEG) {
        scope->as_function->zerouses++;
    }
    as_op->unop_type = node->unop_type;
    as_op->op_size = valuetype->primitive_size;
    as_op->data_type = valuetype;
    as_add_op_to_function(scope->as_function, as_op);
    return valuetype;
}
data_type_T* visitor_visit_int(scope_T* scope, AST_T* node) {
    as_op_T* asop = init_as_op(ASOP_SETLASTIMM);
    asop->op_size = scope->visitor->int_type->primitive_size;
    asop->data_type = scope->visitor->int_type;
    asop->value.int_value = node->int_value;
    as_add_op_to_function(scope->as_function, asop);
    return scope->visitor->int_type;
}
data_type_T* visitor_visit_int_as(scope_T* scope, AST_T* node, data_type_T* data_type) {
    if (data_type->primitive_type == TYPE_CHAR || data_type->primitive_type == TYPE_UCHAR) {
        as_op_T* asop = init_as_op(ASOP_SETLASTIMM);
        if (node->int_value & ~0xff) {
            err_overflow();
        }
        asop->op_size = sizeof(int);
        asop->data_type = data_type;
        asop->value.char_value = node->int_value;
        as_add_op_to_function(scope->as_function, asop);
        return data_type;
    } else {
        as_op_T* asop = init_as_op(ASOP_SETLASTIMM);
        asop->op_size = sizeof(int);
        asop->data_type = data_type;
        asop->value.int_value = node->int_value;
        as_add_op_to_function(scope->as_function, asop);
        return data_type->primitive_type==TYPE_UINT?data_type:scope->visitor->int_type;
    }
}
data_type_T* visitor_visit_string(scope_T* scope, AST_T* node) {
    char* name = calloc(1, (4 + 1 + 5 + 3) * sizeof(char));
    sprintf(name, prefix ".string%zu", scope->visitor->as_file->unnamed_string_count);
    scope->visitor->as_file->unnamed_string_count++;
    as_data_T* as_data = init_as_data(name, scope->visitor->string_type);
    as_data->value.ptr_value = node->string_value;
    as_data->value_type = scope->visitor->string_type;
    as_add_data(scope->visitor->as_file, as_data);
    as_op_T* as_op = init_as_op(ASOP_SYMBADDRREF);
    as_op->name = name;
    as_op->op_size = 8;
    as_add_op_to_function(scope->as_function, as_op);
    return scope->visitor->string_type;
}
data_type_T* visitor_visit_variable_definition(scope_T* scope, AST_T* node) {
    if (node->variable_definition_type) {
        data_type_T* definition_type = scope_get_type(scope, node->variable_definition_type);
        
        if (definition_type==(void*) 0) {
            err_unknown_type(node->variable_definition_type, node);
        }
        if (node->variable_definition_value) {
            data_type_T* value_type = visitor_visit(scope, node->variable_definition_value, definition_type);
            if (value_type->primitive_type == TYPE_PTR) {
                if (value_type->ptr_type == definition_type) {
                    definition_type = value_type;
                } else {
                    err_conflicting_ptr_types(definition_type, value_type->ptr_type, node);
                }
            } else if (definition_type != value_type) {
                err_conflicting_types(definition_type, value_type, node);
            }
            scope_add_variable(scope, node->variable_definition_name, definition_type);
            as_op_T* as_op = init_as_op(ASOP_VDEF);
            as_op->var_location = scope_get_variable_relative_location(scope, node->variable_definition_name);
            if (as_op->var_location == 0) {
                err_undefined_variable(node->variable_definition_name, node);
            }
            as_op->op_size = definition_type->primitive_size;
            as_add_op_to_function(scope->as_function, as_op);
        } else {
            as_op_T* as_op = init_as_op(ASOP_VDEFNULL);
            as_op->op_size = definition_type->primitive_size;
            as_add_op_to_function(scope->as_function, as_op);
            scope_add_variable(scope, node->variable_definition_name, definition_type);
        }
    } else if (node->variable_definition_value->type == AST_FUNCTION_CALL) {
        data_type_T* data_type = scope_get_type(scope, node->variable_definition_value->function_call_function->variable_name);
        scope_add_variable(scope, node->variable_definition_name, data_type);


        as_op_T* as_op = init_as_op(ASOP_VREF);
        as_op->var_location = scope_get_variable_relative_location(scope, node->variable_definition_name);
        as_add_op_to_function(scope->as_function, as_op);

        as_op = init_as_op(ASOP_LEA);
        as_add_op_to_function(scope->as_function, as_op);

        // Move this address to %rax
        as_op = init_as_op(ASOP_MEMTOREG);
        as_op->argno = 0;
        as_op->op_size = 8;
        as_add_op_to_function(scope->as_function, as_op);

        fspec_T* fspec = init_fspec(data_type);
        fspec->is_class_function = 0;

        visitor_visit_function_call_args(scope, node->variable_definition_value, fspec);
        size_t i;
        for (i = 0; i < data_type->instance_functions_size; i++) {
            fspec_T* f = data_type->instance_functions[i];
            if (utils_strcmp(f->name, "constructor")) {
                if (f->unnamed_length == fspec->unnamed_length) {
                    size_t j;
                    for (j = 0; j < f->unnamed_length && f->unnamed_types[j] == fspec->unnamed_types[j]; j++)
                        ;
                    if (j == f->unnamed_length && f->named_length == fspec->named_length) {
                        for (j = 0; j < f->named_length && f->named_types[j] == fspec->named_types[j] && utils_strcmp(f->named_names[j], fspec->named_names[j]); j++)
                            ;
                        if (j == f->named_length) {
                            as_op_T* call_op = init_as_op(ASOP_FCALL);
                            call_op->op_size = 8;
                            call_op->name = f->symbol_name;
                            as_add_op_to_function(scope->as_function, call_op);
                        }
                    }
                }
            }
        }
        if (i > data_type->instance_functions_size && fspec->unnamed_length + fspec->named_length > 0) {
            err_class_no_member(data_type->type_name, "constructor", node);
        }
    } else if (node->variable_definition_value->type == AST_NEW) {
        data_type_T* data_type = scope_get_type(scope, node->variable_definition_value->new_function_call->function_call_function->variable_name);
        data_type_T* ptr_type = init_pointer_to(data_type);
        scope_add_variable(scope, node->variable_definition_name, ptr_type);

        as_op_T* as_op = init_as_op(ASOP_NEW);
        as_op->op_size = 8;
        as_add_op_to_function(scope->as_function, as_op);

        as_op = init_as_op(ASOP_VDEF);
        as_op->var_location = scope_get_variable_relative_location(scope, node->variable_definition_name);
        as_op->op_size = 8;
        as_add_op_to_function(scope->as_function, as_op);

        // Keep 'this' address in %rax

        // as_op = init_as_op(ASOP_ARGTOREG);
        // as_op->argno = 0;
        // as_op->op_size = 8;
        // as_add_op_to_function(scope->as_function, as_op);

        fspec_T* fspec = init_fspec(ptr_type);
        fspec->is_class_function = 0;

        visitor_visit_function_call_args(scope, node->variable_definition_value->new_function_call, fspec);
        size_t i;
        for (i = 0; i < data_type->instance_functions_size; i++) {
            fspec_T* f = data_type->instance_functions[i];
            if (utils_strcmp(f->name, "constructor")) {
                if (f->unnamed_length == fspec->unnamed_length) {
                    size_t j;
                    for (j = 0; j < f->unnamed_length && f->unnamed_types[j] == fspec->unnamed_types[j]; j++)
                        ;
                    if (j == f->unnamed_length && f->named_length == fspec->named_length) {
                        for (j = 0; j < f->named_length && f->named_types[j] == fspec->named_types[j] && utils_strcmp(f->named_names[j], fspec->named_names[j]); j++)
                            ;
                        if (j == f->named_length) {
                            as_op_T* call_op = init_as_op(ASOP_FCALL);
                            call_op->op_size = 8;
                            call_op->name = f->symbol_name;
                            as_add_op_to_function(scope->as_function, call_op);
                        }
                    }
                }
            }
        }
        if (i > data_type->instance_functions_size && fspec->unnamed_length + fspec->named_length > 0) {
            err_class_no_member(data_type->type_name, "constructor", node);
        }
    } else {
        err_unexpected_node(node);
    }
    return (void*) 0;
}
data_type_T* visitor_visit_variable_assignment(scope_T* scope, AST_T* node) {
    data_type_T* variable_type = (void*) 0;
    if (node->left_hand->type == AST_VARIABLE) {
        variable_type = visitor_visit_variable(scope, node->left_hand);
    } else if (node->left_hand->type == AST_MEMBER) {
        variable_type = visitor_visit_member(scope, node->left_hand);
    }
    if (variable_type==(void*) 0) {
        err_undefined_variable(node->left_hand->variable_name, node);
    }

    as_op_T* as_op = init_as_op(ASOP_SETDEST);
    as_add_op_to_function(scope->as_function, as_op);
    data_type_T* value_type = visitor_visit(scope, node->right_hand, variable_type);
    if (variable_type != value_type) {
        err_conflicting_types(variable_type, value_type, node);
    }
    as_op = init_as_op(ASOP_VMOD);
    as_op->op_size = variable_type->primitive_size;
    as_add_op_to_function(scope->as_function, as_op);
    as_op = init_as_op(ASOP_FREEDEST);
    as_add_op_to_function(scope->as_function, as_op);
    return variable_type;
}
data_type_T* visitor_visit_variable(scope_T* scope, AST_T* node) {
    data_type_T* data_type = scope_get_variable(scope, node->variable_name);
    if (data_type == (void*) 0) {
        err_undefined_variable(node->variable_name, node);
    }
    as_op_T* as_op = init_as_op(ASOP_VREF);
    as_op->var_location = scope_get_variable_relative_location(scope, node->variable_name);
    if (as_op->var_location == 0) {
        err_undefined_variable(node->variable_name, node);
    }
    as_op->op_size = data_type->primitive_size;
    as_add_op_to_function(scope->as_function, as_op);
    return data_type;
}
data_type_T* visitor_visit_member(scope_T* scope, AST_T* node) {
    data_type_T* parent = visitor_visit(scope, node->member_parent, (void*) 0);
    size_t memb_offset = 0;
    if (parent->primitive_type == TYPE_PTR) {
        as_op_T* as_op = init_as_op(ASOP_MEMTOREG);
        as_op->op_size = parent->primitive_size;
        as_add_op_to_function(scope->as_function, as_op);
        for (size_t i = 0; i < parent->ptr_type->instance_members_size; i++) {
            // printf("Cmp member %s %x %zu\n", parent->ptr_type->instance_member_names[i], parent->ptr_type->instance_member_types[i]->is_private);
            if ((parent->ptr_type->instance_member_types[i]->is_private == 0 || parent->is_this == 1) && utils_strcmp(node->member_name, parent->ptr_type->instance_member_names[i])) {
                as_op_T* as_op = init_as_op(ASOP_MEMBREF);
                as_op->op_size = parent->ptr_type->instance_member_types[i]->primitive_size;
                as_op->memb_offset = memb_offset;
                as_add_op_to_function(scope->as_function, as_op);
                return parent->ptr_type->instance_member_types[i];
            }
            memb_offset += parent->ptr_type->instance_member_types[i]->primitive_size;
        }
    } else {
        for (size_t i = 0; i < parent->instance_members_size; i++) {
            if (parent->instance_member_types[i]->is_private == 0 && utils_strcmp(node->member_name, parent->instance_member_names[i])) {
                as_op_T* as_op = init_as_op(ASOP_LOCALMEMB);
                as_op->op_size = parent->instance_member_types[i]->primitive_size;
                as_op->memb_offset = memb_offset;
                as_add_op_to_function(scope->as_function, as_op);
                return parent->instance_member_types[i];
            }
            memb_offset += parent->instance_member_types[i]->primitive_size;
        }
    }
    err_class_no_member(parent->type_name, node->member_name, node);
    return (void*) 0;
}
void visitor_visit_function_call_args(scope_T* scope, AST_T* node, fspec_T* fspec) {
    int argno = 0;
    if (fspec->is_class_function == 1) {
        argno = 1;
    }
    for (size_t i = 0; i < node->function_call_unnamed_size; i++) {
        data_type_T* arg_type = visitor_visit(scope, node->function_call_unnamed_values[i], (void*) 0);
        list_add(&fspec->unnamed_types, &fspec->unnamed_length, arg_type);
        as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
        as_op->op_size = arg_type->primitive_size;
        as_op->argno = argno;
        argno++;
        as_add_op_to_function(scope->as_function, as_op);
    }
    for (size_t i = 0; i < node->function_call_named_size; i++) {
        data_type_T* arg_type = visitor_visit(scope, node->function_call_named_values[i], (void*) 0);
        list_add(&fspec->named_names, &fspec->named_length, node->function_call_named_names[i]);
        fspec->named_length--;
        list_add(&fspec->named_types, &fspec->named_length, arg_type);
        as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
        as_op->op_size = arg_type->primitive_size;
        as_op->argno = argno;
        argno++;
        as_add_op_to_function(scope->as_function, as_op);
    }
}
data_type_T* visitor_visit_function_call(scope_T* scope, AST_T* node) {
    fspec_T* fspec = init_fspec((void*) 0);
    // int argno = 0;
    if (node->function_call_function->type == AST_VARIABLE) {
        fspec->is_class_function = 0;
    } else {
        fspec->is_class_function = 1;
    }
    // if (node->function_call_function->type == AST_VARIABLE) {
    //     fspec = scope_get_function(scope, node->function_call_function->variable_name);
    //     if (!fspec) {
    //         if (scope->has_this) {
    //             data_type_T* this_type = scope_get_variable(scope, "this");
    //             for (size_t i = 0; i < this_type->instance_functions_size; i++) {
    //                 if (utils_strcmp(node->function_call_function->variable_name, this_type->instance_functions[i]->name)) {
    //                     fspec = this_type->instance_functions[i];
    //                 }
    //             }
    //         }
    //         err_undefined_function(node->function_call_function->variable_name, node);
    //     }
    // } else if (node->function_call_function->type == AST_MEMBER) {
    //     data_type_T* parent_type = visitor_visit(scope, node->function_call_function->member_parent, (void*) 0);
    //     if (parent_type->primitive_type == TYPE_PTR) {
    //         for (size_t i = 0; i < parent_type->ptr_type->instance_functions_size; i++) {
    //             if (utils_strcmp(node->function_call_function->member_name, parent_type->ptr_type->instance_functions[i]->name)) {
    //                 fspec = parent_type->ptr_type->instance_functions[i];
    //             }
    //         }
    //         if (!fspec) {
    //             err_class_no_member(parent_type->ptr_type->type_name, node->function_call_function->member_name);
    //         }
    //         as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
    //         as_op->op_size = parent_type->primitive_size;
    //         as_op->argno = argno++;
    //         as_add_op_to_function(scope->as_function, as_op);
    //     } else {
    //         for (size_t i = 0; i < parent_type->instance_functions_size; i++) {
    //             if (utils_strcmp(node->function_call_function->member_name, parent_type->instance_functions[i]->name)) {
    //                 fspec = parent_type->instance_functions[i];
    //             }
    //         }
    //         if (!fspec) {
    //             err_class_no_member(parent_type->type_name, node->function_call_function->member_name);
    //         }
    //     }
        
    //     // if (parent_type->primitive_type != TYPE_STATICCLASS) {
    //         // as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
    //         // as_op->op_size = parent_type->primitive_size;
    //         // as_op->argno = argno;
    //         // argno++;
    //         // as_add_op_to_function(scope->as_function, as_op);
    //     // }
    // }
    data_type_T* parent_type = (void*) 0;
    if (node->function_call_function->type == AST_MEMBER) {
        parent_type = visitor_visit(scope, node->function_call_function->member_parent, (void*) 0);
        if (parent_type->primitive_type == TYPE_STRUCT) {
            as_op_T* as_op = init_as_op(ASOP_LEA);
            as_add_op_to_function(scope->as_function, as_op);
        }
        as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
        as_op->op_size = 8;
        as_op->argno = 0;
        as_add_op_to_function(scope->as_function, as_op);
    }
    visitor_visit_function_call_args(scope, node, fspec);
    if (node->function_call_function->type == AST_VARIABLE) {
        for (size_t i = 0; i < scope->global->functions_size; i++) {
            fspec_T* f = scope->global->function_specs[i];
            if (utils_strcmp(f->name, node->function_call_function->variable_name)) {
                if (f->unnamed_length == fspec->unnamed_length) {
                    size_t j;
                    for (j = 0; j < f->unnamed_length && f->unnamed_types[j] == fspec->unnamed_types[j]; j++)
                        ;
                    if (j == f->unnamed_length && f->named_length == fspec->named_length) {
                        for (j = 0; j < f->named_length && f->named_types[j] == fspec->named_types[j] && utils_strcmp(f->named_names[j], fspec->named_names[j]); j++)
                            ;
                        if (j == f->named_length) {
                            char* as_function_name = f->symbol_name;
                            as_op_T* call_op = init_as_op(ASOP_FCALL);
                            call_op->op_size = 8;
                            call_op->name = as_function_name;
                            as_add_op_to_function(scope->as_function, call_op);
                            if (f->return_type->primitive_type != TYPE_NULL) {
                                as_op_T* retval_op = init_as_op(ASOP_RETVAL);
                                retval_op->op_size = f->return_type->primitive_size;
                                as_add_op_to_function(scope->as_function, retval_op);
                            }
                            return f->return_type;
                        }
                    }
                }
            }
        }
        err_undefined_function(node->function_call_function->variable_name, node);
    } else {
        char is_this = parent_type->is_this;
        if (parent_type->primitive_type == TYPE_PTR) {
            parent_type = parent_type->ptr_type;
        }
        for (size_t i = 0; i < parent_type->instance_functions_size; i++) {
            fspec_T* f = parent_type->instance_functions[i];
            if ((f->is_private == 0 || is_this == 1) && utils_strcmp(f->name, node->function_call_function->member_name)) {
                if (f->unnamed_length == fspec->unnamed_length) {
                    size_t j;
                    for (j = 0; j < f->unnamed_length && f->unnamed_types[j] == fspec->unnamed_types[j]; j++)
                        ;
                    if (j == f->unnamed_length && f->named_length == fspec->named_length) {
                        for (j = 0; j < f->named_length && f->named_types[j] == fspec->named_types[j] && utils_strcmp(f->named_names[j], fspec->named_names[j]); j++)
                            ;
                        if (j == f->named_length) {
                            char* as_function_name = f->symbol_name;
                            as_op_T* call_op = init_as_op(ASOP_FCALL);
                            call_op->op_size = 8;
                            call_op->name = as_function_name;
                            as_add_op_to_function(scope->as_function, call_op);
                            if (f->return_type->primitive_type != TYPE_NULL) {
                                as_op_T* retval_op = init_as_op(ASOP_RETVAL);
                                retval_op->op_size = f->return_type->primitive_size;
                                as_add_op_to_function(scope->as_function, retval_op);
                            }
                            return f->return_type;
                        }
                    }
                }
            }
        }
        err_class_no_member(parent_type->type_name, node->function_call_function->member_name, node);
            // for (size_t i = 0; i < parent_type->instance_functions_size; i++) {
            //     if (utils_strcmp(node->function_call_function->member_name, parent_type->instance_functions[i]->name)) {
            //         fspec = parent_type->instance_functions[i];
            //     }
            // }
            // if (!fspec) {
            //     err_class_no_member(parent_type->type_name, node->function_call_function->member_name);
            // }
    }
    return (void*) 0;
}
data_type_T* visitor_visit_class_definition(global_T* scope, AST_T* node) {
    data_type_T* data_type = init_data_type(TYPE_STRUCT, node->class_name);
    data_type_T* this_type = init_data_type(TYPE_PTR, (void*) 0);
    data_type->primitive_size = 0;
    this_type->ptr_type = data_type;
    this_type->primitive_size = 8;
    this_type->is_this = 1;
    for (size_t i = 0; i < node->class_prototype_names_size; i++) {
        data_type_T* prototype_type = scope_get_typeg(scope, node->class_prototype_names[i]);
        if (prototype_type == (void*) 0) {
            err_unknown_type(node->class_prototype_names[i], node);
        }
        list_add(&data_type->class_prototypes, &data_type->class_prototypes_size, prototype_type);
    }
    for (size_t i = 0; i < node->class_members_size; i++) {
        AST_T* def = node->class_members[i];
        // printf("Class member type %s\n", ast_node_type_string(def));
        if (def->type == AST_VARIABLE_DEFINITION) {
            data_type_T* member_type = scope_get_typeg(scope, def->variable_definition_type);
            if (def->variable_definition_is_static == 1) {
                list_add(&data_type->static_member_types, &data_type->static_members_size, member_type);
                data_type->static_members_size--;
                list_add(&data_type->static_member_names, &data_type->static_members_size, def->variable_definition_name);
            } else {
                list_add(&data_type->instance_member_types, &data_type->instance_members_size, member_type);
                data_type->instance_members_size--;
                list_add(&data_type->instance_member_names, &data_type->instance_members_size, def->variable_definition_name);
            }
            data_type->primitive_size += member_type->primitive_size;
            member_type->is_private = def->variable_definition_is_private;
        } else if (def->type == AST_FUNCTION_DEFINITION) {
            as_function_T* as_function = init_as_function((void*) 0);
            scope_T* function_scope = init_function_scope(scope, as_function);
            as_add_function(scope->visitor->as_file, as_function);
            
            if (def->function_definition_is_static == 0) {
                scope_add_variable(function_scope, "this", this_type);
                as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
                as_op->argno = 0;
                as_op->op_size = this_type->primitive_size;
                as_op->var_location = scope_get_variable_relative_location(function_scope, "this");
                function_scope->as_function->argc++;
                as_add_op_to_function(function_scope->as_function, as_op);
            }
            fspec_T* fspec = visitor_visit_function_definition(function_scope, def);
            fspec->name = def->function_definition_name;
            fspec->is_class_function = !def->function_definition_is_static;
            function_scope->has_this = !def->function_definition_is_static;
            fspec->is_private = def->function_definition_is_private;
            fspec->symbol_name = realloc(fspec->symbol_name, (prefix_len + 3 + strlen(data_type->type_name) + 3 + strlen(def->function_definition_name)) * sizeof(char));
            function_scope->as_function->name = fspec->symbol_name;
            sprintf(fspec->symbol_name, prefix "%zu%s%zu%s", strlen(data_type->type_name), data_type->type_name, strlen(def->function_definition_name), def->function_definition_name);
            for (size_t j = 0; j < fspec->unnamed_length; j++) {
                char* type_name = visitor_data_type_to_arg_name(fspec->unnamed_types[j]);
                fspec->symbol_name = realloc(fspec->symbol_name, (strlen(fspec->symbol_name) + strlen(type_name) + 2) * sizeof(char));
                sprintf(fspec->symbol_name, "%s_%s", fspec->symbol_name, type_name);
                free(type_name);
            }
            for (size_t j = 0; j < fspec->named_length; j++) {
                char* type_name = visitor_data_type_to_arg_name(fspec->named_types[j]);
                fspec->symbol_name = realloc(fspec->symbol_name, (strlen(fspec->symbol_name) + 4 + strlen(fspec->named_names[j]) + strlen(type_name)) * sizeof(char));
                sprintf(fspec->symbol_name, "%s%zu%s%s", fspec->symbol_name, strlen(fspec->named_names[j]), fspec->named_names[j], type_name);
                free(type_name);
            }
            char* return_type_name = visitor_data_type_to_arg_name(fspec->return_type);
            fspec->symbol_name = realloc(fspec->symbol_name, (strlen(fspec->symbol_name) + strlen(return_type_name) + 1) * sizeof(char));
            sprintf(fspec->symbol_name, "%s%s", fspec->symbol_name, return_type_name);
            free(return_type_name);
            if (def->function_definition_is_static == 1) {
                list_add(&data_type->static_functions, &data_type->static_functions_size, fspec);
            } else {
                list_add(&data_type->instance_functions, &data_type->instance_functions_size, fspec);
            }
            
            if (utils_strcmp(def->function_definition_name, node->class_name)) {
                scope_add_functiong(scope, fspec);
            }
        } else if (def->type == AST_CONSTRUCTOR) {
            fspec_T* fspec = init_fspec(data_type);
            fspec->symbol_name = calloc(1, (prefix_len + 3 + strlen(data_type->type_name) + strlen("C") + 1) *sizeof(char));
            sprintf(fspec->symbol_name, prefix "%zu%sC", strlen(data_type->type_name), data_type->type_name);
            as_function_T* as_function = init_as_function((void*) 0);
            as_add_function(scope->visitor->as_file, as_function);
            scope_T* constructor_scope = init_function_scope(scope, as_function);
            scope_add_variable(constructor_scope, "this", this_type);


            // Address of "this" is in %rax
            as_op_T* as_op = init_as_op(ASOP_RETVAL);
            as_op->op_size = this_type->primitive_size;
            as_add_op_to_function(as_function, as_op);

            as_op = init_as_op(ASOP_VDEF);
            as_op->op_size = this_type->primitive_size;
            as_op->var_location = scope_get_variable_relative_location(constructor_scope, "this");
            as_add_op_to_function(as_function, as_op);

            for (size_t j = 0; j < def->constructor_args->unnamed_size; j++) {
                data_type_T* arg_type = scope_get_typeg(scope, def->constructor_args->unnamed_types[j]);
                if (arg_type->primitive_type == TYPE_STRUCT) {
                    arg_type = init_pointer_to(arg_type);
                }
                list_add(&fspec->unnamed_types, &fspec->unnamed_length, arg_type);
                scope_add_variable(constructor_scope, def->constructor_args->unnamed_names[j], arg_type);
                as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
                as_op->op_size = arg_type->primitive_size;
                as_op->argno = j;
                as_op->var_location = scope_get_variable_relative_location(constructor_scope, def->constructor_args->unnamed_names[j]);
                as_add_op_to_function(as_function, as_op);
                char* arg_name = visitor_data_type_to_arg_name(data_type);
                fspec->symbol_name = realloc(fspec->symbol_name, strlen(fspec->symbol_name) + strlen(arg_name) + 1);
                strcat(fspec->symbol_name, arg_name);
                free(arg_name);
            }
            for (size_t j = 0; j < def->constructor_args->named_size; j++) {
                data_type_T* arg_type = scope_get_typeg(scope, def->constructor_args->named_types[j]);
                if (arg_type->primitive_type == TYPE_STRUCT) {
                    arg_type = init_pointer_to(arg_type);
                }
                list_add(&fspec->named_types, &fspec->named_length, arg_type);
                fspec->named_length--;
                list_add(&fspec->named_names, &fspec->named_length, def->constructor_args->named_public_names[j]);
                scope_add_variable(constructor_scope, def->constructor_args->named_inside_names[j], arg_type);
                as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
                as_op->op_size = arg_type->primitive_size;
                as_op->argno = def->constructor_args->unnamed_size + j;
                as_op->var_location = scope_get_variable_relative_location(constructor_scope, def->constructor_args->named_inside_names[j]);
                as_add_op_to_function(as_function, as_op);
                char* arg_name = visitor_data_type_to_arg_name(data_type);
                fspec->symbol_name = realloc(fspec->symbol_name, strlen(fspec->symbol_name) + strlen(arg_name) + 1);
                strcat(fspec->symbol_name, arg_name);
                free(arg_name);
            }
            data_type_T* return_type = visitor_visit_compound(constructor_scope, def->constructor_function_body, (void*) 0);
            if (return_type != data_type && return_type != scope->visitor->null_type) {
                err_conflicting_types(return_type, data_type, def);
            } else if (return_type == scope->visitor->null_type) {
                as_op_T* as_op = init_as_op(ASOP_RETNULL);
                as_add_op_to_function(as_function, as_op);
            }
            fspec->name = "constructor";
            as_function->name = fspec->symbol_name;
            list_add(&data_type->instance_functions, &data_type->instance_functions_size, fspec);
        } else {
            err_unexpected_node(def);
        }
    }
    if (data_type->instance_members_size == 0) {
        err_empty_class(data_type->type_name);
    }
    scope_add_typeg(scope, node->class_name, data_type);
    return data_type;
}
data_type_T* visitor_visit_new(scope_T* scope, AST_T* node) {
    as_op_T* as_op = init_as_op(ASOP_NEW);
    as_op->op_size = 8;
    as_add_op_to_function(scope->as_function, as_op);
    data_type_T* data_type;
    if (node->new_function_call->type == AST_FUNCTION_CALL) {
        data_type = scope_get_type(scope, node->new_function_call->function_call_function->variable_name);
    } else if (node->new_function_call->type == AST_VARIABLE) {
        data_type = scope_get_type(scope, node->new_function_call->variable_name);
    } else {
        data_type = NULL;
        err_unexpected_node(node->new_function_call);
    }
    as_op->data_type = data_type;
    data_type_T* new_type = init_pointer_to(data_type);
    new_type->primitive_size = 8;
    if (node->new_function_call->type == AST_FUNCTION_CALL) {
        as_op_T* push_op = init_as_op(ASOP_PUSHREG);
        push_op->op_size = 8;
        as_add_op_to_function(scope->as_function, push_op);
        
        if (node->new_function_call->function_call_unnamed_size + node->new_function_call->function_call_named_size) {
            as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
            as_op->argno = 0;
            as_op->op_size = 8;
            as_add_op_to_function(scope->as_function, as_op);
        }

        fspec_T* fspec = init_fspec(new_type);
        fspec->is_class_function = 1;
        visitor_visit_function_call_args(scope, node->new_function_call, fspec);
        size_t i;
        for (i = 0; i < data_type->instance_functions_size; i++) {
            fspec_T* f = data_type->instance_functions[i];
            if (utils_strcmp(f->name, "constructor")) {
                if (f->unnamed_length == fspec->unnamed_length) {
                    size_t j;
                    for (j = 0; j < f->unnamed_length && f->unnamed_types[j] == fspec->unnamed_types[j]; j++)
                        ;
                    if (j == f->unnamed_length && f->named_length == fspec->named_length) {
                        for (j = 0; j < f->named_length && f->named_types[j] == fspec->named_types[j] && utils_strcmp(f->named_names[j], fspec->named_names[j]); j++)
                            ;
                        if (j == f->named_length) {
                            char* as_function_name = f->symbol_name;
                            as_op_T* call_op = init_as_op(ASOP_FCALL);
                            call_op->op_size = 8;
                            call_op->name = as_function_name;
                            as_add_op_to_function(scope->as_function, call_op);
                            as_op = init_as_op(ASOP_POPREG);
                            as_op->op_size = 8;
                            as_add_op_to_function(scope->as_function, as_op);
                            if ((scope->as_function->last_stack_offset += 8) > scope->as_function->visitor_max_extra_stack) {
                                scope->as_function->visitor_max_extra_stack = scope->as_function->last_stack_offset;
                            }
                            scope->as_function->last_stack_offset -= 8;
                        }
                    }
                }
            }
        }
        if (i > data_type->instance_functions_size && fspec->unnamed_length + fspec->named_length > 0) {
            err_class_no_member(data_type->type_name, "constructor", node);
        } else {
            push_op->type = ASOP_NOP;
        }
    } else if (node->new_function_call->type != AST_VARIABLE) {
        err_unexpected_node(node->new_function_call);
    }
    return new_type;
}
data_type_T* visitor_visit_if(scope_T* scope, AST_T* node) {
    data_type_T* condition = visitor_visit(scope, node->if_condition, scope->visitor->bool_type);
    if (condition != scope->visitor->bool_type) {
        err_conflicting_types(condition, scope->visitor->bool_type, node->if_condition);
    }
    as_op_T* start_op = init_as_op(ASOP_OPENIF);
    as_add_op_to_function(scope->as_function, start_op);

    scope_T* if_scope = init_scope(scope);
    data_type_T* return_type = visitor_visit_compound(if_scope, node->if_body, scope->return_type);
    as_op_T* close_op = init_as_op(ASOP_CLOSEIF);
    start_op->bb_ptr = close_op;
    AST_T* last_statement = node->next_statement;
    as_op_T* last_bb_opener = start_op;
    as_op_T* last_else = NULL;
    while (last_statement) {
        if (last_statement->type == AST_ELSE) {
            scope_T* else_scope = init_scope(scope);
            as_op_T* as_op = init_as_op(ASOP_ELSE);
            last_bb_opener->bb_ptr = as_op;
            as_op->bb_no = -1;
            as_op->bb_ptr = close_op;
            as_add_op_to_function(scope->as_function, as_op);
            data_type_T* else_return_type = visitor_visit_compound(else_scope, last_statement->else_body, scope->return_type);
            if (else_return_type == scope->visitor->null_type) {
                return_type = scope->visitor->null_type;
            }
            // last_bb_opener = as_op;
            break;
        } else if (last_statement->type == AST_ELIF) {
            scope_T* elif_scope = init_scope(scope);
            as_op_T* else_op = init_as_op(ASOP_ELSE);
            last_bb_opener->bb_ptr = else_op;
            else_op->bb_no = -1;
            as_add_op_to_function(scope->as_function, else_op);
            condition = visitor_visit(scope, last_statement->elif_condition, scope->visitor->bool_type);
            if (condition != scope->visitor->bool_type) {
                err_conflicting_types(condition, scope->visitor->bool_type, last_statement->elif_body);
            }
            as_op_T* as_op = init_as_op(ASOP_OPENIF);
            as_op->bb_ptr = close_op;
            as_add_op_to_function(scope->as_function, as_op);
            data_type_T* elif_return_type = visitor_visit_compound(elif_scope, last_statement->elif_body, scope->return_type);
            if (elif_return_type == scope->visitor->null_type) {
                return_type = scope->visitor->null_type;
            }
            as_op->bb_no = -1;
            else_op->bb_ptr = close_op;
            last_else = else_op;
            last_bb_opener = as_op;
        } else {
            err_unexpected_node(last_statement);
        }
        last_statement = last_statement->next_statement;
    }
    // if (close_op->type == ASOP_ELSE) {
    close_op->bb_no = -1;
    // } else {
    //     last_bb_opener->bb_ptr = close_op;
    // }
    as_add_op_to_function(scope->as_function, close_op);
    return return_type;
}