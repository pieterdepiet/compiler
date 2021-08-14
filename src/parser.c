#include "include/parser.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "include/utils.h"
#include "include/errors.h"

static int powi(int base, int exp) {
    if (exp<1) {
        return 1;
    }
    int res = base;
    for (size_t i = 1; i < exp; i++) {
        res *= base;
    }
    return res;
}

parser_T* init_parser(lexer_T* lexer) {
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = lexer_get_next_token(lexer);
    parser->current_token_counter = 0;
    parser->prev_token = parser->current_token;
    parser->all_tokens_size = 1;
    parser->all_tokens = calloc(parser->all_tokens_size, sizeof(struct TOKEN_STRUCT*));
    parser->all_tokens[0] = parser->current_token;

    if (parser->current_token->type == TOKEN_COMMENT) {
        parser_eat(parser, TOKEN_COMMENT);
    }
    return parser;
}

void parser_eat(parser_T* parser, int token_type) {
    if (parser->current_token->type == token_type) {
        parser->prev_token = parser->current_token;
        parser->current_token_counter += 1;
        if (parser->current_token_counter >= parser->all_tokens_size) {
            parser->all_tokens_size += 1;
            parser->all_tokens = realloc(parser->all_tokens, parser->all_tokens_size * sizeof(struct TOKEN_STRUCT*));
            parser->all_tokens[parser->all_tokens_size-1] = lexer_get_next_token(parser->lexer);
        }
        parser->current_token = parser->all_tokens[parser->current_token_counter];
        if (parser->current_token->type == TOKEN_COMMENT) {
            parser_eat(parser, TOKEN_COMMENT);
        }
        return;
    } else {
        err_unexpected_token(parser, token_type);
    }
}
void parser_go_back(parser_T* parser) {
    if (parser->current_token_counter > 0) {
        parser->current_token_counter -= 1;
        parser->current_token = parser->prev_token;
        if (parser->current_token_counter > 0) {
            parser->prev_token = parser->all_tokens[parser->current_token_counter-1];
        }
        if (parser->current_token->type == TOKEN_COMMENT) {
            parser_go_back(parser);
        }
    } else {
        printf("Can't go back when at first token\n");
        exit(1);
    }
}

AST_T* parser_parse(parser_T* parser) {
    return parser_parse_statements(parser);
}

AST_T* parser_parse_statement(parser_T* parser) {
    if (parser->current_token->type == TOKEN_ID) {
        return parser_parse_id(parser);
    }
    err_unexpected_token(parser, -1);
    return (void*) 0;
}

AST_T* parser_parse_id(parser_T* parser) {
    if (utils_strcmp(parser->current_token->value, "var")) {
        return parser_parse_variable_definition(parser);
    } else if (utils_strcmp(parser->current_token->value, "fun")) {
        return parser_parse_function_definition(parser);
    } else if (utils_strcmp(parser->current_token->value, "return")) {
        return parser_parse_return(parser);
    } else {
        return parser_parse_expr(parser);
    }
    // if (utils_strcmp(parser->current_token->value, "if")) {
    //     return parser_parse_if(parser);
    // } else if (utils_strcmp(parser->current_token->value, "while")) {
    //     return parser_parse_while(parser);
    // } else if (utils_strcmp(parser->current_token->value, "for")) {
    //     return parser_parse_for(parser);
    // } else if (utils_strcmp(parser->current_token->value, "undefined")) {
    //     parser_eat(parser, TOKEN_ID);
    //     return init_ast(AST_NOOP);
    // } else if (utils_strcmp(parser->current_token->value, "return")) {
    //     parser_eat(parser, TOKEN_ID);
    //     AST_T* ast_return = init_ast(AST_RETURN);
    //     ast_return->return_value = parser_parse_expr(parser);
    //     return ast_return;
    // } 
}

AST_T* parser_parse_expr(parser_T* parser) {
    AST_T* left_hand;
    switch (parser->current_token->type) {
        case TOKEN_ID: left_hand = parser_parse_variable(parser); break;
        case TOKEN_INT: left_hand = parser_parse_int(parser); break;
        default: err_unexpected_token(parser, -1); break;
    }
    while (parser->current_token->type != TOKEN_RPAREN && parser->current_token->type != TOKEN_SEMI && parser->current_token->type != TOKEN_COMMA && parser->current_token->type != TOKEN_RBRACE && parser->current_token->type != TOKEN_RBRACKET) {
        AST_T* last_hand = left_hand;
        switch (parser->current_token->type) {
            case TOKEN_PLUS: left_hand = parser_parse_binop(parser, left_hand, BINOP_PLUS); break;
            case TOKEN_MINUS: left_hand = parser_parse_binop(parser, left_hand, BINOP_MINUS); break;
            case TOKEN_TIMES: left_hand = parser_parse_binop(parser, left_hand, BINOP_TIMES); break;
            case TOKEN_SLASH: left_hand = parser_parse_binop(parser, left_hand, BINOP_DIV); break;
            case TOKEN_EQEQ: left_hand = parser_parse_binop(parser, left_hand, BINOP_EQEQ); break;
            case TOKEN_NEQ: left_hand = parser_parse_binop(parser, left_hand, BINOP_NEQ); break;
            case TOKEN_GRT: left_hand = parser_parse_binop(parser, left_hand, BINOP_GRT); break;
            case TOKEN_LET: left_hand = parser_parse_binop(parser, left_hand, BINOP_LET); break;
            case TOKEN_GREQ: left_hand = parser_parse_binop(parser, left_hand, BINOP_GREQ); break;
            case TOKEN_LEEQ: left_hand = parser_parse_binop(parser, left_hand, BINOP_LEEQ); break;
            case TOKEN_EQUALS:
            case TOKEN_PLUS_EQUALS:
            case TOKEN_MINUS_EQUALS:
            case TOKEN_TIMES_EQUALS:
            case TOKEN_SLASH_EQUALS: left_hand = parser_parse_variable_assignment(parser, left_hand); break;
            case TOKEN_PLUSPLUS:
            case TOKEN_MINUSMINUS: err_not_implemented("Incrementing or decrementing"); break;
            case TOKEN_LPAREN: left_hand = parser_parse_function_call(parser, left_hand); break;
            default: {
                if (parser->lexer->newline == NEWLINE) {
                    left_hand->parenthetical = PARENTHETICAL;
                    return left_hand;
                } else {
                    err_unexpected_token(parser, TOKEN_SEMI);
                }
                break;
            }
        }
    }
    left_hand->parenthetical = PARENTHETICAL;
    return left_hand;
}
AST_T* parser_parse_lparen(parser_T* parser) {
    parser_eat(parser, TOKEN_LPAREN);
    AST_T* ast_expr = parser_parse_expr(parser);
    parser_eat(parser, TOKEN_RPAREN);
    return ast_expr;
}

AST_T* parser_parse_variable_definition(parser_T* parser) {
    AST_T* ast_variable_definition = init_ast(AST_VARIABLE_DEFINITION);
    parser_eat(parser, TOKEN_ID); // var
    ast_variable_definition->variable_definition_name = parser->current_token->value;
    parser_eat(parser, TOKEN_ID); // varname
    parser_eat(parser, TOKEN_COLON);
    ast_variable_definition->variable_definition_type = parser->current_token->value;
    parser_eat(parser, TOKEN_ID); // type
    parser_eat(parser, TOKEN_EQUALS);
    ast_variable_definition->variable_definition_value = parser_parse_expr(parser);
    return ast_variable_definition;
}
AST_T* parser_parse_variable_assignment(parser_T* parser, AST_T* variable) {
    AST_T* ast_variable_assignment = init_ast(AST_BINOP);
    ast_variable_assignment->binop_type = BINOP_ASSIGN;
    int assignment_type;
    switch (parser->current_token->type) {
        case TOKEN_EQUALS: parser_eat(parser, TOKEN_EQUALS); assignment_type = -1; break;
        case TOKEN_PLUS_EQUALS: parser_eat(parser, TOKEN_PLUS_EQUALS); assignment_type = BINOP_PLUS; break;
        case TOKEN_MINUS_EQUALS: parser_eat(parser, TOKEN_MINUS_EQUALS); assignment_type = BINOP_MINUS; break;
        case TOKEN_TIMES_EQUALS: parser_eat(parser, TOKEN_TIMES_EQUALS); assignment_type = BINOP_TIMES; break;
        case TOKEN_SLASH_EQUALS: parser_eat(parser, TOKEN_SLASH_EQUALS); assignment_type = BINOP_DIV; break;
        default: err_unexpected_token(parser, -1); break;
    }
    if (assignment_type < 0) {
        ast_variable_assignment->right_hand = parser_parse_expr(parser);
    } else {
        ast_variable_assignment->right_hand = init_ast(AST_BINOP);
        ast_variable_assignment->right_hand->binop_type = assignment_type;
        ast_variable_assignment->right_hand->right_hand = parser_parse_expr(parser);
    }
    if (ast_expr_level(variable) <= EXPR_SINGLE_THING) {
        ast_variable_assignment->left_hand = variable;
        if (assignment_type >= 0) {
            ast_variable_assignment->right_hand->left_hand = variable;
        }
        return ast_variable_assignment;
    } else {
        AST_T* last_variable = variable;
        while (1) {
            if (ast_expr_level(last_variable->right_hand) <= EXPR_SINGLE_THING) {
                ast_variable_assignment->left_hand = last_variable->right_hand;
                last_variable->right_hand = ast_variable_assignment;
                return variable;
            } else {
                last_variable = last_variable->right_hand;
            }
        }
    }
}
AST_T* parser_parse_function_definition(parser_T* parser) {
    AST_T* ast_function_definition = init_ast(AST_FUNCTION_DEFINITION);
    parser_eat(parser, TOKEN_ID); // fun
    ast_function_definition->function_definition_name = parser->current_token->value;
    ast_function_definition->function_definition_args = init_ast_arglist();

    parser_eat(parser, TOKEN_ID); // name
    parser_eat(parser, TOKEN_LPAREN);
    while (parser->current_token->type != TOKEN_RPAREN) {
        if (parser->current_token->type != TOKEN_UNDERSCORE) {
            break;
        }
        parser_eat(parser, TOKEN_UNDERSCORE);
        char* arg_name = parser->current_token->value;
        parser_eat(parser, TOKEN_ID);
        parser_eat(parser, TOKEN_COLON);
        char* arg_type = parser->current_token->value;
        parser_eat(parser, TOKEN_ID);
        ast_function_definition->function_definition_args->unnamed_size += 1;
        ast_function_definition->function_definition_args->unnamed_names = realloc(ast_function_definition->function_definition_args->unnamed_names, ast_function_definition->function_definition_args->unnamed_size * sizeof(char*));
        ast_function_definition->function_definition_args->unnamed_types = realloc(ast_function_definition->function_definition_args->unnamed_types, ast_function_definition->function_definition_args->unnamed_size * sizeof(char*));
        ast_function_definition->function_definition_args->unnamed_names[ast_function_definition->function_definition_args->unnamed_size-1] = arg_name;
        ast_function_definition->function_definition_args->unnamed_types[ast_function_definition->function_definition_args->unnamed_size-1] = arg_type;
        if (parser->current_token->type == TOKEN_COMMA) {
            parser_eat(parser, TOKEN_COMMA);
        } else if (parser->current_token->type != TOKEN_RPAREN) {
            err_unexpected_token(parser, -1);
        }
    }
    while (parser->current_token->type != TOKEN_RPAREN) {
        char* public_name = parser->current_token->value;
        char* inside_name;
        parser_eat(parser, TOKEN_ID);
        if (parser->current_token->type == TOKEN_ID) {
            inside_name = parser->current_token->value;
            parser_eat(parser, TOKEN_ID);
        } else {
            inside_name = public_name;
        }
        parser_eat(parser, TOKEN_COLON);
        char* arg_type = parser->current_token->value;
        parser_eat(parser, TOKEN_ID);
        ast_function_definition->function_definition_args->named_size += 1;
        ast_function_definition->function_definition_args->named_inside_names = realloc(ast_function_definition->function_definition_args->named_inside_names, ast_function_definition->function_definition_args->named_size * sizeof(char*));
        ast_function_definition->function_definition_args->named_public_names = realloc(ast_function_definition->function_definition_args->named_public_names, ast_function_definition->function_definition_args->named_size * sizeof(char*));
        ast_function_definition->function_definition_args->named_types = realloc(ast_function_definition->function_definition_args->named_types, ast_function_definition->function_definition_args->named_size * sizeof(char*));
        ast_function_definition->function_definition_args->named_inside_names[ast_function_definition->function_definition_args->named_size-1] = inside_name;
        ast_function_definition->function_definition_args->named_public_names[ast_function_definition->function_definition_args->named_size-1] = public_name;
        ast_function_definition->function_definition_args->named_types[ast_function_definition->function_definition_args->named_size-1] = arg_type;
        if (parser->current_token->type == TOKEN_COMMA) {
            parser_eat(parser, TOKEN_COMMA);
        } else if (parser->current_token->type != TOKEN_RPAREN) {
            err_unexpected_token(parser, -1);
        }
    }
    parser_eat(parser, TOKEN_RPAREN);
    if (parser->current_token->type == TOKEN_COLON) {
        parser_eat(parser, TOKEN_COLON);
        ast_function_definition->function_definition_return_type = parser->current_token->value;
        parser_eat(parser, TOKEN_ID);
    } else {
        ast_function_definition->function_definition_return_type = (void*) 0;
    }
    parser_eat(parser, TOKEN_LBRACE);
    ast_function_definition->function_definition_body = parser_parse_statements(parser);
    parser_eat(parser, TOKEN_RBRACE);
    return ast_function_definition;
}
AST_T* parser_parse_variable(parser_T* parser) {
    char* token_value = parser->current_token->value;
    parser_eat(parser, TOKEN_ID);
    AST_T* ast_variable = init_ast(AST_VARIABLE);
    ast_variable->variable_name = token_value;

    return ast_variable;
}
AST_T* parser_parse_function_call(parser_T* parser, AST_T* function) {
    AST_T* function_call = init_ast(AST_FUNCTION_CALL);
    
    parser_eat(parser, TOKEN_LPAREN);
    if (parser->current_token->type == TOKEN_RPAREN) {
        parser_eat(parser, TOKEN_RPAREN);
    } else {
        while (parser->prev_token->type != TOKEN_RPAREN) {
            if (parser->current_token->type == TOKEN_ID) {
                parser_eat(parser, TOKEN_ID);
                if (parser->current_token->type == TOKEN_COLON) {
                    parser_go_back(parser);
                    break;
                } else {
                    parser_go_back(parser);
                }
            }
            AST_T* ast_expr = parser_parse_expr(parser);
            function_call->function_call_unnamed_size += 1;
            function_call->function_call_unnamed_values = realloc(function_call->function_call_unnamed_values, function_call->function_call_unnamed_size * sizeof(AST_T*));
            function_call->function_call_unnamed_values[function_call->function_call_unnamed_size-1] = ast_expr;

            if (parser->current_token->type == TOKEN_COMMA) {
                parser_eat(parser, TOKEN_COMMA);
            } else if (parser->current_token->type == TOKEN_RPAREN) {
                parser_eat(parser, TOKEN_RPAREN);
            } else {
                err_unexpected_token(parser, -1);
            }
        }
        while (parser->prev_token->type != TOKEN_RPAREN) {
            parser_eat(parser, TOKEN_ID);
            function_call->function_call_named_size++;
            function_call->function_call_named_names = realloc(function_call->function_call_named_names, function_call->function_call_named_size * sizeof(char*));
            function_call->function_call_named_names[function_call->function_call_named_size-1] = parser->current_token->value;
            parser_eat(parser, TOKEN_COLON);
            AST_T* ast_expr = parser_parse_expr(parser);
            function_call->function_call_named_values = realloc(function_call->function_call_named_values, function_call->function_call_named_size * sizeof(char*));
            function_call->function_call_named_values[function_call->function_call_named_size-1] = ast_expr;

            if (parser->current_token->type == TOKEN_COMMA) {
                parser_eat(parser, TOKEN_COMMA);
            } else if (parser->current_token->type == TOKEN_RPAREN) {
                parser_eat(parser, TOKEN_RPAREN);
            } else {
                err_unexpected_token(parser, -1);
            }
        }
    }
    if (ast_expr_level(function) <= EXPR_SINGLE_THING) {
        function_call->function_call_function = function;
        return function_call;
    } else {
        AST_T* last_function = function;
        while (1) {
            if (ast_expr_level(last_function->right_hand) <= EXPR_SINGLE_THING) {
                function_call->function_call_function = last_function->right_hand;
                last_function->right_hand = function_call;
                break;
            } else {
                last_function = last_function->right_hand;
            }
        }
        return function;
    }
}

AST_T* parser_parse_string(parser_T* parser) {
    AST_T* ast_string = ast_create_string(parser->current_token->value);

    parser_eat(parser, TOKEN_STRING);

    return ast_string;
}
AST_T* parser_parse_int(parser_T* parser) {
    AST_T* ast_int = init_ast(AST_INT);

    int int_value = 0;
    size_t len = strlen(parser->current_token->value);
    for (int i = 0; i < len; i++) {
        int_value += (parser->current_token->value[i]-48) * powi(10, len-i-1);
    }
    ast_int->int_value = int_value;
    parser_eat(parser, TOKEN_INT);
    return ast_int;
}
AST_T* parser_parse_float(parser_T* parser) {
    AST_T* ast_float = init_ast(AST_FLOAT);
    float float_value = 0;
    size_t len = strlen(parser->current_token->value);
    int dotpos = -1;
    for (size_t i = 0; i < len; i++) {
        if (parser->current_token->value[i]==46) {
            dotpos = (int) i;
        } else {
            if (dotpos<0) {
                float_value *= 10;
                float_value += parser->current_token->value[i] - 48;
            } else {
                float_value += (parser->current_token->value[i]-48) * powf(10, (float)(dotpos-1));
            }
        }
    }
    ast_float->float_value = float_value;
    parser_eat(parser, TOKEN_FLOAT);
    return ast_float;
}
AST_T* parser_parse_double(parser_T* parser) {
    AST_T* ast_double = init_ast(AST_DOUBLE);
    double double_value = 0;
    size_t len = strlen(parser->current_token->value);
    int dotpos = -1;
    for (size_t i = 0; i < len; i++) {
        if (parser->current_token->value[i]==46) {
            dotpos = (int) i;
        } else {
            if (dotpos<0) {
                double_value *= 10;
                double_value += parser->current_token->value[i] - 48;
            } else {
                double_value += (parser->current_token->value[i]-48) * powf(10, i*-1.0f+dotpos*1.0f);
            }
        }
    }
    ast_double->double_value = double_value;
    parser_eat(parser, TOKEN_DOUBLE);
    return ast_double;
}

AST_T* parser_parse_statements(parser_T* parser) {
    AST_T* compound = init_ast(AST_COMPOUND);

    while (parser->current_token->type != TOKEN_EOF && parser->current_token->type != TOKEN_RBRACE) {
        AST_T* ast_statement = parser_parse_statement(parser);
        compound->compound_size += 1;
        compound->compound_value = realloc(compound->compound_value, compound->compound_size * sizeof(struct AST_STRUCT*));
        compound->compound_value[compound->compound_size-1] = ast_statement;
        if (parser->current_token->type == TOKEN_SEMI) {
            parser_eat(parser, TOKEN_SEMI);
        } else if (parser->prev_token->type != TOKEN_RBRACE && !parser->lexer->newline) {
            err_unexpected_token(parser, TOKEN_SEMI);
        }
    }
    return compound;
}

AST_T* parser_parse_binop(parser_T* parser, AST_T* left_hand, int op_type) {
    AST_T* ast_binop = init_ast(AST_BINOP);
    ast_binop->binop_type = op_type;
    parser_eat(parser, parser->current_token->type); // Just advance
    switch (parser->current_token->type) {
        case TOKEN_ID: ast_binop->right_hand = parser_parse_id(parser); break;
        case TOKEN_STRING: ast_binop->right_hand = parser_parse_string(parser); break;
        case TOKEN_INT: ast_binop->right_hand = parser_parse_int(parser); break;
        case TOKEN_FLOAT: ast_binop->right_hand = parser_parse_float(parser); break;
        case TOKEN_DOUBLE: ast_binop->right_hand = parser_parse_double(parser); break;
        case TOKEN_LPAREN: ast_binop->right_hand = parser_parse_expr(parser); break;
        default: err_unexpected_token(parser, -1); break;
    }
    int op_lvl = ast_binop_level(op_type);
    if (ast_expr_level(left_hand) < op_lvl) {
        ast_binop->left_hand = left_hand;
        return ast_binop;
    } else {
        AST_T* last_hand = left_hand;
        while (1) {
            if (ast_expr_level(last_hand->right_hand) < op_lvl) {
                ast_binop->left_hand = last_hand->right_hand;
                last_hand->right_hand = ast_binop;
                break;
            } else {
                last_hand = last_hand->right_hand;
            }
        }
        return left_hand;
    }
}
AST_T* parser_parse_not(parser_T* parser) {
    AST_T* ast_not = init_ast(AST_NOT);
    parser_eat(parser, TOKEN_EXMARK);
    ast_not->not_value = parser_parse_expr(parser);
    return ast_not;
}


AST_T* parser_parse_if(parser_T* parser) {
    AST_T* ast_if = init_ast(AST_IF);
    AST_T* last_statement = ast_if;
    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LPAREN);
    ast_if->if_condition = parser_parse_expr(parser);
    parser_eat(parser, TOKEN_RPAREN);
    parser_eat(parser, TOKEN_LBRACE);
    ast_if->if_body = parser_parse_statements(parser);
    parser_eat(parser, TOKEN_RBRACE);
    while (parser->current_token->type == TOKEN_ID && utils_strcmp(parser->current_token->value, "else")) {
        parser_eat(parser, TOKEN_ID);
        if (parser->current_token->type==TOKEN_LBRACE) {
            AST_T* ast_else = init_ast(AST_ELSE);
            parser_eat(parser, TOKEN_LBRACE);
            ast_else->else_body = parser_parse_statements(parser);
            parser_eat(parser, TOKEN_RBRACE);
            last_statement->next_statement = ast_else;
        } else if (parser->current_token->type == TOKEN_ID && utils_strcmp(parser->current_token->value, "if")) {
            AST_T* ast_elif = init_ast(AST_ELIF);
            parser_eat(parser, TOKEN_ID);
            parser_eat(parser, TOKEN_LPAREN);
            ast_elif->elif_condition = parser_parse_expr(parser);
            parser_eat(parser, TOKEN_RPAREN);
            parser_eat(parser, TOKEN_LBRACE);
            ast_elif->elif_body = parser_parse_statements(parser);
            parser_eat(parser, TOKEN_RBRACE);
            last_statement->next_statement = ast_elif;
            last_statement = ast_elif;
        } else {
            err_unexpected_token(parser, -1);
        }
    }
    return ast_if;
}

AST_T* parser_parse_for(parser_T* parser) {
    AST_T* ast_for = init_ast(AST_FOR);
    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LPAREN);
    ast_for->for_initializer = parser_parse_statement(parser);
    parser_eat(parser, TOKEN_SEMI);
    ast_for->for_condition = parser_parse_expr(parser);
    parser_eat(parser, TOKEN_SEMI);
    ast_for->for_iterator = parser_parse_statement(parser);
    parser_eat(parser, TOKEN_RPAREN);
    parser_eat(parser, TOKEN_LBRACE);
    ast_for->for_body = parser_parse_statements(parser);
    parser_eat(parser, TOKEN_RBRACE);
    return ast_for;
}
AST_T* parser_parse_while(parser_T* parser) {
    AST_T* ast_while = init_ast(AST_WHILE);
    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LPAREN);
    ast_while->while_condition = parser_parse_expr(parser);
    parser_eat(parser, TOKEN_RPAREN);
    parser_eat(parser, TOKEN_LBRACE);
    ast_while->while_body = parser_parse_statements(parser);
    parser_eat(parser, TOKEN_RBRACE);
    return ast_while;
}

AST_T* parser_parse_return(parser_T* parser) {
    parser_eat(parser, TOKEN_ID); // return
    AST_T* ast_return = init_ast(AST_RETURN);
    ast_return->return_value = parser_parse_expr(parser);
    return ast_return;
}

AST_T* parser_parse_header(parser_T* parser, int headers_type) {
    if (headers_type == HEADER_NORMAL) {
        if (utils_strcmp(parser->current_token->value, "fun")) {
            return parser_parse_header_normal_function(parser);
        } else {
            err_unexpected_token(parser, TOKEN_ID);
            return (void*) 0;
        }
    } else {
        err_not_implemented("Non-normal header parsing\n");
        return (void*) 0;
    }
}
AST_T* parser_parse_header_normal_function(parser_T* parser) {
    AST_T* ast_header = init_ast(AST_HEADER);
    parser_eat(parser, TOKEN_ID);
    ast_header->header_function_name = parser->current_token->value;
    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LPAREN);
    while (parser->current_token->type != TOKEN_RPAREN) {
        if (parser->current_token->type != TOKEN_UNDERSCORE) {
            break;
        }
        parser_eat(parser, TOKEN_UNDERSCORE);
        char* arg_name = parser->current_token->value;
        parser_eat(parser, TOKEN_ID);
        parser_eat(parser, TOKEN_COLON);
        char* arg_type = parser->current_token->value;
        parser_eat(parser, TOKEN_ID);
        ast_header->header_function_args->unnamed_size += 1;
        ast_header->header_function_args->unnamed_names = realloc(ast_header->header_function_args->unnamed_names, ast_header->header_function_args->unnamed_size * sizeof(char*));
        ast_header->header_function_args->unnamed_types = realloc(ast_header->header_function_args->unnamed_types, ast_header->header_function_args->unnamed_size * sizeof(char*));
        ast_header->header_function_args->unnamed_names[ast_header->header_function_args->unnamed_size-1] = arg_name;
        ast_header->header_function_args->unnamed_types[ast_header->header_function_args->unnamed_size-1] = arg_type;
        if (parser->current_token->type == TOKEN_COMMA) {
            parser_eat(parser, TOKEN_COMMA);
        } else if (parser->current_token->type != TOKEN_RPAREN) {
            err_unexpected_token(parser, -1);
        }
    }
    while (parser->current_token->type != TOKEN_RPAREN) {
        char* public_name = parser->current_token->value;
        char* inside_name;
        parser_eat(parser, TOKEN_ID);
        if (parser->current_token->type == TOKEN_ID) {
            inside_name = parser->current_token->value;
            parser_eat(parser, TOKEN_ID);
        } else {
            inside_name = public_name;
        }
        parser_eat(parser, TOKEN_COLON);
        char* arg_type = parser->current_token->value;
        parser_eat(parser, TOKEN_ID);
        ast_header->header_function_args->named_size += 1;
        ast_header->header_function_args->named_inside_names = realloc(ast_header->header_function_args->named_inside_names, ast_header->header_function_args->named_size * sizeof(char*));
        ast_header->header_function_args->named_public_names = realloc(ast_header->header_function_args->named_public_names, ast_header->header_function_args->named_size * sizeof(char*));
        ast_header->header_function_args->named_types = realloc(ast_header->header_function_args->named_types, ast_header->header_function_args->named_size * sizeof(char*));
        ast_header->header_function_args->named_inside_names[ast_header->header_function_args->named_size-1] = inside_name;
        ast_header->header_function_args->named_public_names[ast_header->header_function_args->named_size-1] = public_name;
        ast_header->header_function_args->named_types[ast_header->header_function_args->named_size-1] = arg_type;
        if (parser->current_token->type == TOKEN_COMMA) {
            parser_eat(parser, TOKEN_COMMA);
        } else if (parser->current_token->type != TOKEN_RPAREN) {
            err_unexpected_token(parser, -1);
        }
    }
    parser_eat(parser, TOKEN_RPAREN);
    if (parser->current_token->type == TOKEN_COLON) {
        parser_eat(parser, TOKEN_COLON);
        ast_header->header_function_return_type = parser->current_token->value;
        parser_eat(parser, TOKEN_ID);
    }
    return ast_header;
}
AST_T* parser_parse_headers(parser_T* parser, int headers_type) {
    AST_T* headers = init_ast(AST_HEADER_LIST);
    while (parser->current_token->type != TOKEN_EOF) {
        AST_T* ast_header = parser_parse_header(parser, headers_type);
        headers->headers_size += 1;
        headers->headers_value = realloc(headers->headers_value, headers->headers_size * sizeof(AST_T*));
        headers->headers_value[headers->headers_size-1] = ast_header;
    }
    return headers;
}