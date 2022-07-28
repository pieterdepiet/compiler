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
            list_add(&parser->all_tokens, &parser->all_tokens_size, lexer_get_next_token(parser->lexer));
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
    return parser_parse_expr(parser);
}

AST_T* parser_parse_id(parser_T* parser) {
    if (utils_strcmp(parser->current_token->value, "var")) {
        return parser_parse_variable_definition(parser);
    } else if (utils_strcmp(parser->current_token->value, "fun")) {
        return parser_parse_function_definition(parser);
    } else if (utils_strcmp(parser->current_token->value, "return")) {
        return parser_parse_return(parser);
    } else if (utils_strcmp(parser->current_token->value, "class")) {
        return parser_parse_class_definition(parser);
    } else if (utils_strcmp(parser->current_token->value, "new")) {
        return parser_parse_new(parser);
    } else if (utils_strcmp(parser->current_token->value, "if")) {
        return parser_parse_if(parser);
    } else {
        return parser_parse_variable(parser);
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
    AST_T* left_hand = parser_parse_single_thing(parser);
    while (parser->current_token->type != TOKEN_RPAREN && parser->current_token->type != TOKEN_SEMI && parser->current_token->type != TOKEN_COMMA && parser->current_token->type != TOKEN_RBRACE && parser->current_token->type != TOKEN_RBRACKET) {
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
            case TOKEN_DOT: left_hand = parser_parse_member(parser, left_hand); break;
            case TOKEN_LBRACKET: left_hand = parser_parse_index(parser, left_hand); break;
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
AST_T* parser_parse_single_thing(parser_T* parser) {
    switch (parser->current_token->type) {
        case TOKEN_ID: return parser_parse_id(parser); break;
        case TOKEN_INT: return parser_parse_int(parser); break;
        case TOKEN_LPAREN: return parser_parse_lparen(parser); break;
        case TOKEN_STRING: return parser_parse_string(parser); break;
        case TOKEN_FLOAT: return parser_parse_float(parser); break;
        case TOKEN_DOUBLE: return parser_parse_double(parser); break;
        case TOKEN_EXMARK: case TOKEN_MINUS: return parser_parse_unop(parser); break;
        default: err_unexpected_token(parser, -1); return (void*) 0; break;
    }
}
AST_T* parser_parse_lparen(parser_T* parser) {
    parser_eat(parser, TOKEN_LPAREN);
    AST_T* ast_expr = parser_parse_expr(parser);
    parser_eat(parser, TOKEN_RPAREN);
    return ast_expr;
}

AST_T* parser_parse_variable_definition(parser_T* parser) {
    AST_T* ast_variable_definition = init_ast(AST_VARIABLE_DEFINITION, parser);
    parser_eat(parser, TOKEN_ID); // var
    ast_variable_definition->variable_definition_name = parser->current_token->value;
    parser_eat(parser, TOKEN_ID); // varname
    parser_eat(parser, TOKEN_COLON);
    if (parser->current_token->type == TOKEN_ID && utils_strcmp(parser->current_token->value, "new")) {
        AST_T* ast_new = init_ast(AST_NEW, parser);
        parser_eat(parser, TOKEN_ID); // "new"
        AST_T* ast_type = parser_parse_type(parser);
        ast_new->new_function_call = parser_parse_function_call(parser, ast_type);
        ast_variable_definition->variable_definition_value = ast_new;
        return ast_variable_definition;
    }
    ast_variable_definition->variable_definition_type = parser_parse_type(parser);
    if (parser->current_token->type == TOKEN_EQUALS) {
        parser_eat(parser, TOKEN_EQUALS);
        ast_variable_definition->variable_definition_value = parser_parse_expr(parser);
    } else if (parser->current_token->type == TOKEN_SEMI || parser->lexer->newline) {
        ast_variable_definition->variable_definition_value = (void*) 0;
    } else if (parser->current_token->type == TOKEN_LPAREN) {
        // type_variable->variable_name = ast_variable_definition->variable_definition_type;
        // ast_variable_definition->variable_definition_type = (void*) 0;
        // ast_variable_definition->variable_definition_value = parser_parse_function_call(parser, type_variable);
        ast_variable_definition->variable_definition_value = parser_parse_function_call(parser, ast_variable_definition->variable_definition_type);
        ast_variable_definition->variable_definition_type = (void*) 0;
    } else {
        err_unexpected_token(parser, TOKEN_EQUALS);
    }
    return ast_variable_definition;
}
AST_T* parser_parse_variable_assignment(parser_T* parser, AST_T* variable) {
    AST_T* ast_variable_assignment = init_ast(AST_BINOP, parser);
    ast_variable_assignment->binop_type = BINOP_ASSIGN;
    int assignment_type = 0;
    switch (parser->current_token->type) {
        case TOKEN_EQUALS: parser_eat(parser, TOKEN_EQUALS); assignment_type = -1; break;
        case TOKEN_PLUS_EQUALS: parser_eat(parser, TOKEN_PLUS_EQUALS); assignment_type = BINOP_PLUS; break;
        case TOKEN_MINUS_EQUALS: parser_eat(parser, TOKEN_MINUS_EQUALS); assignment_type = BINOP_MINUS; break;
        case TOKEN_TIMES_EQUALS: parser_eat(parser, TOKEN_TIMES_EQUALS); assignment_type = BINOP_TIMES; break;
        case TOKEN_SLASH_EQUALS: parser_eat(parser, TOKEN_SLASH_EQUALS); assignment_type = BINOP_DIV; break;
        default: err_unexpected_token(parser, -1); break;
    }
    if (assignment_type < 0) {
        ast_variable_assignment->right_hand = parser_parse_single_thing(parser);
    } else {
        ast_variable_assignment->right_hand = init_ast(AST_BINOP, parser);
        ast_variable_assignment->right_hand->binop_type = assignment_type;
        ast_variable_assignment->right_hand->right_hand = parser_parse_single_thing(parser);
    }
    if (ast_expr_level(variable) <= EXPR_INDEX) {
        ast_variable_assignment->left_hand = variable;
        if (assignment_type >= 0) {
            ast_variable_assignment->right_hand->left_hand = variable;
        }
        return ast_variable_assignment;
    } else {
        AST_T* last_variable = variable;
        for (;;) {
            if (ast_expr_level(last_variable->right_hand) <= EXPR_INDEX) {
                ast_variable_assignment->left_hand = last_variable->right_hand;
                last_variable->right_hand = ast_variable_assignment;
                if (assignment_type >= 0) {
                    ast_variable_assignment->right_hand->left_hand = last_variable->right_hand;
                }
                return variable;
            } else {
                last_variable = last_variable->right_hand;
            }
        }
    }
}
AST_T* parser_parse_function_definition(parser_T* parser) {
    AST_T* ast_function_definition = init_ast(AST_FUNCTION_DEFINITION, parser);
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
        AST_T* arg_type = parser_parse_type(parser);
        list_add(&ast_function_definition->function_definition_args->unnamed_types, &ast_function_definition->function_definition_args->unnamed_size, arg_type);
        ast_function_definition->function_definition_args->unnamed_size -= 1;
        list_add(&ast_function_definition->function_definition_args->unnamed_names, &ast_function_definition->function_definition_args->unnamed_size, arg_name);
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
        AST_T* arg_type = parser_parse_type(parser);
        list_add(&ast_function_definition->function_definition_args->named_types, &ast_function_definition->function_definition_args->named_size, arg_type);
        ast_function_definition->function_definition_args->named_size -= 1;        
        list_add(&ast_function_definition->function_definition_args->named_public_names, &ast_function_definition->function_definition_args->named_size, public_name);
        ast_function_definition->function_definition_args->named_size -= 1;        
        list_add(&ast_function_definition->function_definition_args->named_inside_names, &ast_function_definition->function_definition_args->named_size, inside_name);
        if (parser->current_token->type == TOKEN_COMMA) {
            parser_eat(parser, TOKEN_COMMA);
        } else if (parser->current_token->type != TOKEN_RPAREN) {
            err_unexpected_token(parser, -1);
        }
    }
    parser_eat(parser, TOKEN_RPAREN);
    if (parser->current_token->type == TOKEN_COLON) {
        parser_eat(parser, TOKEN_COLON);
        ast_function_definition->function_definition_return_type = parser_parse_type(parser);
    } else {
        ast_function_definition->function_definition_return_type = (void*) 0;
    }
    if (parser->current_token->type == TOKEN_LBRACE) {
        parser_eat(parser, TOKEN_LBRACE);
        ast_function_definition->function_definition_body = parser_parse_statements(parser);
        parser_eat(parser, TOKEN_RBRACE);
        return ast_function_definition;
    } else if (parser->current_token->type == TOKEN_SEMI) {
        ast_function_definition->function_definition_body = (void*) 0;
        return ast_function_definition;
    } else if (parser->current_token->type == TOKEN_STRING) {
        ast_function_definition->function_definition_body = init_ast(AST_STRING, parser);
        ast_function_definition->function_definition_body->string_value = malloc(strlen(parser->current_token->value) + 1);
        strcpy(ast_function_definition->function_definition_body->string_value, parser->current_token->value);
        parser_eat(parser, TOKEN_STRING);
        return ast_function_definition;
    } else {
        err_unexpected_token(parser, TOKEN_LBRACE);
        return (void*) 0;
    }
}
AST_T* parser_parse_variable(parser_T* parser) {
    char* token_value = parser->current_token->value;
    AST_T* ast_variable = init_ast(AST_VARIABLE, parser);
    ast_variable->variable_name = token_value;
    parser_eat(parser, TOKEN_ID);

    return ast_variable;
}
AST_T* parser_parse_function_call(parser_T* parser, AST_T* function) {
    AST_T* function_call = init_ast(AST_FUNCTION_CALL, parser);
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
            list_add(&function_call->function_call_unnamed_values, &function_call->function_call_unnamed_size, ast_expr);

            if (parser->current_token->type == TOKEN_COMMA) {
                parser_eat(parser, TOKEN_COMMA);
            } else if (parser->current_token->type == TOKEN_RPAREN) {
                parser_eat(parser, TOKEN_RPAREN);
            } else {
                err_unexpected_token(parser, -1);
            }
        }
        while (parser->prev_token->type != TOKEN_RPAREN) {
            list_add(&function_call->function_call_named_names, &function_call->function_call_named_size, parser->current_token->value);
            function_call->function_call_named_size--;
            parser_eat(parser, TOKEN_ID);
            parser_eat(parser, TOKEN_COLON);
            AST_T* ast_expr = parser_parse_expr(parser);
            list_add(&function_call->function_call_named_values, &function_call->function_call_named_size, ast_expr);

            if (parser->current_token->type == TOKEN_COMMA) {
                parser_eat(parser, TOKEN_COMMA);
            } else if (parser->current_token->type == TOKEN_RPAREN) {
                parser_eat(parser, TOKEN_RPAREN);
            } else {
                err_unexpected_token(parser, -1);
            }
        }
    }
    if (ast_expr_level(function) <= EXPR_TYPE) {
        function_call->function_call_function = function;
        return function_call;
    } else {
        AST_T* last_function = function;
        for (;;) {
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
    AST_T* ast_string = init_ast(AST_STRING, parser);
    ast_string->string_value = parser->current_token->value;

    parser_eat(parser, TOKEN_STRING);

    return ast_string;
}
AST_T* parser_parse_int(parser_T* parser) {
    AST_T* ast_int = init_ast(AST_INT, parser);

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
    AST_T* ast_float = init_ast(AST_FLOAT, parser);
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
    AST_T* ast_double = init_ast(AST_DOUBLE, parser);
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
    AST_T* compound = init_ast(AST_COMPOUND, parser);

    while (parser->current_token->type != TOKEN_EOF && parser->current_token->type != TOKEN_RBRACE) {
        AST_T* ast_statement = parser_parse_statement(parser);
        list_add(&compound->compound_value, &compound->compound_size, ast_statement);
        if (parser->current_token->type == TOKEN_SEMI) {
            parser_eat(parser, TOKEN_SEMI);
        } else if (parser->prev_token->type != TOKEN_RBRACE && !parser->lexer->newline) {
            err_unexpected_token(parser, TOKEN_SEMI);
        }
    }
    return compound;
}

AST_T* parser_parse_binop(parser_T* parser, AST_T* left_hand, int op_type) {
    AST_T* ast_binop = init_ast(AST_BINOP, parser);
    ast_binop->binop_type = op_type;
    parser_eat(parser, parser->current_token->type); // Just advance
    ast_binop->right_hand = parser_parse_single_thing(parser);
    int op_lvl = ast_binop_level(op_type);
    if (ast_expr_level(left_hand) <= op_lvl) {
        ast_binop->left_hand = left_hand;
        return ast_binop;
    } else {
        AST_T* last_hand = left_hand;
        for (;;) {
            if (ast_expr_level(last_hand->right_hand) < op_lvl) {
                ast_binop->left_hand = last_hand->right_hand;
                last_hand->right_hand = ast_binop;
                return left_hand;
            } else {
                last_hand = last_hand->right_hand;
            }
        }
    }
}
AST_T* parser_parse_member(parser_T* parser, AST_T* left_hand) {
    parser_eat(parser, TOKEN_DOT);
    AST_T* ast_member = init_ast(AST_MEMBER, parser);
    ast_member->member_name = parser->current_token->value;
    parser_eat(parser, TOKEN_ID);
    if (ast_expr_level(left_hand) <= EXPR_MEMBER) {
        ast_member->member_parent = left_hand;
        return ast_member;
    } else {
        AST_T* last_hand = left_hand;
        for (;;) {
            if (ast_expr_level(last_hand->right_hand) <= EXPR_MEMBER) {
                ast_member->member_parent = last_hand->right_hand;
                last_hand->right_hand = ast_member;
                return left_hand;
            } else {
                last_hand = last_hand->right_hand;
            }
        }
    }
}
AST_T* parser_parse_index(parser_T* parser, AST_T* left_hand) {
    AST_T* ast_index = init_ast(AST_BINOP, parser);
    ast_index->binop_type = BINOP_INDEX;
    parser_eat(parser, TOKEN_LBRACKET);
    ast_index->right_hand = parser_parse_expr(parser);
    parser_eat(parser, TOKEN_RBRACKET);
    if (ast_expr_level(left_hand) <= EXPR_INDEX) {
        ast_index->left_hand = left_hand;
        return ast_index;
    } else {
        AST_T* last_hand = left_hand;
        for (;;) {
            if (ast_expr_level(last_hand->right_hand) <= EXPR_INDEX) {
                ast_index->left_hand = last_hand->right_hand;
                last_hand->right_hand = ast_index;
                return left_hand;
            } else {
                last_hand = last_hand->right_hand;
            }
        }
    }
}
AST_T* parser_parse_unop(parser_T* parser) {
    AST_T* ast_unop = init_ast(AST_UNOP, parser);
    if (parser->current_token->type == TOKEN_EXMARK) {
        ast_unop->unop_type = UNOP_NOT;
    } else if (parser->current_token->type == TOKEN_MINUS) {
        ast_unop->unop_type = UNOP_NEG;
    }
    parser_eat(parser, parser->current_token->type);
    ast_unop->unop_value = parser_parse_single_thing(parser);
    return ast_unop;
}


AST_T* parser_parse_if(parser_T* parser) {
    AST_T* ast_if = init_ast(AST_IF, parser);
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
            AST_T* ast_else = init_ast(AST_ELSE, parser);
            parser_eat(parser, TOKEN_LBRACE);
            ast_else->else_body = parser_parse_statements(parser);
            parser_eat(parser, TOKEN_RBRACE);
            last_statement->next_statement = ast_else;
            break;
        } else if (parser->current_token->type == TOKEN_ID && utils_strcmp(parser->current_token->value, "if")) {
            AST_T* ast_elif = init_ast(AST_ELIF, parser);
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
    AST_T* ast_for = init_ast(AST_FOR, parser);
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
    AST_T* ast_while = init_ast(AST_WHILE, parser);
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
    AST_T* ast_return = init_ast(AST_RETURN, parser);
    parser_eat(parser, TOKEN_ID); // return
    if (parser->current_token->type == TOKEN_SEMI) {
        parser_eat(parser, TOKEN_SEMI);
        ast_return->return_value = (void*) 0;
    } else {
        ast_return->return_value = parser_parse_expr(parser);
    }
    return ast_return;
}

// AST_T* parser_parse_header(parser_T* parser, int headers_type) {
//     if (headers_type == HEADER_NORMAL) {
//         if (utils_strcmp(parser->current_token->value, "fun")) {
//             return parser_parse_header_normal_function(parser);
//         } else {
//             err_unexpected_token(parser, TOKEN_ID);
//             return (void*) 0;
//         }
//     } else {
//         err_not_implemented("Non-normal header parsing\n");
//         return (void*) 0;
//     }
// }
// AST_T* parser_parse_header_normal_function(parser_T* parser) {
//     AST_T* ast_header = init_ast(AST_HEADER, parser);
//     parser_eat(parser, TOKEN_ID);
//     ast_header->header_function_name = parser->current_token->value;
//     parser_eat(parser, TOKEN_ID);
//     parser_eat(parser, TOKEN_LPAREN);
//     while (parser->current_token->type != TOKEN_RPAREN) {
//         if (parser->current_token->type != TOKEN_UNDERSCORE) {
//             break;
//         }
//         parser_eat(parser, TOKEN_UNDERSCORE);
//         char* arg_name = parser->current_token->value;
//         parser_eat(parser, TOKEN_ID);
//         parser_eat(parser, TOKEN_COLON);
//         char* arg_type = parser->current_token->value;
//         parser_eat(parser, TOKEN_ID);
//         ast_header->header_function_args->unnamed_size += 1;
//         ast_header->header_function_args->unnamed_names = realloc(ast_header->header_function_args->unnamed_names, ast_header->header_function_args->unnamed_size * sizeof(char*));
//         ast_header->header_function_args->unnamed_types = realloc(ast_header->header_function_args->unnamed_types, ast_header->header_function_args->unnamed_size * sizeof(char*));
//         ast_header->header_function_args->unnamed_names[ast_header->header_function_args->unnamed_size-1] = arg_name;
//         ast_header->header_function_args->unnamed_types[ast_header->header_function_args->unnamed_size-1] = arg_type;
//         if (parser->current_token->type == TOKEN_COMMA) {
//             parser_eat(parser, TOKEN_COMMA);
//         } else if (parser->current_token->type != TOKEN_RPAREN) {
//             err_unexpected_token(parser, -1);
//         }
//     }
//     while (parser->current_token->type != TOKEN_RPAREN) {
//         char* public_name = parser->current_token->value;
//         char* inside_name;
//         parser_eat(parser, TOKEN_ID);
//         if (parser->current_token->type == TOKEN_ID) {
//             inside_name = parser->current_token->value;
//             parser_eat(parser, TOKEN_ID);
//         } else {
//             inside_name = public_name;
//         }
//         parser_eat(parser, TOKEN_COLON);
//         char* arg_type = parser->current_token->value;
//         parser_eat(parser, TOKEN_ID);
//         ast_header->header_function_args->named_size += 1;
//         ast_header->header_function_args->named_inside_names = realloc(ast_header->header_function_args->named_inside_names, ast_header->header_function_args->named_size * sizeof(char*));
//         ast_header->header_function_args->named_public_names = realloc(ast_header->header_function_args->named_public_names, ast_header->header_function_args->named_size * sizeof(char*));
//         ast_header->header_function_args->named_types = realloc(ast_header->header_function_args->named_types, ast_header->header_function_args->named_size * sizeof(char*));
//         ast_header->header_function_args->named_inside_names[ast_header->header_function_args->named_size-1] = inside_name;
//         ast_header->header_function_args->named_public_names[ast_header->header_function_args->named_size-1] = public_name;
//         ast_header->header_function_args->named_types[ast_header->header_function_args->named_size-1] = arg_type;
//         if (parser->current_token->type == TOKEN_COMMA) {
//             parser_eat(parser, TOKEN_COMMA);
//         } else if (parser->current_token->type != TOKEN_RPAREN) {
//             err_unexpected_token(parser, -1);
//         }
//     }
//     parser_eat(parser, TOKEN_RPAREN);
//     if (parser->current_token->type == TOKEN_COLON) {
//         parser_eat(parser, TOKEN_COLON);
//         ast_header->header_function_return_type = parser->current_token->value;
//         parser_eat(parser, TOKEN_ID);
//     }
//     return ast_header;
// }
// AST_T* parser_parse_headers(parser_T* parser, int headers_type) {
//     AST_T* headers = init_ast(AST_HEADER_LIST, parser);
//     while (parser->current_token->type != TOKEN_EOF) {
//         AST_T* ast_header = parser_parse_header(parser, headers_type);
//         headers->headers_size += 1;
//         headers->headers_value = realloc(headers->headers_value, headers->headers_size * sizeof(AST_T*));
//         headers->headers_value[headers->headers_size-1] = ast_header;
//     }
//     return headers;
// }
AST_T* parser_parse_class_definition(parser_T* parser) {
    AST_T* ast_class_definition = init_ast(AST_CLASS_DEFINITION, parser);
    parser_eat(parser, TOKEN_ID); // class
    ast_class_definition->class_name = parser->current_token->value;
    ast_class_definition->class_prototype_names_size = 0;
    parser_eat(parser, TOKEN_ID); // <classname>
    if (parser->current_token->type == TOKEN_COLON) {
        parser_eat(parser, TOKEN_COLON);
        do {
            list_add(&ast_class_definition->class_prototype_names, &ast_class_definition->class_prototype_names_size, parser->current_token->value);
            parser_eat(parser, TOKEN_ID);
            if (parser->current_token->type == TOKEN_COMMA) {
                parser_eat(parser, TOKEN_COMMA);
            } else if (parser->current_token->type == TOKEN_LBRACE) {
                parser_eat(parser, TOKEN_LBRACE);
            } else {
                err_unexpected_token(parser, TOKEN_LBRACE);
            }
        } while (parser->prev_token->type == TOKEN_COMMA);
    } else {
        parser_eat(parser, TOKEN_LBRACE);
    }
    while (parser->current_token->type != TOKEN_RBRACE) {
        if (parser->current_token->type == TOKEN_ID) {
            int is_static = 0;
            int is_private = -1;
            if (utils_strcmp(parser->current_token->value, "static")) {
                is_static = 1;
                parser_eat(parser, TOKEN_ID);
            }
            if (utils_strcmp(parser->current_token->value, "private")) {
                is_private = 1;
                parser_eat(parser, TOKEN_ID);
            } else if (utils_strcmp(parser->current_token->value, "public")) {
                is_private = 0;
                parser_eat(parser, TOKEN_ID);
            }
            if (utils_strcmp(parser->current_token->value, "fun")) {
                AST_T* ast_function_definition = parser_parse_function_definition(parser);
                ast_function_definition->function_definition_is_static = is_static;
                if (is_private == 1) {
                    ast_function_definition->function_definition_is_private = 1;
                } else {
                    ast_function_definition->function_definition_is_private = 0;
                }
                list_add(&ast_class_definition->class_members, &ast_class_definition->class_members_size, ast_function_definition);
            } else if (utils_strcmp(parser->current_token->value, "var")) {
                AST_T* ast_variable_definition = parser_parse_variable_definition(parser);
                ast_variable_definition->variable_definition_is_static = is_static;
                if (is_private == 0) {
                    ast_variable_definition->variable_definition_is_private = 0;
                } else {
                    ast_variable_definition->variable_definition_is_private = 1;
                }
                list_add(&ast_class_definition->class_members, &ast_class_definition->class_members_size, ast_variable_definition);
                if (parser->current_token->type == TOKEN_SEMI) {
                    parser_eat(parser, TOKEN_SEMI);
                } else if (parser->lexer->newline == 0) {
                    err_unexpected_token(parser, TOKEN_SEMI);
                }
            } else if (utils_strcmp(parser->current_token->value, "constructor")) {
                AST_T* ast_constructor = init_ast(AST_CONSTRUCTOR, parser);
                ast_constructor->constructor_args = init_ast_arglist();
                parser_eat(parser, TOKEN_ID); // constructor
                parser_eat(parser, TOKEN_LPAREN);
                while (parser->current_token->type != TOKEN_RPAREN) {
                    if (parser->current_token->type != TOKEN_UNDERSCORE) {
                        break;
                    }
                    parser_eat(parser, TOKEN_UNDERSCORE);
                    char* arg_name = parser->current_token->value;
                    parser_eat(parser, TOKEN_ID);
                    parser_eat(parser, TOKEN_COLON);
                    AST_T* arg_type = parser_parse_type(parser);
                    list_add(&ast_constructor->constructor_args->unnamed_types, &ast_constructor->constructor_args->unnamed_size, arg_type);
                    ast_constructor->constructor_args->unnamed_size -= 1;
                    list_add(&ast_constructor->constructor_args->unnamed_names, &ast_constructor->constructor_args->unnamed_size, arg_name);
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
                    AST_T* arg_type = parser_parse_type(parser);
                    list_add(&ast_constructor->constructor_args->named_types, &ast_constructor->constructor_args->named_size, arg_type);
                    ast_constructor->constructor_args->named_size -= 1;        
                    list_add(&ast_constructor->constructor_args->named_public_names, &ast_constructor->constructor_args->named_size, public_name);
                    ast_constructor->constructor_args->named_size -= 1;        
                    list_add(&ast_constructor->constructor_args->named_inside_names, &ast_constructor->constructor_args->named_size, inside_name);
                    if (parser->current_token->type == TOKEN_COMMA) {
                        parser_eat(parser, TOKEN_COMMA);
                    } else if (parser->current_token->type != TOKEN_RPAREN) {
                        err_unexpected_token(parser, -1);
                    }
                }
                parser_eat(parser, TOKEN_RPAREN);
                parser_eat(parser, TOKEN_LBRACE);
                ast_constructor->constructor_function_body = parser_parse_statements(parser);
                parser_eat(parser, TOKEN_RBRACE);
                list_add(&ast_class_definition->class_members, &ast_class_definition->class_members_size, ast_constructor);
            } else {
                err_unexpected_token(parser, TOKEN_ID);
            }
        } else {
            err_unexpected_token(parser, TOKEN_ID);
        }
    }
    parser_eat(parser, TOKEN_RBRACE);
    return ast_class_definition;
}

AST_T* parser_parse_type(parser_T* parser) {
    AST_T* ast_type = init_ast(AST_TYPE, parser);
    ast_type->type_base = parser->current_token->value;
    parser_eat(parser, TOKEN_ID);
    if (parser->current_token->type == TOKEN_LBRACKET) {
        while (parser->current_token->type == TOKEN_LBRACKET) {
            parser_eat(parser, TOKEN_LBRACKET);
            ast_type->type_array_sizes = realloc(ast_type->type_array_sizes, ++ast_type->type_array_count * sizeof(struct AST_STRUCT));
            if (parser->current_token->type != TOKEN_RBRACKET) {
                ast_type->type_array_sizes[ast_type->type_array_count-1] = parser_parse_expr(parser);
            } else {
                ast_type->type_array_sizes[ast_type->type_array_count-1] = (void*) 0;
            }
            parser_eat(parser, TOKEN_RBRACKET);
        }
    }
    ast_type->type_fullname = calloc(1, (strlen(ast_type->type_base) + ast_type->type_array_count * 2 + 1) * sizeof(char));
    strcpy(ast_type->type_fullname, ast_type->type_base);
    for (size_t i = 0; i < ast_type->type_array_count; i++) {
        strcat(ast_type->type_fullname, "[]");
    }
    return ast_type;
}

AST_T* parser_parse_new(parser_T* parser) {
    AST_T* ast_new = init_ast(AST_NEW, parser);
    parser_eat(parser, TOKEN_ID); // new
    ast_new->new_function_call = parser_parse_expr(parser);
    return ast_new;
}