#include <stdio.h>      // Para operações de entrada e saída (ex: fprintf, fopen).
#include <stdlib.h>     // Para alocação de memória e outras funções padrão (ex: malloc, exit).
#include <string.h>     // Para manipulação de strings (ex: strcmp, strcpy).

// Inclusão dos arquivos de cabeçalho do projeto.
#include "geracao_codigo.h"  // Provavelmente contém o protótipo da função principal gerar_codigo.
#include "arvore.h"          // Contém as definições da estrutura da Árvore Sintática Abstrata (No).
#include "tabela_simbolos.h" // Contém as definições da Tabela de Símbolos e da Pilha de Escopos.

// --- Estruturas e Variáveis Globais ---

// Estrutura para criar uma lista encadeada de literais de string.
// Isso é usado para coletar todas as strings do código fonte e declará-las
// na seção .data do arquivo Assembly.
typedef struct StringLiteral {
    char label[20];                // Rótulo único para a string no assembly (ex: "str_0").
    char* content;                 // Ponteiro para o conteúdo da string (ex: "\"Olá, Mundo!\"").
    struct StringLiteral* next;    // Ponteiro para o próximo elemento na lista.
} StringLiteral;

// Variáveis Globais Estáticas. 'static' significa que são visíveis apenas dentro deste arquivo.
static FILE* arquivo_saida;                  // Ponteiro para o arquivo .asm de saída onde o código MIPS será escrito.
static int contador_label = 0;               // Contador para gerar rótulos (labels) únicos para desvios (if, while).
static int contador_string = 0;              // Contador para gerar rótulos únicos para as strings.
static PilhaDeTabelas pilha_escopos_gc;      // A pilha de tabelas de símbolos para gerenciar escopos (global, funções).
static int offset_global = 0;                // Deslocamento (offset) para alocação de variáveis globais na pilha.
static int offset_local = 0;                 // Deslocamento para alocação de variáveis locais no frame da função atual.
static Simbolo* funcao_atual_gc = NULL;      // Ponteiro para o símbolo da função que está sendo processada.
static StringLiteral* lista_strings = NULL;  // Cabeça da lista encadeada de literais de string.

// Protótipos de funções internas deste arquivo.
void visita_no_gc(No* no);       // Função principal que percorre a árvore (visitor pattern).
void coletar_strings(No* no);    // Função para pré-processar a árvore e encontrar todas as strings.


// --- Funções Auxiliares ---

// Gera um número de rótulo único e o incrementa.
// Usado para criar labels como L_ELSE_1, L_FIM_IF_2, etc.
int novo_label() {
    return contador_label++;
}

// Converte um operador da linguagem fonte para a instrução MIPS correspondente.
// Recebe uma string como "+" e retorna a string "add".
const char* get_op_mips(const char* op) {
    if (strcmp(op, "+") == 0) return "add";   // Adição
    if (strcmp(op, "-") == 0) return "sub";   // Subtração
    if (strcmp(op, "*") == 0) return "mul";   // Multiplicação
    if (strcmp(op, "/") == 0) return "div";   // Divisão
    if (strcmp(op, "e") == 0) return "and";   // E lógico (bitwise)
    if (strcmp(op, "ou") == 0) return "or";   // OU lógico (bitwise)
    if (strcmp(op, "==") == 0) return "seq";  // Set if equal
    if (strcmp(op, "!=") == 0) return "sne";  // Set if not equal
    if (strcmp(op, "<") == 0) return "slt";   // Set if less than
    if (strcmp(op, "<=") == 0) return "sle";  // Set if less than or equal
    if (strcmp(op, ">") == 0) return "sgt";   // Set if greater than
    if (strcmp(op, ">=") == 0) return "sge";  // Set if greater than or equal
    return ""; // Retorna string vazia se o operador não for encontrado.
}

// Calcula e carrega o endereço de uma variável no registrador $t0.
void get_endereco_var(const char* nome) {
    // Busca o símbolo da variável em todos os escopos, do mais interno para o mais externo.
    Simbolo* s = buscar_em_todos_escopos(&pilha_escopos_gc, nome);
    if (!s) {
        // Se a variável não for encontrada, é um erro semântico que deveria ter sido pego antes.
        fprintf(stderr, "Erro de Geração: Variável '%s' não encontrada.\n", nome);
        exit(1);
    }
    // Verifica se a variável é global (escopo 0).
    if (s->escopo == 0) {
        // Variáveis globais são acessadas a partir de um ponteiro base para a área global ($s1).
        // O endereço é ($s1 + offset). O offset está em s->num_params por reutilização do campo.
        fprintf(arquivo_saida, "  addi $t0, $s1, %d\n", s->num_params);
    } else {
        // Variáveis locais e parâmetros são acessados a partir do frame pointer ($fp).
        // O endereço é ($fp + offset). O offset está em s->num_params.
        fprintf(arquivo_saida, "  addi $t0, $fp, %d\n", s->num_params);
    }
}


// --- Funções de Geração de Código por Nó da Árvore ---

// Gera código para uma declaração de função.
void gc_declaracao_funcao(No* no) {
    // Extrai o nome da função do nó da árvore.
    char* nome_funcao = no->lexema;
    
    // Cria um símbolo para a função para inserí-lo na tabela de símbolos do escopo global.
    Simbolo s_funcao;
    strcpy(s_funcao.nome, nome_funcao);
    s_funcao.categoria = CAT_FUNCAO;
    s_funcao.params = no->filho2; // Referência aos nós dos parâmetros na árvore.
    int n_params = 0; // Conta o número de parâmetros.
    for (No* p = no->filho2; p != NULL; p = p->proximo) n_params++;
    s_funcao.num_params = n_params;
    inserir_na_pilha(&pilha_escopos_gc, s_funcao); // Insere no escopo atual (global).
    
    // Define a função atual para referência interna (ex: para a instrução de retorno).
    funcao_atual_gc = buscar_no_escopo_atual(&pilha_escopos_gc, nome_funcao);

    // Inicia a seção de código para a função no arquivo .asm.
    fprintf(arquivo_saida, "\n# ---- Funcao: %s ----\n", nome_funcao);
    fprintf(arquivo_saida, "%s:\n", nome_funcao); // Cria o rótulo (label) da função.

    // Gera o Prólogo da função: prepara a pilha para a execução da função.
    fprintf(arquivo_saida, "  # Prólogo\n");
    fprintf(arquivo_saida, "  addiu $sp, $sp, -4\n"); // Abre espaço na pilha.
    fprintf(arquivo_saida, "  sw $ra, 0($sp)\n");      // Salva o endereço de retorno ($ra).
    fprintf(arquivo_saida, "  addiu $sp, $sp, -4\n"); // Abre espaço na pilha.
    fprintf(arquivo_saida, "  sw $fp, 0($sp)\n");      // Salva o frame pointer antigo ($fp).
    fprintf(arquivo_saida, "  move $fp, $sp\n");       // O novo $fp aponta para o topo da pilha.

    // Cria um novo escopo para a função (parâmetros e variáveis locais).
    empilhar(&pilha_escopos_gc);
    
    // Processa os parâmetros da função, inserindo-os na tabela de símbolos do novo escopo.
    int offset_param = 8; // Offset inicial para o primeiro parâmetro relativo ao $fp.
    for (No* p = no->filho2; p != NULL; p = p->proximo) {
        Simbolo s_param;
        strcpy(s_param.nome, p->filho1->lexema); // Nome do parâmetro.
        s_param.categoria = CAT_PARAMETRO;
        s_param.tipo_dado = string_para_tipo(p->lexema); // Tipo do parâmetro.
        s_param.num_params = offset_param; // Armazena o offset do parâmetro.
        inserir_na_pilha(&pilha_escopos_gc, s_param);
        offset_param += 4; // Move para o próximo offset de parâmetro (cada um ocupa 4 bytes).
    }

    // Aloca espaço na pilha para as variáveis locais.
    offset_local = 0; // Reseta o offset local para esta função.
    int espaco_locais = 0;
    // Calcula o espaço total necessário para as variáveis locais.
    if (no->filho3 && no->filho3->filho1) { // Verifica se há um bloco de declarações.
        for(No* decl = no->filho3->filho1; decl != NULL; decl = decl->proximo) {
            espaco_locais += 4; // Adiciona 4 bytes por variável.
        }
    }
    if (espaco_locais > 0) {
        // Subtrai do stack pointer ($sp) o espaço calculado.
        fprintf(arquivo_saida, "  addiu $sp, $sp, -%d # Aloca espaço para var(es) local(is)\n", espaco_locais);
    }
    
    // Gera o código para o corpo da função (bloco de comandos).
    visita_no_gc(no->filho3);

    // Gera o Epílogo da função: restaura a pilha e retorna ao chamador.
    fprintf(arquivo_saida, "\n%s_epilogo:\n", nome_funcao); // Rótulo para o epílogo (usado pelo 'retorna').
    fprintf(arquivo_saida, "  # Epílogo\n");
    fprintf(arquivo_saida, "  move $sp, $fp\n");       // Restaura o $sp para a posição do $fp.
    fprintf(arquivo_saida, "  lw $fp, 0($sp)\n");      // Restaura o $fp antigo.
    fprintf(arquivo_saida, "  lw $ra, 4($sp)\n");      // Restaura o endereço de retorno $ra.
    fprintf(arquivo_saida, "  addiu $sp, $sp, 8\n");   // Libera o espaço do $fp e $ra salvos.
    fprintf(arquivo_saida, "  addiu $sp, $sp, %d\n", 4 * n_params); // Libera o espaço dos argumentos passados.
    fprintf(arquivo_saida, "  jr $ra\n");              // Retorna para o endereço em $ra (jump register).

    // Destrói o escopo da função.
    desempilhar(&pilha_escopos_gc);
    // Indica que não estamos mais dentro de uma função.
    funcao_atual_gc = NULL;
}

// Gera código para uma declaração de variável.
void gc_declaracao_var(No* no) {
    char* nome_var = no->filho1->lexema; // Nome da variável.
    Simbolo s;
    strcpy(s.nome, nome_var);
    s.tipo_dado = string_para_tipo(no->lexema); // Tipo da variável.
    
    // Verifica se é uma variável global (declarada fora de qualquer função).
    if (funcao_atual_gc == NULL) { // Estamos no escopo global.
        s.escopo = 0; // Marca como escopo global.
        offset_global -= 4; // Decrementa o offset global (pilha cresce para baixo).
        s.num_params = offset_global; // Armazena o offset (reutilizando campo).
        inserir_na_pilha(&pilha_escopos_gc, s);
        // Aloca espaço na "área global" da pilha.
        fprintf(arquivo_saida, "  addiu $sp, $sp, -4\n");
    } else { // É uma variável local.
        offset_local -= 4; // Decrementa o offset local (relativo ao $fp).
        s.num_params = offset_local; // Armazena o offset.
        inserir_na_pilha(&pilha_escopos_gc, s);
        // O espaço para variáveis locais já foi alocado no prólogo da função.
    }
}

// Gera código para uma chamada de função.
void gc_chamada_funcao(No* no) {
    const char* nome_funcao = no->lexema;

    // Tratamento especial para a função "escreva".
    if (strcmp(nome_funcao, "escreva") == 0) {
        No* arg = no->filho1; // Pega o argumento.
        
        // Verifica se o argumento é uma constante string (contém aspas).
        if (arg->tipo_no == NO_CONST_CAR && strchr(arg->lexema, '"')) {
            // Procura a string na lista de strings pré-coletadas.
            StringLiteral* current = lista_strings;
            while(current) {
                if (strcmp(current->content, arg->lexema) == 0) break;
                current = current->next;
            }
            // Gera código para imprimir uma string.
            fprintf(arquivo_saida, "  la $a0, %s\n", current->label); // Carrega o endereço da string em $a0.
            fprintf(arquivo_saida, "  li $v0, 4\n");                 // Código de serviço 4 (print_string).
            fprintf(arquivo_saida, "  syscall\n");                   // Executa a chamada de sistema.
        } else {
            // Se não for uma string, avalia a expressão do argumento.
            visita_no_gc(arg);
            // O resultado da avaliação está em $s0. Move para $a0 (argumento da syscall).
            fprintf(arquivo_saida, "  move $a0, $s0\n");
            
            // Verifica o tipo do argumento para usar a syscall correta.
            if (arg->tipo_dado == TIPO_CAR) {
                fprintf(arquivo_saida, "  li $v0, 11\n"); // Código 11 (print_character).
            } else { // Assume TIPO_INT.
                fprintf(arquivo_saida, "  li $v0, 1\n");  // Código 1 (print_integer).
            }
            fprintf(arquivo_saida, "  syscall\n");
        }
        return; // Finaliza o tratamento de "escreva".
    }

    // Tratamento especial para a função "leia".
    if (strcmp(nome_funcao, "leia") == 0) {
        fprintf(arquivo_saida, "  li $v0, 5\n");      // Código de serviço 5 (read_integer).
        fprintf(arquivo_saida, "  syscall\n");        // O inteiro lido fica em $v0.
        get_endereco_var(no->filho1->lexema);         // Pega o endereço da variável de destino em $t0.
        fprintf(arquivo_saida, "  sw $v0, 0($t0)\n"); // Armazena o valor lido ($v0) no endereço em $t0.
        return;
    }

    // Tratamento especial para a função "novalinha".
    if (strcmp(nome_funcao, "novalinha") == 0) {
        fprintf(arquivo_saida, "  li $a0, '\\n'\n");   // Carrega o caractere de nova linha em $a0.
        fprintf(arquivo_saida, "  li $v0, 11\n");      // Código de serviço 11 (print_character).
        fprintf(arquivo_saida, "  syscall\n");         // Executa.
        return;
    }

    // --- Tratamento para chamadas de funções definidas pelo usuário ---
    
    // Coleta todos os argumentos em um array.
    No* args[20]; // Supõe um máximo de 20 argumentos.
    int n_args = 0;
    for (No* arg = no->filho1; arg != NULL; arg = arg->proximo) {
        args[n_args++] = arg;
    }

    // Empilha os argumentos na ordem inversa (da direita para a esquerda).
    for (int i = n_args - 1; i >= 0; i--) {
        No* arg_atual = args[i];
        // Temporariamente 'desconecta' o nó para visitar apenas ele.
        No* proximo_temp = arg_atual->proximo; 
        arg_atual->proximo = NULL;
        visita_no_gc(arg_atual); // Avalia a expressão do argumento, resultado em $s0.
        arg_atual->proximo = proximo_temp; // 'Reconecta' o nó.

        // Empilha o resultado da avaliação do argumento.
        fprintf(arquivo_saida, "  addiu $sp, $sp, -4\n"); // Abre espaço na pilha.
        fprintf(arquivo_saida, "  sw $s0, 0($sp)\n");      // Salva o resultado ($s0) na pilha.
    }
    
    // Chama a função.
    fprintf(arquivo_saida, "  jal %s\n", nome_funcao); // Jump And Link: salta para a função e salva o endereço de retorno em $ra.
}


// Gera código para uma operação de atribuição.
void gc_atribuicao(No* no) {
    // Avalia o lado direito da atribuição. O resultado vai para $s0.
    visita_no_gc(no->filho2);
    // Pega o endereço da variável do lado esquerdo. O endereço vai para $t0.
    get_endereco_var(no->filho1->lexema);
    // Armazena o resultado ($s0) no endereço da variável ($t0).
    fprintf(arquivo_saida, "  sw $s0, 0($t0)\n");
}


// Gera código para uma operação binária (aritmética, lógica, relacional).
void gc_op_binaria(No* no) {
    // Avalia a expressão da esquerda. Resultado em $s0.
    visita_no_gc(no->filho1);
    // Salva o resultado da esquerda na pilha temporariamente.
    fprintf(arquivo_saida, "  addiu $sp, $sp, -4\n");
    fprintf(arquivo_saida, "  sw $s0, 0($sp)\n");
    // Avalia a expressão da direita. Resultado em $s0.
    visita_no_gc(no->filho2);
    // Recupera o resultado da esquerda da pilha para $t1.
    fprintf(arquivo_saida, "  lw $t1, 0($sp)\n");
    fprintf(arquivo_saida, "  addiu $sp, $sp, 4\n");
    // Executa a operação MIPS. Ex: add $s0, $t1, $s0  ($s0 = $t1 + $s0)
    fprintf(arquivo_saida, "  %s $s0, $t1, $s0\n", get_op_mips(no->lexema));
    // O resultado de uma operação é sempre um inteiro (ou booleano, que é 0 ou 1).
    no->tipo_dado = TIPO_INT; 
}

// Gera código para a instrução 'retorna'.
void gc_retorno(No* no) {
    if (no->filho1) {
        // Se houver uma expressão de retorno, avalia-a.
        // O valor de retorno, por convenção, deve ser colocado em $v0, mas aqui
        // o código o coloca em $s0. O epílogo pode mover de $s0 para $v0 se necessário,
        // ou o chamador pode esperar o resultado em $s0. (Neste código, o resultado da função
        // parece ser implicitamente deixado em $s0, e o chamador o usa a partir daí).
        visita_no_gc(no->filho1);
    }
    // Salta para o epílogo da função para restaurar a pilha e retornar.
    fprintf(arquivo_saida, "  j %s_epilogo\n", funcao_atual_gc->nome);
}

// Gera código para um bloco de comandos.
void gc_bloco(No* no) {
    // Visita a lista de declarações (se houver).
    visita_no_gc(no->filho1);
    // Visita a lista de comandos.
    visita_no_gc(no->filho2);
}

// Gera código para uma estrutura condicional 'se' (if).
void gc_if(No* no) {
    int l_else = novo_label(); // Cria um rótulo para o bloco 'senao'.
    int l_fim = novo_label();  // Cria um rótulo para o final do 'se'.
    
    // Avalia a condição. O resultado (0 para falso, não-zero para verdadeiro) fica em $s0.
    visita_no_gc(no->filho1);
    
    // Se o resultado for zero (falso), salta para o bloco 'senao'.
    fprintf(arquivo_saida, "  beqz $s0, L_ELSE_%d\n", l_else);
    
    // Gera código para o bloco 'entao' (corpo do if).
    visita_no_gc(no->filho2);
    
    // Salta incondicionalmente para o final do 'se' para não executar o 'senao'.
    fprintf(arquivo_saida, "  j L_FIM_IF_%d\n", l_fim);
    
    // Imprime o rótulo do bloco 'senao'.
    fprintf(arquivo_saida, "L_ELSE_%d:\n", l_else);
    if (no->filho3) {
        // Se existir um bloco 'senao', gera o código para ele.
        visita_no_gc(no->filho3);
    }
    
    // Imprime o rótulo do final do 'se'.
    fprintf(arquivo_saida, "L_FIM_IF_%d:\n", l_fim);
}

// Gera código para um laço 'enquanto' (while).
void gc_while(No* no) {
    int l_inicio = novo_label(); // Cria rótulo para o início do laço (teste da condição).
    int l_fim = novo_label();    // Cria rótulo para o fim do laço.
    
    // Imprime o rótulo de início.
    fprintf(arquivo_saida, "L_WHILE_%d:\n", l_inicio);
    
    // Avalia a condição do laço. Resultado em $s0.
    visita_no_gc(no->filho1);
    
    // Se a condição for falsa (resultado é 0), salta para o fim do laço.
    fprintf(arquivo_saida, "  beqz $s0, L_FIM_WHILE_%d\n", l_fim);
    
    // Gera código para o corpo do laço.
    visita_no_gc(no->filho2);
    
    // Salta de volta para o início do laço para reavaliar a condição.
    fprintf(arquivo_saida, "  j L_WHILE_%d\n", l_inicio);
    
    // Imprime o rótulo de fim do laço.
    fprintf(arquivo_saida, "L_FIM_WHILE_%d:\n", l_fim);
}

// Função principal de visitação da árvore (Dispatcher).
// Ela verifica o tipo de cada nó e chama a função de geração de código apropriada.
void visita_no_gc(No* no) {
    if (!no) return; // Condição de parada da recursão.
    
    switch (no->tipo_no) {
        case NO_PROGRAMA:
            // Visita as declarações globais (variáveis e funções).
            visita_no_gc(no->filho1);
            // Inicia o ponto de entrada principal do programa.
            fprintf(arquivo_saida, "\n# ---- Bloco Principal (programa) ----\n");
            fprintf(arquivo_saida, "main:\n");
            // Salva o ponteiro de pilha inicial em $s1 para ser a base das variáveis globais.
            fprintf(arquivo_saida, "  move $s1, $sp\n");
            // Visita o bloco de comandos principal do programa.
            visita_no_gc(no->filho2);
            // Salta para o final do programa para encerrar a execução.
            fprintf(arquivo_saida, "  j end_main\n");
            break;
            
        // Casos que chamam as funções 'gc_' específicas.
        case NO_DECL_VAR:       gc_declaracao_var(no); break;
        case NO_DECL_FUNCAO:    gc_declaracao_funcao(no); break;
        case NO_BLOCO:          gc_bloco(no); break;
        case NO_ATRIBUICAO:     gc_atribuicao(no); break;
        case NO_IF:             gc_if(no); break;
        case NO_WHILE:          gc_while(no); break;
        case NO_CHAMADA_FUNCAO: gc_chamada_funcao(no); break;
        case NO_RETORNO:        gc_retorno(no); break;
        
        // Operadores são todos tratados pela mesma função.
        case NO_OP_ARITMETICO:
        case NO_OP_LOGICO:
        case NO_OP_RELACIONAL:
            gc_op_binaria(no); break;
            
        case NO_NEGACAO: // Operador 'nao'
            visita_no_gc(no->filho1); // Avalia a expressão.
            // Compara o resultado com zero. Se for igual a zero, $s0 = 1, senão $s0 = 0.
            // Isso inverte o valor booleano.
            fprintf(arquivo_saida, "  seq $s0, $s0, $zero\n"); break;
            
        case NO_IDENTIFICADOR: // Uso de uma variável em uma expressão.
            { // Bloco para permitir a declaração de variável local 's'.
                // Pega o endereço da variável e coloca em $t0.
                get_endereco_var(no->lexema);
                // Carrega o valor que está no endereço ($t0) para o registrador $s0.
                fprintf(arquivo_saida, "  lw $s0, 0($t0)\n");
                // Atualiza o tipo do nó na árvore com o tipo da variável (para checagens futuras).
                Simbolo* s = buscar_em_todos_escopos(&pilha_escopos_gc, no->lexema);
                if (s) no->tipo_dado = s->tipo_dado;
            }
            break;
            
        case NO_CONST_INT: // Uma constante inteira.
            // Carrega o valor literal inteiro no registrador $s0.
            fprintf(arquivo_saida, "  li $s0, %s\n", no->lexema);
            no->tipo_dado = TIPO_INT; // Define o tipo do nó.
            break;
            
        case NO_CONST_CAR: // Uma constante caractere ou string.
            // Verifica se é uma string (contém aspas).
            if (strchr(no->lexema, '"')) {
                // Se for string, o código é gerado na chamada de "escreva", não aqui.
            } else {
                // Se for um caractere (ex: 'a'), carrega seu valor ASCII em $s0.
                fprintf(arquivo_saida, "  li $s0, %s\n", no->lexema);
                no->tipo_dado = TIPO_CAR; // Define o tipo do nó.
            }
            break;
            
        default: // Caso padrão para nós não listados (ex: listas).
            // Apenas continua a visitação recursiva pelos filhos.
            visita_no_gc(no->filho1); visita_no_gc(no->filho2);
            visita_no_gc(no->filho3); visita_no_gc(no->filho4); break;
    }
    // Continua a visitação para o próximo nó na mesma lista (nós irmãos).
    visita_no_gc(no->proximo);
}

// Percorre a árvore (antes da geração de código) para encontrar todos os literais de string.
void coletar_strings(No* no) {
    if (!no) return; // Condição de parada da recursão.
    
    // Procura por chamadas da função 'escreva' com argumento string.
    if (no->tipo_no == NO_CHAMADA_FUNCAO && strcmp(no->lexema, "escreva") == 0) {
        if (no->filho1 && no->filho1->tipo_no == NO_CONST_CAR && strchr(no->filho1->lexema, '"')) {
            
            // Verifica se a string já foi adicionada à lista para evitar duplicatas.
            StringLiteral* current = lista_strings; int encontrada = 0;
            while(current) {
                if (strcmp(current->content, no->filho1->lexema) == 0) { encontrada = 1; break; }
                current = current->next;
            }
            
            // Se não encontrou, adiciona a nova string à lista.
            if (!encontrada) {
                StringLiteral* nova_str = (StringLiteral*) malloc(sizeof(StringLiteral));
                sprintf(nova_str->label, "str_%d", contador_string++); // Cria um rótulo "str_N".
                nova_str->content = no->filho1->lexema; // Aponta para o conteúdo.
                nova_str->next = lista_strings; // Insere no início da lista.
                lista_strings = nova_str;
            }
        }
    }
    // Continua a busca recursivamente por toda a árvore.
    coletar_strings(no->filho1); coletar_strings(no->filho2);
    coletar_strings(no->filho3); coletar_strings(no->filho4);
    coletar_strings(no->proximo);
}

// Função principal que orquestra a geração de código MIPS.
void gerar_codigo(No* raiz_arvore, const char* nome_arquivo_saida) {
    // Abre o arquivo de saída para escrita.
    arquivo_saida = fopen(nome_arquivo_saida, "w");
    if (!arquivo_saida) {
        perror("Erro ao criar arquivo de saída");
        exit(1);
    }
    
    // Prepara a pilha de escopos, começando pelo escopo global.
    inicializar_pilha(&pilha_escopos_gc);
    empilhar(&pilha_escopos_gc); // Escopo global.

    // 1. Primeira Passada: Coleta todas as strings para a seção .data.
    coletar_strings(raiz_arvore);

    // 2. Geração da Seção .data
    fprintf(arquivo_saida, ".data\n");
    StringLiteral* current = lista_strings;
    while (current) {
        // Para cada string na lista, escreve sua declaração no arquivo .asm.
        fprintf(arquivo_saida, "%s: .asciiz %s\n", current->label, current->content);
        current = current->next;
    }

    // 3. Geração da Seção .text (código executável)
    fprintf(arquivo_saida, ".text\n");
    fprintf(arquivo_saida, ".globl main\n\n"); // Declara 'main' como um símbolo global.
    fprintf(arquivo_saida, "j main\n\n");     // Salto inicial para o label 'main'.

    // 4. Segunda Passada: Percorre a árvore para gerar o código das instruções.
    visita_no_gc(raiz_arvore);

    // 5. Geração do Código de Finalização do Programa
    fprintf(arquivo_saida, "\nend_main:\n");    // Rótulo para o fim da execução.
    fprintf(arquivo_saida, "  li $v0, 10\n");   // Carrega o código de serviço 10 (exit).
    fprintf(arquivo_saida, "  syscall\n");      // Encerra o programa.

    // Fecha o arquivo de saída.
    fclose(arquivo_saida);

    // Libera a memória alocada para a lista de strings.
    while(lista_strings) {
        StringLiteral* temp = lista_strings;
        lista_strings = lista_strings->next;
        free(temp);
    }
    
    printf("Geração de código concluída. Arquivo '%s' criado.\n", nome_arquivo_saida);
}