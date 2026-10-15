#ifndef VXC_SEMA_H
#define VXC_SEMA_H

#include <stddef.h>
#include "ast.h"

typedef enum {
    TYPE_UNKNOWN,
    TYPE_ERROR,
    TYPE_VOID,
    TYPE_INT
} TypeKind;

const char *type_kind_name(TypeKind kind);

typedef enum {
    SYMBOL_VAR,
    SYMBOL_FN
} SymbolKind;

typedef struct Symbol {
    const char *name;
    int name_len;
    SymbolKind kind;
    TypeKind type;
    int line;
    int column;
    struct Symbol *next;
} Symbol;

typedef struct Scope {
    struct Scope *parent;
    Symbol *symbols;
} Scope;

typedef struct {
    Scope *current_scope;
    Scope *global_scope;
    int error_count;
} Sema;

void sema_init(Sema *sema);
void sema_free(Sema *sema);

/* Analyzes the program AST. Returns the number of semantic errors found (0 on success). */
int sema_analyze_program(Sema *sema, const Program *prog);

#endif
