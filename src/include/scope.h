#ifndef SCOPE_H
#define SCOPE_H
#include "AST.h"
typedef struct SCOPE_STRUCT {
    char** variable_names;
    data_type_T** variable_types;
    size_t variables_size;
    char** function_names;
    fspec_T** function_specs;
    size_t functions_size;
    struct SCOPE_STRUCT* parent;
} scope_T;

scope_T* init_scope(scope_T* parent);

void scope_add_variable(scope_T* scope, char* name, data_type_T* type);
void scope_add_function(scope_T* scope, char* name, fspec_T* fspec);
data_type_T* scope_get_variable(scope_T* scope, char* name);
fspec_T* scope_get_function(scope_T* scope, char* name);
int scope_get_variable_id(scope_T* scope, char* name);
size_t scope_get_variable_relative_location(scope_T* scope, char* name);
size_t scope_get_scope_size(scope_T* scope);

#endif