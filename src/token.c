#include "include/token.h"
#include <stdlib.h>


token_T* init_token(int type, char* value) {
    token_T* token = calloc(1, sizeof(struct TOKEN_STRUCT));
    token->type = type;
    token->value = value;

    return token;
}

char* token_type_string(enum token_type type) {
    switch (type) {
        case TOKEN_ID: return "id"; break;
        case TOKEN_EQUALS: return "equals"; break;
        case TOKEN_PLUSPLUS: return "plusplus"; break;
        case TOKEN_MINUSMINUS: return "minusminus"; break;
        case TOKEN_SEMI: return "semicolon"; break;
        case TOKEN_COLON: return "colon"; break;
        case TOKEN_PLUS: return "plus"; break;
        case TOKEN_MINUS: return "minus"; break;
        case TOKEN_TIMES: return "times"; break;
        case TOKEN_SLASH: return "slash"; break;
        case TOKEN_PLUS_EQUALS: return "plusequals"; break;
        case TOKEN_MINUS_EQUALS: return "minusequals"; break;
        case TOKEN_TIMES_EQUALS: return "timesequals"; break;
        case TOKEN_SLASH_EQUALS: return "slashequals"; break;
        case TOKEN_EQEQ: return "equal"; break;
        case TOKEN_NEQ: return "nonequal"; break;
        case TOKEN_GRT: return "greater than"; break;
        case TOKEN_LET: return "less than"; break;
        case TOKEN_GREQ: return "greater than or equal to"; break;
        case TOKEN_LEEQ: return "less than or equal to"; break;
        case TOKEN_EXMARK: return "not"; break;
        case TOKEN_QMARK: return "question mark"; break;
        case TOKEN_ANDAND: return "double and"; break;
        case TOKEN_OROR: return "double or"; break;
        case TOKEN_LPAREN: return "left parenthesis"; break;
        case TOKEN_RPAREN: return "right parenthesis"; break;
        case TOKEN_LBRACE: return "left brace"; break;
        case TOKEN_RBRACE: return "right brace"; break;
        case TOKEN_LBRACKET: return "left bracket"; break;
        case TOKEN_RBRACKET: return "right bracket"; break;
        case TOKEN_COMMA: return "comma"; break;
        case TOKEN_DOT: return "dot"; break;
        case TOKEN_STRING: return "string"; break;
        case TOKEN_INT: return "int"; break;
        case TOKEN_FLOAT: return "float"; break;
        case TOKEN_DOUBLE: return "double"; break;
        case TOKEN_EOF: return "end of file"; break;
        case TOKEN_COMMENT: return "comment"; break;
        case TOKEN_UNDERSCORE: return "underscore"; break;
        case TOKEN_RIGHTARROW: return "rightarrow"; break;
    }
}