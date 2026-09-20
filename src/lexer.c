#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "token.h"

static const char *source;
static int index_pos, line_no, column_no;

static int peek(void) { return source[index_pos]; }
static int advance_char(void) { int c = source[index_pos++]; if (c == '\n') { line_no++; column_no = 1; } else column_no++; return c; }
static void skip_space(void) {
    for (;;) {
        int c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { advance_char(); continue; }
        if (c == '/' && source[index_pos + 1] == '/') { while (peek() && peek() != '\n') advance_char(); continue; }
        break;
    }
}
static int is_ident_start(int c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_ident_part(int c) { return isalnum((unsigned char)c) || c == '_'; }
static Token make_token(TokenType t, int start, int line, int col) { Token x = {t, source + start, index_pos - start, line, col}; return x; }
static Token error_token(const char *msg, int line, int col) { Token x = {TOK_ERROR, msg, (int)strlen(msg), line, col}; return x; }
static TokenType keyword(const char *s, int n) {
    if (n == 2 && strncmp(s, "fn", 2) == 0) return TOK_FN;
    if (n == 3 && strncmp(s, "let", 3) == 0) return TOK_LET;
    if (n == 3 && strncmp(s, "int", 3) == 0) return TOK_INT;
    if (n == 5 && strncmp(s, "print", 5) == 0) return TOK_PRINT;
    return TOK_IDENTIFIER;
}

void lexer_init(const char *text) { source = text; index_pos = 0; line_no = 1; column_no = 1; }
Token next_token(void) {
    skip_space();
    int start = index_pos, line = line_no, col = column_no, c = peek();
    if (!c) return make_token(TOK_EOF, start, line, col);
    if (is_ident_start(c)) { advance_char(); while (is_ident_part(peek())) advance_char(); return make_token(keyword(source + start, index_pos - start), start, line, col); }
    if (isdigit((unsigned char)c)) { advance_char(); while (isdigit((unsigned char)peek())) advance_char(); return make_token(TOK_INT_LITERAL, start, line, col); }
    advance_char();
    switch (c) {
        case '+': return make_token(TOK_PLUS,start,line,col); case '-': return make_token(TOK_MINUS,start,line,col);
        case '*': return make_token(TOK_STAR,start,line,col); case '/': return make_token(TOK_SLASH,start,line,col);
        case '=': return make_token(TOK_EQUAL,start,line,col); case ':': return make_token(TOK_COLON,start,line,col);
        case ';': return make_token(TOK_SEMICOLON,start,line,col); case '(': return make_token(TOK_LPAREN,start,line,col);
        case ')': return make_token(TOK_RPAREN,start,line,col); case '{': return make_token(TOK_LBRACE,start,line,col);
        case '}': return make_token(TOK_RBRACE,start,line,col);
        default: return error_token("Unexpected character", line, col);
    }
}

const char *token_type_name(TokenType t) {
    switch(t) {
        case TOK_EOF:return "EOF"; case TOK_ERROR:return "ERROR"; case TOK_IDENTIFIER:return "IDENTIFIER";
        case TOK_INT_LITERAL:return "INT_LITERAL"; case TOK_FN:return "FN"; case TOK_LET:return "LET";
        case TOK_INT:return "INT"; case TOK_PRINT:return "PRINT"; case TOK_PLUS:return "PLUS";
        case TOK_MINUS:return "MINUS"; case TOK_STAR:return "STAR"; case TOK_SLASH:return "SLASH";
        case TOK_EQUAL:return "EQUAL"; case TOK_COLON:return "COLON"; case TOK_SEMICOLON:return "SEMICOLON";
        case TOK_LPAREN:return "LPAREN"; case TOK_RPAREN:return "RPAREN"; case TOK_LBRACE:return "LBRACE";
        case TOK_RBRACE:return "RBRACE"; default:return "UNKNOWN";
    }
}
