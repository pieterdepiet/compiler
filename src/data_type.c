#include "include/data_type.h"
#include <stdlib.h>
#include "include/list.h"

data_type_T* init_data_type(int primitive_type, char* type_name) {
    data_type_T* data_type = calloc(1, sizeof(data_type_T));
    data_type->primitive_type = primitive_type;
    data_type->class_member_names = (void*) 0;
    data_type->class_member_types = (void*) 0;
    data_type->class_members_size = 0;
    data_type->type_name = type_name;
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
    list_add(&type->class_member_types, &type->class_members_size, member);
    type->class_members_size--;
    list_add(&type->class_member_names, &type->class_members_size, name);
}
void fspec_add_unnamed_arg(fspec_T* fspec, data_type_T* arg_type) {
    list_add(&fspec->unnamed_types, &fspec->unnamed_length, arg_type);
}
void fspec_add_named_arg(fspec_T* fspec, char* name, data_type_T* arg_type) {
    list_add(&fspec->named_types, &fspec->named_length, arg_type);
    fspec->named_length--;
    list_add(&fspec->named_names, &fspec->named_length, name);
}