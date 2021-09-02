#ifndef DATA_TYPE_H
#define DATA_TYPE_H
#include <sys/types.h>
typedef struct DATA_TYPE_STRUCT {
    enum {
        TYPE_NULL,
        TYPE_INT,
        TYPE_STRING,
        TYPE_CLASS,
        TYPE_STRUCT,
        TYPE_STATICCLASS
    } primitive_type;
    char** class_member_names;
    struct DATA_TYPE_STRUCT** class_member_types;    
    size_t class_members_size;
    struct DATA_TYPE_STRUCT** class_prototypes;
    size_t class_prototypes_size;
    struct FUNCTION_SPECIFICATIONS_STRUCT** class_functions;
    char** class_function_names;
    size_t class_functions_size;
    char* type_name;
    size_t primitive_size;
    struct DATA_TYPE_STRUCT* class_type;
} data_type_T;
typedef struct FUNCTION_SPECIFICATIONS_STRUCT {
    data_type_T* return_type;
    size_t named_length;
    char** named_names;
    data_type_T** named_types;
    data_type_T** unnamed_types;
    size_t unnamed_length;
    char* symbol_name;
    int is_class_function;
} fspec_T;

fspec_T* init_fspec(data_type_T* return_type);
data_type_T* init_data_type(int primitive_type, char* type_name);

void fspec_add_unnamed_arg(fspec_T* fspec, data_type_T* arg_type);
void fspec_add_named_arg(fspec_T* fspec, char* name, data_type_T* arg_type);
#endif