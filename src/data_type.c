#include "include/data_type.h"
#include <stdlib.h>

data_type_T* init_data_type(int primitive_type, char* type_name) {
    data_type_T* data_type = calloc(1, sizeof(data_type_T));
    data_type->primitive_type = primitive_type;
    data_type->class_member_names = (void*) 0;
    data_type->class_member_types = (void*) 0;
    data_type->class_members_size = 0;
    data_type->type_name = (void*) 0;
    return data_type;
}
fspec_T* init_fspec(data_type_T* return_type) {
    fspec_T* fspec = calloc(1, sizeof(fspec_T));
    fspec->named_length = 0;
    fspec->named_names = (void*) 0;
    fspec->named_types = (void*) 0;
    fspec->unnamed_length = 0;
    fspec->unnamed_types = (void*) 0;
    fspec->return_type = return_type;
    fspec->symbol_name = (void*) 0;
    return fspec;
}

void data_type_add_class_member(data_type_T* type, char* name, data_type_T* member) {
    type->class_members_size++;
    type->class_member_types = realloc(type->class_member_types, type->class_members_size * sizeof(data_type_T));
    type->class_member_names = realloc(type->class_member_names, type->class_members_size * sizeof(char*));
    type->class_member_types[type->class_members_size-1] = member;
    type->class_member_names[type->class_members_size-1] = name;
}
void fspec_add_unnamed_arg(fspec_T* fspec, data_type_T* arg_type) {
    fspec->unnamed_length++;
    fspec->unnamed_types = realloc(fspec->unnamed_types, fspec->unnamed_length * sizeof(data_type_T*));
    fspec->unnamed_types[fspec->unnamed_length-1] = arg_type;
}
void fspec_add_named_arg(fspec_T* fspec, char* name, data_type_T* arg_type) {
    fspec->named_length++;
    fspec->named_names = realloc(fspec->named_names, fspec->named_length * sizeof(char*));
    fspec->named_names[fspec->named_length-1] = name;
    fspec->named_types = realloc(fspec->named_types, fspec->named_length * sizeof(data_type_T*));
    fspec->named_types[fspec->named_length-1] = arg_type;
}