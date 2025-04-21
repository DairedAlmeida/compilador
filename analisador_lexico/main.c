#include <stdio.h>
#include <stdlib.h>

extern FILE *yyin;
extern int yylex();
extern char *yytext;
extern int yylineno;
extern char erro_msg[256];
extern int linha;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s <arquivo>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    int token;
    while ((token = yylex()) != 0) {
        if (token < 0) {
            printf("%s\n", erro_msg);
        } else {
            printf("Encontrado o lexema '%s' pertencente ao token de código %d na linha %d\n", yytext, token, linha);
        }
    }

    fclose(yyin);
    return 0;
}
