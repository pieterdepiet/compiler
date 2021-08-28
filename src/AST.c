#include "include/AST.h"
#include "include/errors.h"

AST_T* init_ast(int type) {
    AST_T* ast = calloc(1, sizeof(struct AST_STRUCT));
    ast->type = type;

    // AST_VARIABLE_DEFINITION
    ast->variable_definition_type = (void*) 0;
    ast->variable_definition_name = (void*) 0;
    ast->variable_definition_value = (void*) 0;
    // AST_VARIABLE
    ast->variable_name = (void*) 0;
    // AST_FUNCTION_CALL
    ast->function_call_function = (void*) 0;
    ast->function_call_named_names = (void*) 0;
    ast->function_call_named_values = (void*) 0;
    ast->function_call_named_size = 0;
    ast->function_call_unnamed_values = (void*) 0;
    ast->function_call_unnamed_size = 0;

    // STRING
    ast->string_value = (void*) 0;
    // INT
    ast->int_value = 0;
    // FLOAT
    ast->float_value = 0;
    // DOUBLE
    ast->double_value = 0;
    // BOOL
    ast->bool_value = 0;

    // AST_COMPOUND
    ast->compound_value = (void*) 0;
    ast->compound_size = 0;

    // AST_PLUS, AST_MINUS, AST_TIMES, AST_SLASH, AST_EQ, AST_NEQ, AST_GRT, AST_LET, AST_GREQ, AST_LEEQ, AST_AND, AST_OR
    ast->left_hand = (void*) 0;
    ast->right_hand = (void*) 0;
    ast->parenthetical = NOT_PARENTHETICAL;
    // AST_NOT
    ast->not_value = (void*) 0;

    // AST_IF
    ast->if_condition = (void*) 0;
    ast->if_body = (void*) 0;
    // AST_ELIF
    ast->elif_condition = (void*) 0;
    ast->elif_body = (void*) 0;
    // AST_ELSE
    ast->else_body = (void*) 0;
    ast->next_statement = (void*) 0;


    // AST_WHILE
    ast->while_condition = (void*) 0;
    ast->while_body = (void*) 0;
    // AST_FOR
    ast->for_initializer = (void*) 0;
    ast->for_condition = (void*) 0;
    ast->for_iterator = (void*) 0;
    ast->for_body = (void*) 0;
    
    // AST_RETURN
    ast->return_value = (void*) 0;
    return ast;
}

ast_arglist_T* init_ast_arglist() {
    ast_arglist_T* arglist = calloc(1, sizeof(struct AST_ARGLIST_STRUCT));
    arglist->named_inside_names = (void*) 0;
    arglist->named_public_names = (void*) 0;
    arglist->named_types = (void*) 0;
    arglist->named_size = 0;
    arglist->unnamed_names = (void*) 0;
    arglist->unnamed_types = (void*) 0;
    return arglist;
}

AST_T* ast_create_int(int i) {
    AST_T* ast_int = init_ast(AST_INT);
    ast_int->int_value = i;
    return ast_int;
}
AST_T* ast_create_string(char* str) {
    AST_T* ast_string = init_ast(AST_STRING);
    ast_string->string_value = str;
    return ast_string;
}
enum expr_level ast_binop_level(int binop_type) {
    if (binop_type <= BINOP_MEMBER) {
        return EXPR_MEMBER;
    } else if (binop_type <= BINOP_DIV) {
        return EXPR_MULDIV;
    } else if (binop_type <= BINOP_MINUS) {
        return EXPR_TERM;
    } else if (binop_type <= BINOP_LEEQ) {
        return EXPR_COMPARISON;
    } else if (binop_type <= BINOP_OR) {
        return EXPR_BOOLOPERATION;
    } else if (binop_type <= BINOP_ASSIGN) {
        return EXPR_VARIABLE_ASSIGNMENT;
    } else {
        err_enum_out_of_range("binop type", binop_type);
        return -1;
    }
}
enum expr_level ast_expr_level(AST_T* node) {
    if (node->parenthetical == PARENTHETICAL) {
        return EXPR_SINGLE_THING;
    }
    switch (node->type) {
        case AST_VARIABLE: case AST_FUNCTION_CALL: case AST_INT: case AST_FLOAT: case AST_DOUBLE: case AST_STRING: case AST_NOOP: return EXPR_SINGLE_THING; break;
        case AST_MEMBER: return EXPR_MEMBER; break;
        case AST_NOT: return EXPR_NOT; break;
        case AST_BINOP: return ast_binop_level(node->binop_type); break;
        default: err_enum_out_of_range("expression node type", node->type); break;
    }
    return -1;
}
int ast_is_primitive(AST_T* node) {
    switch (node->type) {
        case AST_STRING: case AST_INT: case AST_FLOAT: case AST_DOUBLE: case AST_NOOP: return 1; break;
        default: return 0; break;
    }
}
char* ast_node_type_string(AST_T* node) {
    switch (node->type) {
        case AST_VARIABLE_DEFINITION: return "variable definition"; break;
        case AST_VARIABLE: return "variable"; break;
        case AST_FUNCTION_CALL: return "fcall"; break;
        case AST_FUNCTION_DEFINITION: return "fdef"; break;
        case AST_CLASS_DEFINITION: return "class definition"; break;
        case AST_STRING: return "string"; break;
        case AST_INT: return "int"; break;
        case AST_FLOAT: return "float"; break;
        case AST_DOUBLE: return "double"; break;
        case AST_NOOP: return "noop"; break;
        case AST_COMPOUND: return "compound"; break;
        case AST_BINOP: return "binop"; break;
        case AST_NOT: return "not"; break;
        case AST_IF: return "if"; break;
        case AST_ELIF: return "elif"; break;
        case AST_ELSE: return "else"; break;
        case AST_FOR: return "for"; break;
        case AST_WHILE: return "while"; break;
        case AST_RETURN: return "return"; break;
        case AST_HEADER_LIST: return "headerlist"; break;
        case AST_HEADER: return "header"; break;
        case AST_MEMBER: return "member"; break;
        case AST_NEW: return "new"; break;
    }
}