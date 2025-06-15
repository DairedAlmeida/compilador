#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arvore.h"

// Função para criar um novo nó da árvore
No* cria_no(TipoNo tipo_no, int linha, char* lexema) {
    No* novo_no = (No*) malloc(sizeof(No));
    if (!novo_no) {
        printf("Erro: Falha de alocação de memória para novo nó.\n");
        exit(1);
    }

    novo_no->tipo_no = tipo_no;
    novo_no->linha = linha;
    novo_no->tipo_dado = TIPO_INDEFINIDO; // Tipo será definido na análise semântica
    
    if (lexema != NULL) {
        novo_no->lexema = strdup(lexema);
    } else {
        novo_no->lexema = NULL;
    }

    novo_no->filho1 = NULL;
    novo_no->filho2 = NULL;
    novo_no->filho3 = NULL;
    novo_no->filho4 = NULL;
    novo_no->proximo = NULL;

    return novo_no;
}

// Adiciona um nó 'filho' à lista de filhos de um nó 'pai'
// Útil para estruturas como corpo de função, if, while, etc.
void adiciona_filho(No* pai, No* filho) {
    if (!pai || !filho) return;

    if (!pai->filho1) pai->filho1 = filho;
    else if (!pai->filho2) pai->filho2 = filho;
    else if (!pai->filho3) pai->filho3 = filho;
    else if (!pai->filho4) pai->filho4 = filho;
}

// Função recursiva para imprimir a árvore (para depuração)
void imprime_arvore(No* no, int profundidade) {
    if (no == NULL) {
        return;
    }

    // Imprime identação para visualização da hierarquia
    for (int i = 0; i < profundidade; i++) {
        printf("  ");
    }

    // Imprime o tipo do nó e o lexema (se houver)
    switch(no->tipo_no) {
        case NO_PROGRAMA: printf("Programa\n"); break;
        case NO_LISTA_DECLARACOES: printf("ListaDeclaracoes\n"); break;
        case NO_DECL_VAR: printf("DeclaracaoVariavel (Tipo: %s)\n", no->lexema); break;
        case NO_DECL_FUNCAO: printf("DeclaracaoFuncao: %s\n", no->lexema); break;
        case NO_LISTA_PARAM: printf("ListaParametros\n"); break;
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
    
    // Chamadas recursivas para os filhos e para o próximo
    imprime_arvore(no->filho1, profundidade + 1);
    imprime_arvore(no->filho2, profundidade + 1);
    imprime_arvore(no->filho3, profundidade + 1);
    imprime_arvore(no->filho4, profundidade + 1);
    imprime_arvore(no->proximo, profundidade);
}


// Libera a memória alocada para a árvore
void libera_arvore(No* raiz) {
    if (raiz == NULL) return;

    libera_arvore(raiz->filho1);
    libera_arvore(raiz->filho2);
    libera_arvore(raiz->filho3);
    libera_arvore(raiz->filho4);
    libera_arvore(raiz->proximo);

    if (raiz->lexema) {
        free(raiz->lexema);
    }
    free(raiz);
}