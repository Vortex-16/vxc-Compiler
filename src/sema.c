#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "sema.h"

const char *type_kind_name(TypeKind kind) {
    switch (kind) {
        case TYPE_INT:     return "int";
        case TYPE_VOID:    return "void";
        case TYPE_ERROR:   return "<error>";
        case TYPE_UNKNOWN:
        default:           return "<unknown>";
    }
}

static TypeKind resolve_type_name(const char *name, int len) {
    if (len == 3 && strncmp(name, "int", 3) == 0) {
        return TYPE_INT;
    }
    if (len == 4 && strncmp(name, "void", 4) == 0) {
        return TYPE_VOID;
    }
    return TYPE_UNKNOWN;
}

static Scope *scope_new(Scope *parent) {
    Scope *s = malloc(sizeof(Scope));
    if (!s) return NULL;
    s->parent = parent;
    s->symbols = NULL;
    return s;
}

static void scope_free(Scope *scope) {
    if (!scope) return;
    Symbol *curr = scope->symbols;
    while (curr) {
        Symbol *next = curr->next;
        free(curr);
        curr = next;
    }
    free(scope);
}

static Symbol *scope_lookup_current(const Scope *scope, const char *name, int len) {
    if (!scope) return NULL;
    for (Symbol *s = scope->symbols; s != NULL; s = s->next) {
        if (s->name_len == len && strncmp(s->name, name, (size_t)len) == 0) {
            return s;
        }
    }
    return NULL;
}

static Symbol *scope_lookup(const Scope *scope, const char *name, int len) {
    for (const Scope *s = scope; s != NULL; s = s->parent) {
        Symbol *sym = scope_lookup_current(s, name, len);
        if (sym) return sym;
    }
    return NULL;
}

static void scope_add_symbol(Scope *scope, const char *name, int name_len,
                             SymbolKind kind, TypeKind type, int line, int column) {
    Symbol *sym = malloc(sizeof(Symbol));
    if (!sym) return;
    sym->name = name;
    sym->name_len = name_len;
    sym->kind = kind;
    sym->type = type;
    sym->line = line;
    sym->column = column;
    sym->next = scope->symbols;
    scope->symbols = sym;
}

static void sema_error(Sema *sema, int line, int col, const char *fmt, ...) {
    sema->error_count++;
    if (line > 0 && col > 0) {
        fprintf(stderr, "[line %d, col %d] Semantic error: ", line, col);
    } else if (line > 0) {
        fprintf(stderr, "[line %d] Semantic error: ", line);
    } else {
        fprintf(stderr, "Semantic error: ");
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static TypeKind check_expr(Sema *sema, const Expr *expr) {
    if (!expr) return TYPE_ERROR;

    switch (expr->kind) {
        case EXPR_INT_LITERAL:
            return TYPE_INT;

        case EXPR_IDENTIFIER: {
            Symbol *sym = scope_lookup(sema->current_scope, expr->as.ident.name, expr->as.ident.length);
            if (!sym) {
                sema_error(sema, expr->line, expr->column,
                           "Undeclared identifier '%.*s'",
                           expr->as.ident.length, expr->as.ident.name);
                return TYPE_ERROR;
            }
            if (sym->kind == SYMBOL_FN) {
                sema_error(sema, expr->line, expr->column,
                           "'%.*s' is a function, not a variable",
                           expr->as.ident.length, expr->as.ident.name);
                return TYPE_ERROR;
            }
            return sym->type;
        }

        case EXPR_BINARY: {
            TypeKind left = check_expr(sema, expr->as.binary.left);
            TypeKind right = check_expr(sema, expr->as.binary.right);

            if (left == TYPE_ERROR || right == TYPE_ERROR) {
                return TYPE_ERROR;
            }

            if (left != TYPE_INT || right != TYPE_INT) {
                sema_error(sema, expr->line, expr->column,
                           "Binary operator '%s' requires integer operands, found '%s' and '%s'",
                           token_type_name(expr->as.binary.op),
                           type_kind_name(left), type_kind_name(right));
                return TYPE_ERROR;
            }

            return TYPE_INT;
        }
    }

    return TYPE_ERROR;
}

static void check_let_stmt(Sema *sema, const Stmt *stmt) {
    const LetStmt *let = &stmt->as.let_stmt;

    Symbol *existing = scope_lookup_current(sema->current_scope, let->name, let->name_len);
    if (existing) {
        sema_error(sema, stmt->line, stmt->column,
                   "Redeclaration of variable '%.*s' (previously declared at line %d, col %d)",
                   let->name_len, let->name, existing->line, existing->column);
        return;
    }

    TypeKind declared_type = resolve_type_name(let->type_name, let->type_len);
    if (declared_type == TYPE_UNKNOWN) {
        sema_error(sema, stmt->line, stmt->column,
                   "Unknown type '%.*s' for variable '%.*s'",
                   let->type_len, let->type_name, let->name_len, let->name);
        declared_type = TYPE_ERROR;
    } else if (declared_type == TYPE_VOID) {
        sema_error(sema, stmt->line, stmt->column,
                   "Variable '%.*s' cannot have type 'void'",
                   let->name_len, let->name);
        declared_type = TYPE_ERROR;
    }

    if (let->initializer) {
        TypeKind init_type = check_expr(sema, let->initializer);
        if (declared_type != TYPE_ERROR && init_type != TYPE_ERROR) {
            if (declared_type != init_type) {
                sema_error(sema, stmt->line, stmt->column,
                           "Type mismatch in variable declaration '%.*s': declared as '%s', but initialized with '%s'",
                           let->name_len, let->name,
                           type_kind_name(declared_type), type_kind_name(init_type));
            }
        }
    }

    scope_add_symbol(sema->current_scope, let->name, let->name_len,
                     SYMBOL_VAR, declared_type, stmt->line, stmt->column);
}

static void check_print_stmt(Sema *sema, const Stmt *stmt) {
    const PrintStmt *print = &stmt->as.print_stmt;
    TypeKind expr_type = check_expr(sema, print->expr);
    if (expr_type != TYPE_ERROR && expr_type != TYPE_INT) {
        sema_error(sema, stmt->line, stmt->column,
                   "Cannot print expression of type '%s'",
                   type_kind_name(expr_type));
    }
}

static void check_stmt(Sema *sema, const Stmt *stmt) {
    if (!stmt) return;

    switch (stmt->kind) {
        case STMT_LET:
            check_let_stmt(sema, stmt);
            break;
        case STMT_PRINT:
            check_print_stmt(sema, stmt);
            break;
        case STMT_EXPR:
            check_expr(sema, stmt->as.expr_stmt);
            break;
    }
}

void sema_init(Sema *sema) {
    sema->global_scope = scope_new(NULL);
    sema->current_scope = sema->global_scope;
    sema->error_count = 0;
}

void sema_free(Sema *sema) {
    if (sema->global_scope) {
        scope_free(sema->global_scope);
        sema->global_scope = NULL;
        sema->current_scope = NULL;
    }
}

int sema_analyze_program(Sema *sema, const Program *prog) {
    if (!prog) return 0;

    /* Pass 1: Declare all top-level functions */
    for (size_t i = 0; i < prog->count; i++) {
        FunctionDecl *fn = prog->functions[i];
        Symbol *existing = scope_lookup_current(sema->global_scope, fn->name, fn->name_len);
        if (existing) {
            sema_error(sema, 0, 0,
                       "Redefinition of function '%.*s'",
                       fn->name_len, fn->name);
        } else {
            scope_add_symbol(sema->global_scope, fn->name, fn->name_len,
                             SYMBOL_FN, TYPE_VOID, 0, 0);
        }
    }

    /* Pass 2: Check each function body in its own scope */
    int has_main = 0;
    for (size_t i = 0; i < prog->count; i++) {
        FunctionDecl *fn = prog->functions[i];
        if (fn->name_len == 4 && strncmp(fn->name, "main", 4) == 0) {
            has_main = 1;
        }

        Scope *fn_scope = scope_new(sema->global_scope);
        sema->current_scope = fn_scope;

        for (size_t j = 0; j < fn->stmt_count; j++) {
            check_stmt(sema, fn->body[j]);
        }

        sema->current_scope = sema->global_scope;
        scope_free(fn_scope);
    }

    if (!has_main) {
        sema_error(sema, 0, 0, "Program does not define a 'main' function");
    }

    return sema->error_count;
}
