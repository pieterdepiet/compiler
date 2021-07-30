#ifndef TOKEN_H
#define TOKEN_H
typedef struct TOKEN_STRUCT
{
    enum token_type {
        TOKEN_ID,
        TOKEN_EQUALS,
        TOKEN_PLUSPLUS,
        TOKEN_MINUSMINUS,
        TOKEN_SEMI,
        TOKEN_COLON,
        TOKEN_RIGHTARROW,
        TOKEN_PLUS,
        TOKEN_MINUS,
        TOKEN_TIMES,
        TOKEN_SLASH,
        TOKEN_PLUS_EQUALS,
        TOKEN_MINUS_EQUALS,
        TOKEN_TIMES_EQUALS,
        TOKEN_SLASH_EQUALS,
        TOKEN_EQEQ,
        TOKEN_NEQ,
        TOKEN_GRT,
        TOKEN_LET,
        TOKEN_GREQ,
        TOKEN_LEEQ,
        TOKEN_EXMARK,
        TOKEN_QMARK,
        TOKEN_ANDAND,
        TOKEN_OROR,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LBRACE,
        TOKEN_RBRACE,
        TOKEN_LBRACKET,
        TOKEN_RBRACKET,
        TOKEN_COMMA,
        TOKEN_DOT,
        TOKEN_STRING,
        TOKEN_INT,
        TOKEN_FLOAT,
        TOKEN_DOUBLE,
        TOKEN_EOF,
        TOKEN_COMMENT,
        TOKEN_UNDERSCORE
    } type;

    char* value;
} token_T;

token_T* init_token(int type, char* value);

char* token_type_string(int type);
#endif