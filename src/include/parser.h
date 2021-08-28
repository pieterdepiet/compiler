#ifndef PARSER_H
#define PARSER_H
#include "AST.h"
#include "lexer.h"


typedef struct PARSER_STRUCT {
    lexer_T* lexer;
    token_T* current_token;
    size_t current_token_counter;
    token_T* prev_token;
    token_T** all_tokens;
    size_t all_tokens_size;
} parser_T;

parser_T* init_parser(lexer_T* lexer);

void parser_eat(parser_T* parser, int token_type);

AST_T* parser_parse(parser_T* parser);

AST_T* parser_parse_statement(parser_T* parser);
AST_T* parser_parse_header(parser_T* parser, int headers_type);

AST_T* parser_parse_id(parser_T* parser);
AST_T* parser_parse_expr(parser_T* parser);
AST_T* parser_parse_lparen(parser_T* parser);

AST_T* parser_parse_variable_definition(parser_T* parser);
AST_T* parser_parse_variable_assignment(parser_T* parser, AST_T* variable);
AST_T* parser_parse_function_definition(parser_T* parser);
AST_T* parser_parse_variable(parser_T* parser);
AST_T* parser_parse_function_call(parser_T* parser, AST_T* function);

AST_T* parser_parse_string(parser_T* parser);
AST_T* parser_parse_int(parser_T* parser);
AST_T* parser_parse_float(parser_T* parser);
AST_T* parser_parse_double(parser_T* parser);

AST_T* parser_parse_statements(parser_T* parser);
AST_T* parser_parse_headers(parser_T* parser, int headers_type);

AST_T* parser_parse_binop(parser_T* parser, AST_T* left_hand, int op_type);
AST_T* parser_parse_member(parser_T* parser, AST_T* left_hand);
AST_T* parser_parse_new(parser_T* parser);

AST_T* parser_parse_not(parser_T* parser);

AST_T* parser_parse_if(parser_T* parser);

AST_T* parser_parse_for(parser_T* parser);
AST_T* parser_parse_while(parser_T* parser);

AST_T* parser_parse_return(parser_T* parser);

AST_T* parser_parse_header_normal_function(parser_T* parser);

AST_T* parser_parse_class_definition(parser_T* parser);
#endif