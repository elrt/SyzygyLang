#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#define MAX_RINGS 50
#define MAX_MODULES 50
#define MAX_IDENTIFIER_LEN 48

typedef struct {
    char name[MAX_IDENTIFIER_LEN + 2];
    int is_finite_field;
    int modulus;
} Ring;

typedef struct {
    char name[MAX_IDENTIFIER_LEN + 2];
    Ring* base_ring;
    int dimension;
    char** generators;
    int generator_count;
} Module;

typedef struct {
    Token tokens[MAX_TOKENS];
    int pos;
    int size;

    Ring rings[MAX_RINGS];
    int ring_count;

    Module modules[MAX_MODULES];
    int module_count;
} Parser;


Parser* parser_create(void);
void parser_destroy(Parser* p);


char* read_file(const char* filename);

void parse(Parser* p);
Token current_token(Parser* p);
Token next_token(Parser* p);
int match(Parser* p, TokenType type);
void expect(Parser* p, TokenType type, const char* msg);

void safe_strcpy(char* dest, const char* src, size_t dest_size);

#endif
