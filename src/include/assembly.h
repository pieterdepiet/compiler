#ifndef AS_TEXT_H
#define AS_TEXT_H
#include "AST.h"

typedef struct AS_TEXT_STRUCT {
    char* buf;
    
} as_text_T;
typedef struct ASSEMBLY_OPERATION_STRUCT {
    enum {
        ASOP_RETURN,
        ASOP_ARGTOSTACK,
        ASOP_ARGTOREG,
        ASOP_SETLASTIMM,
        ASOP_VDEF,
        ASOP_VREF,
        ASOP_VMOD,
        ASOP_FCALL,
        ASOP_RETVAL,
        ASOP_NEXTREG,
        ASOP_FREEREG,
        ASOP_ADD
    } type;
    AST_T* value;
    char* name;
    size_t var_location;
    char op_size;
    int argno;
} as_op_T;
typedef struct assembly_function_struct {
    char* name;
    list_T* operations;
    size_t last_stack_offset;
    size_t scope_size;
    char* last_imm_str;
    enum registers {
        REG_IMM,
        REG_AX,
        REG_BX,
        REG_CX,
        REG_DX,
        REG_DI,
        REG_SI
    } last_register;
    int used_reg;
} as_function_T;
typedef struct assembly_data_struct {
    char* name;
    data_type_T* type;
    AST_T* value;
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

void as_compile(as_function_T* as, AST_T* node);
void as_compile_variable_definition(as_text_T* as, as_function_T* as_function, AST_T* node);
char* as_compile_ast(as_text_T* as, AST_T* node);
void as_compile_data(as_text_T* as, char** as_text, as_data_T* data);
void as_compile_int(as_text_T* as, as_function_T* as_function, AST_T* node);
void as_compile_compound(as_text_T* as, as_function_T* as_function, AST_T* node);
void as_compile_function_definition(as_text_T* as, char** as_text, as_function_T* function);
void as_compile_operation(as_function_T* as, char** as_text, as_op_T* op);
as_text_T* as_compile_file(as_file_T* as);

#endif