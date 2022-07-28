#ifndef AST_H
#define AST_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct AST_ARGLIST_STRUCT {
    char** unnamed_names;
    struct AST_STRUCT** unnamed_types;
    size_t unnamed_size;
    char** named_public_names;
    char** named_inside_names;
    struct AST_STRUCT** named_types;
    size_t named_size;
} ast_arglist_T;

struct SCOPE_STRUCT;

typedef struct AST_STRUCT {
    enum {
        AST_VARIABLE_DEFINITION, // 0
        AST_VARIABLE, // 1
        AST_FUNCTION_CALL, // 2
        AST_FUNCTION_DEFINITION, // 3
        AST_CLASS_DEFINITION, // 4

        AST_STRING, // 5
        AST_INT, // 6
        AST_FLOAT, // 7
        AST_DOUBLE, // 8

        AST_NOOP, // 9
        AST_COMPOUND, // 10

        AST_BINOP, // 11
        AST_UNOP, // 12

        AST_IF, // 13
        AST_ELIF, // 14
        AST_ELSE, // 15

        AST_FOR, // 16
        AST_WHILE, // 17

        AST_RETURN, // 18

        AST_HEADER_LIST, // 19
        AST_HEADER, // 20

        AST_MEMBER, // 21
        AST_NEW, // 22
        AST_CONSTRUCTOR, // 23
        
        AST_TYPE
    } type;
    size_t lineno;
    size_t charno;
    size_t index;
    char* filename;
    enum parenthetical {
        NOT_PARENTHETICAL,
        PARENTHETICAL
    } parenthetical;
    union {
        struct {
            // AST_VARIABLE_DEFINITION
            struct AST_STRUCT* variable_definition_type;
            char* variable_definition_name;
            struct AST_STRUCT* variable_definition_value;
            char variable_definition_is_static;
            char variable_definition_is_private;
        };
        struct {
            // AST_VARIABLE
            char* variable_name;
        };
        struct {
            // AST_FUNCTION_CALL
            struct AST_STRUCT* function_call_function;
            char** function_call_named_names;
            struct AST_STRUCT** function_call_named_values;
            size_t function_call_named_size;
            struct AST_STRUCT** function_call_unnamed_values;
            size_t function_call_unnamed_size;
        };
        struct {
            // AST_FUNCTION_DEFINITION
            char* function_definition_name;
            struct AST_STRUCT* function_definition_body;
            ast_arglist_T* function_definition_args;
            struct AST_STRUCT* function_definition_return_type;
            char function_definition_is_static;
            char function_definition_is_private;
        };
        struct {
            // STRING
            char* string_value;
        };
        struct {
            // INT
            int int_value;
        };
        struct {
            // FLOAT
            float float_value;
        };
        struct {
            // DOUBLE
            double double_value;
        };
        struct {
            // BOOL
            int bool_value;
        };
        struct {
            // AST_COMPOUND
            struct AST_STRUCT** compound_value;
            size_t compound_size;
        };
        struct {
            // AST_HEADER_LIST
            enum {
                HEADER_NORMAL,
                HEADER_C,
                HEADER_CPP
            } headers_type;
            struct AST_STRUCT** headers_value;
            size_t headers_size;
        };
        struct {
            // AST_PLUS, AST_MINUS, AST_TIMES, AST_SLASH, AST_EQ, AST_NEQ, AST_GRT, AST_LET, AST_GREQ, AST_LEEQ, AST_AND, AST_OR, AST_VARIABLE_ASSIGNMENT
            struct AST_STRUCT* left_hand;
            struct AST_STRUCT* right_hand;
            enum binop_type {
                BINOP_INDEX,
                BINOP_TIMES,
                BINOP_DIV,
                BINOP_PLUS,
                BINOP_MINUS,
                BINOP_EQEQ,
                BINOP_NEQ,
                BINOP_GRT,
                BINOP_LET,
                BINOP_GREQ,
                BINOP_LEEQ,
                BINOP_AND,
                BINOP_OR,
                BINOP_ASSIGN
            } binop_type;
        };
        struct {
            // AST_UNOP
            enum unop_type {
                UNOP_NOT,
                UNOP_NEG
            } unop_type;
            struct AST_STRUCT* unop_value;
        };
        struct {
            union {
                struct {
                    // AST_IF
                    struct AST_STRUCT* if_condition;
                    struct AST_STRUCT* if_body;
                };
                struct {
                    // AST_ELIF
                    struct AST_STRUCT* elif_condition;
                    struct AST_STRUCT* elif_body;
                };
                struct {
                    // AST_ELSE
                    struct AST_STRUCT* else_body;
                };
            };
            struct AST_STRUCT* next_statement;
        };
        struct {
            // AST_WHILE
            struct AST_STRUCT* while_condition;
            struct AST_STRUCT* while_body;
        };
        struct {
            // AST_FOR
            struct AST_STRUCT* for_initializer;
            struct AST_STRUCT* for_condition;
            struct AST_STRUCT* for_iterator;
            struct AST_STRUCT* for_body;
        };
        struct {
            // AST_RETURN
            struct AST_STRUCT* return_value;
        };
        struct {
            // AST_HEADER
            char* header_function_name;
            ast_arglist_T* header_function_args;
            char* header_function_return_type;
        };
        struct {
            // AST_CLASS_DEFINITION
            char* class_name;
            char** class_prototype_names;
            size_t class_prototype_names_size;
            struct AST_STRUCT** class_members;
            size_t class_members_size;
        };
        struct {
            // AST_MEMBER
            struct AST_STRUCT* member_parent;
            char* member_name;
        };
        struct {
            // AST_NEW
            struct AST_STRUCT* new_function_call;
        };
        struct {
            // AST_CONSTRUCTOR
            struct AST_STRUCT* constructor_function_body;
            ast_arglist_T* constructor_args;
        };
        struct {
            // AST_TYPE
            char* type_base;
            size_t type_array_count;
            struct AST_STRUCT** type_array_sizes;
            char* type_fullname;
        };
    };
} AST_T;
struct PARSER_STRUCT;
AST_T* init_ast(int type, struct PARSER_STRUCT* parser);
ast_arglist_T* init_ast_arglist();

AST_T* ast_create_string(char* str);
AST_T* ast_create_int(int i);

AST_T* ast_get_copy(AST_T* source);

enum expr_level {
    EXPR_SINGLE_THING,
    EXPR_MEMBER,
    EXPR_INDEX,
    EXPR_TYPE,
    EXPR_UNOP,
    EXPR_MULDIV,
    EXPR_TERM,
    EXPR_COMPARISON,
    EXPR_BOOLOPERATION,
    EXPR_VARIABLE_ASSIGNMENT
};

enum expr_level ast_binop_level(int binop_type);
enum expr_level ast_expr_level(AST_T* node);
int ast_is_primitive(AST_T* node);
char* ast_node_type_string(AST_T* node);
int ast_binop_is_bool(int binop_type);
void ast_release(AST_T* node);
#endif
