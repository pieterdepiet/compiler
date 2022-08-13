#include "include/defs.h"
#include "include/utils.h"
void defs_define_all(global_T* global_scope) {
    data_type_T* null_type = init_data_type(TYPE_NULL, "Null");
    null_type->primitive_size = 0;
    scope_add_typeg(global_scope, null_type);

    data_type_T* bool_type = init_data_type(TYPE_BOOL, "Bool");
    bool_type->primitive_size = 4;
    scope_add_typeg(global_scope, bool_type);
    defs_define_int(global_scope);
    defs_define_string(global_scope);
}

void defs_define(global_T* scope, fspec_T* fspec) {
    scope_add_functiong(scope, fspec);
}
void defs_define_class_member();

void defs_define_int(global_T* scope) {
    data_type_T* int_type = init_data_type(TYPE_INT, "Int");
    int_type->primitive_size = 4;
    fspec_T* fspec = init_fspec(int_type);
    fspec->is_class_function = 1;
    fspec->symbol_name = "_C3Int5print";
    fspec->name = "print";
    list_add(&int_type->instance_functions, &int_type->instance_functions_size, fspec);
    fspec = init_fspec(int_type);
    fspec->symbol_name = "_C3Int3add";
    fspec->name = "add";
    fspec->is_class_function = 1;
    fspec_add_unnamed_arg(fspec, int_type);
    list_add(&int_type->instance_functions, &int_type->instance_functions_size, fspec);
    scope_add_typeg(scope, int_type);
}
void defs_define_string(global_T* scope) {
    data_type_T* string_type = init_data_type(TYPE_STRING, "String");
    string_type->primitive_size = 8;
    scope_add_typeg(scope, string_type);
}