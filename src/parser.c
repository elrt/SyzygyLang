#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"


void safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (dest_size == 0 || !dest || !src) return;

    size_t src_len = strlen(src);
    size_t copy_len = (src_len < dest_size - 1) ? src_len : dest_size - 1;

    strncpy(dest, src, copy_len);
    dest[copy_len] = '\0';
}


Parser* parser_create(void) {
    Parser* parser = malloc(sizeof(Parser));
    if (!parser) return NULL;

    parser->pos = 0;
    parser->size = 0;
    parser->ring_count = 0;
    parser->module_count = 0;


    memset(parser->rings, 0, sizeof(parser->rings));
    memset(parser->modules, 0, sizeof(parser->modules));
    memset(parser->tokens, 0, sizeof(parser->tokens));

    return parser;
}

void parser_destroy(Parser* p) {
    if (!p) return;


    for (int i = 0; i < p->module_count && i < MAX_MODULES; i++) {
        if (p->modules[i].generators) {
            for (int j = 0; j < p->modules[i].generator_count && j < p->modules[i].dimension; j++) {
                if (p->modules[i].generators[j]) {
                    free(p->modules[i].generators[j]);
                }
            }
            free(p->modules[i].generators);
        }
    }

    free(p);
}

char* read_file(const char* filename) {
    if (!filename) {
        printf("Error: NULL filename\n");
        return NULL;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length <= 0) {
        printf("Error: File %s is empty or invalid\n", filename);
        fclose(file);
        return NULL;
    }

    char* content = malloc(length + 1);
    if (!content) {
        printf("Error: Memory allocation failed for file content\n");
        fclose(file);
        return NULL;
    }

    size_t read_length = fread(content, 1, length, file);
    if (read_length != (size_t)length) {
        printf("Warning: File read incomplete, read %zu of %ld bytes\n", read_length, length);
    }

    content[read_length] = '\0';

    fclose(file);
    return content;
}

Token current_token(Parser* p) {
    if (!p || p->pos < 0 || p->pos >= p->size) {
        Token eof = {TOKEN_EOF, ""};
        return eof;
    }
    return p->tokens[p->pos];
}

Token next_token(Parser* p) {
    if (!p || p->pos < 0) {
        Token eof = {TOKEN_EOF, ""};
        return eof;
    }

    if (p->pos < p->size - 1) p->pos++;
    return current_token(p);
}

int match(Parser* p, TokenType type) {
    if (!p) return 0;

    if (current_token(p).type == type) {
        next_token(p);
        return 1;
    }
    return 0;
}

void expect(Parser* p, TokenType type, const char* msg) {
    if (!p) {
        printf("Error: Parser is NULL\n");
        exit(1);
    }

    if (!match(p, type)) {
        printf("Error: Expected %s but got '%s' (type: %d)\n",
               msg, current_token(p).value, current_token(p).type);
        exit(1);
    }
}

Ring* find_ring(Parser* p, const char* name) {
    if (!p || !name) return NULL;

    for (int i = 0; i < p->ring_count && i < MAX_RINGS; i++) {
        if (strcmp(p->rings[i].name, name) == 0) {
            return &p->rings[i];
        }
    }
    return NULL;
}

Module* find_module(Parser* p, const char* name) {
    if (!p || !name) return NULL;

    for (int i = 0; i < p->module_count && i < MAX_MODULES; i++) {
        if (strcmp(p->modules[i].name, name) == 0) {
            return &p->modules[i];
        }
    }
    return NULL;
}

void parse_ring_declaration(Parser* p) {
    if (!p) return;

    expect(p, TOKEN_RING, "'ring'");
    expect(p, TOKEN_IDENTIFIER, "ring name");

    char ring_name[MAX_IDENTIFIER_LEN + 2];
    safe_strcpy(ring_name, p->tokens[p->pos-1].value, sizeof(ring_name));

    expect(p, TOKEN_EQUALS, "'='");

    if (match(p, TOKEN_INTEGERS_MOD)) {
        expect(p, TOKEN_NUMBER, "modulus");
        int modulus = atoi(p->tokens[p->pos-1].value);

        if (modulus <= 0) {
            printf("Error: Invalid modulus %d, must be positive\n", modulus);
            exit(1);
        }

        if (p->ring_count >= MAX_RINGS) {
            printf("Error: Too many rings defined (max %d)\n", MAX_RINGS);
            exit(1);
        }

        Ring* ring = &p->rings[p->ring_count++];
        safe_strcpy(ring->name, ring_name, sizeof(ring->name));
        ring->is_finite_field = 1;
        ring->modulus = modulus;

        printf("Defined finite field: %s = Z/%dZ\n", ring_name, modulus);
    } else if (match(p, TOKEN_RATIONALS)) {
        if (p->ring_count >= MAX_RINGS) {
            printf("Error: Too many rings defined (max %d)\n", MAX_RINGS);
            exit(1);
        }

        Ring* ring = &p->rings[p->ring_count++];
        safe_strcpy(ring->name, ring_name, sizeof(ring->name));
        ring->is_finite_field = 0;
        ring->modulus = 0;

        printf("Defined ring: %s = Q\n", ring_name);
    } else {
        printf("Error: Expected ring type (integers_mod or rationals)\n");
        exit(1);
    }
}

void parse_module_declaration(Parser* p) {
    if (!p) return;

    expect(p, TOKEN_MODULE, "'module'");
    expect(p, TOKEN_IDENTIFIER, "module name");

    char module_name[MAX_IDENTIFIER_LEN + 2];
    safe_strcpy(module_name, p->tokens[p->pos-1].value, sizeof(module_name));

    expect(p, TOKEN_EQUALS, "'='");
    expect(p, TOKEN_FREE_MODULE, "'free_module'");
    expect(p, TOKEN_LPAREN, "'('");

    expect(p, TOKEN_IDENTIFIER, "ring name");

    char ring_name[MAX_IDENTIFIER_LEN + 2];
    safe_strcpy(ring_name, p->tokens[p->pos-1].value, sizeof(ring_name));

    Ring* ring = find_ring(p, ring_name);
    if (!ring) {
        printf("Error: Unknown ring '%s'\n", ring_name);
        exit(1);
    }

    expect(p, TOKEN_COMMA, "','");
    expect(p, TOKEN_NUMBER, "dimension");
    int dimension = atoi(p->tokens[p->pos-1].value);

    if (dimension <= 0) {
        printf("Error: Invalid dimension %d, must be positive\n", dimension);
        exit(1);
    }

    expect(p, TOKEN_RPAREN, "')'");

    if (p->module_count >= MAX_MODULES) {
        printf("Error: Too many modules defined (max %d)\n", MAX_MODULES);
        exit(1);
    }

    Module* module = &p->modules[p->module_count++];
    safe_strcpy(module->name, module_name, sizeof(module->name));
    module->base_ring = ring;
    module->dimension = dimension;
    module->generator_count = 0;
    module->generators = calloc(dimension, sizeof(char*));

    if (!module->generators) {
        printf("Error: Memory allocation failed for module generators\n");
        exit(1);
    }

    printf("Defined module: %s = %s^%d\n", module_name, ring_name, dimension);
}

void parse_generators(Parser* p) {
    if (!p) return;

    expect(p, TOKEN_GENERATORS, "'generators'");
    expect(p, TOKEN_LBRACE, "'{'");

    while (current_token(p).type != TOKEN_RBRACE && current_token(p).type != TOKEN_EOF) {
        expect(p, TOKEN_IDENTIFIER, "generator name");
        char gen_name[MAX_IDENTIFIER_LEN + 2];
        safe_strcpy(gen_name, p->tokens[p->pos-1].value, sizeof(gen_name));

        expect(p, TOKEN_EQUALS, "'='");
        expect(p, TOKEN_LPAREN, "'('");

        printf("  Generator: %s = (", gen_name);


        int first = 1;
        while (current_token(p).type != TOKEN_RPAREN && current_token(p).type != TOKEN_EOF) {
            if (match(p, TOKEN_NUMBER)) {
                if (!first) printf(", ");
                printf("%s", p->tokens[p->pos-1].value);
                first = 0;
            } else if (match(p, TOKEN_COMMA)) {

            } else {
                printf(" [unexpected: %s]", current_token(p).value);
                next_token(p);
            }
        }
        printf(")");

        expect(p, TOKEN_RPAREN, "')'");
        expect(p, TOKEN_IN, "'in'");
        expect(p, TOKEN_IDENTIFIER, "module name");

        char module_name[MAX_IDENTIFIER_LEN + 2];
        safe_strcpy(module_name, p->tokens[p->pos-1].value, sizeof(module_name));
        Module* module = find_module(p, module_name);

        if (module) {
            if (module->generator_count < module->dimension) {
                module->generators[module->generator_count] = malloc(MAX_IDENTIFIER_LEN + 2);
                if (!module->generators[module->generator_count]) {
                    printf(" [ERROR: Memory allocation failed]");
                } else {
                    safe_strcpy(module->generators[module->generator_count], gen_name, MAX_IDENTIFIER_LEN + 2);
                    module->generator_count++;
                    printf(" in %s\n", module_name);
                }
            } else {
                printf(" [ERROR: Module %s is full (dimension %d)]\n", module_name, module->dimension);
            }
        } else {
            printf(" [ERROR: Module %s not found]\n", module_name);
        }

        if (current_token(p).type == TOKEN_SEMICOLON) {
            match(p, TOKEN_SEMICOLON);
        }
    }

    expect(p, TOKEN_RBRACE, "'}'");
}

void parse_relations(Parser* p) {
    if (!p) return;

    expect(p, TOKEN_RELATIONS, "'relations'");
    expect(p, TOKEN_LBRACE, "'{'");

    int relation_count = 0;

    while (current_token(p).type != TOKEN_RBRACE && current_token(p).type != TOKEN_EOF) {
        printf("  Relation %d: ", ++relation_count);

        int brace_count = 0;
        int paren_count = 0;
        int bracket_count = 0;

        while ((current_token(p).type != TOKEN_SEMICOLON || brace_count > 0 || paren_count > 0 || bracket_count > 0) &&
               current_token(p).type != TOKEN_RBRACE &&
               current_token(p).type != TOKEN_EOF) {

            if (current_token(p).type == TOKEN_LBRACE) brace_count++;
            if (current_token(p).type == TOKEN_RBRACE) brace_count--;
            if (current_token(p).type == TOKEN_LPAREN) paren_count++;
            if (current_token(p).type == TOKEN_RPAREN) paren_count--;
            if (current_token(p).type == TOKEN_LBRACKET) bracket_count++;
            if (current_token(p).type == TOKEN_RBRACKET) bracket_count--;


            if (brace_count < 0 || paren_count < 0 || bracket_count < 0) {
                printf(" [ERROR: Unbalanced brackets]");
                break;
            }

            printf("%s ", current_token(p).value);
            next_token(p);
        }
        printf("\n");

        if (current_token(p).type == TOKEN_SEMICOLON) {
            match(p, TOKEN_SEMICOLON);
        }


        if (relation_count > 1000) {
            printf("Warning: Too many relations, stopping at 1000\n");
            break;
        }
    }

    expect(p, TOKEN_RBRACE, "'}'");
}

int is_expression_terminator(Parser* p) {
    if (!p) return 1;

    TokenType type = current_token(p).type;
    return (type == TOKEN_SEMICOLON || type == TOKEN_RBRACE ||
            type == TOKEN_DEFINE || type == TOKEN_RECURSIVE ||
            type == TOKEN_COLIMIT || type == TOKEN_RING ||
            type == TOKEN_MODULE || type == TOKEN_EOF ||
            type == TOKEN_CASE || type == TOKEN_OF);
}

void parse_expression(Parser* p) {
    if (!p) return;

    int paren_count = 0;
    int brace_count = 0;
    int bracket_count = 0;

    while (!is_expression_terminator(p)) {
        TokenType type = current_token(p).type;

        if (type == TOKEN_LPAREN) paren_count++;
        else if (type == TOKEN_RPAREN) {
            paren_count--;
            if (paren_count < 0) {
                printf(" [ERROR: Unbalanced parentheses]");
                break;
            }
        }
        else if (type == TOKEN_LBRACE) brace_count++;
        else if (type == TOKEN_RBRACE) {
            brace_count--;
            if (brace_count < 0) {
                printf(" [ERROR: Unbalanced braces]");
                break;
            }
        }
        else if (type == TOKEN_LBRACKET) bracket_count++;
        else if (type == TOKEN_RBRACKET) {
            bracket_count--;
            if (bracket_count < 0) {
                printf(" [ERROR: Unbalanced brackets]");
                break;
            }
        }


        if (paren_count > 0 || brace_count > 0 || bracket_count > 0) {
            printf("%s ", current_token(p).value);
            next_token(p);
            continue;
        }


        if (is_expression_terminator(p)) {
            break;
        }

        printf("%s ", current_token(p).value);
        next_token(p);


        if (p->pos > p->size) {
            printf(" [ERROR: Parser position overflow]");
            break;
        }
    }


    if (paren_count > 0) printf(" [ERROR: Unclosed parentheses]");
    if (brace_count > 0) printf(" [ERROR: Unclosed braces]");
    if (bracket_count > 0) printf(" [ERROR: Unclosed brackets]");
}

void parse_case_block(Parser* p) {
    if (!p) return;

    expect(p, TOKEN_LBRACE, "'{'");

    int case_count = 0;

    while (current_token(p).type != TOKEN_RBRACE && current_token(p).type != TOKEN_EOF) {
        printf("  Pattern %d: ", ++case_count);


        while (current_token(p).type != TOKEN_ARROW &&
               current_token(p).type != TOKEN_RBRACE &&
               current_token(p).type != TOKEN_EOF) {
            printf("%s ", current_token(p).value);
            next_token(p);
        }

        if (match(p, TOKEN_ARROW)) {
            printf("-> ");
            parse_expression(p);
        }

        printf("\n");


        if (current_token(p).type == TOKEN_SEMICOLON) {
            match(p, TOKEN_SEMICOLON);
        }


        if (case_count > 1000) {
            printf("Warning: Too many case patterns, stopping at 1000\n");
            break;
        }
    }

    expect(p, TOKEN_RBRACE, "'}'");
}

void parse_algebraic_control(Parser* p) {
    if (!p) return;

    if (match(p, TOKEN_DEFINE)) {
        expect(p, TOKEN_IDENTIFIER, "definition name");
        char def_name[MAX_IDENTIFIER_LEN + 2];
        safe_strcpy(def_name, p->tokens[p->pos-1].value, sizeof(def_name));

        expect(p, TOKEN_AS, "'as'");

        printf("Algebraic definition: %s = ", def_name);
        parse_expression(p);
        printf("\n");


        if (current_token(p).type == TOKEN_SEMICOLON) {
            match(p, TOKEN_SEMICOLON);
        }
    }
    else if (match(p, TOKEN_CASE)) {
        printf("Case analysis on: ");
        parse_expression(p);
        expect(p, TOKEN_OF, "'of'");

        printf("\nCase analysis:\n");
        parse_case_block(p);


        if (current_token(p).type == TOKEN_SEMICOLON) {
            match(p, TOKEN_SEMICOLON);
        }
    }
    else if (match(p, TOKEN_RECURSIVE)) {
        expect(p, TOKEN_IDENTIFIER, "recursive name");
        char rec_name[MAX_IDENTIFIER_LEN + 2];
        safe_strcpy(rec_name, p->tokens[p->pos-1].value, sizeof(rec_name));

        expect(p, TOKEN_WHERE, "'where'");
        expect(p, TOKEN_LBRACE, "'{'");

        printf("Recursive definition: %s\n", rec_name);

        while (current_token(p).type != TOKEN_RBRACE && current_token(p).type != TOKEN_EOF) {
            parse_algebraic_control(p);
        }

        expect(p, TOKEN_RBRACE, "'}'");


        if (current_token(p).type == TOKEN_SEMICOLON) {
            match(p, TOKEN_SEMICOLON);
        }
    }
    else if (match(p, TOKEN_FIXED_POINT)) {
        printf("Fixed-point combinator: ");
        parse_expression(p);
        printf("\n");


        if (current_token(p).type == TOKEN_SEMICOLON) {
            match(p, TOKEN_SEMICOLON);
        }
    }
    else if (match(p, TOKEN_COLIMIT) || match(p, TOKEN_LIMIT)) {
        TokenType construct_type = p->tokens[p->pos-1].type;
        const char* construct_name = (construct_type == TOKEN_COLIMIT) ? "colimit" : "limit";

        expect(p, TOKEN_IDENTIFIER, "construct name");
        char construct_id[MAX_IDENTIFIER_LEN + 2];
        safe_strcpy(construct_id, p->tokens[p->pos-1].value, sizeof(construct_id));

        expect(p, TOKEN_WHERE, "'where'");
        expect(p, TOKEN_LBRACE, "'{'");

        printf("Category theory %s: %s\n", construct_name, construct_id);

        while (current_token(p).type != TOKEN_RBRACE && current_token(p).type != TOKEN_EOF) {
            parse_algebraic_control(p);
        }

        expect(p, TOKEN_RBRACE, "'}'");

        if (current_token(p).type == TOKEN_SEMICOLON) {
            match(p, TOKEN_SEMICOLON);
        }
    }
    else {
        printf("Error: Unknown algebraic control structure\n");
        exit(1);
    }
}

void parse(Parser* p) {
    if (!p) return;

    int statement_count = 0;

    while (current_token(p).type != TOKEN_EOF) {
        if (current_token(p).type == TOKEN_RING) {
            parse_ring_declaration(p);
        } else if (current_token(p).type == TOKEN_MODULE) {
            parse_module_declaration(p);
        } else if (current_token(p).type == TOKEN_GENERATORS) {
            parse_generators(p);
        } else if (current_token(p).type == TOKEN_RELATIONS) {
            parse_relations(p);
        } else if (current_token(p).type == TOKEN_DEFINE ||
                  current_token(p).type == TOKEN_CASE ||
                  current_token(p).type == TOKEN_RECURSIVE ||
                  current_token(p).type == TOKEN_FIXED_POINT ||
                  current_token(p).type == TOKEN_COLIMIT ||
                  current_token(p).type == TOKEN_LIMIT) {
            parse_algebraic_control(p);
        } else {

            if (current_token(p).type == TOKEN_RBRACE ||
                current_token(p).type == TOKEN_SEMICOLON) {
                next_token(p);
                continue;
            }

            printf("Warning: Unexpected token '%s' (type: %d), skipping\n",
                   current_token(p).value, current_token(p).type);
            next_token(p);
        }

        statement_count++;


        if (statement_count > 10000) {
            printf("Error: Too many statements, possible infinite loop\n");
            break;
        }

        if (p->pos < 0 || p->pos > p->size) {
            printf("Error: Parser position corrupted\n");
            break;
        }
    }
}
