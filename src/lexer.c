#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

typedef struct {
    const char* keyword;
    TokenType type;
} KeywordMapping;

static const KeywordMapping KEYWORDS[] = {
    {"ring", TOKEN_RING},
    {"module", TOKEN_MODULE},
    {"generators", TOKEN_GENERATORS},
    {"relations", TOKEN_RELATIONS},
    {"homomorphism", TOKEN_HOMOMORPHISM},
    {"in", TOKEN_IN},
    {"integers_mod", TOKEN_INTEGERS_MOD},
    {"rationals", TOKEN_RATIONALS},
    {"free_module", TOKEN_FREE_MODULE},
    {"define", TOKEN_DEFINE},
    {"as", TOKEN_AS},
    {"where", TOKEN_WHERE},
    {"case", TOKEN_CASE},
    {"of", TOKEN_OF},
    {"recursive", TOKEN_RECURSIVE},
    {"fixed_point", TOKEN_FIXED_POINT},
    {"morphism", TOKEN_MORPHISM},
    {"endomorphism", TOKEN_ENDOMORPHISM},
    {"kernel", TOKEN_KERNEL},
    {"image", TOKEN_IMAGE},
    {"compose", TOKEN_COMPOSE},
    {"apply", TOKEN_APPLY},
    {"map", TOKEN_MAP},
    {"fold", TOKEN_FOLD},
    {"unfold", TOKEN_UNFOLD},
    {"colimit", TOKEN_COLIMIT},
    {"limit", TOKEN_LIMIT},
    {"filter", TOKEN_FILTER},
    {"lambda", TOKEN_LAMBDA},
    {"with", TOKEN_WITH},
    {"initial", TOKEN_INITIAL},
    {NULL, TOKEN_IDENTIFIER}

};

static TokenType get_keyword_type(const char* value) {
    for (int i = 0; KEYWORDS[i].keyword != NULL; i++) {
        if (strcmp(value, KEYWORDS[i].keyword) == 0) {
            return KEYWORDS[i].type;
        }
    }
    return TOKEN_IDENTIFIER;
}

static int add_token(Token* tokens, int* t, TokenType type, const char* value, size_t len) {
    if (*t >= MAX_TOKENS - 1) {
        return -1;

    }

    tokens[*t].type = type;
    if (len > 0) {
        strncpy(tokens[*t].value, value, len);
        tokens[*t].value[len] = '\0';
    } else {
        tokens[*t].value[0] = '\0';
    }
    (*t)++;
    return 0;
}

int tokenize(const char* input, Token* tokens, int* token_count) {
    int i = 0, t = 0;

    while (input[i] && t < MAX_TOKENS - 1) {
        if (isspace(input[i])) {
            i++;
            continue;
        }


        if (input[i] == '/' && input[i+1] == '/') {
            while (input[i] && input[i] != '\n') {
                i++;
            }
            continue;
        }


        if (input[i] == '/' && input[i+1] == '*') {
            i += 2;
            while (input[i] && !(input[i] == '*' && input[i+1] == '/')) {
                i++;
            }
            if (input[i]) i += 2;
            continue;
        }

        if (isalpha(input[i]) || input[i] == '_') {
            int start = i;
            while (isalnum(input[i]) || input[i] == '_') {
                i++;
            }
            int len = i - start;
            if (len >= MAX_TOKEN_LEN) {
                len = MAX_TOKEN_LEN - 1;
            }

            char identifier[MAX_TOKEN_LEN];
            strncpy(identifier, &input[start], len);
            identifier[len] = '\0';

            TokenType type = get_keyword_type(identifier);
            if (add_token(tokens, &t, type, identifier, len) != 0) {
                return -1;
            }
            continue;
        }

        if (isdigit(input[i])) {
            int start = i;
            while (isdigit(input[i])) {
                i++;
            }
            int len = i - start;
            if (len >= MAX_TOKEN_LEN) {
                len = MAX_TOKEN_LEN - 1;
            }

            if (add_token(tokens, &t, TOKEN_NUMBER, &input[start], len) != 0) {
                return -1;
            }
            continue;
        }

        if (input[i] == '"') {
            i++;
            int start = i;
            while (input[i] && input[i] != '"') {
                i++;
            }
            int len = i - start;
            if (len >= MAX_TOKEN_LEN) {
                len = MAX_TOKEN_LEN - 1;
            }

            if (add_token(tokens, &t, TOKEN_STRING, &input[start], len) != 0) {
                return -1;
            }
            if (input[i] == '"') i++;
            continue;
        }

        if (input[i] == '=' && input[i+1] == '=') {
            if (add_token(tokens, &t, TOKEN_EQ, "==", 2) != 0) return -1;
            i += 2;
            continue;
        }
        if (input[i] == '!' && input[i+1] == '=') {
            if (add_token(tokens, &t, TOKEN_NE, "!=", 2) != 0) return -1;
            i += 2;
            continue;
        }
        if (input[i] == '<' && input[i+1] == '=') {
            if (add_token(tokens, &t, TOKEN_LE, "<=", 2) != 0) return -1;
            i += 2;
            continue;
        }
        if (input[i] == '>' && input[i+1] == '=') {
            if (add_token(tokens, &t, TOKEN_GE, ">=", 2) != 0) return -1;
            i += 2;
            continue;
        }

        char single_char[2] = {input[i], '\0'};
        TokenType type = TOKEN_EOF;

        switch (input[i]) {
            case '=': type = TOKEN_EQUALS; break;
            case '+': type = TOKEN_PLUS; break;
            case '-':
                if (input[i+1] == '>') {
                    if (add_token(tokens, &t, TOKEN_ARROW, "->", 2) != 0) return -1;
                    i += 2;
                    continue;
                } else {
                    type = TOKEN_MINUS;
                }
                break;
            case '*': type = TOKEN_STAR; break;
            case '/': type = TOKEN_SLASH; break;
            case '%': type = TOKEN_MOD; break;
            case '(': type = TOKEN_LPAREN; break;
            case ')': type = TOKEN_RPAREN; break;
            case '{': type = TOKEN_LBRACE; break;
            case '}': type = TOKEN_RBRACE; break;
            case ',': type = TOKEN_COMMA; break;
            case ';': type = TOKEN_SEMICOLON; break;
            case '<': type = TOKEN_LT; break;
            case '>': type = TOKEN_GT; break;
            case ':': type = TOKEN_COLON; break;
            case '[': type = TOKEN_LBRACKET; break;
            case ']': type = TOKEN_RBRACKET; break;
            case '_': type = TOKEN_UNDERSCORE; break;
            case '.': type = TOKEN_DOT; break;
            default:

                printf("Warning: Unknown character '%c' skipped\n", input[i]);
                i++;
                continue;
        }

        if (add_token(tokens, &t, type, single_char, 1) != 0) {
            return -1;
        }
        i++;
    }

    if (add_token(tokens, &t, TOKEN_EOF, "", 0) != 0) {
        return -1;
    }
    *token_count = t;
    return 0;
}
