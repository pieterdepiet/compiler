#ifndef VISITOR_H
#define VISITOR_H
#include "AST.h"
#include "scope.h"
#include "assembly.h"

typedef struct VISITOR_STRUCT {
    data_type_T* null_type;
    data_type_T* int_type;
    data_type_T* bool_type;
    data_type_T* string_type;
    global_T* global_scope;
    as_file_T* as_file;
    char* current_src;
} visitor_T;

visitor_T* init_visitor(as_file_T* as, global_T* scope);
data_type_T* visitor_visit(scope_T* scope, AST_T* node, data_type_T* suggestion);
data_type_T* visitor_visit_global(global_T* scope, AST_T* root);
data_type_T* visitor_visit_binop(scope_T* scope, AST_T* node, data_type_T* hint);
data_type_T* visitor_visit_unop(scope_T* scope, AST_T* node, data_type_T* hint);
data_type_T* visitor_visit_int_as(scope_T* scope, AST_T* node, data_type_T* hint);
data_type_T* visitor_visit_string(scope_T* scope, AST_T* node);
data_type_T* visitor_visit_variable_definition(scope_T* scope, AST_T* node);
data_type_T* visitor_visit_variable_assignment(scope_T* scope, AST_T* node);
data_type_T* visitor_visit_variable(scope_T* scope, AST_T* node);
data_type_T* visitor_visit_member(scope_T* scope, AST_T* node);
fspec_T* visitor_visit_function_definition(scope_T* scope, AST_T* node);
data_type_T* visitor_visit_compound(scope_T* scope, AST_T* node, data_type_T* return_type);
data_type_T* visitor_visit_function_call(scope_T* scope, AST_T* node);
void visitor_visit_function_call_args(scope_T* scope, AST_T* node, fspec_T* fspec);
data_type_T* visitor_visit_return(scope_T* scope, AST_T* node, data_type_T* return_type);
data_type_T* visitor_visit_class_definition(global_T* scope, AST_T* node);
data_type_T* visitor_visit_new(scope_T* scope, AST_T* node);
data_type_T* visitor_visit_if(scope_T* scope, AST_T* node);
#endif