#include "include/visitor.h"
#include <string.h>
#include "include/utils.h"
#include "include/errors.h"
#include "include/defs.h"
#define prefix "_Pre"
#define prefix_len 4
visitor_T* init_visitor(as_file_T* as_file) {
    visitor_T* visitor = calloc(1, sizeof(struct VISITOR_STRUCT));
    visitor->noop = init_ast(AST_NOOP);

    visitor->int_type = init_data_type(TYPE_INT, "Int");
    visitor->int_type->primitive_size = 4;
    visitor->null_type = init_data_type(TYPE_NULL, "Null");
    visitor->null_type->primitive_size = 0;
    visitor->string_type = init_data_type(TYPE_STRING, "String");
    visitor->string_type->primitive_size = 8;

    visitor->global_scope = init_scope((void*) 0);

    scope_add_variable(visitor->global_scope, "Int", visitor->int_type);
    scope_add_variable(visitor->global_scope, "Null", visitor->null_type);
    scope_add_variable(visitor->global_scope, "String", visitor->string_type);
    visitor->as_file = as_file;

    defs_define_all(visitor->global_scope);
    
    return visitor;
}
data_type_T* get_data_type(visitor_T* visitor, scope_T* scope, char* name) {
    if (name == (void*) 0) {
        return (void*) 0;
    } else if (utils_strcmp(name, "Int")) {
        return visitor->int_type;
    } else if (utils_strcmp(name, "String")) {
        return visitor->string_type;
    } else {
        data_type_T* data_type = scope_get_variable(visitor->global_scope, name);
        if (data_type) {
            return data_type->class_type;
        } else {
            return (void*) 0;
        }
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
        case AST_STRING: return visitor_visit_string(visitor, scope, node, as_function); break;
        case AST_VARIABLE_DEFINITION: return visitor_visit_variable_definition(visitor, scope, node, as_function); break;
        case AST_VARIABLE: return visitor_visit_variable(visitor, scope, node, as_function); break;
        case AST_FUNCTION_CALL: return visitor_visit_function_call(visitor, scope, node, as_function); break;
        case AST_BINOP: return visitor_visit_binop(visitor, scope, node, as_function); break;
        case AST_MEMBER: return visitor_visit_member(visitor, scope, node, as_function); break;
        case AST_NEW: return visitor_visit_new(visitor, scope, node, as_function); break;
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
            as_function_T* as_function = init_as_function(root->compound_value[i]->function_definition_name);
            scope_T* function_scope = init_scope(visitor->global_scope);
            fspec_T* function_definition = visitor_visit_function_definition(visitor, function_scope, root->compound_value[i], as_function);
            function_definition->symbol_name = root->compound_value[i]->function_definition_name;
            if (utils_strcmp(as_function->name, "start")) {
                as_function->name = "_start";
            }
            scope_add_function(visitor->global_scope, root->compound_value[i]->function_definition_name, function_definition);
        } else if (root->compound_value[i]->type == AST_CLASS_DEFINITION) {
            data_type_T* class_type = visitor_visit_class_definition(visitor, visitor->global_scope, root->compound_value[i]);
        } else {
            err_unexpected_node(root->compound_value[i]);
        }
    }
    return (void*) 0;
}
fspec_T* visitor_visit_function_definition(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    data_type_T* return_type;
    if (node->function_definition_return_type == (void*) 0) {
        return_type = visitor->null_type;
    } else {
        return_type = get_data_type(visitor, scope, node->function_definition_return_type);
    }
    fspec_T* fspec = init_fspec(return_type);
    as_add_function(visitor->as_file, as_function);
    for (size_t i = 0; i < node->function_definition_args->unnamed_size; i++) {
        data_type_T* arg_type = get_data_type(visitor, scope, node->function_definition_args->unnamed_types[i]);
        scope_add_variable(scope, node->function_definition_args->unnamed_names[i], arg_type);
        fspec_add_unnamed_arg(fspec, arg_type);
        as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
        as_op->argno = as_function->argc;
        as_function->argc++;
        as_op->op_size = arg_type->primitive_size;
        as_op->var_location = scope_get_variable_relative_location(scope, node->function_definition_args->unnamed_names[i]);
        if (as_op->var_location < 0) {
            err_undefined_variable(node->function_definition_args->unnamed_types[i]);
        }
        as_add_op_to_function(as_function, as_op);
        // char* arg_name = visitor_data_type_to_arg_name(visitor, scope, arg_type);
        // fspec->symbol_name = realloc(fspec->symbol_name, strlen(fspec->symbol_name) + strlen(arg_name) + 1);
        // strcat(fspec->symbol_name, arg_name);
    }
    for (size_t i = 0; i < node->function_definition_args->named_size; i++) {
        data_type_T* arg_type = get_data_type(visitor, scope, node->function_definition_args->named_types[i]);
        scope_add_variable(scope, node->function_definition_args->named_inside_names[i], arg_type);
        list_add(&fspec->named_types, &fspec->named_length, arg_type);
        fspec->named_length--;
        list_add(&fspec->named_names, &fspec->named_length, node->function_definition_args->named_public_names[i]);
        as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
        as_op->argno = as_function->argc;
        as_function->argc++;
        as_op->op_size = arg_type->primitive_size;
        as_op->var_location = scope_get_variable_relative_location(scope, node->function_definition_args->named_inside_names[i]);
        as_add_op_to_function(as_function, as_op);
        // char* arg_name = visitor_data_type_to_arg_name(visitor, scope, arg_type);
        // fspec->symbol_name = realloc(fspec->symbol_name, strlen(fspec->symbol_name) + strlen(arg_name) + 1);
        // strcat(fspec->symbol_name, arg_name);
    }
    data_type_T* compound_type = visitor_visit_compound(visitor, scope, node->function_definition_body, return_type, as_function);
    if (compound_type == visitor->null_type) {
        if (node->function_definition_return_type) {
            err_no_return_value(return_type);
        } else {
            as_op_T* as_op = init_as_op(ASOP_RETNULL);
            as_add_op_to_function(as_function, as_op);
        }
    }
    as_function->scope_size = scope_get_scope_size(scope);
    return fspec;
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
data_type_T* visitor_visit_int(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    as_op_T* asop = init_as_op(ASOP_SETLASTIMM);
    asop->op_size = visitor->int_type->primitive_size;
    asop->data_type = visitor->int_type;
    asop->value.int_value = node->int_value;
    as_add_op_to_function(as_function, asop);
    return visitor->int_type;
}
data_type_T* visitor_visit_string(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    char* name = calloc(1, (4 + 1 + 5 + 3) * sizeof(char));
    sprintf(name, prefix ".string%lu", visitor->as_file->unnamed_string_count);
    visitor->as_file->unnamed_string_count++;
    as_data_T* as_data = init_as_data(name, visitor->string_type);
    as_data->value.ptr_value = node->string_value;
    as_add_data(visitor->as_file, as_data);
    as_op_T* as_op = init_as_op(ASOP_SYMBADDRREF);
    as_op->name = name;
    as_add_op_to_function(as_function, as_op);
    return visitor->string_type;
}
data_type_T* visitor_visit_variable_definition(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    data_type_T* definition_type = get_data_type(visitor, scope, node->variable_definition_type);
    if (definition_type==(void*) 0) {
        err_unknown_type(node->variable_definition_type);
    }
    scope_add_variable(scope, node->variable_definition_name, definition_type);
    if (node->variable_definition_value) {
        data_type_T* value_type = visitor_visit(visitor, scope, node->variable_definition_value, as_function);
        if (definition_type != value_type) {
            err_conflicting_types(definition_type, value_type);
        }
        as_op_T* as_op = init_as_op(ASOP_VDEF);
        as_op->var_location = scope_get_variable_relative_location(scope, node->variable_definition_name);
        if (as_op->var_location < 0) {
            err_undefined_variable(node->variable_definition_name);
        }
        as_op->op_size = definition_type->primitive_size;
        as_add_op_to_function(as_function, as_op);
    } else {
        as_op_T* as_op = init_as_op(ASOP_VDEFNULL);
        as_op->op_size = definition_type->primitive_size;
        as_add_op_to_function(as_function, as_op);
    }
    return definition_type;
}
data_type_T* visitor_visit_variable_assignment(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    data_type_T* variable_type = (void*) 0;
    if (node->left_hand->type == AST_VARIABLE) {
        variable_type = visitor_visit_variable(visitor, scope, node->left_hand, as_function);
    } else if (node->left_hand->type == AST_MEMBER) {
        variable_type = visitor_visit_member(visitor, scope, node->left_hand, as_function);
    }
    if (variable_type==(void*) 0) {
        err_undefined_variable(node->left_hand->variable_name);
    }
    as_op_T* as_op = init_as_op(ASOP_SETDEST);
    as_add_op_to_function(as_function, as_op);
    data_type_T* value_type = visitor_visit(visitor, scope, node->right_hand, as_function);
    if (variable_type != value_type) {
        err_conflicting_types(variable_type, value_type);
    }
    as_op = init_as_op(ASOP_VMOD);
    as_op->op_size = variable_type->primitive_size;
    as_add_op_to_function(as_function, as_op);
    as_op = init_as_op(ASOP_FREEDEST);
    as_add_op_to_function(as_function, as_op);
    return variable_type;
}
data_type_T* visitor_visit_variable(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    data_type_T* data_type = scope_get_variable(scope, node->variable_name);
    if (data_type == (void*) 0) {
        err_undefined_variable(node->variable_name);
    }
    if (data_type->primitive_type == TYPE_STATICCLASS) {
        return data_type;
    } else {
        as_op_T* as_op = init_as_op(ASOP_VREF);
        as_op->var_location = scope_get_variable_relative_location(scope, node->variable_name);
        as_op->op_size = data_type->primitive_size;
        as_add_op_to_function(as_function, as_op);
        return data_type;
    }
}
data_type_T* visitor_visit_member(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    data_type_T* parent = visitor_visit(visitor, scope, node->member_parent, as_function);
    size_t memb_offset = 0;
    for (size_t i = 0; i < parent->class_members_size; i++) {
        if (utils_strcmp(node->member_name, parent->class_member_names[i])) {
            as_op_T* as_op = init_as_op(ASOP_MEMBREF);
            as_op->op_size = parent->class_member_types[i]->primitive_size;
            as_op->memb_offset = memb_offset;
            as_add_op_to_function(as_function, as_op);
            return parent->class_member_types[i];
        }
        memb_offset += parent->class_member_types[i]->primitive_size;
    }
    err_class_no_member(parent->type_name, node->member_name);
    return (void*) 0;
}
void visitor_visit_function_call_args(visitor_T* visitor, scope_T* scope, AST_T* node, fspec_T* fspec, as_function_T* as_function) {
    int argno = 0;
    if (fspec->is_class_function == 1) {
        argno = 1;
    }
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
        as_op->argno = argno;
        argno++;
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
        as_op->argno = argno;
        argno++;
        as_add_op_to_function(as_function, as_op);
    }
}
data_type_T* visitor_visit_function_call(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    fspec_T* fspec = (void*) 0;
    int argno = 0;
    if (node->function_call_function->type == AST_VARIABLE) {
        fspec = scope_get_function(visitor->global_scope, node->function_call_function->variable_name);
        if (!fspec) {
            if (scope->has_this) {
                data_type_T* this_type = scope_get_variable(scope, "this");
                for (size_t i = 0; i < this_type->class_members_size; i++) {
                    if (utils_strcmp(node->function_call_function->variable_name, this_type->class_member_names[i])) {
                        return this_type->class_member_types[i];
                    }
                }
            }
            err_undefined_function(node->function_call_function->variable_name);
        }
    } else if (node->function_call_function->type == AST_MEMBER) {
        // err_not_implemented("Class function calls\n");
        data_type_T* parent_type = visitor_visit(visitor, scope, node->function_call_function->member_parent, as_function);
        for (size_t i = 0; i < parent_type->class_functions_size; i++) {
            if (utils_strcmp(node->function_call_function->member_name, parent_type->class_function_names[i])) {
                fspec = parent_type->class_functions[i];
            }
        }
        if (!fspec) {
            err_class_no_member(parent_type->type_name, node->function_call_function->member_name);
        }
        if (parent_type->primitive_type == TYPE_CLASS || parent_type->primitive_type == TYPE_INT) {
            as_op_T* as_op = init_as_op(ASOP_ARGTOREG);
            as_op->op_size = parent_type->primitive_size;
            as_op->argno = argno;
            argno++;
            as_add_op_to_function(as_function, as_op);
        }
    }
    visitor_visit_function_call_args(visitor, scope, node, fspec, as_function);
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
data_type_T* visitor_visit_class_definition(visitor_T* visitor, scope_T* scope, AST_T* node) {
    data_type_T* data_type = init_data_type(TYPE_STATICCLASS, node->class_name);
    data_type->class_type = init_data_type(TYPE_CLASS, node->class_name);
    scope_add_variable(scope, node->class_name, data_type);
    for (size_t i = 0; i < node->class_prototype_names_size; i++) {
        data_type_T* prototype_type = get_data_type(visitor, scope, node->class_prototype_names[i]);
        if (prototype_type == (void*) 0) {
            err_unknown_type(node->class_prototype_names[i]);
        }
        list_add(&data_type->class_prototypes, &data_type->class_prototypes_size, prototype_type);
    }
    for (size_t i = 0; i < node->class_members_size; i++) {
        if (node->class_members[i]->type == AST_VARIABLE_DEFINITION) {
            data_type_T* member_type = get_data_type(visitor, scope, node->class_members[i]->variable_definition_type);
            data_type_T* type_to_add;
            if (node->class_members[i]->function_definition_is_static == 1) {
                type_to_add = data_type;
            } else {
                type_to_add = data_type->class_type;
            }
            list_add(&type_to_add->class_member_types, &type_to_add->class_members_size, member_type);
            type_to_add->class_members_size--;
            list_add(&type_to_add->class_member_names, &type_to_add->class_members_size, node->class_members[i]->variable_definition_name);
            type_to_add->primitive_size += member_type->primitive_size;
        } else if (node->class_members[i]->type == AST_FUNCTION_DEFINITION) {
            scope_T* function_scope = init_scope(scope);
            as_function_T* as_function = init_as_function((void*) 0);
            data_type_T* type_to_add;
            if (node->class_members[i]->function_definition_is_static == 1) {
                type_to_add = data_type;
            } else {
                type_to_add = data_type->class_type;
                scope_add_variable(function_scope, "this", type_to_add);
                as_op_T* as_op = init_as_op(ASOP_ARGTOSTACK);
                as_op->argno = 0;
                as_op->op_size = type_to_add->primitive_size;
                as_op->var_location = scope_get_variable_relative_location(function_scope, "this");
                as_function->argc++;
                as_add_op_to_function(as_function, as_op);
            }
            fspec_T* fspec = visitor_visit_function_definition(visitor, function_scope, node->class_members[i], as_function);
            fspec->is_class_function = !node->class_members[i]->function_definition_is_static;
            function_scope->has_this = !node->class_members[i]->function_definition_is_static;
            fspec->symbol_name = realloc(fspec->symbol_name, (30) * sizeof(char));
            as_function->name = fspec->symbol_name;
            sprintf(fspec->symbol_name, "__classfunction%zu", i);
            
            list_add(&type_to_add->class_functions, &type_to_add->class_functions_size, fspec);
            type_to_add->class_functions_size--;
            list_add(&type_to_add->class_function_names, &type_to_add->class_functions_size, node->class_members[i]->function_definition_name);
            if (utils_strcmp(node->class_members[i]->function_definition_name, node->class_name)) {
                scope_add_function(scope, node->class_name, fspec);
            }
        }
    }
    if (data_type->class_type->primitive_size == 0) {
        err_empty_class(data_type->class_type->type_name);
    }
    scope_add_variable(scope, node->class_name, data_type);
    return data_type;
}
data_type_T* visitor_visit_new(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function) {
    if (node->new_function_call->type != AST_FUNCTION_CALL) {
        err_unexpected_node(node);
    }
    as_op_T* as_op = init_as_op(ASOP_NEW);
    as_op->op_size = 8;
    as_add_op_to_function(as_function, as_op);
    if (node->new_function_call->function_call_function->type == AST_VARIABLE) {
        data_type_T* new_type = scope_get_variable(scope, node->new_function_call->function_call_function->variable_name);
        as_op->data_type = new_type;
    } else if (node->new_function_call->function_call_function->type == AST_MEMBER) {
        err_not_implemented("member classes");
        data_type_T* new_type = visitor_visit_member(visitor, scope, node->new_function_call->function_call_function, as_function);
        as_op->data_type = new_type->class_type;
    }
    return as_op->data_type->class_type;
}