%{
/* ========================================================================== */
/* --- Seção 1: Código C, Definições e Declarações -------------------------- */
/* ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Funções e variáveis externas que o analisador sintático (parser) precisa acessar. */
extern int yylex();         /* A função principal do analisador léxico (gerada pelo Flex). O parser a chama para obter o próximo token. */
extern int yylineno;        /* A variável que armazena o número da linha atual (do Flex). */
void yyerror(const char *s);/* A função que o parser chama quando encontra um erro de sintaxe. Ela precisa ser implementada em outro lugar (ex: main.c). */
%}

/* O bloco '%code requires' é uma diretiva moderna do Bison. O código aqui é colocado
   em um local do arquivo gerado onde as definições de tipo do Bison (como a %union)
   já são visíveis. É o local correto para incluir cabeçalhos que dependem dessas
   definições, como o da nossa Árvore Sintática Abstrata (ASA). */
%code requires {
    #include "arvore.h"       /* Define a estrutura 'No' e as enumerações de tipos de nós. */
    extern No* raiz_arvore; /* Declara a variável global (definida em main.c) que irá
                               armazenar a raiz da ASA construída. */
}

/* A união '%union' define os diferentes tipos de dados que um símbolo (terminal ou não-terminal)
   pode ter. O analisador léxico e as regras sintáticas usam esta união para trocar informações. */
%union {
    char* str_lexema; /* Para tokens que carregam um valor de string (ex: um ID, um número, um operador como "=="). */
    No* no_ptr;     /* Para símbolos não-terminais que, ao serem reduzidos, resultam em um ponteiro para um nó da ASA. */
}

/* --- Declaração de Tokens (Símbolos Terminais) --- */
/* Aqui listamos todos os tokens que o analisador léxico pode retornar. */
%token <str_lexema> ID INTCONST CARCONST CAD_CAR       /* Tokens que carregam um lexema em 'str_lexema'. */
%token PROGRAM CAR INT RETORNE LEIA ESCREVA NOVALINHA
%token SE ENTAO SENAO ENQUANTO EXECUTE
%token <str_lexema> DIF IGUAL MENOR_IGUAL MAIOR_IGUAL MENOR MAIOR E OU /* Operadores que também carregam o lexema. */

/* --- Declaração de Tipos para Não-Terminais --- */
/* Aqui, associamos os símbolos não-terminais (regras da gramática) a um tipo da %union.
   Quase todos resultarão em um ponteiro para um nó da ASA. */
%type <no_ptr> Programa DeclFuncVar DeclProg DeclVar DeclFunc ListaParametros ListaParametrosCont
%type <no_ptr> Bloco ListaDeclVar Tipo ListaComando Comando Expr OrExpr AndExpr
%type <no_ptr> EqExpr DesigExpr AddExpr MulExpr UnExpr PrimExpr ListExpr

/* --- Precedência e Associatividade de Operadores --- */
/* Esta seção é crucial para resolver ambiguidades em expressões (ex: 2+3*4)
   sem precisar de regras gramaticais extras e complexas.
   A precedência aumenta de cima para baixo (ou seja, '=' tem a menor precedência). */
%right '='                                    /* Associatividade à direita para atribuição (a=b=c -> a=(b=c)). */
%left  OU                                     /* Associatividade à esquerda para os operadores lógicos e relacionais. */
%left  E
%left  IGUAL DIF
%left  MENOR MAIOR MENOR_IGUAL MAIOR_IGUAL
%left  '+' '-'                                /* Associatividade à esquerda para os aritméticos. */
%left  '*' '/'
%right '!' UNEG                               /* Associatividade à direita para operadores unários (prefixo). 'UNEG' é um token fantasma para o menos unário. */
%nonassoc ENTAO                               /* Usado para resolver a ambiguidade do 'dangling-else'. Diz que um 'if-then' não se associa. */
%nonassoc SENAO                               /* O 'else' tem maior precedência, ligando-se ao 'if' mais próximo. */

/* Define o símbolo inicial da gramática. A análise começa aqui. */
%start Programa

%%
/* ========================================================================== */
/* --- Seção 2: Regras da Gramática e Ações Semânticas ---------------------- */
/* ========================================================================== */
/* Dentro das ações { ... }:
   - $$: representa o valor do símbolo à esquerda da regra (o resultado).
   - $1, $2, ...: representam os valores dos símbolos à direita, da esquerda para a direita.
   - yylineno: a linha atual, útil para registrar nos nós da ASA. */

Programa
    : DeclFuncVar DeclProg
    {
        /* Ação: Nó raiz do programa. */
        $$ = cria_no(NO_PROGRAMA, yylineno, NULL); /* Cria o nó 'Programa'. */
        $$->filho1 = $1; /* O primeiro filho é a lista de declarações de funções/variáveis globais. */
        $$->filho2 = $2; /* O segundo filho é o bloco principal 'programa'. */
        raiz_arvore = $$; /* Armazena o nó raiz na variável global. */
    }
    ;

DeclFuncVar
    : Tipo ID DeclVar ';' DeclFuncVar
    {
        /* Regra para declaração de uma ou mais variáveis (ex: int a, b;). */
        /* 1. Cria o nó de declaração para o primeiro ID ($2). */
        No* prim_decl = cria_no(NO_DECL_VAR, yylineno, $1->lexema); /* Usa o lexema do tipo (ex: "int"). */
        prim_decl->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2);
        No* ult_decl = prim_decl; /* Ponteiro para a última declaração na lista que estamos construindo. */

        /* 2. Itera sobre a lista de IDs adicionais retornada por DeclVar ($3). */
        No* id_node = $3;
        while (id_node) {
            No* decl_atual = cria_no(NO_DECL_VAR, yylineno, $1->lexema); /* Cria nó para o ID atual. */
            decl_atual->filho1 = id_node;
            
            ult_decl->proximo = decl_atual; /* Encadeia a nova declaração à lista. */
            ult_decl = decl_atual;

            No* temp = id_node;
            id_node = id_node->proximo; /* Avança para o próximo ID na lista. */
            temp->proximo = NULL; /* Isola o nó de ID que acabamos de usar. */
        }

        /* 3. Conecta a lista que criamos com o resto das declarações de funções/variáveis ($5). */
        ult_decl->proximo = $5;
        $$ = prim_decl; /* O resultado da regra é o início da lista de declarações. */
        free($1->lexema); free($1); free($2); /* Libera memória intermediária. */
    }
    | Tipo ID DeclFunc DeclFuncVar
    {
        /* Regra para uma declaração de função. */
        No* decl_func = $3; /* Pega o nó de função criado pela regra 'DeclFunc'. */
        /* A regra 'DeclFunc' cria um nó genérico; aqui nós o completamos. */
        decl_func->filho1 = $1;   /* Atribui o tipo de retorno. */
        decl_func->lexema = $2;   /* Atribui o nome da função. */
        
        decl_func->proximo = $4; /* Encadeia com o resto das declarações. */
        $$ = decl_func;          /* O resultado é o nó da função. */
    }
    | /* vazio */ { $$ = NULL; } /* Uma lista de declarações pode ser vazia. */
    ;

DeclProg
    : PROGRAM Bloco { $$ = $2; } /* O bloco principal do programa é simplesmente um nó de Bloco. */
    ;

DeclVar
    : ',' ID DeclVar
    {
        /* Regra para listas de IDs em declarações (ex: o ", b, c" de "int a, b, c"). */
        /* Retorna uma lista encadeada de nós de IDENTIFICADOR. */
        $$ = cria_no(NO_IDENTIFICADOR, yylineno, $2);
        $$->proximo = $3;
        free($2);
    }
    | /* vazio */ { $$ = NULL; }
    ;

DeclFunc
    : '(' ListaParametros ')' Bloco
    {
        /* Cria um nó de função PARCIAL. O nome e o tipo são preenchidos pela regra pai (DeclFuncVar). */
        $$ = cria_no(NO_DECL_FUNCAO, yylineno, NULL); /* Nome (lexema) é NULL por enquanto. */
        $$->filho2 = $2; /* O segundo filho são os parâmetros. */
        $$->filho3 = $4; /* O terceiro filho é o corpo da função (bloco). */
    }
    ;

ListaParametros
    : ListaParametrosCont { $$ = $1; }    /* Uma lista de parâmetros. */
    | /* vazio */         { $$ = NULL; } /* Uma função pode não ter parâmetros. */
    ;

ListaParametrosCont
    : Tipo ID
    {
        /* Parâmetro único ou o último de uma lista. */
        $$ = cria_no(NO_PARAM, yylineno, $1->lexema); /* O lexema do nó guarda o tipo do parâmetro. */
        $$->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2); /* O filho guarda o nome. */
        free($1->lexema); free($1); free($2);
    }
    | Tipo ID ',' ListaParametrosCont
    {
        /* Um parâmetro seguido por outros. */
        $$ = cria_no(NO_PARAM, yylineno, $1->lexema);
        $$->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2);
        $$->proximo = $4; /* Encadeia com o resto da lista de parâmetros. */
        free($1->lexema); free($1); free($2);
    }
    ;

Bloco
    : '{' ListaDeclVar ListaComando '}'
    {
        /* Um bloco de código. */
        $$ = cria_no(NO_BLOCO, yylineno, NULL);
        $$->filho1 = $2; /* Filho 1: lista de declarações de variáveis locais. */
        $$->filho2 = $3; /* Filho 2: lista de comandos. */
    }
    ;

ListaDeclVar
    /* Esta regra é uma cópia da lógica de 'DeclFuncVar' para declarações locais. */
    : Tipo ID DeclVar ';' ListaDeclVar
    {
        No* prim_decl = cria_no(NO_DECL_VAR, yylineno, $1->lexema);
        prim_decl->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2);
        No* ult_decl = prim_decl;
        No* id_node = $3;
        while (id_node) {
            No* decl_atual = cria_no(NO_DECL_VAR, yylineno, $1->lexema);
            decl_atual->filho1 = id_node;
            ult_decl->proximo = decl_atual;
            ult_decl = decl_atual;
            No* temp = id_node;
            id_node = id_node->proximo;
            temp->proximo = NULL;
        }
        ult_decl->proximo = $5;
        $$ = prim_decl;
        free($1->lexema); free($1); free($2);
    }
    | /* vazio */ { $$ = NULL; }
    ;

Tipo
    /* Regra para reconhecer tipos. Cria um nó temporário que será usado
       pelas regras de declaração. O lexema do nó é o nome do tipo. */
    : INT { $$ = cria_no(NO_DECL_VAR, yylineno, "int"); }
    | CAR { $$ = cria_no(NO_DECL_VAR, yylineno, "car"); }
    ;

ListaComando
    : Comando ListaComando
    {
        /* Constrói uma lista encadeada de comandos. */
        No* p = $1;
        if(p) { /* Se o primeiro comando ($1) não for nulo (ex: ';'). */
            while (p && p->proximo) p = p->proximo; /* Encontra o fim da lista iniciada por $1. */
            if (p) p->proximo = $2; /* Anexa o resto da lista ($2). */
            $$ = $1;
        } else {
            $$ = $2; /* Se $1 era nulo, a lista começa em $2. */
        }
    }
    | Comando { $$ = $1; }
    ;

Comando
    /* Um comando pode ser uma expressão, um retorno, uma chamada, um condicional, etc. */
    : Expr ';'              { $$ = $1; }
    | RETORNE Expr ';'      { $$ = cria_no(NO_RETORNO, yylineno, NULL); $$->filho1 = $2; }
    
    // REGRA CORRIGIDA PARA LEIA
    | LEIA ID ';'           { 
                                $$ = cria_no(NO_CHAMADA_FUNCAO, yylineno, "leia"); 
                                $$->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2); 
                                free($2);
                            }
    // REGRA CORRIGIDA PARA ESCREVA
    | ESCREVA Expr ';'      { 
                                $$ = cria_no(NO_CHAMADA_FUNCAO, yylineno, "escreva"); 
                                $$->filho1 = $2; 
                            }
    // REGRA CORRIGIDA PARA ESCREVA COM CADEIA DE CARACTERES (já estava quase certo)
    | ESCREVA CAD_CAR ';'   { 
                                No* str_node = cria_no(NO_CONST_CAR, yylineno, $2);
                                $$ = cria_no(NO_CHAMADA_FUNCAO, yylineno, "escreva");
                                $$->filho1 = str_node;
                                free($2);
                            }
    // REGRA CORRIGIDA PARA NOVALINHA
    | NOVALINHA ';'         { 
                                $$ = cria_no(NO_CHAMADA_FUNCAO, yylineno, "novalinha");
                                $$->filho1 = NULL; // Sem argumentos
                            }
    
    | SE '(' Expr ')' ENTAO Comando           { $$ = cria_no(NO_IF, yylineno, NULL); $$->filho1 = $3; $$->filho2 = $6; $$->filho3 = NULL; }
    | SE '(' Expr ')' ENTAO Comando SENAO Comando { $$ = cria_no(NO_IF, yylineno, NULL); $$->filho1 = $3; $$->filho2 = $6; $$->filho3 = $8; }
    | ENQUANTO '(' Expr ')' EXECUTE Comando   { $$ = cria_no(NO_WHILE, yylineno, NULL); $$->filho1 = $3; $$->filho2 = $6; }
    | Bloco                 { $$ = $1; }
    | ';'                   { $$ = NULL; } /* Comando vazio, resulta em nada na ASA. */
    ;

Expr
    /* As expressões são divididas em múltiplos níveis para implementar a precedência. */
    : ID '=' Expr
    {
        /* Atribuição. */
        $$ = cria_no(NO_ATRIBUICAO, yylineno, NULL);
        $$->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $1);
        $$->filho2 = $3;
        free($1);
    }
    | OrExpr { $$ = $1; }
    ;

/* --- Níveis de Precedência de Expressões --- */
/* Cada regra passa o controle para a regra de maior precedência, e se não houver operador
   daquele nível, ela simplesmente passa o resultado da regra de maior precedência para cima. */
OrExpr    : OrExpr OU AndExpr { $$ = cria_no(NO_OP_LOGICO, yylineno, "ou"); $$->filho1 = $1; $$->filho2 = $3; }
          | AndExpr { $$ = $1; } ;

AndExpr   : AndExpr E EqExpr { $$ = cria_no(NO_OP_LOGICO, yylineno, "e"); $$->filho1 = $1; $$->filho2 = $3; }
          | EqExpr { $$ = $1; } ;

EqExpr    : EqExpr IGUAL DesigExpr { $$ = cria_no(NO_OP_RELACIONAL, yylineno, $2); $$->filho1 = $1; $$->filho2 = $3; free($2); }
          | EqExpr DIF DesigExpr { $$ = cria_no(NO_OP_RELACIONAL, yylineno, $2); $$->filho1 = $1; $$->filho2 = $3; free($2); }
          | DesigExpr { $$ = $1; } ;

DesigExpr : DesigExpr MENOR AddExpr { $$ = cria_no(NO_OP_RELACIONAL, yylineno, $2); $$->filho1 = $1; $$->filho2 = $3; free($2); }
          | DesigExpr MAIOR AddExpr { $$ = cria_no(NO_OP_RELACIONAL, yylineno, $2); $$->filho1 = $1; $$->filho2 = $3; free($2); }
          | DesigExpr MAIOR_IGUAL AddExpr { $$ = cria_no(NO_OP_RELACIONAL, yylineno, $2); $$->filho1 = $1; $$->filho2 = $3; free($2); }
          | DesigExpr MENOR_IGUAL AddExpr { $$ = cria_no(NO_OP_RELACIONAL, yylineno, $2); $$->filho1 = $1; $$->filho2 = $3; free($2); }
          | AddExpr { $$ = $1; } ;

AddExpr   : AddExpr '+' MulExpr { $$ = cria_no(NO_OP_ARITMETICO, yylineno, "+"); $$->filho1 = $1; $$->filho2 = $3; }
          | AddExpr '-' MulExpr { $$ = cria_no(NO_OP_ARITMETICO, yylineno, "-"); $$->filho1 = $1; $$->filho2 = $3; }
          | MulExpr { $$ = $1; } ;
          
MulExpr   : MulExpr '*' UnExpr { $$ = cria_no(NO_OP_ARITMETICO, yylineno, "*"); $$->filho1 = $1; $$->filho2 = $3; }
          | MulExpr '/' UnExpr { $$ = cria_no(NO_OP_ARITMETICO, yylineno, "/"); $$->filho1 = $1; $$->filho2 = $3; }
          | UnExpr { $$ = $1; } ;

UnExpr
    : '-' PrimExpr %prec UNEG /* O '%prec UNEG' força a precedência deste operador unário a ser a definida por 'UNEG'. */
    {
        /* Representa o menos unário como uma multiplicação por -1. */
        $$ = cria_no(NO_OP_ARITMETICO, yylineno, "*");
        $$->filho1 = cria_no(NO_CONST_INT, yylineno, "-1");
        $$->filho2 = $2;
    }
    | '!' PrimExpr %prec UNEG
    {
        /* Operador de negação lógica. */
        $$ = cria_no(NO_NEGACAO, yylineno, "!");
        $$->filho1 = $2;
    }
    | PrimExpr { $$ = $1; }
    ;

PrimExpr
    /* Expressões primárias: são os "átomos" das expressões (identificadores, constantes, chamadas de função). */
    : ID '(' ListExpr ')'
    {
        /* Chamada de função com um ou mais argumentos. */
        $$ = cria_no(NO_CHAMADA_FUNCAO, yylineno, $1);
        $$->filho1 = $3;
        free($1);
    }
    | ID '(' ')'
    {
        /* Chamada de função sem argumentos. */
        $$ = cria_no(NO_CHAMADA_FUNCAO, yylineno, $1);
        $$->filho1 = NULL;
        free($1);
    }
    | ID        { $$ = cria_no(NO_IDENTIFICADOR, yylineno, $1); free($1); }
    | INTCONST  { $$ = cria_no(NO_CONST_INT, yylineno, $1); free($1); }
    | CARCONST  { $$ = cria_no(NO_CONST_CAR, yylineno, $1); free($1); }
    | '(' Expr ')' { $$ = $2; } /* Expressão entre parênteses para forçar a ordem de avaliação. */
    ;

ListExpr
    /* Regra para a lista de argumentos em uma chamada de função. */
    : ListExpr ',' Expr
    {
        /* Anexa uma nova expressão ($3) à lista de argumentos existente ($1). */
        No* p = $1;
        while(p && p->proximo) p = p->proximo;
        if (p) p->proximo = $3;
        $$ = $1;
    }
    | Expr { $$ = $1; } /* Uma lista de argumentos pode ser apenas uma única expressão. */
    ;

%%
/* ========================================================================== */
/* --- Seção 3: Código C Adicional ------------------------------------------ */
/* ========================================================================== */
/* A função yyerror(const char*) deve ser definida em um dos seus arquivos .c
   (provavelmente main.c), pois o linker a encontrará e a conectará com as
   chamadas feitas pelo parser gerado. */