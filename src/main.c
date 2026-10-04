#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "ast.h"
#include "parser.h"

void lexer_init(const char *text);
Token next_token(void);

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [options] <source.vxc>\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --dump-tokens    Print token stream from lexer\n");
    fprintf(stderr, "  --dump-ast       Print AST after parsing (default)\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *filepath = NULL;
    int dump_tokens = 0;
    int dump_ast = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dump-tokens") == 0) {
            dump_tokens = 1;
            dump_ast = 0;
        } else if (strcmp(argv[i], "--dump-ast") == 0) {
            dump_ast = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            filepath = argv[i];
        }
    }

    if (!filepath) {
        fprintf(stderr, "Error: No input file specified.\n");
        print_usage(argv[0]);
        return 1;
    }

    FILE *f = fopen(filepath, "rb");
    if (!f) {
        perror(filepath);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);

    char *text = malloc((size_t)n + 1);
    if (!text) {
        fclose(f);
        return 1;
    }

    if (fread(text, 1, (size_t)n, f) != (size_t)n && n > 0) {
        fprintf(stderr, "Error reading file %s\n", filepath);
        free(text);
        fclose(f);
        return 1;
    }
    text[n] = '\0';
    fclose(f);

    lexer_init(text);

    if (dump_tokens) {
        Token t;
        do {
            t = next_token();
            printf("%-12s line=%d col=%d lexeme=\"%.*s\"\n",
                   token_type_name(t.type), t.line, t.column, t.length, t.start);
        } while (t.type != TOK_EOF);
        free(text);
        return 0;
    }

    Parser parser;
    parser_init(&parser);
    Program *prog = parser_parse_program(&parser);

    int exit_code = 0;
    if (parser.had_error) {
        fprintf(stderr, "Parsing failed with errors.\n");
        exit_code = 1;
    } else if (dump_ast) {
        ast_print_program(prog);
    }

    ast_free_program(prog);
    free(text);
    return exit_code;
}
