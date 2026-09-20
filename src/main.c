#include <stdio.h>
#include <stdlib.h>
#include "token.h"
void lexer_init(const char *text); Token next_token(void);
int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s <source.vxc>\n", argv[0]); return 1; }
    FILE *f = fopen(argv[1], "rb"); if (!f) { perror(argv[1]); return 1; }
    fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
    char *text = malloc((size_t)n + 1); if (!text) return 1;
    fread(text, 1, (size_t)n, f); text[n] = '\0'; fclose(f);
    lexer_init(text); Token t;
    do { t = next_token(); printf("%-12s line=%d col=%d lexeme=\"%.*s\"\n", token_type_name(t.type), t.line, t.column, t.length, t.start); } while (t.type != TOK_EOF);
    free(text); return 0;
}
