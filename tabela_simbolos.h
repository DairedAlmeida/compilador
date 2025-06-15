#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include "arvore.h" // Inclui para usar o 'TipoDado'

#define TAMANHO_TABELA 256

// Categoria de um símbolo (variável, função, etc.)
typedef enum {
    CAT_VARIAVEL,
    CAT_FUNCAO,
    CAT_PARAMETRO
} CategoriaSimbolo;

// Estrutura para armazenar informações sobre um símbolo
typedef struct Simbolo {
    char nome[100];
    CategoriaSimbolo categoria;
    TipoDado tipo_dado;
    int linha;
    int escopo;

    // Para funções, armazena a lista de parâmetros
    struct No* params; 
    int num_params;

    struct Simbolo* proximo; // Para tratamento de colisões na hash table
} Simbolo;

// A tabela de símbolos é um array de ponteiros para Símbolo (hash table)
typedef struct TabelaDeSimbolos {
    Simbolo* simbolos[TAMANHO_TABELA];
} TabelaDeSimbolos;

// Pilha de tabelas para gerenciar escopos
typedef struct PilhaDeTabelas {
    TabelaDeSimbolos* tabelas[100];
    int topo;
} PilhaDeTabelas;

// --- Funções da Tabela ---
void inicializar_tabela(TabelaDeSimbolos* tabela);
void inserir_simbolo(TabelaDeSimbolos* tabela, Simbolo s);
Simbolo* buscar_simbolo(TabelaDeSimbolos* tabela, const char* nome);
void liberar_tabela(TabelaDeSimbolos* tabela);

// --- Funções da Pilha de Tabelas (Escopos) ---
void inicializar_pilha(PilhaDeTabelas* pilha);
void empilhar(PilhaDeTabelas* pilha);
void desempilhar(PilhaDeTabelas* pilha);
TabelaDeSimbolos* topo_pilha(PilhaDeTabelas* pilha);
Simbolo* buscar_em_todos_escopos(PilhaDeTabelas* pilha, const char* nome);
Simbolo* buscar_no_escopo_atual(PilhaDeTabelas* pilha, const char* nome);
void inserir_na_pilha(PilhaDeTabelas* pilha, Simbolo s);

#endif // TABELA_SIMBOLOS_H