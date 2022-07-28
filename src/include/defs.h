#ifndef DEFS_H
#define DEFS_H
#include "scope.h"

void defs_define_all(global_T* global_scope);

void defs_define(global_T* scope, fspec_T* fspec);

void defs_define_int(global_T* scope);
void defs_define_string(global_T* scope);
#endif