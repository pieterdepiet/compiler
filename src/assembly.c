#include "include/assembly.h"
#include <string.h>
#include "include/AST.h"
#include "include/errors.h"
#include "include/utils.h"

// Definitions
#define int_max_length 8
#define size_str_max_length 20
#define reg_length 4

void as_compile_unop(as_function_T* as, char** as_text, as_op_T* op, char* src);

int registers[] = {REG_CX, REG_DX, REG_SI, REG_DI, REG_8D, REG_9D, REG_10D, REG_11D};

const char* as_data_section_start = ".data\n";
const char* as_text_section_start = ".text\n.globl _start\n";
const char* return_format = "  addq $%lu, %%rsp\n"
                            "  pop %%rbp\n"
                            "  ret\n";
const size_t return_format_min_length = 11 + 15 + size_str_max_length + 6 + 1;
const char* vdef_format =   "  mov%c %s, -%lu(%%rbp)\n";
const size_t vdef_format_min_length = 17 + size_str_max_length + 1;
const char* vref_format =   "-%lu(%%rbp)";
const size_t vref_format_length = 7 + size_str_max_length + 1;
const char* vmod_format =     "  mov%c %s, %s\n";
const size_t vmod_format_min_length = 17 + size_str_max_length + 1;
const char* argtostack_format =   "  mov%c %s, -%lu(%%rbp)\n";
const size_t argtostack_format_min_length = 17 + reg_length + size_str_max_length + 1;
const char* argtoreg_format =     "  mov%c %s, %s\n";
const size_t argtoreg_format_min_length = 11 + reg_length + 1;
const char* fcall_format =        "  call %s\n";
const size_t fcall_format_min_length = 9 + 1;
const char* toreg_format =    "  mov%c %s, %s\n";
const size_t toreg_format_min_length = 10 + reg_length + 1;
const char* binop_format =    "  %s%c %s, %s\n";
const size_t binop_format_min_length = 7 + reg_length + 1;
const char* retnull_format =    "  xorl %%eax, %%eax\n"
                                "  addq $%lu, %%rsp\n"
                                "  pop %%rbp\n"
                                "  ret\n";
const size_t retnull_format_length = 18 + 15 + size_str_max_length + 11 + 6 + 1;
const char* new_format =    "  movl $1, %%edi\n"
                            "  movq $%lu, %%rsi\n"
                            "  callq _calloc\n";
const size_t new_format_length = 16 + 8 + size_str_max_length + 7 + 18 + 1;
const char* memtoreg_format = "  mov%c -%lu(%%rbp), %s\n";
const size_t memtoreg_format_min_length = 8 + size_str_max_length + 9 + 1;
const char* membref_format =  "  movq -%lu(%%rbp), %%rax\n";
const size_t membref_format_min_length = 14 + 1;
const char* membref_operand_format =  "%lu(%%rax)";
const size_t membref_operand_format_length = 7 + size_str_max_length + 1;
const char* symbref_format =  "%s(%%rip)";
const size_t symbref_format_min_length = 6 + 1;
const char* lea_format =  "  lea%c %s(%%rip), %s\n";
const size_t lea_format_min_length = 7 + 9 + 1;
const char* lea_memb_format = "  leaq -%lu(%%rax), %s\n";
const size_t lea_memb_format_len = 6 + size_str_max_length + 8 + 1;
const char* lea_mem_format =  "  leaq -%lu(%%rbp), %s\n";
const size_t lea_mem_format_len = 8 + size_str_max_length + 9 + 1;
const char* push_format =   "  mov%c %s, -%lu(%%rbp)\n";
const size_t push_format_min_len = 10 + size_str_max_length + 7 + 1;
const char* pop_format =    "  mov%c -%lu(%%rbp), %s\n";
const size_t pop_format_min_len = 8 + size_str_max_length + 9 + 1;
const char* jcond_format =  "  j%s LBB%lu.%u\n";
const size_t jcond_len = 9 + 2 * size_str_max_length + 1;
const char* jmp_format =    "  jmp LBB%lu.%u\n";
const size_t jmp_len = 11 + 2 * size_str_max_length + 1;
const char* bb_format = "LBB%lu.%u:\n";
const size_t bb_len = 6 + 2 * size_str_max_length + 1;
const char* membmod_format =   "  mov%c %s, %lu(%s)\n";
const size_t membmod_format_min_length = 17 + size_str_max_length + 1;
const char* indexmod_format = "  mov%c %s, %s\n";
const size_t indexmode_format_min_length = 10 + 1;
const char* index_format = "(%s,%s,%hhd)";
const size_t index_format_length = 4 + size_str_max_length + 1;


int arg_registers[] = {REG_DI, REG_SI, REG_DX, REG_CX};

int get_nextreg(as_function_T* as) {
    for (size_t i = 0; i < sizeof(registers); i++) {
        int reg = registers[i];
        if (reg == as->ptrdest_reg) {
            goto next;
        }
        for (size_t j = 0; j < as->used_registers_size; j++) {
            if (reg == as->used_registers[j]) {
                goto next;
            }
        }
        as->used_registers = realloc(as->used_registers, ++as->used_registers_size * sizeof(int));
        as->used_registers[as->used_registers_size-1] = reg;
        return reg;
        next:
        ;
    }
    return REG_VOID;
}
int free_register(as_function_T* as) {
    int reg = as->used_registers[--as->used_registers_size];
    as->used_registers[as->used_registers_size] = REG_VOID;
    return reg;
}

char* as_comp_str(enum comparison comp_type) {
    switch (comp_type) {
        case COMP_A: return "a";
        case COMP_AE: return "ae";
        case COMP_B: return "b";
        case COMP_BE: return "be";
        case COMP_E: return "e";
        case COMP_Z: return "z";
        case COMP_G: return "g";
        case COMP_GE: return "ge";
        case COMP_L: return "l";
        case COMP_LE: return "le";
        case COMP_NA: return "na";
        case COMP_NAE: return "nae";
        case COMP_NB: return "nb";
        case COMP_NBE: return "nbe";
        case COMP_NE: return "ne";
        case COMP_NG: return "ng";
        case COMP_NGE: return "nge";
        case COMP_NL: return "nl";
        case COMP_NLE: return "nle";
        case COMP_NZ: return "nz";
    }
}
int inv_dict[] = {
    COMP_NA,
    COMP_NAE,
    COMP_NB,
    COMP_NBE,
    COMP_NE,
    COMP_NZ,
    COMP_NG,
    COMP_NGE,
    COMP_NL,
    COMP_NLE,
    COMP_A,
    COMP_AE,
    COMP_B,
    COMP_BE,
    COMP_E,
    COMP_Z,
    COMP_G,
    COMP_GE,
    COMP_L,
    COMP_LE
};

char* as_comp_inv_str(int comp_type) {
    return as_comp_str(inv_dict[comp_type]);
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
    char* reg_str = calloc(1, 6 * sizeof(char));
    if (reg >= REG_8D) {
        strcpy(reg_str, "%r");
    } else if (size==2) {
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
        case REG_8D: strcat(reg_str, "8d"); break;
        case REG_9D: strcat(reg_str, "9d"); break;
        case REG_10D: strcat(reg_str, "10d"); break;
        case REG_11D: strcat(reg_str, "11d"); break;
        case REG_12D: strcat(reg_str, "12d"); break;
        case REG_13D: strcat(reg_str, "13d"); break;
        case REG_14D: strcat(reg_str, "14d"); break;
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
    return as_file;
}
as_function_T* init_as_function(char* name) {
    as_function_T* as_function = calloc(1, sizeof(struct ASSEMBLY_FUNCTION_STRUCT));
    as_function->name = name;
    as_function->last_register = 0;
    as_function->last_imm_str = (void*) 0;
    as_function->last_stack_offset = 0;
    as_function->scope_size = 0;
    as_function->used_reg = 0;
    as_function->zeroreg = "$0";
    as_function->zerouses = 0;
    return as_function;
}
as_data_T* init_as_data(char* name, enum astype as_type) {
    as_data_T* as_data = calloc(1, sizeof(struct assembly_data_struct));
    as_data->type = as_type;
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
// Small functions
void as_add_data(as_file_T* as, as_data_T* data) {
    list_add(&as->data, &as->data_size, data);
}
void as_add_function(as_file_T* as, as_function_T* function) {
    function->function_no = as->functions_size;
    list_add(&as->functions, &as->functions_size, function);
}
void as_add_op_to_function(as_function_T* function, as_op_T* op) {
    list_add(&function->operations, &function->operations_size, op);
}

void as_compile_data(char** as_text, as_data_T* data) {
    char* format = "%s  %s: .%s %s\n";
    char* value_string = as_compile_to_imm(data->type, data->value);
    size_t as_text_size = 
    strlen(*as_text)
    + 2 // <space><space>
    + strlen(data->name) // %s
    + 3 // : .
    + 5 // %s
    + 1 // space
    + strlen(value_string)
    + 2 // \n\0
    ;
    *as_text = realloc(*as_text, as_text_size * sizeof(char));
    char* assembly_type = (void*) 0;
    switch (data->type) {
        case ASTYPE_CHAR: assembly_type = "byte"; break;
        case ASTYPE_SHORT: assembly_type = "short"; break;
        case ASTYPE_INT: assembly_type = "long"; break;
        case ASTYPE_LONG: assembly_type = "quad"; break;
        case ASTYPE_STRING: assembly_type = "asciz"; free(data->value.ptr_value); break;
    }
    sprintf(*as_text, format, *as_text, data->name, assembly_type, value_string);
    free(value_string);
    free(data->name);
    free(data);
}
char* as_compile_file(as_file_T* as_file) {
    char* buf = calloc(1, sizeof(char));
    buf[0] = '\0';
    utils_strcat(&buf, as_data_section_start);
    for (size_t i = 0; i < as_file->data_size; i++) {
        as_compile_data(&buf, as_file->data[i]);
    }
    utils_strcat(&buf, as_text_section_start);
    for (size_t i = 0; i < as_file->functions_size; i++) {
        as_compile_function_definition(&buf, as_file->functions[i]);
    }
    utils_strcat(&buf, 
        "\n  .section __TEXT,__cstring\n"
    );
    for (size_t i = 0; i < as_file->unnamed_strings_size; i++) {
        char* tmp = calloc(1, 6 + size_str_max_length + 12 + strlen(as_file->unnamed_strings[i]) + 3);
        sprintf(tmp, "L_.str%zu:\n  .asciz \"%s\"\n", i, as_file->unnamed_strings[i]);
        utils_strcat(&buf, tmp);
        free(tmp);
        free(as_file->unnamed_strings[i]);
    }
    return buf;
}
void as_compile_function_definition(char** as_text, as_function_T* function) {
    function->last_stack_offset = function->scope_size;
    *as_text = realloc(*as_text, (strlen(*as_text) + strlen(function->name) + 48 + size_str_max_length /* :\n  push %rbp\n  movq %rsp, %rbp\n */ + 1 /* \0 */) * sizeof(char));
    sprintf(*as_text, 
    "%s%s:\n"
    "  push %%rbp\n"
    "  movq %%rsp, %%rbp\n"
    "  subq $%lu, %%rsp\n", *as_text, function->name, function->scope_size + function->visitor_max_extra_stack + 16 - ((function->scope_size + function->visitor_max_extra_stack) % 16));
    for (size_t i = 0; i < function->operations_size; i++) {
        as_compile_operation(function, as_text, function->operations[i]);
    }
}
char* as_compile_to_imm(enum astype type, as_value_U value) {
    if (type==ASTYPE_INT) {
        char* tmp = calloc(1, (1 + int_max_length) * sizeof(char));
        sprintf(tmp, "$%d", value.int_value);
        return tmp;
    } else if (type==ASTYPE_STRING) {
        char* tmp = calloc(1, (1 + strlen((char*) value.ptr_value) + 1 + 1));
        sprintf(tmp, "\"%s\"", (char*) value.ptr_value);
        return tmp;
    } else {
        return "$0";
    }
}
char* as_ensure_no_mem(as_function_T* as, char** as_text, as_op_T* op) {
    int reg = get_nextreg(as);
    char* regstr = data_type_size_register(reg, op->op_size);
    if (as->last_register == REG_MEM) {
        char* tmp1 = calloc(1, (memtoreg_format_min_length + strlen(regstr)) * sizeof(char));
        sprintf(tmp1, memtoreg_format, data_type_size_op_char(op->op_size), as->mem_loc, regstr);
        utils_strcat(as_text, tmp1);
        as->last_register = reg;
    } else if (as->last_register == REG_SYMBADDR) {
        char* format = "  lea%c %s(%%rip), %s\n";
        char* tmp = calloc(1, (7 + strlen(as->last_imm_str) + 9 + strlen(regstr) + 1) * sizeof(char));
        sprintf(tmp, format, data_type_size_op_char(op->op_size), as->last_imm_str, regstr);
        utils_strcat(as_text, tmp);
        as->last_register = reg;
    } else if (as->last_register == REG_SYMB) {
        char* format = "  mov%c %s(%%rip), %s\n";
        char* tmp = calloc(1, (7 + strlen(as->last_imm_str) + 9 + strlen(regstr) + 1) * sizeof(char));
        sprintf(tmp, format, data_type_size_op_char(op->op_size), as->last_imm_str, regstr);
        utils_strcat(as_text, tmp);
        as->last_register = reg;
    }
    free_register(as);
    if (as->last_register == REG_IMM) {
        return as->last_imm_str;
    } else {
        return data_type_size_register(as->last_register, op->op_size);
    }
}
void as_compile_operation(as_function_T* as, char** as_text, as_op_T* op) {
    if (op->type == ASOP_SETLASTIMM) {
        as->last_imm_str = as_compile_to_imm(op->data_type, op->value);
        as->last_register = REG_IMM;
    } else if (op->type == ASOP_VREF) {
        as->last_register = REG_MEM;
        as->mem_loc = op->var_location;
        as->memb_offset = -1;
    } else if (op->type == ASOP_ARGTOSTACK) {
        char* temp = calloc(1, argtostack_format_min_length * sizeof(char));
        sprintf(temp, argtostack_format, data_type_size_op_char(op->op_size), data_type_size_register(arg_registers[op->argno], op->op_size), op->var_location);
        utils_strcat(as_text, temp);
        free(temp);
    } else if (op->type == ASOP_FCALL) {
        char* temp = calloc(1, (fcall_format_min_length + strlen(op->name)) * sizeof(char));
        sprintf(temp, fcall_format, op->name);
        utils_strcat(as_text, temp);
        free(temp);
    } else if (op->type == ASOP_RETVAL) {
        as->last_register = REG_AX;
    } else if (op->type == ASOP_FREEREG) {
        as->last_register = free_register(as);
    } else if (op->type == ASOP_RETNULL) {
        char* temp = calloc(1, (retnull_format_length) * sizeof(char));
        sprintf(temp, retnull_format, as->scope_size + as->visitor_max_extra_stack + 16 - ((as->scope_size + as->visitor_max_extra_stack) % 16));
        utils_strcat(as_text, temp);
        free(temp);
    } else if (op->type == ASOP_NEW) {
        char* temp = calloc(1, (new_format_length) * sizeof(char));
        sprintf(temp, new_format, op->op_size);
        utils_strcat(as_text, temp);
        as->last_register = REG_AX;
        free(temp);
    } else if (op->type == ASOP_SYMBOLREF) {
        as->last_register = REG_SYMB;
        as->last_imm_str = op->name;
    } else if (op->type == ASOP_SYMBADDRREF) {
        as->last_register = REG_SYMBADDR;
        as->last_imm_str = op->name;
    } else if (op->type == ASOP_STRINGREF) {
        as->last_register = REG_SYMBADDR;
        as->last_imm_str = calloc(1, 6 + size_str_max_length + 1);
        sprintf(as->last_imm_str, "L_.str%zu", op->string_index);
    } else if (op->type == ASOP_LEA) {
        if (as->last_register == REG_MEM) {
            as->last_register = REG_MEMADDR;
        } else if (as->last_register == REG_MEMB) {
            as->last_register = REG_MEMBADDR;
        }
    } else if (op->type == ASOP_LOCALMEMB) {
        if (as->last_register == REG_MEM) {
            as->mem_loc += op->memb_offset;
        } else {
            err_unexpected_as_op(op);
        }
    } else if (op->type == ASOP_MEMTOREG && as->last_register == REG_MEM) {
        char* temp = calloc(1, memtoreg_format_min_length + 4);
        int reg = REG_AX;
        sprintf(temp, memtoreg_format, data_type_size_op_char(op->op_size), as->mem_loc, data_type_size_register(reg, 8));
        utils_strcat(as_text, temp);
        free(temp);
    } else if (op->type == ASOP_JCOND) {
        char* cond = as_comp_inv_str(as->last_comparison);
        char* temp = calloc(1, (jcond_len + strlen(cond)) * sizeof(char));
        sprintf(temp, jcond_format, cond, as->function_no, op->bb_no);
        utils_strcat(as_text, temp);
        free(temp);
    } else if (op->type == ASOP_JMP) {
        char* temp = calloc(1, jmp_len * sizeof(char));
        sprintf(temp, jmp_format, as->function_no, op->bb_no);
        utils_strcat(as_text, temp);
        free(temp);
    } else if (op->type == ASOP_BB) {
        char* temp = calloc(1, bb_len * sizeof(char));
        sprintf(temp, bb_format, as->function_no, op->bb_no);
        utils_strcat(as_text, temp);
        free(temp);
    } else if (op->type == ASOP_POPREG) {
        as->last_register = REG_MEM;
        as->mem_loc = as->last_stack_offset;
        as->last_stack_offset -= op->op_size;
    } else if (op->type == ASOP_FREEPTRREG) {
        as->ptrdest_reg = 0;
    } else if (op->type == ASOP_SETLASTCMP) {
        as->last_comparison = op->cmp_type;
    } else {
        char* src;
        if (as->last_register == REG_SYMBADDR && op->type == ASOP_ARGTOREG) {
            char* reg = data_type_size_register(arg_registers[op->argno], op->op_size);
            char* temp = calloc(1, (lea_format_min_length + strlen(as->last_imm_str) + strlen(reg)) * sizeof(char));
            sprintf(temp, lea_format, data_type_size_op_char(op->op_size), as->last_imm_str, reg);
            utils_strcat(as_text, temp);
            free(temp);
        } else if (as->last_register == REG_SYMBADDR && op->type == ASOP_RETURN) {
            char* temp = calloc(1, (lea_format_min_length + strlen(as->last_imm_str)) * sizeof(char));
            sprintf(temp, lea_format, data_type_size_op_char(op->op_size), as->last_imm_str, data_type_size_register(REG_AX, op->op_size));
            utils_strcat(as_text, temp);
            temp = realloc(temp, (return_format_min_length + 1) * sizeof(char));
            sprintf(temp, return_format, as->scope_size + as->visitor_max_extra_stack + 16 - ((as->scope_size + as->visitor_max_extra_stack) % 16));
            utils_strcat(as_text, temp);
            as->last_register = REG_CX;
            free(temp);
        } else if (as->last_register == REG_SYMBADDR && op->type == ASOP_NEXTREG) {
            int reg = get_nextreg(as);
            if (reg == REG_VOID) {
                err_reg_full();
            }
            char* temp = calloc(1, (lea_format_min_length + strlen(src)) * sizeof(char));
            sprintf(temp, lea_format, data_type_size_op_char(op->op_size), as->last_imm_str, data_type_size_register(reg, op->op_size));
            utils_strcat(as_text, temp);
            free(temp);
            as->last_register = reg;
        } else if (as->last_register == REG_MEMADDR && op->type == ASOP_NEXTREG) {
            int reg = get_nextreg(as);
            if (reg == REG_VOID) {
                err_reg_full();
            }
            char* regstr = data_type_size_register(reg, 8);
            char* temp = calloc(1, (lea_mem_format_len + strlen(regstr)) * sizeof(char));
            sprintf(temp, lea_mem_format, as->mem_loc, regstr);
            utils_strcat(as_text, temp);
            free(temp);
            as->last_register = reg;
        } else if (as->last_register == REG_MEMADDR && op->type == ASOP_ARGTOREG) {
            char* reg = data_type_size_register(arg_registers[op->argno], op->op_size);
            char* temp = calloc(1, lea_mem_format_len + strlen(reg));
            sprintf(temp, lea_mem_format, as->mem_loc, reg);
            utils_strcat(as_text, temp);
            free(temp);
        } else if (as->last_register == REG_MEMBADDR && op->type == ASOP_ARGTOREG) {
            char* reg = data_type_size_register(arg_registers[op->argno], op->op_size);
            char* temp = calloc(1, lea_memb_format_len + strlen(reg));
            sprintf(temp, lea_memb_format, as->mem_loc, reg);
            utils_strcat(as_text, temp);
            free(temp);
        } else if (as->last_register == REG_MEMADDR && op->type == ASOP_MEMTOREG) {
            char* temp = calloc(1, lea_mem_format_len + 4);
            int reg;
            if (as->ptrdest_reg) {
                reg = registers[as->used_reg++];
            } else {
                reg = as->ptrdest_reg = REG_AX;
            }
            sprintf(temp, lea_mem_format, as->mem_loc, data_type_size_register(reg,8));
            utils_strcat(as_text, temp);
            free(temp);
        } else {
            if (as->last_register == REG_SYMBADDR) {
                int reg = get_nextreg(as);
                char* regstr = data_type_size_register(reg, op->op_size);
                char* temp = calloc(1, (lea_format_min_length + strlen(as->last_imm_str) + strlen(regstr)) * sizeof(char));
                sprintf(temp, lea_format, data_type_size_op_char(op->op_size), as->last_imm_str, regstr);
                utils_strcat(as_text, temp);
                free(temp);
                as->last_register = reg;
                src = data_type_size_register(as->last_register, op->op_size);
            } else if (as->last_register == REG_SYMB) {
                src = calloc(1, (symbref_format_min_length + strlen(as->last_imm_str)));
                sprintf(src, symbref_format, as->last_imm_str);
            } else if (as->last_register == REG_IMM) {
                src = as->last_imm_str;
            } else if (as->last_register == REG_MEM) {
                src = calloc(1, (vref_format_length) * sizeof(char));
                sprintf(src, vref_format, as->mem_loc);
            } else if (as->last_register == REG_MEMB) {
                as->mem_loc = 0;
                if (as->memb_offset == 0) {
                    src = "(%rax)";
                } else {
                    src = calloc(1, (membref_operand_format_length) * sizeof(char));
                    sprintf(src, membref_operand_format, as->memb_offset);
                }
                as->memb_offset = -1;
            } else if (as->last_register == REG_INDEX) {
                char* arrreg = data_type_size_register(as->used_registers[as->used_registers_size-2], 8);
                char* ireg = data_type_size_register(as->used_registers[as->used_registers_size-1], 8);
                src = calloc(1, index_format_length + strlen(arrreg) + strlen(ireg));
                sprintf(src, index_format, arrreg, ireg, op->op_size);
            } else {
                src = data_type_size_register(as->last_register, op->op_size);
            }
            if (op->type == ASOP_ARGTOREG) {
                char* temp = calloc(1, (argtoreg_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, argtoreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(arg_registers[op->argno], op->op_size));
                utils_strcat(as_text, temp);
                free(temp);
            } else if (op->type == ASOP_RETURN) {
                char* temp = calloc(1, (toreg_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, toreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(REG_AX, op->op_size));
                utils_strcat(as_text, temp);
                temp = realloc(temp, (return_format_min_length + 1) * sizeof(char));
                sprintf(temp, return_format, as->scope_size + as->visitor_max_extra_stack + 16 - ((as->scope_size + as->visitor_max_extra_stack) % 16));
                utils_strcat(as_text, temp);
                free(temp);
            } else if (op->type == ASOP_VMOD) {
                src = as_ensure_no_mem(as, as_text, op);
                char* temp = calloc(1, (vdef_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, vdef_format, data_type_size_op_char(op->op_size), src, op->var_location);
                utils_strcat(as_text, temp);
                free(temp);
            } else if (op->type == ASOP_PTRMEMBMOD) {
                src = data_type_size_register(as->used_registers[as->used_registers_size-1], op->op_size);
                char* temp = calloc(1, (membmod_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, membmod_format, data_type_size_op_char(op->op_size), src, op->memb_offset, data_type_size_register(as->ptrdest_reg, 8));
                utils_strcat(as_text, temp);
                free(temp);
            } else if (op->type == ASOP_NEXTREG) {
                int reg = get_nextreg(as);
                if (reg == REG_VOID) {
                    err_reg_full();
                }
                char* temp = calloc(1, (toreg_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, toreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(reg, op->op_size));
                utils_strcat(as_text, temp);
                free(temp);
            } else if (op->type == ASOP_BINOP) {
                as_compile_binop(as, as_text, op, src);
            } else if (op->type == ASOP_UNOP) {
                as_compile_unop(as, as_text, op, src);
            } else if (op->type == ASOP_MEMBREF) {
                as->last_register = REG_MEMB;
                as->memb_offset = op->memb_offset;
                as->mem_loc = 0;
            } else if (op->type == ASOP_PUSHREG) {
                char* temp = calloc(1, (push_format_min_len + strlen(src)) * sizeof(char));
                as->last_stack_offset += op->op_size;
                sprintf(temp, push_format, data_type_size_op_char(op->op_size), src, as->last_stack_offset);
                utils_strcat(as_text, temp);
            } else if (op->type == ASOP_PTRTOREG) {
                as->ptrdest_reg = get_nextreg(as);
                free_register(as);
                char* reg = data_type_size_register(as->ptrdest_reg, 8);
                char* temp = calloc(1, (toreg_format_min_length + strlen(src) + strlen(reg)));
                sprintf(temp, toreg_format, data_type_size_op_char(8), src, reg);
                utils_strcat(as_text, temp);
                free(temp);
                free(reg);
            } else if (op->type == ASOP_LOCALMEMBMOD) {
                char* reg = data_type_size_register(as->used_registers[as->used_registers_size-1], op->op_size);
                char* temp = calloc(1, (toreg_format_min_length + strlen(src) + strlen(reg)));
                sprintf(temp, toreg_format, data_type_size_op_char(op->op_size), reg, src);
                utils_strcat(as_text, temp);
                free(temp);
                free(reg);
            } else if (op->type == ASOP_INDEXMOD) {
                char* valreg = data_type_size_register(as->used_registers[as->used_registers_size-3], op->op_size);
                char* arrreg = data_type_size_register(as->used_registers[as->used_registers_size-2], 8);
                char* ireg = data_type_size_register(as->used_registers[as->used_registers_size-1], 8);
                char* dest = calloc(1, (index_format_length + strlen(arrreg) + strlen(ireg)) * sizeof(char));
                sprintf(dest, index_format, arrreg, ireg, op->op_size);
                char* temp = calloc(1, (indexmode_format_min_length + strlen(valreg) + strlen(dest)) * sizeof(char));
                sprintf(temp, indexmod_format, data_type_size_op_char(op->op_size), valreg, dest);
                utils_strcat(as_text, temp);
                free(valreg);
                free(arrreg);
                free(ireg);
                free(dest);
                free(temp);
            } else if (op->type == ASOP_INDEX) {
                as->last_register = REG_INDEX;
            } else {
                err_unexpected_as_op(op);
            }
        }
    }
}
void as_compile_binop(as_function_T* as, char** as_text, as_op_T* op, char* src) {
    char* instr;
    if (op->data_type == ASTYPE_INT) {
        if (op->binop_type == ASBINOP_ADD) {
            instr = "add";
        } else if (op->binop_type == ASBINOP_SUB) {
            instr = "sub";
        } else if (op->binop_type == ASBINOP_IMUL) {
            instr = "imul";
        } else if (op->binop_type == ASBINOP_CMP) {
            instr = "cmp";
        // } else if (op->binop_type == BINOP_LET) {
        //     instr = "cmp";
        //     as->last_comparison = COMP_L;
        // } else if (op->binop_type == BINOP_GRT) {
        //     instr = "cmp";
        //     as->last_comparison = COMP_G;
        // } else if (op->binop_type == BINOP_LEEQ) {
        //     instr = "cmp";
        //     as->last_comparison = COMP_LE;
        // } else if (op->binop_type == BINOP_GREQ) {
        //     instr = "cmp";
        //     as->last_comparison = COMP_GE;
        // } else if (op->binop_type == BINOP_EQEQ) {
        //     instr = "cmp";
        //     as->last_comparison = COMP_E;
        // } else if (op->binop_type == BINOP_NEQ) {
        //     instr = "cmp";
        //     as->last_comparison = COMP_NE;
        } else {
            return err_unexpected_as_op(op);
        }
    } else {
        return err_unexpected_as_op(op);
    }
    char* temp = calloc(1, (binop_format_min_length + strlen(instr) + strlen(src)));
    char* reg = data_type_size_register(as->used_registers[as->used_registers_size-1], op->op_size);
    sprintf(temp, binop_format, instr, data_type_size_op_char(op->op_size), src, reg);
    utils_strcat(as_text, temp);
}
void as_compile_unop(as_function_T* as, char** as_text, as_op_T* op, char* src) {
    // char* instr;
    if (op->data_type == ASTYPE_INT) {
        if (op->unop_type == UNOP_NEG) {
            // char* neg_format =  "  movl %s, %s\n"
            //                     "  subl %s, %s\n";
            // size_t neg_format_len = 10 + 10 + 1;

        } else {
            return err_unexpected_as_op(op);
        }
    } else {
        return err_unexpected_as_op(op);
    }
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
        case ASOP_VMOD: return "vmod";
        case ASOP_VREF: return "vref";
        case ASOP_RETNULL: return "retnull";
        case ASOP_NEW: return "new";
        case ASOP_MEMBREF: return "membref"; break;
        case ASOP_SYMBOLREF: return "symbref"; break;
        case ASOP_SYMBADDRREF: return "symbaddrref"; break;
        case ASOP_UNOP: return "unop"; break;
        case ASOP_LEA: return "lea"; break;
        case ASOP_MEMTOREG: return "memtoreg"; break;
        case ASOP_LOCALMEMB: return "localmemb"; break;
        case ASOP_POPREG: return "popreg"; break;
        case ASOP_PUSHREG: return "pushreg"; break;
        case ASOP_JCOND: return "jcond"; break;
        case ASOP_JMP: return "jmp"; break;
        case ASOP_BB: return "bb"; break;
        case ASOP_STRINGREF: return "stringref"; break;
        case ASOP_PTRMEMBMOD: return "ptrmembmod"; break;
        case ASOP_SETLASTCMP: return "setlastcmp"; break;
        case ASOP_FREEPTRREG: return "freeptrreg"; break;
        case ASOP_PTRTOREG: return "ptrtoreg"; break;
        case ASOP_LOCALMEMBMOD: return "localmembmod"; break;
        case ASOP_INDEX: return "index"; break;
        case ASOP_INDEXMOD: return "indexmod"; break;
        // default: return "unknown"; break;
    }
}