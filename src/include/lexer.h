#ifndef LEXER_H
#define LEXER_H
#include "token.h"


typedef struct LEXER_STRUCT
{
    char c;
    unsigned int i;
    char* contents;
    unsigned long lineno;
    unsigned long charno;
    char* empty_string;
    enum {
        NO_NEWLINE,
        NEWLINE
    } newline;
} lexer_T;

lexer_T* init_lexer(char* contents);

void lexer_advance(lexer_T* lexer);

void lexer_go_back(lexer_T* lexer);

void lexer_skip_whitespace(lexer_T* lexer);

token_T* lexer_get_next_token(lexer_T* lexer);

token_T* lexer_collect_string(lexer_T* lexer);

token_T* lexer_collect_id(lexer_T* lexer);

token_T* lexer_collect_number(lexer_T* lexer);

token_T* lexer_read_plus(lexer_T* lexer);
token_T* lexer_read_minus(lexer_T* lexer);
token_T* lexer_read_times(lexer_T* lexer);
token_T* lexer_read_slash(lexer_T* lexer);

token_T* lexer_read_equals(lexer_T* lexer);
token_T* lexer_read_grt(lexer_T* lexer);
token_T* lexer_read_let(lexer_T* lexer);
token_T* lexer_read_not(lexer_T* lexer);
token_T* lexer_read_and(lexer_T* lexer);
token_T* lexer_read_or(lexer_T* lexer);
token_T* lexer_read_underscore(lexer_T* lexer);

token_T* lexer_advance_with_token(lexer_T* lexer, token_T* token);

char* lexer_get_current_char_as_string(lexer_T* lexer);

#endif