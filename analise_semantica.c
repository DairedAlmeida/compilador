#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analise_semantica.h"
#include "tabela_simbolos.h"

// --- Variáveis Globais Estáticas ---
// 'static' significa que estas variáveis só são visíveis dentro deste arquivo.

// A pilha de escopos é a estrutura de dados central para a análise semântica.
static PilhaDeTabelas pilha_escopos;
// Ponteiro para o símbolo da função que está sendo analisada no momento.
// É usado para verificar se os comandos 'retorne' são compatíveis com a assinatura da função.
static Simbolo* funcao_atual = NULL;

// Função de conveniência para reportar erros semânticos.
// De acordo com a especificação do projeto, a compilação termina no primeiro erro encontrado.
void erro_semantico(const char* mensagem, int linha) {
    fprintf(stderr, "\nERRO SEMÂNTICO (linha %d): %s\n", linha, mensagem);
    fprintf(stderr, "Compilação abortada.\n");
    exit(1); // Encerra o programa com status de erro.
}

// Converte uma string de tipo (ex: "int") para o valor enum 'TipoDado' correspondente.
TipoDado string_para_tipo(char* str) {
    if (strcmp(str, "int") == 0) return TIPO_INT;
    if (strcmp(str, "car") == 0) return TIPO_CAR;
    if (strcmp(str, "void") == 0) return TIPO_VOID;
    return TIPO_INDEFINIDO; // Retorna indefinido se a string não for um tipo válido.
}

// Protótipo da função 'visita_no'. Como as funções se chamam mutuamente (recursão mútua),
// é necessário declarar a assinatura de 'visita_no' antes de ser chamada por outras funções.
void visita_no(No* no);

// Adiciona os símbolos das funções nativas da linguagem (leia, escreva) na tabela de símbolos global.
void inicializar_simbolos_nativos() {
    // Cria e insere o símbolo para a função 'leia'.
    Simbolo s_leia;
    strcpy(s_leia.nome, "leia");
    s_leia.categoria = CAT_FUNCAO;
    s_leia.tipo_dado = TIPO_VOID; // leia não retorna valor.
    s_leia.num_params = 1;
    s_leia.params = NULL; // A verificação de parâmetros de funções nativas pode ser simplificada.
    inserir_na_pilha(&pilha_escopos, s_leia);

    // Cria e insere o símbolo para a função 'escreva'.
    Simbolo s_escreva;
    strcpy(s_escreva.nome, "escreva");
    s_escreva.categoria = CAT_FUNCAO;
    s_escreva.tipo_dado = TIPO_VOID;
    s_escreva.num_params = 1;
    s_escreva.params = NULL;
    inserir_na_pilha(&pilha_escopos, s_escreva);

    // Cria e insere o símbolo para a função 'novalinha'.
    Simbolo s_novalinha;
    strcpy(s_novalinha.nome, "novalinha");
    s_novalinha.categoria = CAT_FUNCAO;
    s_novalinha.tipo_dado = TIPO_VOID;
    s_novalinha.num_params = 0;
    s_novalinha.params = NULL;
    inserir_na_pilha(&pilha_escopos, s_novalinha);
}

// Analisa um nó de declaração de variável.
void analisa_declaracao_var(No* no) {
    // O nome da variável está no lexema do primeiro filho do nó de declaração.
    char* nome_var = no->filho1->lexema;
    // Verifica se a variável já foi declarada NO ESCOPO ATUAL.
    if (buscar_no_escopo_atual(&pilha_escopos, nome_var)) {
        char msg[200];
        sprintf(msg, "Variável ou parâmetro '%s' já declarado neste escopo.", nome_var);
        erro_semantico(msg, no->linha);
    }

    // Se não houve erro, cria um novo símbolo para a variável.
    Simbolo s;
    strcpy(s.nome, nome_var);
    s.categoria = CAT_VARIAVEL;
    // O tipo da variável está no lexema do próprio nó de declaração.
    s.tipo_dado = string_para_tipo(no->lexema);
    s.linha = no->linha;
    s.params = NULL;
    s.num_params = 0;

    // Define o escopo do símbolo.
    if (funcao_atual == NULL) { // Se não estamos dentro de uma função, é uma variável global.
        s.escopo = 0; // Escopo global é 0.
    } else { // Se estamos dentro de uma função, é uma variável local.
        s.escopo = pilha_escopos.topo;
    }

    // Insere o novo símbolo na tabela de símbolos do escopo atual.
    inserir_na_pilha(&pilha_escopos, s);
}

// Analisa um nó de declaração de função.
void analisa_declaracao_funcao(No* no) {
    char* nome_funcao = no->lexema;
    // Funções só podem ser declaradas no escopo global. Verifica se já existe um símbolo com esse nome.
    if (buscar_no_escopo_atual(&pilha_escopos, nome_funcao)) {
        char msg[200];
        sprintf(msg, "Função ou variável '%s' já declarada.", nome_funcao);
        erro_semantico(msg, no->linha);
    }

    // Cria o símbolo para a função.
    Simbolo s_funcao;
    strcpy(s_funcao.nome, nome_funcao);
    s_funcao.categoria = CAT_FUNCAO;
    s_funcao.tipo_dado = string_para_tipo(no->filho1->lexema); // Tipo de retorno.
    s_funcao.linha = no->linha;
    s_funcao.params = no->filho2; // Ponteiro para a lista de parâmetros na ASA.
    
    // Conta o número de parâmetros.
    int n_params = 0;
    for (No* p = no->filho2; p != NULL; p = p->proximo) n_params++;
    s_funcao.num_params = n_params;

    // Insere a função na tabela do escopo atual (global).
    inserir_na_pilha(&pilha_escopos, s_funcao);
    // Atualiza a variável global 'funcao_atual' para sabermos que estamos dentro desta função.
    funcao_atual = buscar_no_escopo_atual(&pilha_escopos, nome_funcao);

    // --- Início do Escopo da Função ---
    // Cria um novo escopo para os parâmetros e o corpo da função.
    empilhar(&pilha_escopos);

    // Itera sobre a lista de parâmetros, adicionando-os como símbolos no novo escopo.
    for (No* p = no->filho2; p != NULL; p = p->proximo) {
        // Verifica se há parâmetros com nomes duplicados.
        if (buscar_no_escopo_atual(&pilha_escopos, p->filho1->lexema)){
            char msg[200];
            sprintf(msg, "Parâmetro '%s' redeclarado na função '%s'.", p->filho1->lexema, nome_funcao);
            erro_semantico(msg, p->linha);
        }
        // Cria e insere o símbolo do parâmetro.
        Simbolo s_param;
        strcpy(s_param.nome, p->filho1->lexema);
        s_param.categoria = CAT_PARAMETRO;
        s_param.tipo_dado = string_para_tipo(p->lexema);
        s_param.linha = p->linha;
        s_param.params = NULL;
        s_param.num_params = 0;
        inserir_na_pilha(&pilha_escopos, s_param);
    }

    // Analisa recursivamente o corpo da função (que é um bloco).
    visita_no(no->filho3);

    // --- Fim do Escopo da Função ---
    // Fecha o escopo da função, removendo seus parâmetros e variáveis locais.
    desempilhar(&pilha_escopos);
    // Reseta 'funcao_atual', pois saímos da função.
    funcao_atual = NULL;
}

// Analisa um nó de bloco de código `{...}`.
void analisa_bloco(No* no) {
    // Abre um novo escopo para o bloco.
    empilhar(&pilha_escopos);
    // As declarações locais do bloco são o filho1. Visita cada uma delas.
    for (No* decl = no->filho1; decl != NULL; decl = decl->proximo) visita_no(decl);
    // Os comandos do bloco são o filho2. Visita cada um deles.
    for (No* cmd = no->filho2; cmd != NULL; cmd = cmd->proximo) visita_no(cmd);

    desempilhar(&pilha_escopos);
}

// Analisa um nó de identificador (uso de uma variável ou função).
void analisa_identificador(No* no) {
    // Busca o identificador em todos os escopos, do atual ao global.
    Simbolo* s = buscar_em_todos_escopos(&pilha_escopos, no->lexema);
    // Se não encontrou, é um erro de "identificador não declarado".
    if (!s) {
        char msg[200];
        sprintf(msg, "Identificador '%s' não declarado.", no->lexema);
        erro_semantico(msg, no->linha);
    }
    // Se encontrou, "anota" o nó da ASA com o tipo de dado do símbolo.
    // Isso é fundamental para a checagem de tipos em expressões.
    no->tipo_dado = s->tipo_dado;
}

// Analisa um nó de atribuição (`=`).
void analisa_atribuicao(No* no) {
    // Analisa recursivamente o lado esquerdo (variável) e o lado direito (expressão).
    // Isso vai preencher os campos 'tipo_dado' desses nós filhos.
    visita_no(no->filho1);
    visita_no(no->filho2);
    // Verifica se os tipos são compatíveis. Nesta linguagem, eles devem ser iguais.
    if (no->filho1->tipo_dado != no->filho2->tipo_dado) {
        erro_semantico("Tipos incompatíveis em comando de atribuição.", no->linha);
    }
    // O tipo de uma atribuição é o tipo da variável que a recebe.
    no->tipo_dado = no->filho1->tipo_dado;
}

// Analisa um nó de operação binária (+, *, <, ==, e, ou, etc.).
void analisa_op_binaria(No* no) {
    // Analisa recursivamente os dois operandos.
    visita_no(no->filho1);
    visita_no(no->filho2);
    TipoDado t1 = no->filho1->tipo_dado;
    TipoDado t2 = no->filho2->tipo_dado;

    // Regras para operadores aritméticos (+, -, *, /)
    if (no->tipo_no == NO_OP_ARITMETICO) {
        if (t1 != TIPO_INT || t2 != TIPO_INT) {
            erro_semantico("Operadores aritméticos só podem ser usados com o tipo 'int'.", no->linha);
        }
        no->tipo_dado = TIPO_INT; // O resultado de uma operação aritmética é 'int'.
    }
    // Regras para operadores lógicos (e, ou)
    else if (no->tipo_no == NO_OP_LOGICO) {
        // Na nossa linguagem, operações lógicas também operam sobre inteiros (0=falso, !=0=verdadeiro).
        if (t1 != TIPO_INT || t2 != TIPO_INT) {
            erro_semantico("Operadores lógicos só podem ser usados com operandos do tipo 'int'.", no->linha);
        }
        no->tipo_dado = TIPO_INT; // O resultado é 'int'.
    }
    // Regras para operadores relacionais (>, <, ==, etc.)
    else if (no->tipo_no == NO_OP_RELACIONAL) {
        if (t1 != t2) { // Os tipos dos operandos devem ser iguais.
             erro_semantico("Tipos incompatíveis para operador relacional.", no->linha);
        }
        no->tipo_dado = TIPO_INT; // O resultado de uma comparação é um valor booleano, representado como 'int'.
    }
}

// Analisa um nó 'se'.
void analisa_if(No* no) {
    // Analisa a expressão da condição.
    visita_no(no->filho1);
    // A condição de um 'se' deve resultar em um valor inteiro.
    if (no->filho1->tipo_dado != TIPO_INT) {
        erro_semantico("Expressão em comando 'se' deve ser avaliada como um inteiro.", no->filho1->linha);
    }
    // Analisa o bloco 'então'.
    visita_no(no->filho2);
    // Se houver um bloco 'senão' (filho3), analisa-o também.
    if (no->filho3) visita_no(no->filho3);
}

// Analisa um nó 'enquanto'.
void analisa_while(No* no) {
    // Analisa a expressão da condição.
    visita_no(no->filho1);
    // A condição de um 'enquanto' deve resultar em um valor inteiro.
    if (no->filho1->tipo_dado != TIPO_INT) {
        erro_semantico("Expressão em comando 'enquanto' deve ser avaliada como um inteiro.", no->filho1->linha);
    }
    // Analisa o bloco de repetição.
    visita_no(no->filho2);
}

// Analisa um nó de chamada de função.
void analisa_chamada_funcao(No* no) {
    // Busca a função na tabela de símbolos.
    Simbolo* s = buscar_em_todos_escopos(&pilha_escopos, no->lexema);
    if (!s) {
        char msg[200]; sprintf(msg, "Função '%s' não declarada.", no->lexema);
        erro_semantico(msg, no->linha);
    }
    // Verifica se o identificador encontrado é de fato uma função.
    if (s->categoria != CAT_FUNCAO) {
        char msg[200]; sprintf(msg, "'%s' não é uma função.", no->lexema);
        erro_semantico(msg, no->linha);
    }

    // O tipo do nó da chamada é o tipo de retorno da função.
    no->tipo_dado = s->tipo_dado;

    // --- Checagem de Número e Tipos dos Argumentos ---
    No* arg = no->filho1;      // Ponteiro para o primeiro argumento na chamada.
    No* param = s->params;     // Ponteiro para o primeiro parâmetro na declaração da função.
    int n_args = 0;            // Contador de argumentos.
    int n_params = s->num_params; // Número de parâmetros esperado.

    // Itera enquanto houver argumentos e parâmetros para comparar.
    while(arg != NULL && param != NULL) {
        visita_no(arg); // Analisa o argumento para descobrir seu tipo.
        // Compara o tipo do argumento com o tipo esperado do parâmetro.
        if (arg->tipo_dado != string_para_tipo(param->lexema)) {
             char msg[200]; sprintf(msg, "Tipo do argumento na chamada da função '%s' não corresponde ao tipo do parâmetro.", no->lexema);
             erro_semantico(msg, arg->linha);
        }
        n_args++;
        arg = arg->proximo;
        param = param->proximo;
    }
    // Se, após o laço, ainda sobraram argumentos, conta-os.
    if (arg != NULL) while(arg != NULL) { n_args++; arg = arg->proximo; }

    // Compara o número de argumentos contados com o número de parâmetros esperado.
    if (n_args != n_params) {
        char msg[200]; sprintf(msg, "Número incorreto de argumentos para a função '%s'. Esperado: %d, Recebido: %d.", no->lexema, n_params, n_args);
        erro_semantico(msg, no->linha);
    }
}

// Analisa um nó 'retorne'.
void analisa_retorno(No* no) {
    // Verifica se o comando 'retorne' está dentro de uma função.
    if (!funcao_atual) {
        erro_semantico("Comando 'retorne' fora de uma função.", no->linha);
    }
    // Obtém o tipo de retorno esperado da função atual.
    TipoDado tipo_retorno_esperado = funcao_atual->tipo_dado;
    TipoDado tipo_retornado;

    if (no->filho1) { // Se há uma expressão de retorno (ex: `retorne x;`)
        visita_no(no->filho1); // Analisa a expressão.
        tipo_retornado = no->filho1->tipo_dado; // Pega o tipo da expressão.
    } else { // Se não há expressão (ex: `retorne;`)
        tipo_retornado = TIPO_VOID; // O tipo retornado é 'void'.
    }

    // Compara o tipo efetivamente retornado com o tipo que a função declarou que retornaria.
    if (tipo_retorno_esperado != tipo_retornado) {
        erro_semantico("Tipo de retorno incompatível com a declaração da função.", no->linha);
    }
}

// Função 'visitante' principal. Ela percorre a ASA e direciona cada nó para a função de análise correta.
void visita_no(No* no) {
    if (!no) return; // Se o nó for nulo, não faz nada.

    // Um grande 'switch' que atua como um despachante, chamando a função de análise apropriada para cada tipo de nó.
    switch (no->tipo_no) {
        // --- Nós Estruturais ---
        case NO_PROGRAMA:
            // Para o nó do programa, visita a lista de declarações e depois o bloco principal (se houver).
            for (No* p = no->filho1; p != NULL; p = p->proximo) visita_no(p);
            visita_no(no->filho2);
            break;
        // --- Nós de Análise Específica ---
        case NO_DECL_VAR: analisa_declaracao_var(no); break;
        case NO_DECL_FUNCAO: analisa_declaracao_funcao(no); break;
        case NO_BLOCO: analisa_bloco(no); break;
        case NO_IDENTIFICADOR: analisa_identificador(no); break;
        // --- Nós Folha (Tipos Conhecidos) ---
        case NO_CONST_INT: no->tipo_dado = TIPO_INT; break; // Uma constante inteira sempre tem tipo 'int'.
        case NO_CONST_CAR: no->tipo_dado = TIPO_CAR; break; // Uma constante char sempre tem tipo 'car'.
        // --- Nós de Comando e Expressão ---
        case NO_ATRIBUICAO: analisa_atribuicao(no); break;
        case NO_OP_ARITMETICO:
        case NO_OP_LOGICO:
        case NO_OP_RELACIONAL:
            analisa_op_binaria(no);
            break;
        case NO_NEGACAO:
            visita_no(no->filho1); // Analisa a expressão sendo negada.
            if(no->filho1->tipo_dado != TIPO_INT) erro_semantico("Operador de negação '!' só pode ser usado com o tipo 'int'.", no->linha);
            no->tipo_dado = TIPO_INT; // O resultado da negação é 'int'.
            break;
        case NO_IF: analisa_if(no); break;
        case NO_WHILE: analisa_while(no); break;
        case NO_CHAMADA_FUNCAO: analisa_chamada_funcao(no); break;
        case NO_RETORNO: analisa_retorno(no); break;
        // --- Caso Padrão ---
        default:
            // Para qualquer outro tipo de nó não listado, visita recursivamente seus filhos.
            // Isso garante que toda a árvore seja percorrida.
            visita_no(no->filho1);
            visita_no(no->filho2);
            visita_no(no->filho3);
            visita_no(no->filho4);
            break;
    }
}

// Função de entrada para a fase de análise semântica.
int analisar(No* raiz_arvore) {
    // 1. Inicializa a pilha de escopos.
    inicializar_pilha(&pilha_escopos);
    // 2. Empilha a primeira tabela, que será o escopo global.
    empilhar(&pilha_escopos);
    // 3. Adiciona as funções nativas da linguagem ao escopo global.
    inicializar_simbolos_nativos();
    // 4. Inicia o percurso da árvore a partir do nó raiz.
    visita_no(raiz_arvore);

    // 5. Retorna 0. Se algum erro tivesse ocorrido, a função `erro_semantico` já teria encerrado o programa.
    return 0;
}