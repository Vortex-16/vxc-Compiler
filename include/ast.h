#ifndef VXC_AST_H
#define VXC_AST_H

#include <stddef.h>
#include "token.h"

typedef enum {
    EXPR_INT_LITERAL,
    EXPR_IDENTIFIER,
    EXPR_BINARY
} ExprKind;

typedef struct Expr Expr;

typedef struct {
    TokenType op;
    Expr *left;
    Expr *right;
} BinaryExpr;

struct Expr {
    ExprKind kind;
    int line;
    int column;
    union {
        long long int_val;
        struct {
            const char *name;
            int length;
        } ident;
        BinaryExpr binary;
    } as;
};

typedef enum {
    STMT_LET,
    STMT_PRINT,
    STMT_EXPR
} StmtKind;

typedef struct Stmt Stmt;

typedef struct {
    const char *name;
    int name_len;
    const char *type_name;
    int type_len;
    Expr *initializer;
} LetStmt;

typedef struct {
    Expr *expr;
} PrintStmt;

struct Stmt {
    StmtKind kind;
    int line;
    int column;
    union {
        LetStmt let_stmt;
        PrintStmt print_stmt;
        Expr *expr_stmt;
    } as;
};

typedef struct {
    const char *name;
    int name_len;
    Stmt **body;
    size_t stmt_count;
    size_t stmt_capacity;
} FunctionDecl;

typedef struct {
    FunctionDecl **functions;
    size_t count;
    size_t capacity;
} Program;

/* AST Node constructors */
Expr *ast_make_int_literal(long long val, int line, int col);
Expr *ast_make_ident(const char *name, int len, int line, int col);
Expr *ast_make_binary(TokenType op, Expr *left, Expr *right, int line, int col);

Stmt *ast_make_let(const char *name, int name_len, const char *type_name, int type_len, Expr *init, int line, int col);
Stmt *ast_make_print(Expr *expr, int line, int col);
Stmt *ast_make_expr_stmt(Expr *expr, int line, int col);

FunctionDecl *ast_make_function(const char *name, int name_len);
void ast_function_add_stmt(FunctionDecl *fn, Stmt *stmt);

Program *ast_make_program(void);
void ast_program_add_fn(Program *prog, FunctionDecl *fn);

/* Memory cleanup */
void ast_free_expr(Expr *expr);
void ast_free_stmt(Stmt *stmt);
void ast_free_function(FunctionDecl *fn);
void ast_free_program(Program *prog);

/* AST Pretty Printer */
void ast_print_program(const Program *prog);

#endif
