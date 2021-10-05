#ifndef VISITOR_H
#define VISITOR_H
#include "AST.h"
#include "scope.h"
#include "assembly.h"

typedef struct VISITOR_STRUCT {
    AST_T* noop;
    data_type_T* null_type;
    data_type_T* int_type;
    data_type_T* bool_type;
    data_type_T* string_type;
    scope_T* global_scope;
    as_file_T* as_file;
} visitor_T;

visitor_T* init_visitor(as_file_T* as);
char* visitor_data_type_to_arg_name(visitor_T* visitor, scope_T* scope, data_type_T* data_type);
data_type_T* visitor_visit(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_global(visitor_T* visitor, AST_T* root);
as_value_U visitor_visit_data(visitor_T* visitor, data_type_T* data_type, AST_T* node);
data_type_T* visitor_visit_binop(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_plus(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_int(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_string(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_variable_definition(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_variable_assignment(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_variable(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_member(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
fspec_T* visitor_visit_function_definition(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_compound(visitor_T* visitor, scope_T* scope, AST_T* node, data_type_T* return_type, as_function_T* as_function);
data_type_T* visitor_visit_function_call(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
void visitor_visit_function_call_args(visitor_T* visitor, scope_T* scope, AST_T* node, fspec_T* fspec, as_function_T* as_function);
data_type_T* visitor_visit_return(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function, data_type_T* return_type);
data_type_T* visitor_visit_class_definition(visitor_T* visitor, scope_T* scope, AST_T* node);
data_type_T* visitor_visit_new(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
data_type_T* visitor_visit_if(visitor_T* visitor, scope_T* scope, AST_T* node, as_function_T* as_function);
#endif