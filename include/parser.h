#ifndef VXC_PARSER_H
#define VXC_PARSER_H

#include "ast.h"
#include "token.h"

typedef struct {
    Token current;
    Token previous;
    int had_error;
    int panic_mode;
} Parser;

void parser_init(Parser *parser);
Program *parser_parse_program(Parser *parser);

#endif
