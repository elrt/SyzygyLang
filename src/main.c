#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename.sz>\n", argv[0]);
        printf("Example: %s test.sz\n", argv[0]);
        return 1;
    }

    printf("Syzygy Algebraic Interpreter Improved (SAII)\n");
    printf("==================================\n\n");

    char* program = read_file(argv[1]);
    if (!program) {
        return 1;
    }

    printf("Parsing file: %s\n", argv[1]);
    printf("----------------------------------------\n");

    Parser* parser = parser_create();
    if (!parser) {
        free(program);
        return 1;
    }

    int token_count;
    if (tokenize(program, parser->tokens, &token_count) != 0) {
        free(program);
        parser_destroy(parser);
        return 1;
    }
    parser->size = token_count;

    printf("Tokens found: %d\n", token_count);
    printf("----------------------------------------\n");

    parse(parser);

    printf("----------------------------------------\n");
    printf("Algebraic execution completed!\n");
    printf("Structures defined:\n");
    printf("  Rings: %d\n", parser->ring_count);
    printf("  Modules: %d\n", parser->module_count);

    free(program);
    parser_destroy(parser);

    return 0;
}
