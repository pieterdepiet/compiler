#include "include/data_type.h"
#include <stdlib.h>
#include <string.h>
#include "include/utils.h"

data_type_T* init_data_type(int primitive_type, char* type_name) {
    data_type_T* data_type = calloc(1, sizeof(data_type_T));
    data_type->primitive_type = primitive_type;
    data_type->instance_member_names = (void*) 0;
    data_type->instance_member_types = (void*) 0;
    data_type->instance_members_size = 0;
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

data_type_T* init_pointer_to(data_type_T* data_type) {
    data_type_T* ptr_type = init_data_type(TYPE_PTR, data_type->type_name);
    ptr_type->ptr_type = data_type;
    ptr_type->primitive_size = 8;
    return ptr_type;
}

void data_type_add_instance_member(data_type_T* type, char* name, data_type_T* member) {
    list_add(&type->instance_member_types, &type->instance_members_size, member);
    type->instance_members_size--;
    list_add(&type->instance_member_names, &type->instance_members_size, name);
}
void fspec_add_unnamed_arg(fspec_T* fspec, data_type_T* arg_type) {
    list_add(&fspec->unnamed_types, &fspec->unnamed_length, arg_type);
}
void fspec_add_named_arg(fspec_T* fspec, char* name, data_type_T* arg_type) {
    list_add(&fspec->named_types, &fspec->named_length, arg_type);
    fspec->named_length--;
    list_add(&fspec->named_names, &fspec->named_length, name);
}
int data_type_equals(data_type_T* type1, data_type_T* type2) {
    if (type1->primitive_type == TYPE_PTR) {
        type1 = type1->ptr_type;
    }
    if (type2->primitive_type == TYPE_PTR) {
        type2 = type2->ptr_type;
    }
    return type1 == type2;
}
int data_type_prototype_contains(data_type_T* type1, data_type_T* type2) {
    for (size_t i = 0; i < type1->class_prototypes_size; i++) {
        data_type_T* proto1 = type1->class_prototypes[i];
        for (size_t j = 0; j < type2->class_prototypes_size; j++) {
            if (data_type_equals(proto1, type2)) {
                return 1;
            }
        }
    }
    return 0;
}


int fspec_check_equals(fspec_T* fspec1, fspec_T* fspec2) {
    if (fspec1->unnamed_length != fspec2->unnamed_length || fspec1->named_length != fspec2->named_length || strlen(fspec1->name) != strlen(fspec2->name) || strcmp(fspec1->name, fspec2->name) != 0) {
        return 0;
    }
    for (size_t i = 0; i < fspec1->unnamed_length; i++) {
        data_type_T* data_type1 = fspec1->unnamed_types[i];
        if (data_type1->primitive_type == TYPE_PTR) {
            data_type1 = data_type1->ptr_type;
        }
        data_type_T* data_type2 = fspec2->unnamed_types[i];
        if (data_type2->primitive_type == TYPE_PTR) {
            data_type2 = data_type2->ptr_type;
        }
        if (!data_type_prototype_contains(data_type1, data_type2)) {
            return 0;
        }
    }
    for (size_t i = 0; i < fspec1->named_length; i++) {
        data_type_T* data_type1 = fspec1->named_types[i];
        if (data_type1->primitive_type == TYPE_PTR) {
            data_type1 = data_type1->ptr_type;
        }
        data_type_T* data_type2 = fspec2->named_types[i];
        if (data_type2->primitive_type == TYPE_PTR) {
            data_type2 = data_type2->ptr_type;
        }
        if (strlen(fspec1->named_names[i]) != strlen(fspec2->named_names[i]) || strcmp(fspec1->named_names[i], fspec2->named_names[i]) != 0) {
            return 0;
        }
        if (!data_type_prototype_contains(data_type1, data_type2)) {
            return 0;
        }
    }
    return 1;
}