#ifndef SCOPE_H
#define SCOPE_H
#include "data_type.h"
typedef struct GLOBAL_SCOPE_STRUCT {
    char** type_names;
    data_type_T** types;
    size_t types_size;
    char** variable_names;
    data_type_T** variable_types;
    size_t variables_size;
    fspec_T** function_specs;
    size_t functions_size;
    struct VISITOR_STRUCT* visitor;
} global_T;
typedef struct SCOPE_STRUCT {
    char** variable_names;
    data_type_T** variable_types;
    size_t variables_size;
    int has_this;
    struct SCOPE_STRUCT* parent;
    data_type_T* return_type;
    struct VISITOR_STRUCT* visitor;
    struct ASSEMBLY_FUNCTION_STRUCT* as_function;
    global_T* global;
} scope_T;

global_T* init_global_scope(struct VISITOR_STRUCT* visitor);
scope_T* init_function_scope(global_T* parent, struct ASSEMBLY_FUNCTION_STRUCT* as_function);
scope_T* init_scope(scope_T* parent);

void scope_add_variable(scope_T* scope, char* name, data_type_T* type);
void scope_add_type(scope_T* scope, char* name, data_type_T* type);
void scope_add_typeg(global_T* scope, char* name, data_type_T* type);
void scope_add_function(scope_T* scope, fspec_T* fspec);
void scope_add_functiong(global_T* scope, fspec_T* fspec);
data_type_T* scope_get_variable(scope_T* scope, char* name);
data_type_T* scope_get_type(scope_T* scope, char* name);
data_type_T* scope_get_typeg(global_T* scope, char* name);
fspec_T* scope_get_function(scope_T* scope, char* name);
fspec_T* scope_get_functiong(global_T* scope, char* name);
int scope_get_variable_id(scope_T* scope, char* name);
size_t scope_get_variable_relative_location(scope_T* scope, char* name);
size_t scope_get_scope_size(scope_T* scope);

#endif