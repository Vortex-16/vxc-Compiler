#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

Expr *ast_make_int_literal(long long val, int line, int col) {
    Expr *e = malloc(sizeof(Expr));
    if (!e) return NULL;
    e->kind = EXPR_INT_LITERAL;
    e->line = line;
    e->column = col;
    e->as.int_val = val;
    return e;
}

Expr *ast_make_ident(const char *name, int len, int line, int col) {
    Expr *e = malloc(sizeof(Expr));
    if (!e) return NULL;
    e->kind = EXPR_IDENTIFIER;
    e->line = line;
    e->column = col;
    e->as.ident.name = name;
    e->as.ident.length = len;
    return e;
}

Expr *ast_make_binary(TokenType op, Expr *left, Expr *right, int line, int col) {
    Expr *e = malloc(sizeof(Expr));
    if (!e) return NULL;
    e->kind = EXPR_BINARY;
    e->line = line;
    e->column = col;
    e->as.binary.op = op;
    e->as.binary.left = left;
    e->as.binary.right = right;
    return e;
}

Stmt *ast_make_let(const char *name, int name_len, const char *type_name, int type_len, Expr *init, int line, int col) {
    Stmt *s = malloc(sizeof(Stmt));
    if (!s) return NULL;
    s->kind = STMT_LET;
    s->line = line;
    s->column = col;
    s->as.let_stmt.name = name;
    s->as.let_stmt.name_len = name_len;
    s->as.let_stmt.type_name = type_name;
    s->as.let_stmt.type_len = type_len;
    s->as.let_stmt.initializer = init;
    return s;
}

Stmt *ast_make_print(Expr *expr, int line, int col) {
    Stmt *s = malloc(sizeof(Stmt));
    if (!s) return NULL;
    s->kind = STMT_PRINT;
    s->line = line;
    s->column = col;
    s->as.print_stmt.expr = expr;
    return s;
}

Stmt *ast_make_expr_stmt(Expr *expr, int line, int col) {
    Stmt *s = malloc(sizeof(Stmt));
    if (!s) return NULL;
    s->kind = STMT_EXPR;
    s->line = line;
    s->column = col;
    s->as.expr_stmt = expr;
    return s;
}

FunctionDecl *ast_make_function(const char *name, int name_len) {
    FunctionDecl *fn = malloc(sizeof(FunctionDecl));
    if (!fn) return NULL;
    fn->name = name;
    fn->name_len = name_len;
    fn->body = NULL;
    fn->stmt_count = 0;
    fn->stmt_capacity = 0;
    return fn;
}

void ast_function_add_stmt(FunctionDecl *fn, Stmt *stmt) {
    if (!fn || !stmt) return;
    if (fn->stmt_count >= fn->stmt_capacity) {
        size_t new_cap = fn->stmt_capacity == 0 ? 4 : fn->stmt_capacity * 2;
        Stmt **new_body = realloc(fn->body, new_cap * sizeof(Stmt *));
        if (!new_body) return;
        fn->body = new_body;
        fn->stmt_capacity = new_cap;
    }
    fn->body[fn->stmt_count++] = stmt;
}

Program *ast_make_program(void) {
    Program *prog = malloc(sizeof(Program));
    if (!prog) return NULL;
    prog->functions = NULL;
    prog->count = 0;
    prog->capacity = 0;
    return prog;
}

void ast_program_add_fn(Program *prog, FunctionDecl *fn) {
    if (!prog || !fn) return;
    if (prog->count >= prog->capacity) {
        size_t new_cap = prog->capacity == 0 ? 4 : prog->capacity * 2;
        FunctionDecl **new_fns = realloc(prog->functions, new_cap * sizeof(FunctionDecl *));
        if (!new_fns) return;
        prog->functions = new_fns;
        prog->capacity = new_cap;
    }
    prog->functions[prog->count++] = fn;
}

void ast_free_expr(Expr *expr) {
    if (!expr) return;
    if (expr->kind == EXPR_BINARY) {
        ast_free_expr(expr->as.binary.left);
        ast_free_expr(expr->as.binary.right);
    }
    free(expr);
}

void ast_free_stmt(Stmt *stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
        case STMT_LET:
            ast_free_expr(stmt->as.let_stmt.initializer);
            break;
        case STMT_PRINT:
            ast_free_expr(stmt->as.print_stmt.expr);
            break;
        case STMT_EXPR:
            ast_free_expr(stmt->as.expr_stmt);
            break;
    }
    free(stmt);
}

void ast_free_function(FunctionDecl *fn) {
    if (!fn) return;
    for (size_t i = 0; i < fn->stmt_count; i++) {
        ast_free_stmt(fn->body[i]);
    }
    free(fn->body);
    free(fn);
}

void ast_free_program(Program *prog) {
    if (!prog) return;
    for (size_t i = 0; i < prog->count; i++) {
        ast_free_function(prog->functions[i]);
    }
    free(prog->functions);
    free(prog);
}

static void print_indent(int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
}

static void print_expr(const Expr *e, int depth) {
    if (!e) {
        print_indent(depth);
        printf("<null expr>\n");
        return;
    }
    switch (e->kind) {
        case EXPR_INT_LITERAL:
            print_indent(depth);
            printf("IntLiteral(%lld)\n", e->as.int_val);
            break;
        case EXPR_IDENTIFIER:
            print_indent(depth);
            printf("Identifier(%.*s)\n", e->as.ident.length, e->as.ident.name);
            break;
        case EXPR_BINARY:
            print_indent(depth);
            printf("BinaryExpr(op: %s)\n", token_type_name(e->as.binary.op));
            print_expr(e->as.binary.left, depth + 1);
            print_expr(e->as.binary.right, depth + 1);
            break;
    }
}

static void print_stmt(const Stmt *s, int depth) {
    if (!s) return;
    switch (s->kind) {
        case STMT_LET:
            print_indent(depth);
            printf("LetStmt(name: %.*s, type: %.*s)\n",
                   s->as.let_stmt.name_len, s->as.let_stmt.name,
                   s->as.let_stmt.type_len, s->as.let_stmt.type_name);
            if (s->as.let_stmt.initializer) {
                print_expr(s->as.let_stmt.initializer, depth + 1);
            }
            break;
        case STMT_PRINT:
            print_indent(depth);
            printf("PrintStmt\n");
            print_expr(s->as.print_stmt.expr, depth + 1);
            break;
        case STMT_EXPR:
            print_indent(depth);
            printf("ExprStmt\n");
            print_expr(s->as.expr_stmt, depth + 1);
            break;
    }
}

void ast_print_program(const Program *prog) {
    if (!prog) {
        printf("Program: <null>\n");
        return;
    }
    printf("Program\n");
    for (size_t i = 0; i < prog->count; i++) {
        FunctionDecl *fn = prog->functions[i];
        print_indent(1);
        printf("FunctionDecl(name: %.*s)\n", fn->name_len, fn->name);
        for (size_t j = 0; j < fn->stmt_count; j++) {
            print_stmt(fn->body[j], 2);
        }
    }
}
