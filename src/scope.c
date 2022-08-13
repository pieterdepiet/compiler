#include "include/scope.h"
#include <stdlib.h>
#include "include/utils.h"
#include <stdio.h>

global_T* init_global_scope(struct VISITOR_STRUCT* visitor) {
    global_T* scope = calloc(1, sizeof(global_T));
    scope->variable_names = (void*) 0;
    scope->variable_types = (void*) 0;
    scope->variables_size = 0;
    scope->function_specs = (void*) 0;
    scope->functions_size = 0;
    scope->types = (void*) 0;
    scope->types_size = 0;
    scope->visitor = visitor;
    return scope;
}
scope_T* init_function_scope(global_T* parent, struct ASSEMBLY_FUNCTION_STRUCT* as_function) {
    scope_T* scope = calloc(1, sizeof(struct SCOPE_STRUCT));
    scope->global = parent;
    scope->visitor = parent->visitor;
    scope->as_function = as_function;
    scope->return_type = (void*) 0;
    scope->variable_names = (void*) 0;
    scope->variable_types = (void*) 0;
    scope->variables_size = 0;
    return scope;
}

scope_T* init_scope(scope_T* parent) {
    scope_T* scope = calloc(1, sizeof(struct SCOPE_STRUCT));
    scope->parent = parent;
    scope->global = parent->global;
    scope->visitor = parent->visitor;
    scope->as_function = parent->as_function;
    scope->return_type = parent->return_type;
    scope->variable_names = (void*) 0;
    scope->variable_types = (void*) 0;
    return scope;
}

void scope_add_variable(scope_T* scope, char* name, data_type_T* type) {
    list_add((void***) &scope->variable_types, &scope->variables_size, type);
    scope->variables_size--;
    list_add((void***) &scope->variable_names, &scope->variables_size, name);
}
void scope_add_function(scope_T* scope, fspec_T* fspec) {
    list_add((void***) &scope->global->function_specs, &scope->global->functions_size, fspec);
}
void scope_add_functiong(global_T* scope, fspec_T* fspec) {
    list_add((void***) &scope->function_specs, &scope->functions_size, fspec);
}
void scope_add_type(scope_T* scope, data_type_T* type) {
    list_add((void***) &scope->global->types, &scope->global->types_size, type);
}
void scope_add_typeg(global_T* scope, data_type_T* type) {
    list_add((void***) &scope->types, &scope->types_size, type);
}
data_type_T* scope_get_variable(scope_T* scope, char* name) {
    for (size_t i = 0; i < scope->variables_size; i++) {
        if (utils_strcmp(name, scope->variable_names[i])) {
            return scope->variable_types[i];
        }
    }
    if (scope->parent) {
        return scope_get_variable(scope->parent, name);
    }
    return (void*) 0;
}
data_type_T* scope_get_type(scope_T* scope, char* name) {
    for (size_t i = 0; i < scope->global->types_size; i++) {
        if (utils_strcmp(name, scope->global->types[i]->type_name)) {
            return scope->global->types[i];
        }
    }
    return (void*) 0;
}
data_type_T* scope_get_typeg(global_T* scope, char* name) {
    for (size_t i = 0; i < scope->types_size; i++) {
        if (utils_strcmp(name, scope->types[i]->type_name)) {
            return scope->types[i];
        }
    }
    return (void*) 0;
}
fspec_T* scope_get_function(scope_T* scope, char* name) {
    for (size_t i = 0; i < scope->global->functions_size; i++) {
        if (utils_strcmp(name, scope->global->function_specs[i]->name)) {
            return scope->global->function_specs[i];
        }
    }
    return (void*) 0;
}
fspec_T* scope_get_functiong(global_T* scope, char* name) {
    for (size_t i = 0; i < scope->functions_size; i++) {
        if (utils_strcmp(name, scope->function_specs[i]->name)) {
            return scope->function_specs[i];
        }
    }
    return (void*) 0;
}
int scope_get_variable_id(scope_T* scope, char* name) {
    for (size_t i = 0; i < scope->variables_size; i++) {
        if (utils_strcmp(scope->variable_names[i], name)) {
            return i;
        }
    }
    return -1;
}
size_t scope_get_variable_relative_location(scope_T* scope, char* name) {
    size_t curr_location = 0;
    if (scope->parent != (void*) 0) {
        curr_location = scope_get_scope_size(scope->parent);
    }
    for (size_t i = 0; i < scope->variables_size; i++) {
        curr_location += scope->variable_types[i]->primitive_size;
        if (utils_strcmp(scope->variable_names[i], name)) {
            return curr_location;
        }
    }
    if (scope->parent) {
        return scope_get_variable_relative_location(scope->parent, name);
    } else {
        return 0;
    }
}
size_t scope_get_scope_size(scope_T* scope) {
    size_t size = 0;
    if (scope->parent != (void*) 0) {
        size = scope_get_scope_size(scope->parent);
    }
    for (size_t i = 0; i < scope->variables_size; i++) {
        size += scope->variable_types[i]->primitive_size;
    }
    return size;
}