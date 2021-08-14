#include "include/assembly.h"
#include <string.h>
#include "include/errors.h"
#include "include/utils.h"

// Definitions
#define int_max_length 8
#define size_str_max_length 20
#define reg_length 4

static char* assembly_types[] = {"quad" /*null ptr*/, "long"};
static size_t assembly_types_lengths[] = {4, 4};
static char* assembly_type_formats[] = {"", "%d"};
static size_t assembly_type_formats_lengths[] = {0, 11};
int math_registers[] = {REG_IMM, REG_CX, REG_DX, REG_SI, REG_DI};

char* as_data_section_start = ".data\n";
char* as_text_section_start = ".text\n.globl _start\n";
char* return_format =   "  mov%c %s, %s\n"
                        "  addq $%lu, %%rsp\n"
                        "  pop %%rbp\n"
                        "  ret\n";
size_t return_format_min_length = 10 + 11 + 15 + size_str_max_length + 6 + 1;
char* vdef_format =     "  mov%c %s, -%lu(%%rbp)\n";
size_t vdef_format_min_length = 17 + size_str_max_length + 1;
char* vref_format =     "-%lu(%%rbp)";
size_t vref_format_length = 7 + size_str_max_length + 1;
char* vmod_format =     "  mov%c %s, -%lu(%%rbp)\n";
size_t vmod_format_min_length = 17 + size_str_max_length + 1;
char* argtostack_format =   "  mov%c %s, -%lu(%%rbp)\n";
size_t argtostack_format_min_length = 17 + reg_length + size_str_max_length + 1;
char* argtoreg_format =     "  mov%c %s, %s\n";
size_t argtoreg_format_min_length = 11 + reg_length + 1;
char* fcall_format =        "  call%c %s\n";
size_t fcall_format_min_length = 9 + 1;
char* toreg_format =    "  mov%c %s, %s\n";
size_t toreg_format_min_length = 10 + reg_length + 1;
char* binop_format =    "  %s%c %s, %s\n";
size_t binop_format_min_length = 7 + reg_length + 1;

int arg_registers[] = {REG_DI, REG_SI, REG_DX, REG_CX};

static int is_primitive(AST_T* node) {
    switch (node->type) {
        case AST_INT: return 1; break;
        default: return 0;
    }
}
char data_type_size_op_char(char size) {
    switch (size) {
        case 1: return 'b'; break;
        case 2: return 'w'; break;
        case 4: return 'l'; break;
        case 8: default: return 'q'; break;
    }
}
char* data_type_size_register(int reg, char size) {
    char* reg_str = calloc(1, 5 * sizeof(char));
    if (size==2) {
        strcpy(reg_str, "%w");
    } else if (size==4) {
        strcpy(reg_str, "%e");
    } else if (size==8) {
        strcpy(reg_str, "%r");
    } else {
        strcpy(reg_str, "%");
    }
    switch (reg) {
        case REG_AX: strcat(reg_str, "ax"); break;
        case REG_BX: strcat(reg_str, "bx"); break;
        case REG_CX: strcat(reg_str, "cx"); break;
        case REG_DX: strcat(reg_str, "dx"); break;
        case REG_DI: strcat(reg_str, "di"); break;
        case REG_SI: strcat(reg_str, "si"); break;
        default: err_enum_out_of_range("registers", reg); break;
    }
    if (size==1) {
        strcat(reg_str, "l");
    }
    return reg_str;
}
// Init functions
as_file_T* init_as_file() {
    as_file_T* as_file = calloc(1, sizeof(struct assembly_file_struct));
    as_file->data = init_list();
    as_file->functions = init_list();
    return as_file;
}
as_function_T* init_as_function(char* name) {
    as_function_T* as_function = calloc(1, sizeof(struct assembly_function_struct));
    as_function->name = name;
    as_function->operations = init_list();
    as_function->last_register = 0;
    as_function->last_imm_str = (void*) 0;
    as_function->last_stack_offset = 0;
    as_function->scope_size = 0;
    as_function->used_reg = 0;
    return as_function;
}
as_data_T* init_as_data(char* name, data_type_T* data_type) {
    as_data_T* as_data = calloc(1, sizeof(struct assembly_data_struct));
    as_data->value_type = data_type;
    as_data->name = name;
    return as_data;
}
as_op_T* init_as_op(int type) {
    as_op_T* as_op = calloc(1, sizeof(struct ASSEMBLY_OPERATION_STRUCT));
    as_op->type = type;
    as_op->argno = 0;
    as_op->name = (void*) 0;
    as_op->op_size = 0;
    as_op->var_location = 0;
    as_op->value.ptr_value = 0;
    return as_op;
}
as_text_T* init_as_text() {
    as_text_T* as_text = calloc(1, sizeof(struct AS_TEXT_STRUCT));
    as_text->buf = (void*) 0;
    return as_text;
}
// Small functions
void as_add_data(as_file_T* as, as_data_T* data) {
    list_add(as->data, data);
}
void as_add_function(as_file_T* as, as_function_T* function) {
    list_add(as->functions, function);
}
void as_add_op_to_function(as_function_T* function, as_op_T* op) {
    list_add(function->operations, op);
}

void as_compile_data(as_text_T* as, char** as_text, as_data_T* data) {
    char* format = "%s  _%s: .%s %s\n";
    char* value_string = as_compile_to_imm(data->value_type, data->value);
    size_t as_text_size = 
    strlen(*as_text)
    + 3 //   _
    + strlen(data->name) // %s
    + 3 // : .
    + assembly_types_lengths[data->value_type->primitive_type] // %s
    + 1 // space
    + strlen(value_string)
    + 2 // \n\0
    ;
    *as_text = realloc(*as_text, as_text_size * sizeof(char));
    sprintf(*as_text, format, *as_text, data->name, assembly_types[data->value_type->primitive_type], value_string);
}
as_text_T* as_compile_file(as_file_T* as_file) {
    as_text_T* as = init_as_text();
    as->buf = calloc(1, sizeof(char));
    as->buf[0] = '\0';
    utils_strcat(&as->buf, as_data_section_start);
    for (size_t i = 0; i < as_file->data->size; i++) {
        as_compile_data(as, &as->buf, as_file->data->el[i]);
    }
    utils_strcat(&as->buf, as_text_section_start);
    for (size_t i = 0; i < as_file->functions->size; i++) {
        as_compile_function_definition(as, &as->buf, as_file->functions->el[i]);
    }
    return as;
}
void as_compile_function_definition(as_text_T* as, char** as_text, as_function_T* function) {
    *as_text = realloc(*as_text, (strlen(*as_text) + strlen(function->name) + 48 + size_str_max_length /* :\n  push %rbp\n  movq %rsp, %rbp\n */ + 1 /* \0 */) * sizeof(char));
    sprintf(*as_text, 
    "%s%s:\n"
    "  push %%rbp\n"
    "  movq %%rsp, %%rbp\n"
    "  subq $%lu, %%rsp\n", *as_text, function->name, function->scope_size + (function->scope_size % 16));
    for (size_t i = 0; i < function->operations->size; i++) {
        as_compile_operation(function, as_text, function->operations->el[i]);
    }
    char format[] = "  addq $%lu, %%rsp\n"
                    "  pop %%rbp\n"
                    "  ret\n";
    char* tmp = calloc(1, (15 + size_str_max_length + 12 + 6 + 1) * sizeof(char));
    sprintf(tmp, format, function->scope_size + (function->scope_size % 16));
    utils_strcat(as_text, tmp);
    free(tmp);
}
char* as_compile_to_imm(data_type_T* type, as_value_U value) {
    if (type->primitive_type==TYPE_INT) {
        char* tmp = calloc(1, (1 + int_max_length) * sizeof(char));
        sprintf(tmp, "$%d", value.int_value);
        return tmp;
    } else {
        return "$0";
    }
}
char* as_ensure_no_mem(as_function_T* as, char** as_text, as_op_T* op) {
    if (as->last_register == REG_IMM && as->imm_is_mem) {
        char* tmp1 = calloc(1, (toreg_format_min_length + strlen(as->last_imm_str)) * sizeof(char));
        sprintf(tmp1, toreg_format, data_type_size_op_char(op->op_size), as->last_imm_str, data_type_size_register(math_registers[as->used_reg+1], op->op_size));
        utils_strcat(as_text, tmp1);
        as->imm_is_mem = 0;
        as->last_register = math_registers[as->used_reg+1];
    }
    if (as->last_register == REG_IMM) {
        return as->last_imm_str;
    } else {
        return data_type_size_register(as->last_register, op->op_size);
    }
}
void as_compile_operation(as_function_T* as, char** as_text, as_op_T* op) {
    char* src;
    if (as->last_register == REG_IMM) {
        src = as->last_imm_str;
    } else {
        src = data_type_size_register(as->last_register, op->op_size);
    }
    if (op->type==ASOP_RETURN) {
        char* dest = data_type_size_register(REG_AX, op->op_size);
        char* temp = calloc(1, (return_format_min_length + strlen(src) + strlen(dest) + 1) * sizeof(char));
        sprintf(temp, return_format, data_type_size_op_char(op->op_size), src, dest, as->scope_size + (as->scope_size % 16));
        utils_strcat(as_text, temp);
        as->last_register = REG_CX;
    } else if (op->type == ASOP_SETLASTIMM) {
        as->last_imm_str = as_compile_to_imm(op->data_type, op->value);
        as->imm_is_mem = 0;
        as->last_register = REG_IMM;
    } else if (op->type == ASOP_VDEF) {
        src = as_ensure_no_mem(as, as_text, op);
        char* temp = calloc(1, (vdef_format_min_length + strlen(src)) * sizeof(char));
        sprintf(temp, vdef_format, data_type_size_op_char(op->op_size), src, op->var_location);
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_VREF) {
        as->last_register = REG_IMM;
        char* temp = calloc(1, (vref_format_length) * sizeof(char));
        sprintf(temp, vref_format, op->var_location);
        as->imm_is_mem = 1;
        as->last_imm_str = temp;
    } else if (op->type == ASOP_VMOD) {
        src = as_ensure_no_mem(as, as_text, op);
        char* temp = calloc(1, (vmod_format_min_length + strlen(src)) * sizeof(char));
        sprintf(temp, vmod_format, data_type_size_op_char(op->op_size), src, op->var_location);
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_ARGTOSTACK) {
        char* temp = calloc(1, argtostack_format_min_length * sizeof(char));
        sprintf(temp, argtostack_format, data_type_size_op_char(op->op_size), data_type_size_register(arg_registers[op->argno], op->op_size), op->var_location);
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_ARGTOREG) {
        char* temp = calloc(1, (argtoreg_format_min_length + strlen(src)) * sizeof(char));
        sprintf(temp, argtoreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(arg_registers[op->argno], op->op_size));
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_FCALL) {
        char* temp = calloc(1, (fcall_format_min_length + strlen(op->name)) * sizeof(char));
        sprintf(temp, fcall_format, data_type_size_op_char(op->op_size), op->name);
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_RETVAL) {
        as->last_register = REG_AX;
    } else if (op->type == ASOP_NEXTREG) {
        if (as->used_reg >= sizeof(math_registers)) {
            err_reg_full();
        }
        as->used_reg++;
        char* temp = calloc(1, (toreg_format_min_length + strlen(src)) * sizeof(char));
        sprintf(temp, toreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(math_registers[as->used_reg], op->op_size));
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_FREEREG) {
        if (as->used_reg < 1) {
            err_no_info();
        }
        as->last_register = math_registers[as->used_reg];
        as->used_reg--;
    } else if (op->type == ASOP_BINOP) {
        as_compile_binop(as, as_text, op, src);
    } else {
        err_unexpected_as_op(op);
    }
}
void as_compile_binop(as_function_T* as, char** as_text, as_op_T* op, char* src) {
    char* instr;
    if (op->data_type->primitive_type == TYPE_INT) {
        if (op->binop_type == BINOP_PLUS) {
            instr = "add";
        } else if (op->binop_type == BINOP_MINUS){
            instr = "sub";
        }
    }
    char* temp = calloc(1, (binop_format_min_length + strlen(instr) + strlen(src)));
    char* reg = data_type_size_register(math_registers[as->used_reg], op->op_size);
    sprintf(temp, binop_format, instr, data_type_size_op_char(op->op_size), src, reg);
    utils_strcat(as_text, temp);
}
char* as_op_type_string(enum op_type type) {
    switch (type) {
        case ASOP_BINOP: return "binop";
        case ASOP_ARGTOREG: return "argtoreg";
        case ASOP_ARGTOSTACK: return "argtostack";
        case ASOP_FCALL: return "fcall";
        case ASOP_FREEREG: return "freereg";
        case ASOP_NEXTREG: return "nextreg";
        case ASOP_RETURN: return "return";
        case ASOP_RETVAL: return "retval";
        case ASOP_SETLASTIMM: return "setlastimm";
        case ASOP_VDEF: return "vdef";
        case ASOP_VMOD: return "vmod";
        case ASOP_VREF: return "vref";
    }
}