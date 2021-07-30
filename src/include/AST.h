#ifndef AST_H
#define AST_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "data_type.h"
#include "list.h"

typedef struct AST_ARG_STRUCT {
    char* definition_name;
    char* inside_name;
    data_type_T* data_type;
} ast_arg_T;
typedef struct AST_ARGLIST_STRUCT {
    char** unnamed_names;
    char** unnamed_types;
    size_t unnamed_size;
    char** named_public_names;
    char** named_inside_names;
    char** named_types;
    size_t named_size;
} ast_arglist_T;

struct SCOPE_STRUCT;

typedef struct AST_STRUCT {
    enum {
        AST_VARIABLE_DEFINITION, // 0
        AST_VARIABLE_ASSIGNMENT, // 1
        AST_VARIABLE, // 2
        AST_FUNCTION_CALL, // 3
        AST_FUNCTION_DEFINITION, // 4
        
        AST_STRING, // 5
        AST_INT, // 6
        AST_FLOAT, // 7
        AST_DOUBLE, // 8

        AST_NOOP, // 9
        AST_COMPOUND, // 10

        AST_PLUS, // 11
        AST_MINUS, // 12
        AST_TIMES, // 13
        AST_SLASH, // 14
        AST_EQ, // 15
        AST_NEQ, // 16
        AST_GRT, // 17
        AST_LET, // 18
        AST_GREQ, // 19
        AST_LEEQ, // 20
        AST_NOT, // 21
        AST_AND, // 22
        AST_OR, // 23

        AST_IF, // 24
        AST_ELIF, // 25
        AST_ELSE, // 26

        AST_FOR, // 27
        AST_WHILE, // 28

        AST_RETURN // 29
    } type;

    char* str1;
    char* str2;
    float flt1;
    double dbl1;
    int int1;
    int int2;
    data_type_T* data_type;
    struct AST_STRUCT* ast1;
    struct AST_STRUCT* ast2;
    struct AST_STRUCT* ast3;
    list_T* list;


    // AST_VARIABLE_DEFINITION
    char* variable_definition_type;
    char* variable_definition_name;
    struct AST_STRUCT* variable_definition_value;

    // AST_VARIABLE
    char* variable_name;
    // AST_FUNCTION_CALL
    struct AST_STRUCT* function_call_function;
    char** function_call_named_names;
    struct AST_STRUCT** function_call_named_values;
    size_t function_call_named_size;
    struct AST_STRUCT** function_call_unnamed_values;
    size_t function_call_unnamed_size;
    // AST_FUNCTION_DEFINITION
    char* function_definition_name;
    struct AST_STRUCT* function_definition_body;
    // ast_arg_T** function_definition_args;
    ast_arglist_T* function_definition_args;
    char* function_definition_return_type;


    // STRING
    char* string_value;
    // INT
    int int_value;
    // FLOAT
    float float_value;
    // DOUBLE
    double double_value;
    // BOOL
    int bool_value;

    // AST_COMPOUND
    struct AST_STRUCT** compound_value;
    size_t compound_size;
    struct SCOPE_STRUCT* compound_scope;

    // AST_PLUS, AST_MINUS, AST_TIMES, AST_SLASH, AST_EQ, AST_NEQ, AST_GRT, AST_LET, AST_GREQ, AST_LEEQ, AST_AND, AST_OR, AST_VARIABLE_ASSIGNMENT
    struct AST_STRUCT* left_hand;
    struct AST_STRUCT* right_hand;
    enum parenthetical {
        NOT_PARENTHETICAL,
        PARENTHETICAL
    } parenthetical;
    // AST_NOT
    struct AST_STRUCT* not_value;

    // AST_IF
    struct AST_STRUCT* if_condition;
    struct AST_STRUCT* if_body;
    // AST_ELIF
    struct AST_STRUCT* elif_condition;
    struct AST_STRUCT* elif_body;
    // AST_ELSE
    struct AST_STRUCT* else_body;

    struct AST_STRUCT* next_statement;

    // AST_WHILE
    struct AST_STRUCT* while_condition;
    struct AST_STRUCT* while_body;
    // AST_FOR
    struct AST_STRUCT* for_initializer;
    struct AST_STRUCT* for_condition;
    struct AST_STRUCT* for_iterator;
    struct AST_STRUCT* for_body;

    // AST_RETURN
    struct AST_STRUCT* return_value;
} AST_T;

AST_T* init_ast(int type);
ast_arglist_T* init_ast_arglist();

AST_T* ast_create_string(char* str);
AST_T* ast_create_int(int i);

AST_T* ast_get_copy(AST_T* source);

enum expr_level {
    EXPR_SINGLE_THING,
    EXPR_NOT,
    EXPR_MULDIV,
    EXPR_TERM,
    EXPR_COMPARISON,
    EXPR_BOOLOPERATION,
    EXPR_VARIABLE_ASSIGNMENT
};

enum expr_level ast_expr_level(AST_T* node);
int ast_is_primitive(AST_T* node);
char* ast_node_type_string(AST_T* node);
#endif
