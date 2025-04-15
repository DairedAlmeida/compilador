#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela_de_simbolos.h"

//TABELA

unsigned int hash(const char* str) {
    unsigned int hash = 0;
    while (*str) hash = (hash << 5) + *str++;
    return hash % TAMANHO_TABELA;
}

TabelaDeSimbolos* criarTabela() {
    TabelaDeSimbolos* tabela = malloc(sizeof(TabelaDeSimbolos));
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        tabela->simbolos[i] = NULL;
    }
    return tabela;
}

void liberarTabela(TabelaDeSimbolos* tabela) {
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        Simbolo* atual = tabela->simbolos[i];
        while (atual) {
            Simbolo* temp = atual;
            atual = atual->proximo;
            free(temp);
        }
    }
    free(tabela);
}

void inserirSimbolo(TabelaDeSimbolos* tabela, Simbolo simbolo) {
    unsigned int idx = hash(simbolo.nome);
    Simbolo* novo = malloc(sizeof(Simbolo));
    *novo = simbolo;
    novo->proximo = tabela->simbolos[idx];
    tabela->simbolos[idx] = novo;
}

void inserirVariavel(TabelaDeSimbolos* tabela, const char* nome, int tipo, int posicao) {
    Simbolo simbolo = {.tipo = tipo, .posicao = posicao, .quantidadeParametros = 0, .tipoRetorno = 0};
    strncpy(simbolo.nome, nome, 64);
    inserirSimbolo(tabela, simbolo);
}

void inserirFuncao(TabelaDeSimbolos* tabela, const char* nome, int quantidadeParametros, int tipoRetorno) {
    Simbolo simbolo = {.tipo = 0, .posicao = 0, .quantidadeParametros = quantidadeParametros, .tipoRetorno = tipoRetorno};
    strncpy(simbolo.nome, nome, 64);
    inserirSimbolo(tabela, simbolo);
}

void inserirParametro(TabelaDeSimbolos* tabela, const char* nome, int tipo, int posicao, Simbolo* simboloFuncao) {
    Simbolo simbolo = {.tipo = tipo, .posicao = posicao, .quantidadeParametros = 0, .tipoRetorno = 0};
    strncpy(simbolo.nome, nome, 64);
    inserirSimbolo(tabela, simbolo);
}

Simbolo* buscarSimbolo(TabelaDeSimbolos* tabela, const char* nome) {
    unsigned int idx = hash(nome);
    Simbolo* atual = tabela->simbolos[idx];
    while (atual) {
        if (strcmp(atual->nome, nome) == 0) return atual;
        atual = atual->proximo;
    }
    return NULL;
}

// PILHA

void inicializarPilha(PilhaDeTabelas* pilha) {
    pilha->topo = -1;
}

void empilhar(PilhaDeTabelas* pilha) {
    if (pilha->topo < 99) {
        pilha->tabelas[++pilha->topo] = criarTabela();
    }
}

void desempilhar(PilhaDeTabelas* pilha) {
    if (pilha->topo >= 0) {
        liberarTabela(pilha->tabelas[pilha->topo--]);
    }
}

Simbolo* buscarNaPilha(PilhaDeTabelas* pilha, const char* nome) {
    for (int i = pilha->topo; i >= 0; i--) {
        Simbolo* s = buscarSimbolo(pilha->tabelas[i], nome);
        if (s) return s;
    }
    return NULL;
}

void inserirVariavelNaPilha(PilhaDeTabelas* pilha, const char* nome, int tipo, int posicao) {
    if (pilha->topo >= 0) {
        inserirVariavel(pilha->tabelas[pilha->topo], nome, tipo, posicao);
    }
}

void inserirFuncaoNaPilha(PilhaDeTabelas* pilha, const char* nome, int quantidadeParametros, int tipoRetorno) {
    if (pilha->topo >= 0) {
        inserirFuncao(pilha->tabelas[pilha->topo], nome, quantidadeParametros, tipoRetorno);
    }
}

void inserirParametroNaPilha(PilhaDeTabelas* pilha, const char* nome, int tipo, int posicao, Simbolo* simboloFuncao) {
    if (pilha->topo >= 0) {
        inserirParametro(pilha->tabelas[pilha->topo], nome, tipo, posicao, simboloFuncao);
    }
}

void imprimirPilha(PilhaDeTabelas* pilha) {
    for (int i = 0; i <= pilha->topo; i++) {
        printf("Escopo Nível %d:\n", i);
        for (int j = 0; j < TAMANHO_TABELA; j++) {
            Simbolo* atual = pilha->tabelas[i]->simbolos[j];
            while (atual) {
                printf("  %s (Tipo: %d, Posição: %d", atual->nome, atual->tipo, atual->posicao);
                if (atual->quantidadeParametros > 0) {
                    printf(", Parâmetros: %d, Tipo Retorno: %d", atual->quantidadeParametros, atual->tipoRetorno);
                }
                printf(")\n");
                atual = atual->proximo;
            }
        }
    }
}