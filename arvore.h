#ifndef ARVORE_H
#define ARVORE_H

// Enum para os tipos de dados da linguagem Goianinha
typedef enum {
    TIPO_INT,
    TIPO_CAR,
    TIPO_VOID, // Para funções sem retorno
    TIPO_INDEFINIDO // Para nós que ainda não tiveram o tipo verificado
} TipoDado;

// Enum para as diferentes categorias de nós da árvore
typedef enum {
    NO_PROGRAMA,
    NO_LISTA_DECLARACOES,
    NO_DECL_VAR,
    NO_DECL_FUNCAO,
    NO_LISTA_PARAM,
    NO_PARAM,
    NO_BLOCO,
    NO_LISTA_COMANDOS,
    NO_IF,
    NO_WHILE,
    NO_ATRIBUICAO,
    NO_RETORNO,
    NO_CHAMADA_FUNCAO,
    NO_LISTA_ARGS,
    NO_NEGACAO, // Operador !
    NO_OP_LOGICO, // AND, OR
    NO_OP_RELACIONAL, // <, >, <=, >=, ==, !=
    NO_OP_ARITMETICO, // +, -, *, /
    NO_IDENTIFICADOR,
    NO_CONST_INT,
    NO_CONST_CAR
} TipoNo;

// Estrutura de um nó da Árvore Sintática Abstrata
typedef struct No {
    TipoNo tipo_no;
    char* lexema;       // Para identificadores e constantes
    TipoDado tipo_dado; // Para checagem de tipos
    int linha;

    // Filhos da árvore (usamos até 4 para cobrir todas as estruturas)
    struct No *filho1;
    struct No *filho2;
    struct No *filho3;
    struct No *filho4;

    // Ponteiro para o próximo comando ou declaração em uma lista
    struct No *proximo;
} No;

// Assinaturas das funções da árvore
No* cria_no(TipoNo tipo_no, int linha, char* lexema);
void adiciona_filho(No* pai, No* filho);
void imprime_arvore(No* raiz, int profundidade);
void libera_arvore(No* raiz);

#endif // ARVORE_H