#ifndef TABELA_DE_SIMBOLOS_H
#define TABELA_DE_SIMBOLOS_H

#define TAMANHO_TABELA 256

typedef struct Simbolo {
    char nome[64];
    int tipo;
    int posicao;
    int quantidadeParametros;
    int tipoRetorno;
    struct Simbolo* proximo;
} Simbolo;

typedef struct TabelaDeSimbolos {
    Simbolo* simbolos[TAMANHO_TABELA];
} TabelaDeSimbolos;

typedef struct PilhaDeTabelas {
    TabelaDeSimbolos* tabelas[100];
    int topo;
} PilhaDeTabelas;

// Funções da Tabela
TabelaDeSimbolos* criarTabela();
void liberarTabela(TabelaDeSimbolos* tabela);
void inserirVariavel(TabelaDeSimbolos* tabela, const char* nome, int tipo, int posicao);
void inserirFuncao(TabelaDeSimbolos* tabela, const char* nome, int quantidadeParametros, int tipoRetorno);
void inserirParametro(TabelaDeSimbolos* tabela, const char* nome, int tipo, int posicao, Simbolo* simboloFuncao);
Simbolo* buscarSimbolo(TabelaDeSimbolos* tabela, const char* nome);

// Funções da Pilha
void inicializarPilha(PilhaDeTabelas* pilha);
void empilhar(PilhaDeTabelas* pilha);
void desempilhar(PilhaDeTabelas* pilha);
Simbolo* buscarNaPilha(PilhaDeTabelas* pilha, const char* nome);
void inserirVariavelNaPilha(PilhaDeTabelas* pilha, const char* nome, int tipo, int posicao);
void inserirFuncaoNaPilha(PilhaDeTabelas* pilha, const char* nome, int quantidadeParametros, int tipoRetorno);
void inserirParametroNaPilha(PilhaDeTabelas* pilha, const char* nome, int tipo, int posicao, Simbolo* simboloFuncao);
void imprimirPilha(PilhaDeTabelas* pilha);

#endif
