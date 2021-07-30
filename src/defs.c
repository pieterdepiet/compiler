#include "include/defs.h"

void defs_define_all(scope_T* global_scope) {
    defs_define_io(global_scope);
}

void defs_define(scope_T* scope, char* name, fspec_T* fspec) {
    scope_add_function(scope, name, fspec);
}
void defs_define_class_member();

void defs_define_io(scope_T* scope) {
    // Print
    fspec_T* fspec = init_fspec(scope_get_variable(scope, "Null"));
    fspec_add_unnamed_arg(fspec, scope_get_variable(scope, "Int"));
    fspec->symbol_name = "_printi";
    defs_define(scope, "print", fspec);
}