%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int linha = 1;
char erro_msg[256];
%}

%option noyywrap yylineno

%%

[ \t\r]+        {/* Ignora espaços e tabulações */}
\n              { linha++; }

"programa"      { return 258; }
"car"           { return 259; }
"int"           { return 260; }
"retorne"       { return 261; }
"leia"          { return 262; }
"escreva"       { return 263; }
"novalinha"     { return 264; }
"se"            { return 265; }
"entao"         { return 266; }
"senao"         { return 267; }
"enquanto"      { return 268; }
"execute"       { return 269; }

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
"!="            { return 273; }
"<="            { return 274; }
">="            { return 275; }
"<"             { return 276; }
">"             { return 277; }

\"([^\"\n]*)\"  { return 271; }  /* Cadeia de caracteres válida */

[a-zA-Z_][a-zA-Z0-9_]*  { return 270; }  /* Identificadores */

[0-9]+ { return 272; }  /* Números inteiros */

"/*"([^*]|\*+[^*/])*\*+"/" { /* Ignora comentários */ }

"/*"([^*]|\*+[^*/])*\*?  {
    snprintf(erro_msg, sizeof(erro_msg), "ERRO: COMENTÁRIO NÃO TERMINADO - Linha %d", linha);
    return -1;
}

\"([^\"\n]*) {
    snprintf(erro_msg, sizeof(erro_msg), "CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA - Linha %d", linha);
    return -2;
}

(.|\n) {
    snprintf(erro_msg, sizeof(erro_msg), "ERRO: CARACTERE INVÁLIDO - Linha %d", linha);
    return -3;
}

%%