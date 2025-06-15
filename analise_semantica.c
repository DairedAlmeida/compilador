#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analise_semantica.h"
#include "tabela_simbolos.h"

// Variáveis globais para o analisador semântico
static PilhaDeTabelas pilha_escopos;
static int num_erros_semanticos = 0;

// Função de conveniência para reportar erros
void erro_semantico(const char* mensagem, int linha) {
    fprintf(stderr, "ERRO SEMÂNTICO (linha %d): %s\n", linha, mensagem);
    num_erros_semanticos++;
}

// Converte uma string de tipo ("int", "car") para o enum TipoDado
TipoDado string_para_tipo(char* str) {
    if (strcmp(str, "int") == 0) return TIPO_INT;
    if (strcmp(str, "car") == 0) return TIPO_CAR;
    if (strcmp(str, "void") == 0) return TIPO_VOID;
    return TIPO_INDEFINIDO;
}

// Percorre a árvore recursivamente para realizar a análise
void percorre_analisando(No* no);

// Analisa um nó de declaração de variável
void analisa_declaracao_var(No* no) {
    char* nome_tipo = no->lexema;
    char* nome_var = no->filho1->lexema;

    // Verifica se a variável já foi declarada no escopo atual
    if (buscar_no_escopo_atual(&pilha_escopos, nome_var)) {
        char msg[200];
        sprintf(msg, "Variável '%s' já declarada neste escopo.", nome_var);
        erro_semantico(msg, no->linha);
        return;
    }

    // Cria e insere o novo símbolo na tabela
    Simbolo s;
    strcpy(s.nome, nome_var);
    s.categoria = CAT_VARIAVEL;
    s.tipo_dado = string_para_tipo(nome_tipo);
    s.linha = no->linha;
    s.params = NULL;
    s.num_params = 0;
    inserir_na_pilha(&pilha_escopos, s);
}

// Analisa a declaração de uma função
void analisa_declaracao_funcao(No* no) {
    char* nome_funcao = no->lexema;

    // Verifica se já existe símbolo com mesmo nome no escopo atual
    if (buscar_no_escopo_atual(&pilha_escopos, nome_funcao)) {
        char msg[200];
        sprintf(msg, "Função ou variável '%s' já declarada.", nome_funcao);
        erro_semantico(msg, no->linha);
        return; // Não continua para evitar erros em cascata
    }

    // Insere o símbolo da função no escopo atual (global)
    Simbolo s_funcao;
    strcpy(s_funcao.nome, nome_funcao);
    s_funcao.categoria = CAT_FUNCAO;
    s_funcao.tipo_dado = string_para_tipo(no->filho1->lexema); // Tipo de retorno
    s_funcao.linha = no->linha;
    s_funcao.params = no->filho2; // Ponteiro para a lista de parâmetros na árvore
    s_funcao.num_params = 0; // Será contado abaixo
    inserir_na_pilha(&pilha_escopos, s_funcao);

    // Abre um novo escopo para os parâmetros e corpo da função
    empilhar(&pilha_escopos);

    // Processa os parâmetros, inserindo-os no novo escopo
    No* param_atual = no->filho2;
    while (param_atual) {
        s_funcao.num_params++; // Conta os parâmetros
        
        char* nome_param = param_atual->filho1->lexema;
        if (buscar_no_escopo_atual(&pilha_escopos, nome_param)) {
            char msg[200];
            sprintf(msg, "Parâmetro '%s' redeclarado na função '%s'.", nome_param, nome_funcao);
            erro_semantico(msg, param_atual->linha);
        } else {
            Simbolo s_param;
            strcpy(s_param.nome, nome_param);
            s_param.categoria = CAT_PARAMETRO;
            s_param.tipo_dado = string_para_tipo(param_atual->lexema);
            s_param.linha = param_atual->linha;
            s_param.params = NULL;
            s_param.num_params = 0;
            inserir_na_pilha(&pilha_escopos, s_param);
        }
        param_atual = param_atual->proximo;
    }
    
    // Atualiza o número de parâmetros no símbolo da função
    Simbolo* s_funcao_ptr = buscar_no_escopo_atual(&pilha_escopos, nome_funcao);
    if(s_funcao_ptr) s_funcao_ptr->num_params = s_funcao.num_params;


    // Analisa o corpo da função
    percorre_analisando(no->filho3);

    // Fecha o escopo da função
    desempilhar(&pilha_escopos);
}

// Analisa um bloco de comandos
void analisa_bloco(No* no) {
    // Abre um novo escopo para o bloco
    empilhar(&pilha_escopos);
    
    // Analisa as declarações e comandos dentro do bloco
    percorre_analisando(no->filho1); // lista_declaracoes
    percorre_analisando(no->filho2); // lista_comandos

    // Fecha o escopo do bloco
    desempilhar(&pilha_escopos);
}


// Analisa um nó de identificador (uso de uma variável)
void analisa_identificador(No* no) {
    Simbolo* s = buscar_em_todos_escopos(&pilha_escopos, no->lexema);
    if (!s) {
        char msg[200];
        sprintf(msg, "Identificador '%s' não declarado.", no->lexema);
        erro_semantico(msg, no->linha);
        no->tipo_dado = TIPO_INDEFINIDO;
    } else {
        // O tipo do nó na árvore recebe o tipo do símbolo encontrado
        no->tipo_dado = s->tipo_dado;
    }
}

// Analisa uma atribuição
void analisa_atribuicao(No* no) {
    // Analisa o lado esquerdo (deve ser um identificador)
    percorre_analisando(no->filho1);
    
    // Analisa o lado direito (a expressão)
    percorre_analisando(no->filho2);

    // Checagem de tipos
    if (no->filho1->tipo_dado != TIPO_INDEFINIDO &&
        no->filho2->tipo_dado != TIPO_INDEFINIDO &&
        no->filho1->tipo_dado != no->filho2->tipo_dado) {
        
        char msg[200];
        sprintf(msg, "Tipos incompatíveis na atribuição para '%s'.", no->filho1->lexema);
        erro_semantico(msg, no->linha);
    }
    no->tipo_dado = no->filho1->tipo_dado; // O tipo da atribuição é o tipo da variável
}

// Analisa operadores aritméticos
void analisa_op_aritmetico(No* no) {
    percorre_analisando(no->filho1);
    percorre_analisando(no->filho2);

    // Ambos os operandos devem ser do tipo INT
    if (no->filho1->tipo_dado != TIPO_INT || no->filho2->tipo_dado != TIPO_INT) {
        erro_semantico("Operadores aritméticos só podem ser usados com o tipo 'int'.", no->linha);
    }
    
    no->tipo_dado = TIPO_INT; // O resultado de uma operação aritmética é sempre int
}

// Analisa operadores relacionais
void analisa_op_relacional(No* no) {
    percorre_analisando(no->filho1);
    percorre_analisando(no->filho2);

    // Os operandos devem ter o mesmo tipo
    if (no->filho1->tipo_dado != TIPO_INDEFINIDO &&
        no->filho2->tipo_dado != TIPO_INDEFINIDO &&
        no->filho1->tipo_dado != no->filho2->tipo_dado) {
        erro_semantico("Tipos incompatíveis para operador relacional.", no->linha);
    }
    
    no->tipo_dado = TIPO_INT; // O resultado de uma comparação é um valor lógico (representado por int)
}

// Analisa um comando IF ou WHILE
void analisa_condicional(No* no) {
    // Analisa a expressão da condição
    percorre_analisando(no->filho1);

    // A expressão deve resultar em um valor lógico (tipo int)
    if (no->filho1->tipo_dado != TIPO_INDEFINIDO && no->filho1->tipo_dado != TIPO_INT) {
        erro_semantico("A expressão da condição deve ser do tipo 'int'.", no->linha);
    }

    // Analisa o corpo do if/while
    percorre_analisando(no->filho2);

    // Analisa o corpo do else, se existir
    if (no->filho3) {
        percorre_analisando(no->filho3);
    }
}


// Função principal que percorre a árvore, chamando a função de análise apropriada para cada tipo de nó.
void percorre_analisando(No* no) {
    if (!no) return;

    // A análise de alguns nós depende da análise de seus filhos (pós-ordem).
    // Para outros, o nó pai precisa ser analisado primeiro (pré-ordem), como blocos e funções, para controle de escopo.

    switch (no->tipo_no) {
        // --- Estruturas que controlam escopo (analisadas em pré-ordem) ---
        case NO_DECL_FUNCAO:
            analisa_declaracao_funcao(no);
            return; // A própria função cuida da recursão
        case NO_BLOCO:
            analisa_bloco(no);
            return; // A própria função cuida da recursão

        default:
            // Para os outros nós, a análise é feita em pós-ordem
            break;
    }
    
    // Recursão para os filhos e para o próximo nó na lista
    percorre_analisando(no->filho1);
    percorre_analisando(no->filho2);
    percorre_analisando(no->filho3);
    percorre_analisando(no->filho4);
    percorre_analisando(no->proximo);

    // --- Análise do nó atual (após filhos terem sido analisados) ---
    switch (no->tipo_no) {
        case NO_DECL_VAR:
            analisa_declaracao_var(no);
            break;
        case NO_IDENTIFICADOR:
            analisa_identificador(no);
            break;
        case NO_ATRIBUICAO:
            analisa_atribuicao(no);
            break;
        case NO_CONST_INT:
            no->tipo_dado = TIPO_INT;
            break;
        case NO_CONST_CAR:
            no->tipo_dado = TIPO_CAR;
            break;
        case NO_OP_ARITMETICO:
            analisa_op_aritmetico(no);
            break;
        case NO_OP_RELACIONAL:
            analisa_op_relacional(no);
            break;
        case NO_IF:
        case NO_WHILE:
            // A análise da condição e dos blocos já foi feita na recursão.
            // Aqui poderíamos fazer algo extra se necessário.
            // Para este exemplo, deixamos vazio.
            break;
        
        // Adicionar outros casos conforme necessário (chamada de função, retorno, etc.)

        default:
            // Nós como NO_PROGRAMA, NO_LISTA_COMANDOS não precisam de ação aqui
            break;
    }
}


// Função de entrada do módulo de análise semântica
int analisar(No* raiz_arvore) {
    num_erros_semanticos = 0;
    
    inicializar_pilha(&pilha_escopos);
    empilhar(&pilha_escopos); // Abre o escopo global

    percorre_analisando(raiz_arvore);

    desempilhar(&pilha_escopos); // Fecha o escopo global

    return num_erros_semanticos;
}