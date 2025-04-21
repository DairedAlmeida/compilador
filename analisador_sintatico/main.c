#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "goianinha.tab.h"

extern FILE *yyin;
extern int yyparse();
extern int yylineno;
extern char *yytext;

/* Variável global para controle de verbosidade */
int debug_mode = 0;

/* Funções de erro */
void yyerror_lex(const char *s) {
    fprintf(stderr, "ERRO LÉXICO: %s - Linha %d\n", s, yylineno);
    if (debug_mode && yytext) {
        fprintf(stderr, "Token problemático: '%s'\n", yytext);
    }
}

void yyerror(const char *s) {
    fprintf(stderr, "ERRO SINTÁTICO: %s - Linha %d\n", s, yylineno);
    if (debug_mode && yytext) {
        fprintf(stderr, "Token inesperado: '%s'\n", yytext);
    }
}

/* Implementação de main */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.g> [-d]\n", argv[0]);
        return 1;
    }

    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        debug_mode = 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    int result = yyparse();  // Chamada de análise sintática
    
    if (result == 0) {
        printf("Compilação concluída com sucesso!\n");
    } else {
        fprintf(stderr, "Compilação abortada com erros\n");
    }

    fclose(yyin);
    return result;
}