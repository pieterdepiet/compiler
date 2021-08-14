#ifndef IMPORT_H
#define IMPORT_H
#include "data_type.h"
#include "scope.h"

typedef struct PACKAGE_STRUCT {
    size_t symbols_size;
    char** symbols;
    scope_T* package_scope;
} package_T;

void import_import_package(scope_T* scope, char* package);

#endif