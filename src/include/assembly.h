#ifndef AS_TEXT_H
#define AS_TEXT_H
#include "data_type.h"
#include "list.h"

typedef union assembly_value_union {
    char char_value;
    short short_value;
    int int_value;
    long long_value;
    void* ptr_value;
} as_value_U;
typedef struct AS_TEXT_STRUCT {
    char* buf;
} as_text_T;
typedef struct assembly_data_struct {
    char* name;
    enum astype {
        ASTYPE_CHAR,
        ASTYPE_SHORT,
        ASTYPE_INT,
        ASTYPE_LONG,
        ASTYPE_STRING
    } type;
    as_value_U value;
} as_data_T;
typedef struct ASSEMBLY_OPERATION_STRUCT {
    enum op_type {
        ASOP_NOP,
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
        ASOP_BINOP,
        ASOP_RETNULL,
        ASOP_LEA,
        ASOP_NEW,
        ASOP_MEMBREF,
        ASOP_SETDEST,
        ASOP_FREEDEST,
        ASOP_SYMBOLREF,
        ASOP_SYMBADDRREF,
        ASOP_JCOND,
        ASOP_JMP,
        ASOP_BB,
        ASOP_UNOP,
        ASOP_MEMTOREG,
        ASOP_LOCALMEMB,
        ASOP_PUSHREG,
        ASOP_POPREG,
    } type;
    char* name;
    size_t var_location;
    size_t memb_offset;
    char op_size;
    int argno;
    as_value_U value;
    enum astype data_type;
    int binop_type;
    int unop_type;
    size_t bb_no;
} as_op_T;
typedef struct ASSEMBLY_FUNCTION_STRUCT {
    char* name;
    as_op_T** operations;
    size_t operations_size;
    size_t last_stack_offset;
    size_t scope_size;
    size_t argc;
    char* last_imm_str;
    size_t mem_loc;
    int memb_offset;
    size_t visitor_max_extra_stack;
    enum registers {
        REG_VOID,
        REG_IMM,
        REG_MEM,
        REG_MEMADDR,
        REG_MEMB,
        REG_MEMBADDR,
        REG_SYMB,
        REG_SYMBADDR,
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
    size_t function_no;
    size_t last_bb;
    char* zeroreg;
    size_t zerouses;
    enum comparison {
        COMP_A,
        COMP_AE,
        COMP_B,
        COMP_BE,
        COMP_E,
        COMP_Z,
        COMP_G,
        COMP_GE,
        COMP_L,
        COMP_LE,
        COMP_NA,
        COMP_NAE,
        COMP_NB,
        COMP_NBE,
        COMP_NE,
        COMP_NZ,
        COMP_NG,
        COMP_NGE,
        COMP_NL,
        COMP_NLE
    } last_comparison;
} as_function_T;


typedef struct assembly_file_struct {
    as_function_T** functions;
    size_t functions_size;
    as_data_T** data;
    size_t data_size;
    size_t unnamed_string_count;
} as_file_T;


as_file_T* init_as_file();
as_data_T* init_as_data(char* name, enum astype as_type);
as_function_T* init_as_function(char* name);
as_op_T* init_as_op(int type);
as_text_T* init_as_text();
void as_add_data(as_file_T* as, as_data_T* data);
void as_add_function(as_file_T* as, as_function_T* function);
void as_add_op_to_function(as_function_T* function, as_op_T* op);

char* as_compile_to_imm(enum astype type, as_value_U value);
char* as_ensure_no_mem(as_function_T* as_function, char** as_text, as_op_T* as_op);
void as_compile_data(as_text_T* as, char** as_text, as_data_T* data);
void as_compile_function_definition(as_text_T* as, char** as_text, as_function_T* function);
void as_compile_binop(as_function_T* as, char** as_text, as_op_T* op, char* src);
void as_compile_operation(as_function_T* as, char** as_text, as_op_T* op);
as_text_T* as_compile_file(as_file_T* as);
char* as_op_type_string(enum op_type type);
char* as_comp_str(enum comparison comp_type);
char* as_comp_inv_str(int comp_type);

#endif