%{
#include <string.h>
#include "goianinha.tab.h" // Contém os nomes dos tokens: PROGRAM, ID, etc.
#include "arvore.h"       // Necessário por transitividade de includes

// Função para tratar erros léxicos, chamada pelas regras abaixo.
void yyerror_lex(const char *s) {
    fprintf(stderr, "ERRO LÉXICO (linha %d): %s, texto: '%s'\n", yylineno, s, yytext);
}

%}

/* Opções do Flex:
   - yylineno: mantém a contagem de linhas automaticamente.
   - noyywrap: não precisa da função yywrap().
   - nounput/noinput: remove funções não utilizadas, eliminando avisos.
*/
%option yylineno noyywrap nounput noinput

/* Definições Regulares (da sua versão antiga, que são mais robustas) */
ID      [a-zA-Z_][a-zA-Z0-9_]*
DIGITO  [0-9]+
CAR     '[^'\n]'
CADEIA  \"([^\"\n]*)\"

/* Regras para comentários (combinando as duas versões) */
COMENTARIO_BLOCO "/*"([^*]|\*+[^*/])*\*+"/"
COMENTARIO_LINHA "//".*

%%

[ \t\r]+    { /* Ignora espaços em branco */ }
\n          { /* O Flex já incrementa yylineno por causa da opção %option */ }

"programa"  { return PROGRAM; }
"car"       { return CAR; }
"int"       { return INT; }
"retorne"   { return RETORNE; }
"leia"      { return LEIA; }
"escreva"   { return ESCREVA; }
"novalinha" { return NOVALINHA; }
"se"        { return SE; }
"entao"     { return ENTAO; }
"senao"     { return SENAO; }
"enquanto"  { return ENQUANTO; }
"execute"   { return EXECUTE; }

"ou"        { yylval.str_lexema = strdup(yytext); return OU; }
"e"         { yylval.str_lexema = strdup(yytext); return E; }

"="         { return '='; }
"+"         { return '+'; }
"-"         { return '-'; }
"*"         { return '*'; }
"/"         { return '/'; }
"("         { return '('; }
")"         { return ')'; }
"{"         { return '{'; }
"}"         { return '}'; }
";"         { return ';'; }
","         { return ','; }
"!"         { return '!'; }
"=="        { yylval.str_lexema = strdup(yytext); return IGUAL; }
"!="        { yylval.str_lexema = strdup(yytext); return DIF; }
"<="        { yylval.str_lexema = strdup(yytext); return MENOR_IGUAL; }
">="        { yylval.str_lexema = strdup(yytext); return MAIOR_IGUAL; }
"<"         { yylval.str_lexema = strdup(yytext); return MENOR; }
">"         { yylval.str_lexema = strdup(yytext); return MAIOR; }

{ID}        { yylval.str_lexema = strdup(yytext); return ID; }
{DIGITO}    { yylval.str_lexema = strdup(yytext); return INTCONST; }
{CAR}       { yylval.str_lexema = strdup(yytext); return CARCONST; }
{CADEIA}    { yylval.str_lexema = strdup(yytext); return CAD_CAR; }

{COMENTARIO_BLOCO} { /* Ignora comentário de bloco */ }
{COMENTARIO_LINHA} { /* Ignora comentário de linha */ }

"/*"([^*]|\*+[^*/])*\*? {
    yyerror_lex("Comentário de bloco não terminado");
}

\"([^\"\n])* {
    yyerror_lex("Cadeia de caracteres não terminada (falta aspas duplas)");
}

\'([^\\'\n]{2,}|\\.+)\' {
    yyerror_lex("Constante de caractere com mais de um símbolo");
}

. {
    yyerror_lex("Caractere inválido");
}

%%