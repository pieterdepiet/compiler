#include "include/defs.h"
#include "include/list.h"
void defs_define_all(scope_T* global_scope) {
    defs_define_io(global_scope);
    defs_define_int(global_scope);
}

void defs_define(scope_T* scope, char* name, fspec_T* fspec) {
    scope_add_function(scope, name, fspec);
}
void defs_define_class_member();

void defs_define_io(scope_T* scope) {
    // Print
    data_type_T* null_type = scope_get_variable(scope, "Null");
    fspec_T* fspec = init_fspec(null_type);
    fspec_add_unnamed_arg(fspec, scope_get_variable(scope, "Int"));
    fspec->symbol_name = "_printi";
    defs_define(scope, "print", fspec);
    fspec = init_fspec(null_type);
    fspec->symbol_name = "_prints";
    fspec_add_unnamed_arg(fspec, scope_get_variable(scope, "String"));
    defs_define(scope, "prints", fspec);
}
void defs_define_int(scope_T* scope) {
    data_type_T* int_type = scope_get_variable(scope, "Int");
    fspec_T* fspec = init_fspec(int_type);
    fspec->is_class_function = 1;
    fspec->symbol_name = "_C3Int5print";
    list_add(&int_type->class_functions, &int_type->class_functions_size, fspec);
    int_type->class_functions_size--;
    list_add(&int_type->class_function_names, &int_type->class_functions_size, "print");
    fspec = init_fspec(int_type);
    fspec->symbol_name = "_C3Int3add";
    fspec->is_class_function = 1;
    fspec_add_unnamed_arg(fspec, int_type);
    list_add(&int_type->class_functions, &int_type->class_functions_size, fspec);
    int_type->class_functions_size--;
    list_add(&int_type->class_function_names, &int_type->class_functions_size, "add");
}