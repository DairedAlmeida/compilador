#include <stdio.h>      // Para funções de entrada e saída, como printf.
#include <stdlib.h>     // Para alocação de memória (malloc, free) e para a função exit.
#include <string.h>     // Para funções de manipulação de strings, como strdup.
#include "arvore.h"     // Inclui as definições das estruturas e enums que serão usadas aqui.

// Função para criar um novo nó da árvore.
// Esta é uma função "fábrica" que simplifica a criação de nós.
No* cria_no(TipoNo tipo_no, int linha, char* lexema) {
    // Aloca memória do tamanho de uma estrutura 'No'.
    No* novo_no = (No*) malloc(sizeof(No));
    // Verifica se a alocação de memória foi bem-sucedida. Se não, o programa é encerrado.
    if (!novo_no) {
        printf("Erro: Falha de alocação de memória para novo nó.\n");
        exit(1);
    }

    // Inicializa os campos do nó com os valores passados como parâmetros.
    novo_no->tipo_no = tipo_no;
    novo_no->linha = linha;
    // O tipo de dado é inicialmente indefinido. A análise semântica irá preencher este campo.
    novo_no->tipo_dado = TIPO_INDEFINIDO;

    // Se um lexema foi fornecido (ex: um nome de variável ou um número)...
    if (lexema != NULL) {
        // `strdup` cria uma cópia da string 'lexema' em uma nova área de memória.
        // Isso é crucial porque o buffer original do analisador léxico (yytext) pode ser sobrescrito.
        novo_no->lexema = strdup(lexema);
    } else {
        // Se não houver lexema, o ponteiro é definido como NULL.
        novo_no->lexema = NULL;
    }

    // Inicializa todos os ponteiros de filhos e do próximo nó como NULL.
    // Os filhos serão adicionados posteriormente conforme a árvore é construída.
    novo_no->filho1 = NULL;
    novo_no->filho2 = NULL;
    novo_no->filho3 = NULL;
    novo_no->filho4 = NULL;
    novo_no->proximo = NULL;

    // Retorna o ponteiro para o nó recém-criado.
    return novo_no;
}

// Adiciona um nó 'filho' à lista de filhos de um nó 'pai'.
// É uma função de conveniência para simplificar a construção da árvore pelo parser.
void adiciona_filho(No* pai, No* filho) {
    // Verifica se os ponteiros são válidos para evitar erros de segmentação.
    if (!pai || !filho) return;

    // Procura o primeiro ponteiro de filho disponível (de filho1 a filho4) e o atribui.
    if (!pai->filho1) pai->filho1 = filho;
    else if (!pai->filho2) pai->filho2 = filho;
    else if (!pai->filho3) pai->filho3 = filho;
    else if (!pai->filho4) pai->filho4 = filho;
    // Nota: Se todos os 4 filhos já estiverem ocupados, este código não faz nada.
    // Isso assume que a gramática da linguagem não precisa de mais de 4 filhos diretos.
}

// Função recursiva para imprimir a árvore (usada para depuração).
void imprime_arvore(No* no, int profundidade) {
    // Condição de parada da recursão: se o nó é nulo, não há nada a fazer.
    if (no == NULL) {
        return;
    }

    // Imprime a indentação para visualizar a hierarquia da árvore.
    // A cada nível de profundidade, dois espaços são adicionados.
    for (int i = 0; i < profundidade; i++) {
        printf("  ");
    }

    // Usa um 'switch' no tipo do nó para imprimir uma descrição apropriada.
    switch(no->tipo_no) {
        case NO_PROGRAMA: printf("Programa\n"); break;
        case NO_LISTA_DECLARACOES: printf("ListaDeclaracoes\n"); break;
        case NO_DECL_VAR: printf("DeclaracaoVariavel (Tipo: %s)\n", no->lexema); break;
        case NO_DECL_FUNCAO: printf("DeclaracaoFuncao: %s\n", no->lexema); break;
        case NO_LISTA_PARAM: printf("ListaParametros\n"); break;
        // Para um parâmetro, o tipo está no nó pai (lexema) e o nome no filho1.
        case NO_PARAM: printf("Parametro: %s (Tipo: %s)\n", no->filho1->lexema, no->lexema); break;
        case NO_BLOCO: printf("Bloco\n"); break;
        case NO_LISTA_COMANDOS: printf("ListaComandos\n"); break;
        case NO_IF: printf("If\n"); break;
        case NO_WHILE: printf("While\n"); break;
        case NO_ATRIBUICAO: printf("Atribuicao\n"); break;
        case NO_RETORNO: printf("Retorno\n"); break;
        case NO_CHAMADA_FUNCAO: printf("ChamadaFuncao: %s\n", no->lexema); break;
        case NO_LISTA_ARGS: printf("ListaArgumentos\n"); break;
        case NO_NEGACAO: printf("Negacao\n"); break;
        case NO_OP_LOGICO: printf("OpLogico: %s\n", no->lexema); break;
        case NO_OP_RELACIONAL: printf("OpRelacional: %s\n", no->lexema); break;
        case NO_OP_ARITMETICO: printf("OpAritmetico: %s\n", no->lexema); break;
        case NO_IDENTIFICADOR: printf("ID: %s\n", no->lexema); break;
        case NO_CONST_INT: printf("ConstInt: %s\n", no->lexema); break;
        case NO_CONST_CAR: printf("ConstCar: %s\n", no->lexema); break;
        default: printf("Nó desconhecido\n"); break;
    }

    // Chamadas recursivas para percorrer toda a árvore.
    // Primeiro, visita todos os filhos, aumentando o nível de profundidade.
    imprime_arvore(no->filho1, profundidade + 1);
    imprime_arvore(no->filho2, profundidade + 1);
    imprime_arvore(no->filho3, profundidade + 1);
    imprime_arvore(no->filho4, profundidade + 1);
    // Depois, visita o "irmão" (nó 'proximo'), mantendo o mesmo nível de profundidade.
    imprime_arvore(no->proximo, profundidade);
}


// Libera a memória alocada para a árvore de forma recursiva.
void libera_arvore(No* raiz) {
    // Condição de parada da recursão.
    if (raiz == NULL) return;

    // Usa um percurso em pós-ordem: primeiro libera os descendentes, depois o próprio nó.
    // Isso evita "ponteiros perdidos" (dangling pointers).
    libera_arvore(raiz->filho1);
    libera_arvore(raiz->filho2);
    libera_arvore(raiz->filho3);
    libera_arvore(raiz->filho4);
    libera_arvore(raiz->proximo);

    // Se o nó continha um lexema (copiado com strdup), a memória dele também precisa ser liberada.
    if (raiz->lexema) {
        free(raiz->lexema);
    }
    // Finalmente, libera a memória da própria estrutura do nó.
    free(raiz);
}