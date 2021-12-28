#include "include/errors.h"
#include <string.h>

#define ErrorStart "\e[0;31mError:\e[0m "

void err_unexpected_token(parser_T* parser, int expected) {
    printf(ErrorStart "Unexpected token '%s' with type %s; Expected %s [%lu:%lu]\n", parser->current_token->value, token_type_string(parser->current_token->type), token_type_string(expected), parser->lexer->lineno, parser->lexer->charno);
    exit(1);
}
void err_unexpected_node(AST_T* node) {
    printf(ErrorStart "unexpected node with type %d [%lu:%lu]\n", node->type, node->lineno, node->charno);
    exit(1);
}
void err_unknown_type(char* type_name, AST_T* node) {
    printf(ErrorStart "type %s not defined [%lu:%lu]\n", type_name, node->lineno, node->charno);
    exit(1);
}
void err_undefined_variable(char* variable_name, AST_T* node) {
    printf(ErrorStart "variable %s is not defined [%lu:%lu]\n", variable_name, node->lineno, node->charno);
    exit(1);
}
void err_undefined_function(char* function_name, AST_T* node) {
    printf(ErrorStart "function %s is not defined [%lu:%lu]\n", function_name, node->lineno, node->charno);
    exit(1);
}
char* get_type_info(data_type_T* type) {
    if (type->primitive_type == TYPE_STRUCT) {
        return "instance of ";
    } else if (type->primitive_type == TYPE_PTR) {
        return "ptr";
    } else {
        return "";
    }
}
void err_conflicting_types(data_type_T* type1, data_type_T* type2, AST_T* node) {
    printf(ErrorStart "conflicting types %s%s and %s%s [%lu:%lu]\n", get_type_info(type1), type1->type_name, get_type_info(type2), type2->type_name, node->lineno, node->charno);
    exit(1);
}
void err_conflicting_ptr_types(data_type_T* type1, data_type_T* type2, AST_T* node) {
    printf(ErrorStart "conflicting pointer types %s and %s [%lu:%lu]\n", type1->type_name, type2->type_name, node->lineno, node->charno);
    exit(1);
}
void err_undefined_member(char* parent_name, char* member_name) {
}
void err_unexpected_as_op(as_op_T* as_op) {
    if (as_op->type == ASOP_BINOP) {
        printf(ErrorStart "unexpected assembly binary operation type %d\n", as_op->binop_type);
    } else {
        printf(ErrorStart "unexpected assembly operation with type %d\n", as_op->type);
    }
    exit(1);
}
void err_bad_global_type(data_type_T* data_type) {
    // if (data_type) {
        printf("Error: bad type '%s' for global variable\n", data_type->type_name);
    // }
    exit(1);
}
void err_bad_immediate(AST_T* node) {
    printf(ErrorStart "bad immediate type: %d\n", node->type);
    exit(1);
}

void err_no_file_specified() {
    printf(ErrorStart "no file specified\n");
    exit(1);
}
void err_file_err(int err_number) {
    printf(ErrorStart "error while opening file: %s", strerror(err_number));
    exit(1);
}
void err_unknown_option(char* option) {
    printf(ErrorStart "option %s is unknown. It will be ignored\n", option);
    exit(1);
}
void err_node_not_convertable_to_data_type(AST_T* node, data_type_T* data_type) {
    printf(ErrorStart "Node \n");
    exit(1);
}
void err_no_info(void) {
    printf(ErrorStart "Something went wrong, but no information specified\n");
    exit(1);
}
void err_enum_out_of_range(char* enum_name, int enum_value) {
    printf(ErrorStart "value %d was out of range in enum %s\n", enum_value, enum_name);
    exit(1);
}
void err_no_named_arg(char* name) {
    printf(ErrorStart "function has no named parameter %s\n", name);
    exit(1);
}
void err_too_many_args_in_fcall(void) {
    printf(ErrorStart "too many arguments in function call\n");
    exit(1);
}
void err_reg_full() {
    printf(ErrorStart "out of math register. Try shortening or spreading your calculation\n");
    exit(1);
}
void err_no_return_value(data_type_T* return_type) {
    printf(ErrorStart "function sould return %s but does not return a value\n", return_type->type_name);
    exit(1);
}
void err_not_implemented(char* feature) {
    printf(ErrorStart "%s is not implemented yet\n", feature);
    exit(1);
}
void err_class_no_member(char* class, char* member) {
    printf(ErrorStart "class %s has no member %s\n", class, member);
    exit(1);
}
void err_empty_class(char* class) {
    printf(ErrorStart "type %s needs at least one non-static member\n", class);
    exit(1);
}
void err_pointer_is_null(char* pointer_description) {
    printf(ErrorStart "pointer %s is null\n", pointer_description);
    exit(1);
}
void err_overflow() {
    printf(ErrorStart "number overflow\n");
    exit(1);
}