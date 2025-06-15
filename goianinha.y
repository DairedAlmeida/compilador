%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Funções e variáveis externas
extern int yylex();
extern int yylineno;
void yyerror(const char *s);
%}

%code requires {
    #include "arvore.h"
    extern No* raiz_arvore;
}

%union {
    char* str_lexema;
    No* no_ptr;
}

// ==========================================================
// TOKENS DA SUA GRAMÁTICA ORIGINAL
// ==========================================================
%token <str_lexema> ID INTCONST CARCONST CAD_CAR
%token PROGRAM CAR INT RETORNE LEIA ESCREVA NOVALINHA
%token SE ENTAO SENAO ENQUANTO EXECUTE
%token <str_lexema> DIF IGUAL MENOR_IGUAL MAIOR_IGUAL MENOR MAIOR E OU

// ==========================================================
// TIPOS DOS NÃO-TERMINAIS DA SUA GRAMÁTICA ORIGINAL
// ==========================================================
%type <no_ptr> Programa DeclFuncVar DeclProg DeclVar DeclFunc ListaParametros ListaParametrosCont
%type <no_ptr> Bloco ListaDeclVar Tipo ListaComando Comando Expr OrExpr AndExpr
%type <no_ptr> EqExpr DesigExpr AddExpr MulExpr UnExpr PrimExpr ListExpr

// ==========================================================
// PRECEDÊNCIA DE OPERADORES DA SUA GRAMÁTICA ORIGINAL
// ==========================================================
%right '='
%left OU
%left E
%left IGUAL DIF
%left MENOR MAIOR MENOR_IGUAL MAIOR_IGUAL
%left '+' '-'
%left '*' '/'
%right '!' UNEG // '!' é o NEG da sua gramática original

%start Programa

%%

/*******************************************************
 * GRAMÁTICA 1:1 COM A VERSÃO ANTIGA, COM LÓGICA DE ASA *
 *******************************************************/

Programa
    : DeclFuncVar DeclProg
    {
        $$ = cria_no(NO_PROGRAMA, yylineno, NULL);
        $$->filho1 = $1;
        $$->filho2 = $2;
        raiz_arvore = $$;
    }
    ;

DeclFuncVar
    : Tipo ID DeclVar ';' DeclFuncVar
    {
        // Lógica para tratar declarações múltiplas (ex: int a, b, c;)
        // 1. Cria a declaração para o primeiro ID
        No* prim_decl = cria_no(NO_DECL_VAR, yylineno, $1->lexema);
        prim_decl->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2);
        No* ult_decl = prim_decl;

        // 2. Itera sobre a lista de IDs de DeclVar (se houver)
        No* id_node = $3;
        while (id_node) {
            No* decl_atual = cria_no(NO_DECL_VAR, yylineno, $1->lexema);
            decl_atual->filho1 = id_node;
            
            ult_decl->proximo = decl_atual;
            ult_decl = decl_atual;

            No* temp = id_node;
            id_node = id_node->proximo;
            temp->proximo = NULL; // Isola o nó de ID antes de usá-lo
        }

        // 3. Junta com o resto das declarações de funções/variáveis
        ult_decl->proximo = $5;
        $$ = prim_decl;
        free($1->lexema); free($1); free($2);
    }
    | Tipo ID DeclFunc DeclFuncVar
    {
        No* decl_func = $3;
        // O nó de função precisa do tipo e do nome
        decl_func->filho1 = $1;
        decl_func->lexema = $2; // Atribui o nome da função
        
        // Conecta na lista principal de declarações
        decl_func->proximo = $4;
        $$ = decl_func;
    }
    | /* vazio */ { $$ = NULL; }
    ;

DeclProg
    : PROGRAM Bloco { $$ = $2; }
    ;

DeclVar
    : ',' ID DeclVar
    {
        // Retorna uma lista de nós de identificadores
        $$ = cria_no(NO_IDENTIFICADOR, yylineno, $2);
        $$->proximo = $3;
        free($2);
    }
    | /* vazio */ { $$ = NULL; }
    ;

DeclFunc
    : '(' ListaParametros ')' Bloco
    {
        // Cria um nó de função ainda sem nome ou tipo, que serão preenchidos na regra pai
        $$ = cria_no(NO_DECL_FUNCAO, yylineno, NULL);
        $$->filho2 = $2; // Parâmetros
        $$->filho3 = $4; // Bloco
    }
    ;

ListaParametros
    : ListaParametrosCont { $$ = $1; }
    | /* vazio */ { $$ = NULL; }
    ;

ListaParametrosCont
    : Tipo ID
    {
        $$ = cria_no(NO_PARAM, yylineno, $1->lexema);
        $$->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2);
        free($1->lexema); free($1); free($2);
    }
    | Tipo ID ',' ListaParametrosCont
    {
        $$ = cria_no(NO_PARAM, yylineno, $1->lexema);
        $$->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2);
        $$->proximo = $4;
        free($1->lexema); free($1); free($2);
    }
    ;

Bloco
    : '{' ListaDeclVar ListaComando '}'
    {
        $$ = cria_no(NO_BLOCO, yylineno, NULL);
        $$->filho1 = $2;
        $$->filho2 = $3;
    }
    ;

ListaDeclVar
    : Tipo ID DeclVar ';' ListaDeclVar
    {
        // Lógica para tratar declarações múltiplas locais
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
    : INT { $$ = cria_no(NO_DECL_VAR, yylineno, "int"); }
    | CAR { $$ = cria_no(NO_DECL_VAR, yylineno, "car"); }
    ;

ListaComando
    : Comando ListaComando
    {
        No* p = $1;
        if(p) {
            while (p && p->proximo) p = p->proximo;
            if (p) p->proximo = $2;
            $$ = $1;
        } else {
            $$ = $2;
        }
    }
    | Comando { $$ = $1; }
    ;

Comando
    : Expr ';'            { $$ = $1; }
    | RETORNE Expr ';'    { $$ = cria_no(NO_RETORNO, yylineno, NULL); $$->filho1 = $2; }
    | LEIA ID ';'         { $$ = cria_no(NO_IDENTIFICADOR, yylineno, "leia"); $$->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $2); free($2); }
    | ESCREVA Expr ';'    { $$ = cria_no(NO_IDENTIFICADOR, yylineno, "escreva"); $$->filho1 = $2; }
    | ESCREVA CAD_CAR ';' { $$ = cria_no(NO_IDENTIFICADOR, yylineno, "escreva"); $$->filho1 = cria_no(NO_CONST_CAR, yylineno, $2); free($2); }
    | NOVALINHA ';'       { $$ = cria_no(NO_IDENTIFICADOR, yylineno, "novalinha"); }
    | SE '(' Expr ')' ENTAO Comando               { $$ = cria_no(NO_IF, yylineno, NULL); $$->filho1 = $3; $$->filho2 = $6; $$->filho3 = NULL; }
    | SE '(' Expr ')' ENTAO Comando SENAO Comando { $$ = cria_no(NO_IF, yylineno, NULL); $$->filho1 = $3; $$->filho2 = $6; $$->filho3 = $8; }
    | ENQUANTO '(' Expr ')' EXECUTE Comando       { $$ = cria_no(NO_WHILE, yylineno, NULL); $$->filho1 = $3; $$->filho2 = $6; }
    | Bloco               { $$ = $1; }
    | ';'                 { $$ = NULL; }
    ;

Expr
    : ID '=' Expr
    {
        $$ = cria_no(NO_ATRIBUICAO, yylineno, NULL);
        $$->filho1 = cria_no(NO_IDENTIFICADOR, yylineno, $1);
        $$->filho2 = $3;
        free($1);
    }
    | OrExpr { $$ = $1; }
    ;

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
    : '-' PrimExpr %prec UNEG
    {
        $$ = cria_no(NO_OP_ARITMETICO, yylineno, "*");
        $$->filho1 = cria_no(NO_CONST_INT, yylineno, "-1");
        $$->filho2 = $2;
    }
    | '!' PrimExpr %prec UNEG
    {
        $$ = cria_no(NO_NEGACAO, yylineno, "!");
        $$->filho1 = $2;
    }
    | PrimExpr { $$ = $1; }
    ;

PrimExpr
    : ID '(' ListExpr ')'
    {
        $$ = cria_no(NO_CHAMADA_FUNCAO, yylineno, $1);
        $$->filho1 = $3;
        free($1);
    }
    | ID '(' ')'
    {
        $$ = cria_no(NO_CHAMADA_FUNCAO, yylineno, $1);
        $$->filho1 = NULL;
        free($1);
    }
    | ID          { $$ = cria_no(NO_IDENTIFICADOR, yylineno, $1); free($1); }
    | INTCONST    { $$ = cria_no(NO_CONST_INT, yylineno, $1); free($1); }
    | CARCONST    { $$ = cria_no(NO_CONST_CAR, yylineno, $1); free($1); }
    | '(' Expr ')'{ $$ = $2; }
    ;

ListExpr
    : ListExpr ',' Expr
    {
        No* p = $1;
        while(p && p->proximo) p = p->proximo;
        if (p) p->proximo = $3;
        $$ = $1;
    }
    | Expr { $$ = $1; }
    ;

%%