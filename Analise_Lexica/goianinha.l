%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "goianinha.tab.h"

// Declarações externas para localização
extern YYLTYPE yylloc;

// Função auxiliar para reportar erros
void reportar_erro(const char* mensagem, int linha);
%}

%option yylineno

%{
// Toda vez que o lexer reconhecer um token, ele atualiza a linha onde ocorreu.
#define YY_USER_ACTION yylloc.first_line = yylineno; yylloc.last_line = yylineno;
%}

DIGITO       [0-9]
LETRA        [a-zA-Z]
IDENTIFICADOR [_]{LETRA}|{LETRA}({LETRA}|{DIGITO}|[_])*
INT_CONST    {DIGITO}+
CAR_CONST    "'"[^'\n]"'"
COMENTARIO   "/*"([^*]|"*"[^/])*"*/"
COMENTARIO_NAO_FECHADO "/*"([^*]|"*"[^/])*
ESPACO       [ \t\r]
NOVA_LINHA   \n

/* REAGAS LEXICAS */
%%
"programa"    { return PROGRAMA; }
"car"         { return CAR; }
"int"         { return INT; }
"retorne"     { return RETORNE; }
"leia"        { return LEIA; }
"escreva"     { return ESCREVA; }
"novalinha"   { return NOVALINHA; }
"se"          { return SE; }
"entao"       { return ENTAO; }
"senao"       { return SENAO; }
"enquanto"    { return ENQUANTO; }
"execute"     { return EXECUTE; }

"ou"          { return OU; }
"e"           { return E; }

","           { return VIRGULA; }
";"           { return PONTO_VIRGULA; }
"("           { return ABRE_PAR; }
")"           { return FECHA_PAR; }
"{"           { return ABRE_CHAVE; }
"}"           { return FECHA_CHAVE; }
"="           { return ATRIBUICAO; }
"=="          { return IGUAL; }
"!="          { return DIFERENTE; }
"<"           { return MENOR; }
">"           { return MAIOR; }
"<="          { return MENOR_IGUAL; }
">="          { return MAIOR_IGUAL; }
"+"           { return MAIS; }
"-"           { return MENOS; }
"*"           { return MULT; }
"/"           { return DIV; }
"!"           { return NOT; }

{INT_CONST}    { 
  yylval.text = strdup(yytext);  
  return INTCONST;
}
{CAR_CONST} {
  yylval.text = strdup(yytext);
  return CARCONST;
}
{IDENTIFICADOR} { 
  yylval.text = strdup(yytext);
  return ID;
}

\"[^"\n]*\"    { 
                // Verifica se a string ocupa mais de uma linha
                char* ptr = yytext;
                int quebras = 0;
                while (*ptr) {
                    if (*ptr == '\n') quebras++;
                    ptr++;
                }
                if (quebras > 0) {
                    reportar_erro("CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA", yylineno);
                    exit(1);
                }
                yylval.text = strdup(yytext);
                return CADEIA_CARACTERES; 
              }

\"[^"\n]*$     { 
                reportar_erro("CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA", yylineno);
                exit(1);
              }

{COMENTARIO}   { /* Ignora comentários */ }
{COMENTARIO_NAO_FECHADO} { 
                reportar_erro("COMENTARIO NAO TERMINA", yylineno);
                exit(1);
              }

{ESPACO}       { /* Ignora espaços em branco */ }
{NOVA_LINHA}   { /* Conta linhas automaticamente */ }

.              { 
                reportar_erro("CARACTERE INVALIDO, LINHA", yylineno);
                exit(1);
              }

%%

// Função para reportar erros
void reportar_erro(const char* mensagem, int linha) {
    printf("\nERRO: %s %d\n\n", mensagem, linha);
}