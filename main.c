#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "goianinha.tab.h" // Arquivo gerado pelo Bison/Yacc, contém as definições dos tokens.
#include "arvore.h"       // Inclui a definição da Árvore Sintática Abstrata.
#include "tabela_simbolos.h" // Inclui a definição da Tabela de Símbolos.
#include "analise_semantica.h" // Inclui a função principal da análise semântica.

// --- Variáveis Globais Externas ---
// Estas variáveis são definidas e gerenciadas pelo analisador léxico (Flex/Lex)
// e pelo analisador sintático (Bison/Yacc).

extern FILE *yyin;     // Ponteiro para o arquivo de entrada que o Lexer irá ler.
extern int yylineno;   // Armazena o número da linha atual no arquivo de entrada.
extern char *yytext;   // Ponteiro para o texto do token (lexema) mais recente encontrado.

// --- Definição de Variável Global ---
// A variável 'raiz_arvore' é declarada como 'extern' no arquivo do parser (goianinha.y, não fornecido),
// o que significa que o parser espera que ela seja definida em outro lugar.
// Aqui, nós a definimos e inicializamos. Ela guardará a raiz da ASA construída pelo parser.
No* raiz_arvore = NULL;

// Variável para controlar o modo de depuração.
int debug_mode = 0;

// Função de erro exigida pelo Yacc/Bison.
// É chamada automaticamente pelo parser (`yyparse`) quando encontra um erro de sintaxe.
void yyerror(const char *s) {
    // Imprime uma mensagem de erro formatada, incluindo a mensagem do parser,
    // a linha do erro e o token que causou o problema.
    fprintf(stderr, "ERRO SINTÁTICO: %s na linha %d, próximo a '%s'\n", s, yylineno, yytext);
}

// Função principal do programa.
int main(int argc, char **argv) {
    // Verifica se o usuário forneceu o nome do arquivo de entrada.
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.g> [-d]\n", argv[0]);
        return 1; // Retorna 1 para indicar erro.
    }

    // Verifica se o argumento opcional "-d" (debug) foi fornecido.
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        debug_mode = 1;
        printf("Modo de depuração ativado.\n");
    }

    // Abre o arquivo de código-fonte fornecido pelo usuário em modo de leitura ("r").
    yyin = fopen(argv[1], "r");
    // Verifica se o arquivo foi aberto com sucesso.
    if (!yyin) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    // --- Fases do Compilador ---

    // 1. Análise Léxica e Sintática
    // A chamada `yyparse()` inicia o processo. O parser (Bison) chama o lexer (Flex)
    // para obter tokens e constrói a Árvore Sintática Abstrata, cuja raiz é armazenada em 'raiz_arvore'.
    printf("Iniciando análise léxica e sintática...\n");
    int result = yyparse();
    fclose(yyin); // Fecha o arquivo de entrada após a análise.

    // Verifica o resultado da análise sintática.
    if (result == 0) { // 0 significa sucesso.
        printf("Análise léxica e sintática concluída com sucesso!\n\n");

        // Se o modo de depuração estiver ativo, imprime a ASA.
        if (debug_mode) {
            printf("--- Árvore Sintática Abstrata ---\n");
            imprime_arvore(raiz_arvore, 0);
            printf("---------------------------------\n\n");
        }

        // 2. Análise Semântica
        printf("Iniciando análise semântica...\n");
        // Chama a função principal do analisador semântico, passando a ASA.
        int erros_semanticos = analisar(raiz_arvore);

        // Verifica o resultado da análise semântica.
        if (erros_semanticos == 0) { // 0 significa sucesso.
            printf("Análise semântica concluída sem erros!\n");
            printf("\nCompilação concluída com sucesso!\n");
        } else {
            // Embora o analisador semântico já aborte no primeiro erro, esta mensagem seria
            // mostrada caso ele fosse modificado para contar os erros em vez de abortar.
            fprintf(stderr, "\nCompilação abortada com %d erro(s) semântico(s).\n", erros_semanticos);
            result = 1; // Define o status de saída como erro.
        }

    } else { // Se `yyparse` retornou um valor diferente de 0, houve erros sintáticos.
        fprintf(stderr, "\nCompilação abortada com erros sintáticos.\n");
    }

    // 3. Liberação de Memória
    // É crucial liberar a memória alocada para a ASA para evitar vazamentos de memória (memory leaks).
    libera_arvore(raiz_arvore);

    // Retorna o status final da compilação (0 para sucesso, 1 para falha).
    return result;
}