#ifndef AS_TEXT_H
#define AS_TEXT_H
#include "data_type.h"
#include "list.h"

typedef union assembly_value_union {
    int int_value;
    long long_value;
    char char_value;
    void* ptr_value;
} as_value_U;
typedef struct AS_TEXT_STRUCT {
    char* buf;
} as_text_T;
typedef struct ASSEMBLY_OPERATION_STRUCT {
    enum op_type {
        ASOP_RETURN, // 0
        ASOP_ARGTOSTACK, // 1
        ASOP_ARGTOREG, // 2
        ASOP_SETLASTIMM, // 3
        ASOP_VDEF, // 4
        ASOP_VREF, // 5
        ASOP_VMOD, // 6
        ASOP_FCALL, // 7
        ASOP_RETVAL, // 8
        ASOP_NEXTREG, // 9
        ASOP_FREEREG, // 10
        ASOP_BINOP, // 11
        ASOP_RETNULL, // 12
        ASOP_VDEFNULL, // 13
        ASOP_NEW, // 14
        ASOP_MEMBREF, // 15
        ASOP_SETDEST, // 16
        ASOP_FREEDEST // 17
    } type;
    char* name;
    size_t var_location;
    size_t memb_offset;
    char op_size;
    int argno;
    as_value_U value;
    data_type_T* data_type;
    int binop_type;
} as_op_T;
typedef struct assembly_function_struct {
    char* name;
    list_T* operations;
    size_t last_stack_offset;
    size_t scope_size;
    size_t argc;
    char* last_imm_str;
    size_t mem_loc;
    int memb_offset;
    enum registers {
        REG_IMM,
        REG_MEM,
        REG_MEMB,
        REG_AX,
        REG_BX,
        REG_CX,
        REG_DX,
        REG_DI,
        REG_SI
    } last_register;
    int used_reg;
    size_t* dest_locs;
    size_t* dest_offsets;
    size_t dest_size;
} as_function_T;
typedef struct assembly_data_struct {
    char* name;
    data_type_T* value_type;
    as_value_U value;
} as_data_T;


typedef struct assembly_file_struct {
    list_T* functions;
    list_T* data;
} as_file_T;


as_file_T* init_as_file();
as_data_T* init_as_data(char* name, data_type_T* type);
as_function_T* init_as_function(char* name);
as_op_T* init_as_op(int type);
as_text_T* init_as_text();
void as_add_data(as_file_T* as, as_data_T* data);
void as_add_function(as_file_T* as, as_function_T* function);
void as_add_op_to_function(as_function_T* function, as_op_T* op);

char* as_compile_to_imm(data_type_T* type, as_value_U value);
char* as_ensure_no_mem(as_function_T* as_function, char** as_text, as_op_T* as_op);
void as_compile_data(as_text_T* as, char** as_text, as_data_T* data);
void as_compile_function_definition(as_text_T* as, char** as_text, as_function_T* function);
void as_compile_binop(as_function_T* as, char** as_text, as_op_T* op, char* src);
void as_compile_operation(as_function_T* as, char** as_text, as_op_T* op);
as_text_T* as_compile_file(as_file_T* as);
char* as_op_type_string(enum op_type type);

#endif