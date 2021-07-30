#ifndef DEFS_H
#define DEFS_H
#include "scope.h"

void defs_define_all(scope_T* global_scope);

void defs_define(scope_T* scope, char* name, fspec_T* fspec);

void defs_define_io(scope_T* scope);
#endif