#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela_simbolos.h"

// Função de hash simples para distribuir os símbolos na tabela.
// O objetivo é converter uma string (o nome do símbolo) em um número inteiro (o índice na tabela).
unsigned int hash(const char* str) {
    unsigned int hash = 0;
    // Itera por cada caractere da string.
    while (*str) {
        // A fórmula (hash << 5) + *str++ é uma maneira simples e eficaz de gerar
        // um valor de hash que depende de todos os caracteres da string.
        hash = (hash << 5) + *str++;
    }
    // O operador '%' (módulo) garante que o índice calculado esteja dentro dos limites do array da tabela.
    return hash % TAMANHO_TABELA;
}

// Inicializa uma tabela de símbolos, definindo todos os ponteiros como NULL.
// Isso significa que, no início, todas as "listas" da tabela de hash estão vazias.
void inicializar_tabela(TabelaDeSimbolos* tabela) {
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        tabela->simbolos[i] = NULL;
    }
}

// Cria e inicializa uma nova tabela de símbolos.
TabelaDeSimbolos* criar_tabela() {
    // Aloca memória para a estrutura da tabela.
    TabelaDeSimbolos* tabela = malloc(sizeof(TabelaDeSimbolos));
    // Verifica se a alocação foi bem-sucedida.
    if (!tabela) {
        printf("Erro: Falha de alocação de memória para Tabela de Símbolos.\n");
        exit(1);
    }
    // Chama a função para inicializar a tabela recém-criada.
    inicializar_tabela(tabela);
    return tabela;
}

// Insere um símbolo na tabela de símbolos (que é uma tabela de hash).
void inserir_simbolo(TabelaDeSimbolos* tabela, Simbolo simbolo) {
    // Calcula o índice da tabela de hash onde o símbolo deve ser inserido.
    unsigned int idx = hash(simbolo.nome);
    // Aloca memória para um novo nó de símbolo.
    Simbolo* novo = malloc(sizeof(Simbolo));
    if (!novo) {
        printf("Erro: Falha de alocação de memória para Símbolo.\n");
        exit(1);
    }
    // Copia os dados do símbolo para a estrutura recém-alocada.
    *novo = simbolo;

    // --- Lógica de Encadeamento Separado (Separate Chaining) ---
    // O novo símbolo é inserido no início da lista ligada naquela posição da tabela.
    // 1. O 'proximo' do novo símbolo aponta para o antigo início da lista.
    novo->proximo = tabela->simbolos[idx];
    // 2. A posição da tabela agora aponta para o novo símbolo, que se torna o novo início da lista.
    tabela->simbolos[idx] = novo;
}

// Busca um símbolo pelo nome na tabela fornecida.
Simbolo* buscar_simbolo(TabelaDeSimbolos* tabela, const char* nome) {
    // Calcula o índice onde o símbolo deveria estar.
    unsigned int idx = hash(nome);
    // Pega o ponteiro para o início da lista ligada naquele índice.
    Simbolo* atual = tabela->simbolos[idx];
    // Percorre a lista ligada (se houver alguma).
    while (atual) {
        // Compara o nome do símbolo atual com o nome procurado.
        if (strcmp(atual->nome, nome) == 0) {
            return atual; // Símbolo encontrado.
        }
        // Move para o próximo símbolo na lista de colisão.
        atual = atual->proximo;
    }
    return NULL; // Símbolo não encontrado nesta tabela.
}

// Libera a memória de uma tabela e todos os seus símbolos.
void liberar_tabela(TabelaDeSimbolos* tabela) {
    // Itera por cada uma das posições da tabela de hash.
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        Simbolo* atual = tabela->simbolos[i];
        // Itera pela lista ligada em cada posição, liberando cada nó.
        while (atual) {
            Simbolo* temp = atual;
            atual = atual->proximo;
            free(temp);
        }
    }
    // Por fim, libera a própria estrutura da tabela.
    free(tabela);
}

// --- Funções da Pilha de Tabelas (Gerenciamento de Escopo) ---

// Inicializa a pilha de escopos.
void inicializar_pilha(PilhaDeTabelas* pilha) {
    // O topo em -1 indica que a pilha está vazia.
    pilha->topo = -1;
}

// Empilha uma nova tabela, ou seja, cria um novo escopo.
void empilhar(PilhaDeTabelas* pilha) {
    // Verifica se há espaço na pilha (evita estouro de pilha/stack overflow).
    if (pilha->topo < 99) {
        // Incrementa o topo.
        pilha->topo++;
        // Cria uma nova tabela de símbolos para este novo nível de escopo.
        pilha->tabelas[pilha->topo] = criar_tabela();
    } else {
        // Se o limite de escopos aninhados for atingido, o compilador aborta.
        printf("Erro: Estouro da pilha de tabelas de símbolos (limite de 100 escopos aninhados).\n");
        exit(1);
    }
}

// Desempilha a tabela do topo, ou seja, fecha o escopo atual.
void desempilhar(PilhaDeTabelas* pilha) {
    // Verifica se a pilha não está vazia.
    if (pilha->topo >= 0) {
        // Libera toda a memória usada pela tabela do escopo que está sendo fechado.
        liberar_tabela(pilha->tabelas[pilha->topo]);
        // Decrementa o topo, fazendo o escopo anterior se tornar o escopo atual.
        pilha->topo--;
    }
}

// Retorna um ponteiro para a tabela que está no topo da pilha (o escopo atual).
TabelaDeSimbolos* topo_pilha(PilhaDeTabelas* pilha) {
    if (pilha->topo >= 0) {
        return pilha->tabelas[pilha->topo];
    }
    return NULL; // Retorna NULL se a pilha estiver vazia.
}

// Insere um símbolo no escopo atual.
void inserir_na_pilha(PilhaDeTabelas* pilha, Simbolo s) {
    // Obtém a tabela de símbolos do escopo atual.
    TabelaDeSimbolos* tabela_atual = topo_pilha(pilha);
    if (tabela_atual) {
        // Associa o nível de escopo atual ao símbolo.
        s.escopo = pilha->topo;
        // Insere o símbolo na tabela do escopo atual.
        inserir_simbolo(tabela_atual, s);
    }
}

// Busca um símbolo em todos os escopos, do mais interno para o mais externo.
// Isso implementa a regra de "sombreamento" de variáveis (shadowing).
Simbolo* buscar_em_todos_escopos(PilhaDeTabelas* pilha, const char* nome) {
    // Começa a busca do topo da pilha (escopo mais interno) para baixo.
    for (int i = pilha->topo; i >= 0; i--) {
        // Busca o símbolo na tabela do escopo 'i'.
        Simbolo* s = buscar_simbolo(pilha->tabelas[i], nome);
        // Se encontrou, retorna imediatamente. A primeira correspondência encontrada é a correta.
        if (s) {
            return s;
        }
    }
    return NULL; // Se percorreu todos os escopos e não encontrou, retorna NULL.
}

// Busca um símbolo apenas no escopo atual.
// Usado principalmente para detectar erros de redeclaração de variáveis no mesmo escopo.
Simbolo* buscar_no_escopo_atual(PilhaDeTabelas* pilha, const char* nome) {
    // Pega a tabela do topo da pilha.
    TabelaDeSimbolos* tabela_atual = topo_pilha(pilha);
    if (tabela_atual) {
        // Busca o símbolo apenas nesta tabela.
        return buscar_simbolo(tabela_atual, nome);
    }
    return NULL;
}