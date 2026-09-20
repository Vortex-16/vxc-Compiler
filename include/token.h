#ifndef VXC_TOKEN_H
#define VXC_TOKEN_H

typedef enum {
    TOK_EOF, TOK_ERROR, TOK_IDENTIFIER, TOK_INT_LITERAL,
    TOK_FN, TOK_LET, TOK_INT, TOK_PRINT,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_EQUAL,
    TOK_COLON, TOK_SEMICOLON, TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACE, TOK_RBRACE
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
    int line;
    int column;
} Token;

const char *token_type_name(TokenType type);

#endif
