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

int math_registers[] = {REG_IMM, REG_CX, REG_DX, REG_SI, REG_DI};

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
const char* fcall_format =        "  call%c %s\n";
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
const char* openif_format =   "  j%s LBB%lu_%lu\n";
const size_t openif_format_min_length = 9 + 2 * size_str_max_length + 1;
const char* else_format = "  jmp LBB%lu_%lu\n"
                        "LBB%lu_%lu:\n";
const size_t else_format_min_length = 11 + 2 * size_str_max_length + 6 + 2 * size_str_max_length + 1;
const char* closeif_format =  "LBB%lu_%lu:\n";
const size_t closeif_format_length = 6 + 2 * size_str_max_length + 1;
const char* lea_memb_format = "  leaq -%lu(%%rax), %s\n";
const size_t lea_memb_format_len = 6 + size_str_max_length + 8 + 1;
const char* lea_mem_format =  "  leaq -%lu(%%rbp), %s\n";
const size_t lea_mem_format_len = 6 + size_str_max_length + 8 + 1;

int arg_registers[] = {REG_DI, REG_SI, REG_DX, REG_CX};


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
    as_function_T* as_function = calloc(1, sizeof(struct ASSEMBLY_FUNCTION_STRUCT));
    as_function->name = name;
    as_function->operations = init_list();
    as_function->last_register = 0;
    as_function->last_imm_str = (void*) 0;
    as_function->last_stack_offset = 0;
    as_function->scope_size = 0;
    as_function->used_reg = 0;
    as_function->zeroreg = "$0";
    as_function->zerouses = 0;
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
    list_add_list(as->data, data);
}
void as_add_function(as_file_T* as, as_function_T* function) {
    function->function_no = as->functions->size;
    list_add_list(as->functions, function);
}
void as_add_op_to_function(as_function_T* function, as_op_T* op) {
    list_add_list(function->operations, op);
}

void as_compile_data(as_text_T* as, char** as_text, as_data_T* data) {
    char* format = "%s  %s: .%s %s\n";
    char* value_string = as_compile_to_imm(data->value_type, data->value);
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
    switch (data->value_type->primitive_type) {
        case TYPE_STRING: assembly_type = "asciz"; break;
        case TYPE_INT: assembly_type = "long"; break;
        default: err_bad_global_type(data->value_type); break;
    }
    sprintf(*as_text, format, *as_text, data->name, assembly_type, value_string);
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
    printf("\tCompile function %s\n", function->name);
    *as_text = realloc(*as_text, (strlen(*as_text) + strlen(function->name) + 48 + size_str_max_length /* :\n  push %rbp\n  movq %rsp, %rbp\n */ + 1 /* \0 */) * sizeof(char));
    sprintf(*as_text, 
    "%s%s:\n"
    "  push %%rbp\n"
    "  movq %%rsp, %%rbp\n"
    "  subq $%lu, %%rsp\n", *as_text, function->name, function->scope_size + 16 - (function->scope_size % 16));
    for (size_t i = 0; i < function->operations->size; i++) {
        as_compile_operation(function, as_text, function->operations->el[i]);
    }
}
char* as_compile_to_imm(data_type_T* type, as_value_U value) {
    if (type->primitive_type==TYPE_INT) {
        char* tmp = calloc(1, (1 + int_max_length) * sizeof(char));
        sprintf(tmp, "$%d", value.int_value);
        return tmp;
    } else if (type->primitive_type==TYPE_STRING) {
        char* tmp = calloc(1, (1 + strlen((char*) value.ptr_value) + 1 + 1));
        sprintf(tmp, "\"%s\"", (char*) value.ptr_value);
        return tmp;
    } else {
        return "$0";
    }
}
char* as_ensure_no_mem(as_function_T* as, char** as_text, as_op_T* op) {
    char* reg = data_type_size_register(math_registers[as->used_reg+1], op->op_size);
    if (as->last_register == REG_MEM) {
        char* tmp1 = calloc(1, (memtoreg_format_min_length + strlen(reg)) * sizeof(char));
        sprintf(tmp1, memtoreg_format, data_type_size_op_char(op->op_size), as->mem_loc, reg);
        utils_strcat(as_text, tmp1);
        as->last_register = math_registers[as->used_reg+1];
    } else if (as->last_register == REG_SYMBADDR) {
        char* format = "  lea%c %s(%%rip), %s\n";
        char* tmp = calloc(1, (7 + strlen(as->last_imm_str) + 9 + strlen(reg) + 1) * sizeof(char));
        sprintf(tmp, format, data_type_size_op_char(op->op_size), as->last_imm_str, reg);
        utils_strcat(as_text, tmp);
        as->last_register = math_registers[as->used_reg+1];
    } else if (as->last_register == REG_SYMB) {
        char* format = "  mov%c %s(%%rip), %s\n";
        char* tmp = calloc(1, (7 + strlen(as->last_imm_str) + 9 + strlen(reg) + 1) * sizeof(char));
        sprintf(tmp, format, data_type_size_op_char(op->op_size), as->last_imm_str, reg);
        utils_strcat(as_text, tmp);
        as->last_register = math_registers[as->used_reg+1];
    }
    if (as->last_register == REG_IMM) {
        return as->last_imm_str;
    } else {
        return data_type_size_register(as->last_register, op->op_size);
    }
}
void as_compile_operation(as_function_T* as, char** as_text, as_op_T* op) {
    // printf("Asop %s op size %d lastreg %d\n", as_op_type_string(op->type), op->op_size, as->last_register);
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
    } else if (op->type == ASOP_FCALL) {
        char* temp = calloc(1, (fcall_format_min_length + strlen(op->name)) * sizeof(char));
        sprintf(temp, fcall_format, data_type_size_op_char(op->op_size), op->name);
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_RETVAL) {
        as->last_register = REG_AX;
    } else if (op->type == ASOP_FREEREG) {
        if (as->used_reg < 1) {
            err_enum_out_of_range("used register", as->used_reg);
        }
        as->last_register = math_registers[as->used_reg];
        as->used_reg--;
    } else if (op->type == ASOP_RETNULL) {
        char* temp = calloc(1, (retnull_format_length) * sizeof(char));
        sprintf(temp, retnull_format, as->scope_size + 16 - (as->scope_size % 16));
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_NEW) {
        char* temp = calloc(1, (new_format_length) * sizeof(char));
        size_t class_size = 0;
        for (size_t i = 0; i < op->data_type->instance_members_size; i++) {
            class_size += op->data_type->instance_member_types[i]->primitive_size;
        }
        sprintf(temp, new_format, class_size);
        utils_strcat(as_text, temp);
        as->last_register = REG_AX;
    } else if (op->type == ASOP_SETDEST) {
        as->dest_size++;
        as->dest_offsets = realloc(as->dest_offsets, as->dest_size * sizeof(size_t));
        as->dest_locs = realloc(as->dest_locs, as->dest_size * sizeof(size_t));
        as->dest_offsets[as->dest_size-1] = as->memb_offset;
        as->dest_locs[as->dest_size-1] = as->mem_loc;
    } else if (op->type == ASOP_FREEDEST) {
        as->dest_size -= 1;
        as->dest_offsets = realloc(as->dest_offsets, as->dest_size * sizeof(size_t));
        as->dest_locs = realloc(as->dest_locs, as->dest_size * sizeof(size_t));
    } else if (op->type == ASOP_SYMBOLREF) {
        as->last_register = REG_SYMB;
        as->last_imm_str = op->name;
    } else if (op->type == ASOP_SYMBADDRREF) {
        as->last_register = REG_SYMBADDR;
        as->last_imm_str = op->name;
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
        sprintf(temp, memtoreg_format, data_type_size_op_char(op->op_size), as->mem_loc, "%rax");
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_OPENIF) {
        if (op->bb_ptr->bb_no == -1) {
            op->bb_ptr->bb_no = as->last_bb;
            as->last_bb++;
        }
        op->bb_no = op->bb_ptr->bb_no;
        char* condition = as_comp_inv_str(as->last_comparison);
        char* temp = calloc(1, (openif_format_min_length + strlen(condition)) * sizeof(char));
        sprintf(temp, openif_format, condition, as->function_no, op->bb_no);
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_CLOSEIF) {
        char* temp = calloc(1, (closeif_format_length) * sizeof(char));
        sprintf(temp, closeif_format, as->function_no, op->bb_no);
        utils_strcat(as_text, temp);
    } else if (op->type == ASOP_ELSE) {
        if (op->bb_ptr) {
            if (op->bb_ptr->bb_no == -1) {
                op->bb_ptr->bb_no = as->last_bb;
                as->last_bb++;
            }
            char* temp = calloc(1, (else_format_min_length) * sizeof(char));
            sprintf(temp, else_format, as->function_no, op->bb_ptr->bb_no, as->function_no, op->bb_no);
            utils_strcat(as_text, temp);
        } else {
            printf("Error: No bb opener in else\n");
            exit(1);
        }
    } else if (op->type == ASOP_VDEFNULL) {
        
    } else {
        char* src;
        if (as->last_register == REG_SYMBADDR && op->type == ASOP_ARGTOREG) {
            char* reg = data_type_size_register(arg_registers[op->argno], op->op_size);
            char* temp = calloc(1, (lea_format_min_length + strlen(as->last_imm_str) + strlen(reg)) * sizeof(char));
            sprintf(temp, lea_format, data_type_size_op_char(op->op_size), as->last_imm_str, reg);
            utils_strcat(as_text, temp);
        } else if (as->last_register == REG_SYMBADDR && op->type == ASOP_RETURN) {
            char* temp = calloc(1, (lea_format_min_length + strlen(as->last_imm_str)) * sizeof(char));
            sprintf(temp, lea_format, data_type_size_op_char(op->op_size), as->last_imm_str, data_type_size_register(REG_AX, op->op_size));
            utils_strcat(as_text, temp);
            temp = calloc(1, (return_format_min_length + 1) * sizeof(char));
            sprintf(temp, return_format, as->scope_size + 16 - (as->scope_size % 16));
            utils_strcat(as_text, temp);
            as->last_register = REG_CX;
        } else if (as->last_register == REG_SYMBADDR && op->type == ASOP_NEXTREG) {
            if (as->used_reg >= sizeof(math_registers)) {
                err_reg_full();
            }
            as->used_reg++;
            if (math_registers[as->used_reg] != as->last_register) {
                char* temp = calloc(1, (lea_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, lea_format, data_type_size_op_char(op->op_size), as->last_imm_str, data_type_size_register(math_registers[as->used_reg], op->op_size));
                utils_strcat(as_text, temp);
            }
        } else if (as->last_register == REG_MEMADDR && op->type == ASOP_ARGTOREG) {
            char* reg = data_type_size_register(arg_registers[op->argno], op->op_size);
            char* temp = calloc(1, lea_mem_format_len + strlen(reg));
            sprintf(temp, lea_mem_format, as->mem_loc, reg);
            utils_strcat(as_text, temp);
        } else if (as->last_register == REG_MEMBADDR && op->type == ASOP_ARGTOREG) {
            char* reg = data_type_size_register(arg_registers[op->argno], op->op_size);
            char* temp = calloc(1, lea_memb_format_len + strlen(reg));
            sprintf(temp, lea_memb_format, as->mem_loc, reg);
            utils_strcat(as_text, temp);
        } else if (as->last_register == REG_MEMADDR && op->type == ASOP_MEMTOREG) {
            char* temp = calloc(1, memtoreg_format_min_length + 4);
            sprintf(temp, memtoreg_format, data_type_size_op_char(op->op_size), op->var_location, "%rax");
            utils_strcat(as_text, temp);
        } else {
            if (as->last_register == REG_SYMBADDR) {
                char* reg = data_type_size_register(math_registers[as->used_reg+1], op->op_size);
                char* temp = calloc(1, (lea_format_min_length + strlen(as->last_imm_str) + strlen(reg)) * sizeof(char));
                sprintf(temp, lea_format, data_type_size_op_char(op->op_size), as->last_imm_str, reg);
                utils_strcat(as_text, temp);
                as->last_register = math_registers[as->used_reg+1];
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
                // char* temp = calloc(1, (membref_format_min_length + strlen(src)) * sizeof(char));
                // sprintf(temp, membref_format, as->mem_loc);
                // utils_strcat(as_text, temp);
                as->mem_loc = 0;
                if (as->memb_offset == 0) {
                    src = "(%rax)";
                } else {
                    src = calloc(1, (membref_operand_format_length) * sizeof(char));
                    sprintf(src, membref_operand_format, as->memb_offset);
                }
                as->memb_offset = -1;
            } else {
                src = data_type_size_register(as->last_register, op->op_size);
            }
            if (op->type == ASOP_ARGTOREG) {
                char* temp = calloc(1, (argtoreg_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, argtoreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(arg_registers[op->argno], op->op_size));
                utils_strcat(as_text, temp);
            } else if (op->type == ASOP_RETURN) {
                char* temp = calloc(1, (toreg_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, toreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(REG_AX, op->op_size));
                utils_strcat(as_text, temp);
                temp = calloc(1, (return_format_min_length + 1) * sizeof(char));
                sprintf(temp, return_format, as->scope_size + 16 - (as->scope_size % 16));
                utils_strcat(as_text, temp);
            } else if (op->type == ASOP_VDEF) {
                src = as_ensure_no_mem(as, as_text, op);
                char* temp = calloc(1, (vdef_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, vdef_format, data_type_size_op_char(op->op_size), src, op->var_location);
                utils_strcat(as_text, temp);
            } else if (op->type == ASOP_VMOD) {
                src = as_ensure_no_mem(as, as_text, op);
                char* dest;
                if (as->dest_locs[as->dest_size-1] > 0) {
                    dest = calloc(1, (vref_format_length) * sizeof(char));
                    sprintf(dest, vref_format, as->dest_locs[as->dest_size-1]);
                } else {
                    if (as->last_register == REG_IMM) {
                        char* tmp = calloc(1, (toreg_format_min_length + strlen(src)) * sizeof(char));
                        sprintf(tmp, toreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(math_registers[as->used_reg+1], op->op_size));
                        utils_strcat(as_text, tmp);
                        src = data_type_size_register(math_registers[as->used_reg+1], op->op_size);
                    }
                    if (as->dest_offsets[as->dest_size-1] == 0) {
                        dest = "(%rax)";
                    } else {
                        dest = calloc(1, (membref_operand_format_length) * sizeof(char));
                        sprintf(dest, membref_operand_format, as->dest_offsets[as->dest_size-1]);
                    }
                }
                char* temp = calloc(1, (vmod_format_min_length + strlen(src)) * sizeof(char));
                sprintf(temp, vmod_format, data_type_size_op_char(op->op_size), src, dest);
                utils_strcat(as_text, temp);
            } else if (op->type == ASOP_NEXTREG) {
                if (as->used_reg >= sizeof(math_registers)) {
                    err_reg_full();
                }
                as->used_reg++;
                if (math_registers[as->used_reg] != as->last_register) {
                    char* temp = calloc(1, (toreg_format_min_length + strlen(src)) * sizeof(char));
                    sprintf(temp, toreg_format, data_type_size_op_char(op->op_size), src, data_type_size_register(math_registers[as->used_reg], op->op_size));
                    utils_strcat(as_text, temp);
                }
            } else if (op->type == ASOP_BINOP) {
                as_compile_binop(as, as_text, op, src);
            } else if (op->type == ASOP_UNOP) {
                as_compile_unop(as, as_text, op, src);
            } else if (op->type == ASOP_MEMBREF) {
                as->last_register = REG_MEMB;
                as->memb_offset = op->memb_offset;
                as->mem_loc = 0;
            } else {
                err_unexpected_as_op(op);
            }
        }
    }
}
void as_compile_binop(as_function_T* as, char** as_text, as_op_T* op, char* src) {
    char* instr;
    if (op->data_type->primitive_type == TYPE_INT) {
        if (op->binop_type == BINOP_PLUS) {
            instr = "add";
        } else if (op->binop_type == BINOP_MINUS) {
            instr = "sub";
        } else if (op->binop_type == BINOP_TIMES) {
            instr = "imul";
        } else if (op->binop_type == BINOP_LET) {
            instr = "cmp";
            as->last_comparison = COMP_L;
        } else if (op->binop_type == BINOP_GRT) {
            instr = "cmp";
            as->last_comparison = COMP_G;
        } else if (op->binop_type == BINOP_LEEQ) {
            instr = "cmp";
            as->last_comparison = COMP_LE;
        } else if (op->binop_type == BINOP_GREQ) {
            instr = "cmp";
            as->last_comparison = COMP_GE;
        } else if (op->binop_type == BINOP_EQEQ) {
            instr = "cmp";
            as->last_comparison = COMP_E;
        } else if (op->binop_type == BINOP_NEQ) {
            instr = "cmp";
            as->last_comparison = COMP_NE;
        } else {
            return err_unexpected_as_op(op);
        }
    } else {
        return err_unexpected_as_op(op);
    }
    char* temp = calloc(1, (binop_format_min_length + strlen(instr) + strlen(src)));
    char* reg = data_type_size_register(math_registers[as->used_reg], op->op_size);
    sprintf(temp, binop_format, instr, data_type_size_op_char(op->op_size), src, reg);
    utils_strcat(as_text, temp);
}
void as_compile_unop(as_function_T* as, char** as_text, as_op_T* op, char* src) {
    // char* instr;
    if (op->data_type->primitive_type == TYPE_INT) {
        if (op->unop_type == UNOP_NEG) {
            char* neg_format =  "  movl %s, %s\n"
                                "  subl %s, %s\n";
            size_t neg_format_len = 10 + 10 + 1;

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
        case ASOP_VDEF: return "vdef";
        case ASOP_VMOD: return "vmod";
        case ASOP_VREF: return "vref";
        case ASOP_RETNULL: return "retnull";
        case ASOP_VDEFNULL: return "vdefnull";
        case ASOP_NEW: return "new";
        case ASOP_MEMBREF: return "membref"; break;
        case ASOP_SETDEST: return "setdest"; break;
        case ASOP_FREEDEST: return "freedest"; break;
        case ASOP_SYMBOLREF: return "symbref"; break;
        case ASOP_SYMBADDRREF: return "symbaddrref"; break;
        case ASOP_OPENIF: return "openif"; break;
        case ASOP_CLOSEIF: return "closeif"; break;
        case ASOP_ELSE: return "else"; break;
        case ASOP_UNOP: return "unop"; break;
        case ASOP_LEA: return "lea"; break;
        case ASOP_MEMTOREG: return "memtoreg"; break;
        case ASOP_LOCALMEMB: return "localmemb"; break;
        // default: return "unknown"; break;
    }
}