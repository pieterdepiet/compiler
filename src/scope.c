#include "include/scope.h"
#include <stdlib.h>
#include "include/utils.h"

scope_T* init_scope(scope_T* parent) {
    scope_T* scope = calloc(1, sizeof(struct SCOPE_STRUCT));
    scope->parent = parent;
    scope->variable_names = (void*) 0;
    scope->variable_types = (void*) 0;
    return scope;
}

void scope_add_variable(scope_T* scope, char* name, data_type_T* type) {
    scope->variables_size++;
    scope->variable_names = realloc(scope->variable_names, scope->variables_size * sizeof(char*));
    scope->variable_types = realloc(scope->variable_types, scope->variables_size * sizeof(data_type_T*));
    scope->variable_names[scope->variables_size-1] = name;
    scope->variable_types[scope->variables_size-1] = type;
}
void scope_add_function(scope_T* scope, char* name, fspec_T* fspec) {
    scope->functions_size++;
    scope->function_names = realloc(scope->function_names, scope->functions_size * sizeof(char*));
    scope->function_specs = realloc(scope->function_specs, scope->functions_size * sizeof(fspec_T*));
    scope->function_names[scope->functions_size-1] = name;
    scope->function_specs[scope->functions_size-1] = fspec;
}
data_type_T* scope_get_variable(scope_T* scope, char* name) {
    for (size_t i = 0; i < scope->variables_size; i++) {
        if (utils_strcmp(name, scope->variable_names[i])) {
            return scope->variable_types[i];
        }
    }
    return (void*) 0;
}
fspec_T* scope_get_function(scope_T* scope, char* name) {
    for (size_t i = 0; i < scope->functions_size; i++) {
        if (utils_strcmp(name, scope->function_names[i])) {
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
    for (size_t i = 0; i < scope->variables_size; i++) {
        curr_location += scope->variable_types[i]->primitive_size;
        if (utils_strcmp(scope->variable_names[i], name)) {
            return curr_location;
        }
    }
    return -1;
}
size_t scope_get_scope_size(scope_T* scope) {
    size_t size;
    for (size_t i = 0; i < scope->variables_size; i++) {
        size += scope->variable_types[i]->primitive_size;
    }
    return size;
}