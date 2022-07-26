#ifndef ERRORS_H
#define ERRORS_H
#include "AST.h"
#include "assembly.h"
#include "parser.h"
#include "data_type.h"

void err_unexpected_token(parser_T* parser, int expected);
void err_unexpected_node(AST_T* node);
void err_unknown_type(AST_T* ast_type, AST_T* node);
void err_undefined_variable(char* variable_name, AST_T* node);
void err_undefined_function(char* function_name, AST_T* node);
void err_undefined_member(char* parent_name, char* member_name);
void err_conflicting_types(data_type_T* type1, data_type_T* type2, AST_T* node);
void err_conflicting_ptr_types(data_type_T* type1, data_type_T* type2, AST_T* node);
void err_unexpected_as_op(as_op_T* as_op);
void err_bad_global_type(data_type_T* data_type);
void err_bad_immediate(AST_T* node);
void err_no_file_specified();
void err_file_err(int err_number);
void err_unknown_option(char* option);
void err_node_not_convertable_to_data_type(AST_T* node, data_type_T* data_type);
void err_enum_out_of_range(char* enum_name, int enum_value);
void err_no_named_arg(char* name);
void err_too_many_args_in_fcall(void);
void err_reg_full();
void err_no_return_value(data_type_T* return_type);
void err_not_implemented(char* feature);
void err_class_no_member(char* class, char* member, AST_T* node);
void err_empty_class(char* class);
void err_pointer_is_null(char* pointer_description);
void err_overflow();
void err_duplicate_function(AST_T* node);
#endif