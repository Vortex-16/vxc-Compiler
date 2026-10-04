#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

Token next_token(void);

static void error_at(Parser *p, Token *t, const char *msg) {
    if (p->panic_mode) return;
    p->panic_mode = 1;
    p->had_error = 1;
    fprintf(stderr, "[line %d, col %d] Error", t->line, t->column);
    if (t->type == TOK_EOF) {
        fprintf(stderr, " at end of file");
    } else if (t->type == TOK_ERROR) {
        /* error token itself contains lexer msg */
    } else {
        fprintf(stderr, " at '%.*s'", t->length, t->start);
    }
    fprintf(stderr, ": %s\n", msg);
}

static void error_current(Parser *p, const char *msg) {
    error_at(p, &p->current, msg);
}

static void advance(Parser *p) {
    p->previous = p->current;
    for (;;) {
        p->current = next_token();
        if (p->current.type != TOK_ERROR) break;
        error_at(p, &p->current, "Lexical error");
    }
}

static int check(Parser *p, TokenType type) {
    return p->current.type == type;
}

static int match(Parser *p, TokenType type) {
    if (!check(p, type)) return 0;
    advance(p);
    return 1;
}

static void consume(Parser *p, TokenType type, const char *msg) {
    if (p->current.type == type) {
        advance(p);
        return;
    }
    error_current(p, msg);
}

static void synchronize(Parser *p) {
    p->panic_mode = 0;
    while (p->current.type != TOK_EOF) {
        if (p->previous.type == TOK_SEMICOLON) return;
        switch (p->current.type) {
            case TOK_FN:
            case TOK_LET:
            case TOK_PRINT:
            case TOK_RBRACE:
                return;
            default:
                break;
        }
        advance(p);
    }
}

/* Forward declarations */
static Expr *parse_expression(Parser *p);
static Stmt *parse_statement(Parser *p);

/* Primary := INT_LITERAL | IDENTIFIER | '(' expr ')' */
static Expr *parse_primary(Parser *p) {
    if (match(p, TOK_INT_LITERAL)) {
        char buf[32];
        int len = p->previous.length < 31 ? p->previous.length : 31;
        snprintf(buf, len + 1, "%.*s", len, p->previous.start);
        long long val = atoll(buf);
        return ast_make_int_literal(val, p->previous.line, p->previous.column);
    }
    if (match(p, TOK_IDENTIFIER)) {
        return ast_make_ident(p->previous.start, p->previous.length, p->previous.line, p->previous.column);
    }
    if (match(p, TOK_LPAREN)) {
        Expr *e = parse_expression(p);
        consume(p, TOK_RPAREN, "Expect ')' after expression.");
        return e;
    }
    error_current(p, "Expect expression.");
    return NULL;
}

/* Multiplication & Division (*, /) */
static Expr *parse_multiplicative(Parser *p) {
    Expr *expr = parse_primary(p);
    if (!expr) return NULL;

    while (match(p, TOK_STAR) || match(p, TOK_SLASH)) {
        TokenType op = p->previous.type;
        int line = p->previous.line;
        int col = p->previous.column;
        Expr *right = parse_primary(p);
        if (!right) {
            ast_free_expr(expr);
            return NULL;
        }
        expr = ast_make_binary(op, expr, right, line, col);
    }
    return expr;
}

/* Addition & Subtraction (+, -) */
static Expr *parse_additive(Parser *p) {
    Expr *expr = parse_multiplicative(p);
    if (!expr) return NULL;

    while (match(p, TOK_PLUS) || match(p, TOK_MINUS)) {
        TokenType op = p->previous.type;
        int line = p->previous.line;
        int col = p->previous.column;
        Expr *right = parse_multiplicative(p);
        if (!right) {
            ast_free_expr(expr);
            return NULL;
        }
        expr = ast_make_binary(op, expr, right, line, col);
    }
    return expr;
}

static Expr *parse_expression(Parser *p) {
    return parse_additive(p);
}

/* let <id>: <type> = <expr>; */
static Stmt *parse_let_statement(Parser *p) {
    int line = p->previous.line;
    int col = p->previous.column;
    consume(p, TOK_IDENTIFIER, "Expect variable name after 'let'.");
    Token name_tok = p->previous;

    consume(p, TOK_COLON, "Expect ':' after variable name.");

    /* Currently supporting type names like 'int' or identifiers */
    if (!match(p, TOK_INT) && !match(p, TOK_IDENTIFIER)) {
        error_current(p, "Expect type annotation.");
    }
    Token type_tok = p->previous;

    consume(p, TOK_EQUAL, "Expect '=' after variable type.");

    Expr *init = parse_expression(p);
    consume(p, TOK_SEMICOLON, "Expect ';' after variable declaration.");

    return ast_make_let(name_tok.start, name_tok.length,
                        type_tok.start, type_tok.length,
                        init, line, col);
}

/* print(<expr>); */
static Stmt *parse_print_statement(Parser *p) {
    int line = p->previous.line;
    int col = p->previous.column;
    consume(p, TOK_LPAREN, "Expect '(' after 'print'.");
    Expr *expr = parse_expression(p);
    consume(p, TOK_RPAREN, "Expect ')' after expression.");
    consume(p, TOK_SEMICOLON, "Expect ';' after print statement.");

    return ast_make_print(expr, line, col);
}

/* statement := let_stmt | print_stmt | expr_stmt */
static Stmt *parse_statement(Parser *p) {
    if (match(p, TOK_LET)) {
        return parse_let_statement(p);
    }
    if (match(p, TOK_PRINT)) {
        return parse_print_statement(p);
    }

    /* Fallback: expression statement */
    int line = p->current.line;
    int col = p->current.column;
    Expr *expr = parse_expression(p);
    consume(p, TOK_SEMICOLON, "Expect ';' after expression.");
    return ast_make_expr_stmt(expr, line, col);
}

/* function := "fn" IDENT "(" ")" "{" statement* "}" */
static FunctionDecl *parse_function(Parser *p) {
    consume(p, TOK_IDENTIFIER, "Expect function name after 'fn'.");
    Token name = p->previous;

    consume(p, TOK_LPAREN, "Expect '(' after function name.");
    /* In this phase, function parameter parsing can be empty */
    consume(p, TOK_RPAREN, "Expect ')' after parameter list.");
    consume(p, TOK_LBRACE, "Expect '{' before function body.");

    FunctionDecl *fn = ast_make_function(name.start, name.length);

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        Stmt *s = parse_statement(p);
        if (s) {
            ast_function_add_stmt(fn, s);
        } else {
            synchronize(p);
        }
    }

    consume(p, TOK_RBRACE, "Expect '}' after function body.");
    return fn;
}

void parser_init(Parser *parser) {
    parser->had_error = 0;
    parser->panic_mode = 0;
    advance(parser);
}

Program *parser_parse_program(Parser *parser) {
    Program *prog = ast_make_program();
    while (!check(parser, TOK_EOF)) {
        if (match(parser, TOK_FN)) {
            FunctionDecl *fn = parse_function(parser);
            if (fn) {
                ast_program_add_fn(prog, fn);
            }
        } else {
            error_current(parser, "Expect top-level function declaration ('fn').");
            advance(parser);
            synchronize(parser);
        }
    }
    return prog;
}
