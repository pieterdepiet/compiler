#include "include/lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


lexer_T* init_lexer(char* contents) {
    lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));
    lexer->contents = contents;
    lexer->i = 0;
    lexer->c = contents[lexer->i];
    lexer->lineno = 1;
    lexer->charno = 0;
    lexer->empty_string = "";
    lexer->newline = NO_NEWLINE;
    return lexer;
}
static token_T* unexpchar(lexer_T* lexer) {
    printf("Unexpected character '%c' [%lu:%lu]\n", lexer->contents[lexer->i], lexer->lineno, lexer->charno);
    exit(1);
    return init_token(TOKEN_EOF, lexer->empty_string);
}

void lexer_advance(lexer_T* lexer) {
    if (lexer->c != '\0' && lexer->i < strlen(lexer->contents)) {
        if (lexer->c == '\n') {
            lexer->lineno++;
            lexer->charno = 0;
        } else {
            lexer->charno++;
        }
        lexer->i += 1;
        lexer->c = lexer->contents[lexer->i];
    }
}

void lexer_go_back(lexer_T* lexer) {
    if (lexer->i>0) {
        lexer->i -= 1;
        lexer->c = lexer->contents[lexer->i]; 
        if (lexer->c == '\n') {
            lexer->lineno--;
            size_t i;
            if (lexer->i > 0) {
                for (i = lexer->i - 1; i > 0 && lexer->contents[i] != '\n'; i--);
            }
            lexer->charno = lexer->i - i;
        } else {
            lexer->charno--;
        }
    }
}

void lexer_skip_whitespace(lexer_T* lexer) {
    while (lexer->c == ' ' || lexer->c == 10) {
        if (lexer->c == 10) {
            lexer->newline = NEWLINE;
        }
        lexer_advance(lexer);
    }
}

token_T* lexer_get_next_token(lexer_T* lexer) {
    while (lexer->c != '\0' && lexer->i < strlen(lexer->contents)) {
        lexer->newline = NO_NEWLINE;
        if (lexer->c == ' ' || lexer->c == 10)
            lexer_skip_whitespace(lexer);
        if (lexer->c == '/') {
            return lexer_read_slash(lexer);
        }
        if (isalpha(lexer->c)) {
            return lexer_collect_id(lexer);
        }
        if (isdigit(lexer->c))
            return lexer_collect_number(lexer);
        if (lexer->c == '"')
            return lexer_collect_string(lexer);

        switch (lexer->c) {
            case ';': return lexer_advance_with_token(lexer, init_token(TOKEN_SEMI, lexer->empty_string)); break;
            case '(': return lexer_advance_with_token(lexer, init_token(TOKEN_LPAREN, lexer->empty_string)); break;
            case ')': return lexer_advance_with_token(lexer, init_token(TOKEN_RPAREN, lexer->empty_string)); break;
            case '{': return lexer_advance_with_token(lexer, init_token(TOKEN_LBRACE, lexer->empty_string)); break;
            case '}': return lexer_advance_with_token(lexer, init_token(TOKEN_RBRACE, lexer->empty_string)); break;
            case '[': return lexer_advance_with_token(lexer, init_token(TOKEN_LBRACKET, lexer->empty_string)); break;
            case ']': return lexer_advance_with_token(lexer, init_token(TOKEN_RBRACKET, lexer->empty_string)); break;
            case ',': return lexer_advance_with_token(lexer, init_token(TOKEN_COMMA, lexer->empty_string)); break;
            case '.': return lexer_advance_with_token(lexer, init_token(TOKEN_DOT, lexer->empty_string)); break;
            case ':': return lexer_advance_with_token(lexer, init_token(TOKEN_COLON, lexer->empty_string)); break;
            case '?': return lexer_advance_with_token(lexer, init_token(TOKEN_QMARK, lexer->empty_string)); break;
            case '>': return lexer_read_grt(lexer); break;
            case '<': return lexer_read_let(lexer); break;
            case '!': return lexer_read_not(lexer); break;
            case '=': return lexer_read_equals(lexer); break;
            case '+': return lexer_read_plus(lexer); break;
            case '-': return lexer_read_minus(lexer); break;
            case '*': return lexer_read_times(lexer); break;
            case '/': return lexer_read_slash(lexer); break;
            case '&': return lexer_read_and(lexer); break;
            case '|': return lexer_read_or(lexer); break;
            case '_': return lexer_read_underscore(lexer);
            case 0: return init_token(TOKEN_EOF, lexer->empty_string); break;
            default: return unexpchar(lexer); break;
        }
        printf("Unexpected character '%c' %d\n", lexer->contents[lexer->i], lexer->i);
        exit(1);
        return init_token(TOKEN_EOF, lexer->empty_string);
    }
    return init_token(TOKEN_EOF, lexer->empty_string);
}

token_T* lexer_collect_string(lexer_T* lexer) {
    lexer_advance(lexer);
    char* value = calloc(1, sizeof(char));
    value[0] = '\0';
    while (lexer->c != '"') {
        char* s = lexer_get_current_char_as_string(lexer);
        if (lexer->c == 92) {
            lexer_advance(lexer);
            size_t len = strlen(value);
            switch (lexer->c) {
                case 34: case 39: case 92: s = lexer_get_current_char_as_string(lexer); value = realloc(value, (len + 1 + 1)* sizeof(char)); strcat(value, s); break;
                case 97: value = realloc(value, (len + 1 + 1) * sizeof(char)); strcat(value, "\a"); break;
                case 98: value = realloc(value, (len + 1 + 1) * sizeof(char)); strcat(value, "\b"); break;
                case 102: value = realloc(value, (len + 1 + 1) * sizeof(char)); strcat(value, "\f"); break;
                case 110: value = realloc(value, (len + 1 + 1) * sizeof(char)); strcat(value, "\n"); break;
                case 114: value = realloc(value, (len + 1 + 1) * sizeof(char)); strcat(value, "\r"); break;
                case 116: value = realloc(value, (len + 1 + 1) * sizeof(char)); strcat(value, "\t"); break;
                case 118: value = realloc(value, (len + 1 + 1) * sizeof(char)); strcat(value, "\v"); break;
                case 120:
                    value = realloc(value, (len + 1 + 1) * sizeof(char));
                    lexer_advance(lexer);
                    char hex[2];
                    if (isdigit(lexer->c)) {
                        hex[0] = lexer->c - 48;
                    } else if (isupper(lexer->c)) {
                        hex[0] = lexer->c - 55;
                    } else {
                        printf("Error: bad hex escape code\n");
                        exit(1);
                    }
                    lexer_advance(lexer);
                    if (isdigit(lexer->c)) {
                        hex[1] = lexer->c - 48;
                    } else if (isupper(lexer->c)) {
                        hex[1] = lexer->c - 55;
                    }
                    value[len] = '\0';
                    value[len-1] = hex[0] * 16 + hex[1];
                    break;
                default: 
                    if (isdigit(lexer->c)) {
                        value = realloc(value, (len + 1 + 1) * sizeof(char));
                        char oct[3];
                        for (size_t i = 0; i < 3; i++) {
                            if (isdigit(lexer->c)) {
                                oct[i] = lexer->c - 48;
                            } else {
                                printf("Error: bad octal escape code\n");
                                exit(1);
                            }
                            lexer_advance(lexer);
                        }
                        value[len] = '\0';
                        value[len-1] = oct[0] * 64 + oct[1] * 8 + oct[2];
                    } else {
                        printf("Unexpected escape code \\%c\n", lexer->c);
                    }
                    break;
            }
        } else {
            value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
            strcat(value, s);
        }
        lexer_advance(lexer);
    }

    lexer_advance(lexer);

    return init_token(TOKEN_STRING, value);
}

token_T* lexer_collect_id(lexer_T* lexer) {
    char* value = calloc(1, sizeof(char));
    value[0] = '\0';
    while (isalnum(lexer->c)||lexer->c==95) {
        char* s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        lexer_advance(lexer);
    }
    return init_token(TOKEN_ID, value);
}

token_T* lexer_collect_number(lexer_T* lexer) {
    char* value = calloc(1, sizeof(char));
    value[0] = '\0';

    while (isdigit(lexer->c)) {
        char* s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        lexer_advance(lexer);
    }
    if (lexer->c == 46) {

        char* dot = lexer_get_current_char_as_string(lexer);


        char* decimals = calloc(1, sizeof(char));
        decimals[0] = '\0';
        strcat(decimals, dot);
        free(dot);
        size_t i;
        for (i = 0; i < 15; i++) {
            lexer_advance(lexer);
            if (isdigit(lexer->c)) {
                char* s = lexer_get_current_char_as_string(lexer);
                decimals = realloc(decimals, (strlen(decimals) + strlen(s) + 1) * sizeof(char));
                strcat(decimals, s);
            } else {
                if (isalpha(lexer->c)||lexer->c==95) {
                    for (size_t j = 0; j < i; j++) {
                        lexer_go_back(lexer);
                    }
                    lexer_go_back(lexer);
                    return init_token(TOKEN_INT, value);
                } else {
                    break;
                }
            }
        }
        value = realloc(value, (strlen(value) + strlen(decimals) + 1) * sizeof(char));
        strcat(value, decimals);
        free(decimals);
        if (i > 6) {
            return init_token(TOKEN_DOUBLE, value);
        } else {
            return init_token(TOKEN_FLOAT, value);
        }
    } else {
        return init_token(TOKEN_INT, value);
    }
}


token_T* lexer_read_plus(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c=='=') {
        lexer_advance(lexer);
        return init_token(TOKEN_PLUS_EQUALS, lexer->empty_string);
    } else if (lexer->c=='+') {
        lexer_advance(lexer);
        return init_token(TOKEN_PLUSPLUS, lexer->empty_string);
    } else {
        return init_token(TOKEN_PLUS, lexer->empty_string);
    }
}
token_T* lexer_read_minus(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c=='=') {
        lexer_advance(lexer);
        return init_token(TOKEN_MINUS_EQUALS, lexer->empty_string);
    } else if (lexer->c=='-') {
        lexer_advance(lexer);
        return init_token(TOKEN_MINUSMINUS, lexer->empty_string);
    } else {
        return init_token(TOKEN_MINUS, lexer->empty_string);
    }
}
token_T* lexer_read_times(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c=='=') {
        lexer_advance(lexer);
        return init_token(TOKEN_TIMES_EQUALS, lexer->empty_string);
    } else {
        return init_token(TOKEN_TIMES, lexer->empty_string);
    }
}
token_T* lexer_read_slash(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c=='/') {
        while (lexer->c != 10 && lexer->c != 0) {
            lexer_advance(lexer);
        }
        return init_token(TOKEN_COMMENT, lexer->empty_string);
    } else if (lexer->c=='=') {
        lexer_advance(lexer);
        return init_token(TOKEN_SLASH_EQUALS, lexer->empty_string);
    } else {
        return init_token(TOKEN_SLASH, lexer->empty_string);
    }
}

token_T* lexer_read_equals(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c == '=') {
        lexer_advance(lexer);
        return init_token(TOKEN_EQEQ, lexer->empty_string);
    } else if (lexer->c == '>') {
        lexer_advance(lexer);
        return init_token(TOKEN_RIGHTARROW, lexer->empty_string);
    } else {
        return init_token(TOKEN_EQUALS, lexer->empty_string);
    }
}

token_T* lexer_read_grt(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c == '=') {
        lexer_advance(lexer);
        return init_token(TOKEN_GREQ, lexer->empty_string);
    } else {
        return init_token(TOKEN_GRT, lexer->empty_string);
    }
}

token_T* lexer_read_let(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c == '=') {
        lexer_advance(lexer);
        return init_token(TOKEN_LEEQ, lexer->empty_string);
    } else {
        return init_token(TOKEN_LET, lexer->empty_string);
    }
}

token_T* lexer_read_not(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c == '=') {
        lexer_advance(lexer);
        return init_token(TOKEN_NEQ, lexer->empty_string);
    } else {
        return init_token(TOKEN_EXMARK, lexer->empty_string);
    }
}
token_T* lexer_read_and(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c == '&') {
        lexer_advance(lexer);
        return init_token(TOKEN_ANDAND, lexer->empty_string);
    } else {
        return unexpchar(lexer);
    }
}
token_T* lexer_read_or(lexer_T* lexer) {
    lexer_advance(lexer);
    if (lexer->c == '|') {
        lexer_advance(lexer);
        return init_token(TOKEN_OROR, lexer->empty_string);
    } else {
        return unexpchar(lexer);
    }
}
token_T* lexer_read_underscore(lexer_T* lexer) {
    lexer_advance(lexer);
    if (isalnum(lexer->c)||lexer->c==95) {
        lexer_go_back(lexer);
        return lexer_collect_id(lexer);
    } else {
        return init_token(TOKEN_UNDERSCORE, lexer->empty_string);
    }
}

token_T* lexer_advance_with_token(lexer_T* lexer, token_T* token) {
    lexer_advance(lexer);
    return token;
}

char* lexer_get_current_char_as_string(lexer_T* lexer) {
    char* str = calloc(2, sizeof(char));
    str[0] = lexer->c;
    str[1] = '\0';

    return str;
}
