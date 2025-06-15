#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "goianinha.tab.h"
#include "arvore.h"
#include "tabela_simbolos.h"
#include "analise_semantica.h"

// Variáveis globais
extern FILE *yyin;
extern int yylineno;
extern char *yytext;

// ##################################################################
// ## CORREÇÃO: Definição da variável global.                      ##
// ## Ela é declarada como 'extern' em goianinha.y e definida aqui.##
// ##################################################################
No* raiz_arvore = NULL; 

int debug_mode = 0;

void yyerror(const char *s) {
    fprintf(stderr, "ERRO SINTÁTICO: %s na linha %d, próximo a '%s'\n", s, yylineno, yytext);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.g> [-d]\n", argv[0]);
        return 1;
    }

    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        debug_mode = 1;
        printf("Modo de depuração ativado.\n");
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    // Inicia a análise sintática
    int result = yyparse();
    fclose(yyin);

    if (result == 0) {
        printf("Análise sintática concluída com sucesso!\n\n");

        if (debug_mode) {
            printf("--- Árvore Sintática Abstrata ---\n");
            imprime_arvore(raiz_arvore, 0);
            printf("---------------------------------\n\n");
        }

        printf("Iniciando análise semântica...\n");
        int erros_semanticos = analisar(raiz_arvore);

        if (erros_semanticos == 0) {
            printf("Análise semântica concluída sem erros!\n");
            printf("\nCompilação concluída com sucesso!\n");
        } else {
            fprintf(stderr, "\nCompilação abortada com %d erro(s) semântico(s).\n", erros_semanticos);
            result = 1;
        }

    } else {
        fprintf(stderr, "\nCompilação abortada com erros sintáticos.\n");
    }

    // Libera a memória da árvore ao final
    //imprime_arvore(raiz_arvore,10);
    libera_arvore(raiz_arvore);

    return result;
}