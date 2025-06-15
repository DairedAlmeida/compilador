#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela_simbolos.h"

// Função de hash simples para distribuir os símbolos na tabela
unsigned int hash(const char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % TAMANHO_TABELA;
}

// Inicializa uma tabela de símbolos, definindo todos os ponteiros como NULL
void inicializar_tabela(TabelaDeSimbolos* tabela) {
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        tabela->simbolos[i] = NULL;
    }
}

// Cria e inicializa uma nova tabela de símbolos
TabelaDeSimbolos* criar_tabela() {
    TabelaDeSimbolos* tabela = malloc(sizeof(TabelaDeSimbolos));
    if (!tabela) {
        printf("Erro: Falha de alocação de memória para Tabela de Símbolos.\n");
        exit(1);
    }
    inicializar_tabela(tabela);
    return tabela;
}

// Insere um símbolo na tabela de símbolos
void inserir_simbolo(TabelaDeSimbolos* tabela, Simbolo simbolo) {
    unsigned int idx = hash(simbolo.nome);
    Simbolo* novo = malloc(sizeof(Simbolo));
    if (!novo) {
        printf("Erro: Falha de alocação de memória para Símbolo.\n");
        exit(1);
    }
    *novo = simbolo;
    novo->proximo = tabela->simbolos[idx];
    tabela->simbolos[idx] = novo;
}

// Busca um símbolo pelo nome na tabela fornecida
Simbolo* buscar_simbolo(TabelaDeSimbolos* tabela, const char* nome) {
    unsigned int idx = hash(nome);
    Simbolo* atual = tabela->simbolos[idx];
    while (atual) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL;
}

// Libera a memória de uma tabela e todos os seus símbolos
void liberar_tabela(TabelaDeSimbolos* tabela) {
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

// --- Funções da Pilha de Tabelas (Gerenciamento de Escopo) ---

// Inicializa a pilha de escopos
void inicializar_pilha(PilhaDeTabelas* pilha) {
    pilha->topo = -1;
}

// Empilha uma nova tabela (cria um novo escopo)
void empilhar(PilhaDeTabelas* pilha) {
    if (pilha->topo < 99) {
        pilha->topo++;
        pilha->tabelas[pilha->topo] = criar_tabela();
    } else {
        printf("Erro: Estouro da pilha de tabelas de símbolos (limite de 100 escopos aninhados).\n");
        exit(1);
    }
}

// Desempilha a tabela do topo (fecha um escopo)
void desempilhar(PilhaDeTabelas* pilha) {
    if (pilha->topo >= 0) {
        liberar_tabela(pilha->tabelas[pilha->topo]);
        pilha->topo--;
    }
}

// Retorna a tabela que está no topo da pilha (escopo atual)
TabelaDeSimbolos* topo_pilha(PilhaDeTabelas* pilha) {
    if (pilha->topo >= 0) {
        return pilha->tabelas[pilha->topo];
    }
    return NULL;
}

// Insere um símbolo no escopo atual
void inserir_na_pilha(PilhaDeTabelas* pilha, Simbolo s) {
    TabelaDeSimbolos* tabela_atual = topo_pilha(pilha);
    if (tabela_atual) {
        s.escopo = pilha->topo;
        inserir_simbolo(tabela_atual, s);
    }
}

// Busca um símbolo em todos os escopos, do mais interno para o mais externo
Simbolo* buscar_em_todos_escopos(PilhaDeTabelas* pilha, const char* nome) {
    for (int i = pilha->topo; i >= 0; i--) {
        Simbolo* s = buscar_simbolo(pilha->tabelas[i], nome);
        if (s) {
            return s;
        }
    }
    return NULL;
}

// Busca um símbolo apenas no escopo atual
Simbolo* buscar_no_escopo_atual(PilhaDeTabelas* pilha, const char* nome) {
    TabelaDeSimbolos* tabela_atual = topo_pilha(pilha);
    if (tabela_atual) {
        return buscar_simbolo(tabela_atual, nome);
    }
    return NULL;
}