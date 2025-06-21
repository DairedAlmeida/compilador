#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

// Inclui o cabeçalho da árvore, pois a estrutura do símbolo precisa conhecer
// os tipos de dados (`TipoDado`) e também pode apontar para nós da árvore (`struct No* params`).
#include "arvore.h"

// Define o tamanho da nossa tabela de hash. 256 é um valor comum.
// A tabela de hash é usada para armazenar e buscar símbolos de forma eficiente.
#define TAMANHO_TABELA 256

// Enumeração para a categoria de um símbolo. Ajuda a distinguir
// entre uma variável e uma função que possam ter o mesmo nome.
typedef enum {
    CAT_VARIAVEL,     // O símbolo é uma variável.
    CAT_FUNCAO,       // O símbolo é uma função.
    CAT_PARAMETRO     // O símbolo é um parâmetro de função.
} CategoriaSimbolo;

// Estrutura que armazena todas as informações relevantes sobre um símbolo (identificador).
typedef struct Simbolo {
    char nome[100];             // O nome do identificador (ex: "minhaVariavel").
    CategoriaSimbolo categoria; // A categoria do símbolo (CAT_VARIAVEL, CAT_FUNCAO, etc.).
    TipoDado tipo_dado;         // O tipo de dado associado ao símbolo (TIPO_INT, TIPO_VOID, etc.).
    int linha;                  // A linha onde o símbolo foi declarado.
    int escopo;                 // O nível de escopo onde foi declarado (0 para global, 1 para o primeiro nível de aninhamento, etc.).

    // Campos específicos para funções:
    struct No* params;          // Ponteiro para a lista de nós de parâmetros na ASA. Usado para verificar os tipos dos argumentos na chamada da função.
    int num_params;             // O número de parâmetros que a função espera.

    // Ponteiro para o próximo símbolo em caso de colisão na tabela de hash.
    // A nossa tabela usa uma técnica chamada "encadeamento separado" (separate chaining).
    struct Simbolo* proximo;
} Simbolo;

// Estrutura da Tabela de Símbolos. É basicamente uma tabela de hash.
// É um array de ponteiros para Símbolo. Cada posição do array é a "cabeça" de uma lista ligada de símbolos.
typedef struct TabelaDeSimbolos {
    Simbolo* simbolos[TAMANHO_TABELA];
} TabelaDeSimbolos;

// Estrutura da Pilha de Tabelas. Essencial para o gerenciamento de escopos.
// Quando entramos em um novo escopo (como uma função ou um bloco), uma nova tabela é "empilhada".
// Quando saímos do escopo, ela é "desempilhada".
typedef struct PilhaDeTabelas {
    TabelaDeSimbolos* tabelas[100]; // Um array de ponteiros para tabelas de símbolos. Suporta até 100 níveis de escopo aninhados.
    int topo;                       // Um índice que aponta para o topo da pilha (o escopo atual).
} PilhaDeTabelas;


// --- Assinaturas das Funções da Tabela de Símbolos (operações em uma única tabela) ---
void inicializar_tabela(TabelaDeSimbolos* tabela);
void inserir_simbolo(TabelaDeSimbolos* tabela, Simbolo s);
Simbolo* buscar_simbolo(TabelaDeSimbolos* tabela, const char* nome);
void liberar_tabela(TabelaDeSimbolos* tabela);


// --- Assinaturas das Funções da Pilha de Tabelas (Gerenciamento de Escopos) ---
void inicializar_pilha(PilhaDeTabelas* pilha);
void empilhar(PilhaDeTabelas* pilha); // Cria um novo escopo.
void desempilhar(PilhaDeTabelas* pilha); // Fecha o escopo atual.
TabelaDeSimbolos* topo_pilha(PilhaDeTabelas* pilha); // Retorna a tabela do escopo atual.

// Funções de busca que operam na pilha de escopos:
Simbolo* buscar_em_todos_escopos(PilhaDeTabelas* pilha, const char* nome); // Procura do escopo atual para o global.
Simbolo* buscar_no_escopo_atual(PilhaDeTabelas* pilha, const char* nome);  // Procura apenas no escopo atual (para checar redeclarações).

// Insere um símbolo no escopo atual (no topo da pilha).
void inserir_na_pilha(PilhaDeTabelas* pilha, Simbolo s);

#endif // TABELA_SIMBOLOS_H