#include "include/defs.h"
#include "include/list.h"
void defs_define_all(scope_T* global_scope) {
    data_type_T* null_type = init_data_type(TYPE_STATICCLASS, "Null");
    null_type->class_type = init_data_type(TYPE_NULL, "Null");
    null_type->class_type->class_type = null_type;
    null_type->primitive_size = 0;
    scope_add_variable(global_scope, "Null", null_type);
    defs_define_int(global_scope);
    defs_define_string(global_scope);
    defs_define_io(global_scope);
}

void defs_define(scope_T* scope, char* name, fspec_T* fspec) {
    scope_add_function(scope, name, fspec);
}
void defs_define_class_member();

void defs_define_io(scope_T* scope) {
    // Print
    data_type_T* null_type = scope_get_variable(scope, "Null");
    fspec_T* fspec = init_fspec(null_type);
    fspec_add_unnamed_arg(fspec, scope_get_variable(scope, "Int")->class_type);
    fspec->symbol_name = "_printi";
    defs_define(scope, "print", fspec);
    fspec = init_fspec(null_type);
    fspec->symbol_name = "_prints";
    fspec_add_unnamed_arg(fspec, scope_get_variable(scope, "String")->class_type);
    defs_define(scope, "prints", fspec);
}
void defs_define_int(scope_T* scope) {
    data_type_T* int_type = init_data_type(TYPE_STATICCLASS, "Int");
    int_type->primitive_size = 0;
    int_type->class_type = init_data_type(TYPE_INT, "Int");
    int_type->class_type->primitive_size = 4;
    int_type->class_type->class_type = int_type;
    fspec_T* fspec = init_fspec(int_type->class_type);
    fspec->is_class_function = 1;
    fspec->symbol_name = "_C3Int5print";
    list_add(&int_type->class_type->class_functions, &int_type->class_type->class_functions_size, fspec);
    int_type->class_functions_size--;
    list_add(&int_type->class_type->class_function_names, &int_type->class_type->class_functions_size, "print");
    fspec = init_fspec(int_type->class_type);
    fspec->symbol_name = "_C3Int3add";
    fspec->is_class_function = 1;
    fspec_add_unnamed_arg(fspec, int_type->class_type);
    list_add(&int_type->class_type->class_functions, &int_type->class_type->class_functions_size, fspec);
    int_type->class_functions_size--;
    list_add(&int_type->class_type->class_function_names, &int_type->class_type->class_functions_size, "add");
    scope_add_variable(scope, "Int", int_type);
}
void defs_define_string(scope_T* scope) {
    data_type_T* string_type = init_data_type(TYPE_STATICCLASS, "String");
    string_type->class_type = init_data_type(TYPE_STRING, "String");
    string_type->class_type->primitive_size = 8;
    string_type->class_type->class_type = string_type;
    scope_add_variable(scope, "String", string_type);
}