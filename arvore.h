#ifndef ARVORE_H
#define ARVORE_H

// --- Início das "Include Guards" ---
// O bloco #ifndef/#define/#endif é um mecanismo padrão em C para evitar que o conteúdo
// deste arquivo de cabeçalho seja incluído mais de uma vez em um mesmo arquivo de código.
// Se isso acontecesse, teríamos erros de "redefinição" de tipos e estruturas.

// Enumeração (enum) para os tipos de dados que a nossa linguagem "Goianinha" suporta.
// Usar uma enumeração torna o código mais legível e seguro do que usar números inteiros (ex: 0 para int, 1 para char).
typedef enum {
    TIPO_INT,         // Representa o tipo inteiro.
    TIPO_CAR,         // Representa o tipo caractere.
    TIPO_VOID,        // Representa o tipo 'vazio', usado para funções que não retornam valor.
    TIPO_INDEFINIDO   // Um tipo padrão para nós da árvore que ainda não tiveram seu tipo verificado
                      // pela análise semântica.
} TipoDado;

// Enumeração para os diferentes tipos de nós que podem compor a Árvore Sintática Abstrata.
// Cada item aqui corresponde a uma construção gramatical da linguagem.
typedef enum {
    // Nós Estruturais
    NO_PROGRAMA,          // Nó raiz da árvore, representa o programa inteiro.
    NO_LISTA_DECLARACOES, // Nó que representa uma lista de declarações de variáveis ou funções.
    NO_BLOCO,             // Nó que representa um bloco de código, como o corpo de uma função ou um bloco `se`/`enquanto`.

    // Nós de Declaração
    NO_DECL_VAR,          // Nó para a declaração de uma variável (ex: `int x;`).
    NO_DECL_FUNCAO,       // Nó para a declaração de uma função (ex: `int soma(...) { ... }`).
    NO_LISTA_PARAM,       // Nó que representa uma lista de parâmetros de uma função.
    NO_PARAM,             // Nó para um único parâmetro de uma função.

    // Nós de Comando
    NO_LISTA_COMANDOS,    // Nó que representa uma lista de comandos sequenciais.
    NO_IF,                // Nó para o comando condicional 'se'.
    NO_WHILE,             // Nó para o laço de repetição 'enquanto'.
    NO_ATRIBUICAO,        // Nó para um comando de atribuição (ex: `x = 5;`).
    NO_RETORNO,           // Nó para o comando 'retorne'.
    NO_CHAMADA_FUNCAO,    // Nó para a chamada de uma função (ex: `escreva(x);`).
    NO_LISTA_ARGS,        // Nó que representa a lista de argumentos passados em uma chamada de função.

    // Nós de Expressão
    NO_NEGACAO,           // Nó para o operador de negação unário '!'.
    NO_OP_LOGICO,         // Nó para operadores lógicos binários ('e', 'ou').
    NO_OP_RELACIONAL,     // Nó para operadores relacionais (ex: '<', '>', '==').
    NO_OP_ARITMETICO,     // Nó para operadores aritméticos (ex: '+', '-', '*', '/').

    // Nós Folha (Terminais)
    NO_IDENTIFICADOR,     // Nó para um identificador (nome de variável ou função).
    NO_CONST_INT,         // Nó para uma constante do tipo inteiro.
    NO_CONST_CAR          // Nó para uma constante do tipo caractere.
} TipoNo;

// Estrutura principal que define um nó da Árvore Sintática Abstrata.
typedef struct No {
    // --- Campos de Identificação do Nó ---
    TipoNo tipo_no;     // Guarda o tipo do nó (ex: NO_IF, NO_DECL_VAR), definido na enumeração `TipoNo`.
    char* lexema;       // Armazena o texto (lexema) associado ao nó, como o nome de uma variável ("x")
                        // ou o valor de uma constante ("123"). É NULL para nós que não têm lexema (ex: NO_BLOCO).
    TipoDado tipo_dado; // Armazena o tipo de dado do nó (ex: TIPO_INT). É preenchido durante a análise semântica.
    int linha;          // Armazena o número da linha no código-fonte onde este nó se origina. Essencial para mensagens de erro.

    // --- Ponteiros para a Estrutura da Árvore ---
    // Filhos da árvore. Usamos até 4 ponteiros para cobrir todas as estruturas da nossa linguagem.
    // Ex: em um `se-senao`, filho1=condição, filho2=bloco 'se', filho3=bloco 'senao'.
    struct No *filho1;
    struct No *filho2;
    struct No *filho3;
    struct No *filho4;

    // Ponteiro para o "irmão" do nó. Usado para criar listas de nós no mesmo nível hierárquico.
    // Ex: uma lista de comandos em um bloco, ou uma lista de declarações de variáveis.
    struct No *proximo;
} No;

// --- Assinaturas das Funções de Manipulação da Árvore ---
// Estas são as "promessas" das funções que estão implementadas no arquivo arvore.c.
// Elas permitem que outros arquivos (.c) usem estas funções.

// Aloca memória e cria um novo nó da árvore.
No* cria_no(TipoNo tipo_no, int linha, char* lexema);

// Adiciona um nó 'filho' à estrutura de um nó 'pai'.
void adiciona_filho(No* pai, No* filho);

// Imprime a árvore no console (usado para depuração).
void imprime_arvore(No* raiz, int profundidade);

// Libera toda a memória alocada para a árvore recursivamente.
void libera_arvore(No* raiz);

// Cria uma cópia profunda (deep copy) de uma árvore.
No* copia_arvore(No* raiz);

#endif // ARVORE_H