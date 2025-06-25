#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "goianinha.tab.h" // Arquivo gerado pelo Bison/Yacc, contém as definições dos tokens.
#include "arvore.h"       // Inclui a definição da Árvore Sintática Abstrata.
#include "tabela_simbolos.h" // Inclui a definição da Tabela de Símbolos.
#include "analise_semantica.h" // Inclui a função principal da análise semântica.
#include "geracao_codigo.h"

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
int debug_mode = 1;

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
    fclose(yyin); 

    if (result == 0) {
        printf("Análise léxica e sintática concluída com sucesso!\n\n");

        if (debug_mode) {
            printf("--- Árvore Sintática Abstrata Original ---\n");
            imprime_arvore(raiz_arvore, 0);
            printf("----------------------------------------\n\n");
        }

        // --- Cria cópias da árvore para as próximas fases ---
        No* arvore_para_semantica = copia_arvore(raiz_arvore);
        No* arvore_para_geracao = copia_arvore(raiz_arvore);

        // 2. Análise Semântica (usa a primeira cópia)
        printf("Iniciando análise semântica...\n");
        int erros_semanticos = analisar(arvore_para_semantica);

        if (erros_semanticos == 0) {
            printf("Análise semântica concluída sem erros!\n\n");

            // 3. Geração de Código (usa a segunda cópia, intacta)
            printf("Iniciando geração de código...\n");
            char nome_arquivo_saida[256];
            strcpy(nome_arquivo_saida, argv[1]);
            char *ponto = strrchr(nome_arquivo_saida, '.');
            if (ponto) {
                strcpy(ponto, ".asm");
            } else {
                strcat(nome_arquivo_saida, ".asm");
            }
            gerar_codigo(arvore_para_geracao, nome_arquivo_saida);

            printf("\nCompilação concluída com sucesso!\n");
        } else {
            fprintf(stderr, "\nCompilação abortada com erro(s) semântico(s).\n");
            result = 1; 
        }

        // Libera a memória de TODAS as árvores criadas
        libera_arvore(arvore_para_semantica);
        libera_arvore(arvore_para_geracao);

    } else { 
        fprintf(stderr, "\nCompilação abortada com erros sintáticos.\n");
    }

    // Libera a memória da árvore original
    libera_arvore(raiz_arvore);

    return result;
}