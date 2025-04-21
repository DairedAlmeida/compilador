%{
#include "goianinha.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void yyerror(const char *s);
void yyerror_lex(const char *s);

int linha = 1;
%}

%option noyywrap yylineno nounput noinput

%%

[ \t\r]+        { /* Ignora espaços */ }
\n              { linha++; }

"programa"      { return PROGRAM; }
"car"           { return CAR; }
"int"           { return INT; }
"retorne"       { return RETORNE; }
"leia"          { return LEIA; }
"escreva"       { return ESCREVA; }
"novalinha"     { return NOVALINHA; }
"se"            { return SE; }
"entao"         { return ENTAO; }
"senao"         { return SENAO; }
"enquanto"      { return ENQUANTO; }
"execute"       { return EXECUTE; }
"ou"            { return OU; }
"e"             { return E; }

"="             { return '='; }
"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"("             { return '('; }
")"             { return ')'; }
"{"             { return '{'; }
"}"             { return '}'; }
";"             { return ';'; }
","             { return ','; }
"=="            { return IGUAL; }
"!="            { return DIF; }
"<="            { return MENOR_IGUAL; }
">="            { return MAIOR_IGUAL; }
"<"             { return MENOR; }
">"             { return MAIOR; }

[a-zA-Z_][a-zA-Z0-9_]*  { return ID; }
[0-9]+                  { return INTCONST; }
'[^'\n]'                { return CARCONST; }
\"([^\"\n]*)\"          { return CAD_CAR; }

"/*"([^*]|\*+[^*/])*\*+"/" { /* Ignora comentários */ }

"/*"([^*]|\*+[^*/])*\*?  {
    char msg[100];
    snprintf(msg, sizeof(msg), "COMENTÁRIO NÃO TERMINADO");
    yyerror_lex(msg);
    return ERROR;
}

\"([^\"\n]*) {
    char msg[100];
    snprintf(msg, sizeof(msg), "CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA");
    yyerror_lex(msg);
    return ERROR;
}

\'([^\\'\n]|\\.)*[\n] {
    char msg[100];
    snprintf(msg, sizeof(msg), "CARACTERE OCUPA MAIS DE UMA LINHA");
    yyerror_lex(msg);
    return ERROR;
}

\'([^\\'\n]{2,}|\\.+)\' {
    char msg[100];
    snprintf(msg, sizeof(msg), "CARACTERE COM MAIS DE UM SÍMBOLO");
    yyerror_lex(msg);
    return ERROR;
}

. {
    char msg[100];
    snprintf(msg, sizeof(msg), "CARACTERE INVÁLIDO '%s'", yytext);
    yyerror_lex(msg);
    return ERROR;
}

%%