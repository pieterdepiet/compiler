#ifndef DATA_TYPE_H
#define DATA_TYPE_H
#include <sys/types.h>


typedef struct DATA_TYPE_STRUCT {
    enum {
        TYPE_NULL,
        TYPE_INT,
        TYPE_UINT,
        TYPE_CHAR,
        TYPE_UCHAR,
        TYPE_LONG,
        TYPE_ULONG,
        TYPE_BOOL,
        TYPE_STRING,
        TYPE_STRUCT,
        TYPE_PTR
    } primitive_type;
    struct DATA_TYPE_STRUCT** class_prototypes;
    size_t class_prototypes_size;

    // Instance
    char** instance_member_names;
    struct DATA_TYPE_STRUCT** instance_member_types;
    size_t instance_members_size;
    struct FUNCTION_SPECIFICATIONS_STRUCT** instance_functions;
    size_t instance_functions_size;

    // Static
    char** static_member_names;
    struct DATA_TYPE_STRUCT** static_member_types;
    size_t static_members_size;
    struct FUNCTION_SPECIFICATIONS_STRUCT** static_functions;
    size_t static_functions_size;

    struct DATA_TYPE_STRUCT* ptr_type;
    
    char* type_name;
    size_t primitive_size;
    struct {
        unsigned char is_private : 1;
        unsigned char is_this : 1;
    };
} data_type_T;
typedef struct FUNCTION_SPECIFICATIONS_STRUCT {
    data_type_T* return_type;
    size_t named_length;
    char** named_names;
    data_type_T** named_types;
    data_type_T** unnamed_types;
    size_t unnamed_length;
    char* symbol_name;
    char* name;
    struct {
        unsigned char is_class_function : 1;
        unsigned char is_private : 1;
    };
} fspec_T;

fspec_T* init_fspec(data_type_T* return_type);
data_type_T* init_data_type(int primitive_type, char* type_name);

data_type_T* init_pointer_to(data_type_T* data_type);

void fspec_add_unnamed_arg(fspec_T* fspec, data_type_T* arg_type);
void fspec_add_named_arg(fspec_T* fspec, char* name, data_type_T* arg_type);

int fspec_check_equals(fspec_T* fspec1, fspec_T* fspec2);
#endif