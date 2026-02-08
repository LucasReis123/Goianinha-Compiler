%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./AST/ast.h"

extern int yylex(void);

// Declarações externas do analisador léxico
extern int yylineno;

// Função para reportar erros
void yyerror(const char *s);

%}

// Union para definir o tipo de valor semântico.
// Todos os não-terminais e terminais (que carregam valor) terão um ponteiro para a AST.
%union {
    struct AST_Node *node;   // Para todos os não-terminais e tokens com valor
    char *text;              // Para tokens como ID, INTCONST, etc., que carregam o lexema
}

%locations
// Defina o tipo de retorno dos símbolos:
%type <node> Programa DeclFuncVar DeclProg DeclVar DeclFunc ListaParametros ListaParametrosCont Bloco ListaDeclVar Tipo ListaComando Comando Expr OrExpr AndExpr EqExpr DesigExpr AddExpr MulExpr UnExpr PrimExpr ListExpr
%token <text> ID INTCONST CARCONST CADEIA_CARACTERES

%token PROGRAMA CAR INT RETORNE LEIA ESCREVA NOVALINHA
%token SE ENTAO SENAO ENQUANTO EXECUTE
%token VIRGULA PONTO_VIRGULA ABRE_PAR FECHA_PAR ABRE_CHAVE FECHA_CHAVE
%token ATRIBUICAO IGUAL DIFERENTE MENOR MAIOR MENOR_IGUAL MAIOR_IGUAL
%token MAIS MENOS MULT DIV NOT OU E

// Definindo a raiz da gramática
%start Programa

%%

Programa: DeclFuncVar DeclProg
    {
        // Nó raiz do programa. 
        // Child1: Lista de declarações. 
        // Child2: Programa principal.

        $$ = new_ast_node(AST_PROGRAMA, @2.first_line);
        $$->child1 = $1;
        $$->child2 = $2;
        root_ast = $$; // Define a raiz global
    }
    ;

DeclFuncVar: Tipo ID DeclVar PONTO_VIRGULA DeclFuncVar
    {
        // Tipo
        AST_Node* type_node = $1;

        // Lista de IDs adicionais
        AST_Node* id_list = $3; 
        
        // Cria o nó para a PRIMEIRA variável
        AST_Node* head = new_ast_node(AST_DECL_VAR, @1.first_line);
        head->child1 = type_node;
        head->child2 = new_ast_leaf(AST_EXPR_ID, $2, @2.first_line);
        
        // Itera sobre a lista de IDs restantes e cria um nó AST_DECL_VAR para cada.
        AST_Node* current_decl = head;
        AST_Node* current_id = id_list;
        
        while (current_id != NULL) {
            // Cria um NOVO nó AST_DECL_VAR para a variável
            AST_Node* new_decl = new_ast_node(AST_DECL_VAR, @1.first_line);
            new_decl->child1 = type_node;
            
            // Desliga o nó ID da lista para usá-lo como child2
            AST_Node* next_id_temp = current_id->next;
            current_id->next = NULL; 
            
            new_decl->child2 = current_id;
            
            // Liga a nova declaração (new_decl) à lista de declarações globais.
            current_decl->next = new_decl;
            current_decl = new_decl;
            
            current_id = next_id_temp; 
        }
        
        // Liga a lista de novas declarações (head) ao restante das globais.
        current_decl->next = $5;
        $$ = head; 
    }
    | Tipo ID DeclFunc DeclFuncVar
    {
        // Declaração de Função
        AST_Node* func_node = new_ast_node(AST_DECL_FUNC, @1.first_line);
        func_node->child1 = $1;                                           // Tipo de retorno
        func_node->child2 = new_ast_leaf(AST_EXPR_ID, $2, @2.first_line); // ID
        func_node->child3 = $3->child1;                                   // Lista de Parâmetros (extraída do nó DeclFunc)
        func_node->child4 = $3->child2;                                   // Bloco de Comandos (extraído do nó DeclFunc)
        
        // Encadeia a nova função no topo da lista DeclFuncVar
        $$ = create_list_item(AST_LISTA_DECL_FUNC_VAR, func_node, $4);
    }
    | /* epsilon */
    { $$ = NULL; }

DeclProg: PROGRAMA Bloco
    { $$ = $2; } // O programa principal é simplesmente o Bloco

DeclVar: VIRGULA ID DeclVar
    {
        // Cria um nó para a variável atual (ID) e encadeia no próximo nó ($3)
        AST_Node* current_id = new_ast_leaf(AST_EXPR_ID, $2, @2.first_line);
        current_id->next = $3; // Encadeia o restante da lista de IDs
        $$ = current_id;
    }
    | /* epsilon */
    { $$ = NULL; }

DeclFunc: ABRE_PAR ListaParametros FECHA_PAR Bloco
    {
        // Cria um nó temporário para a função. Child1: Parâmetros. Child2: Bloco.
        $$ = new_ast_node(AST_DECL_FUNC, @1.first_line);
        $$->child1 = $2;                                  // ListaParametros
        $$->child2 = $4;                                  // Bloco
    }
    ;

ListaParametros: /* epsilon */
    { $$ = NULL; } // Lista de parâmetros vazia, retorna NULL
    | ListaParametrosCont
    { $$ = $1; } // Passa o cabeçalho da lista de parâmetros

ListaParametrosCont: Tipo ID
    {
        // Cria o nó para este Parâmetro individual.
        AST_Node* param_node = new_ast_node(AST_LISTA_PARAMETROS, @1.first_line);
        param_node->child1 = $1;                                           // O Tipo (nó AST_TIPO_INT ou AST_TIPO_CAR)
        param_node->child2 = new_ast_leaf(AST_EXPR_ID, $2, @2.first_line); // O ID (nome do parâmetro)
        
        $$ = param_node;
    }
    | Tipo ID VIRGULA ListaParametrosCont
    {
        // Cria o nó para o Parâmetro atual.
        AST_Node* param_node = new_ast_node(AST_LISTA_PARAMETROS, @1.first_line);
        param_node->child1 = $1; // O Tipo
        param_node->child2 = new_ast_leaf(AST_EXPR_ID, $2, @2.first_line); // O ID

        // Encadeia o restante da lista
        param_node->next = $4;
        
        $$ = param_node;
    }
    ;

Bloco: ABRE_CHAVE ListaDeclVar ListaComando FECHA_CHAVE
    {
        // Child1: Declarações Locais. 
        // Child2: Lista de Comandos.
        
        $$ = new_ast_node(AST_BLOCO, @1.first_line);
        $$->child1 = $2;
        $$->child2 = $3;
    }
    ;

ListaDeclVar: /* epsilon */
    { $$ = NULL; }
    | Tipo ID DeclVar PONTO_VIRGULA ListaDeclVar
    {
        // Tipo
        AST_Node* type_node = $1;
        
        // Lista de IDs
        AST_Node* id_list = $3; 
        
        // Cria o nó para a PRIMEIRA
        AST_Node* head = new_ast_node(AST_DECL_VAR, @1.first_line);
        head->child1 = type_node; 
        head->child2 = new_ast_leaf(AST_EXPR_ID, $2, @2.first_line); // ID
        
        // Itera sobre a lista de IDs restantes e cria um nó AST_DECL_VAR para cada.
        AST_Node* current_decl = head;
        AST_Node* current_id = id_list;
        
        while (current_id != NULL) {
            // Cria um NOVO nó AST_DECL_VAR para o ID atual
            AST_Node* new_decl = new_ast_node(AST_DECL_VAR, @1.first_line);
            new_decl->child1 = type_node; 
            
            // Desligando o nó ID para usá-lo como child2
            AST_Node* next_id_temp = current_id->next;
            current_id->next = NULL; 
            
            new_decl->child2 = current_id; 
            
            // Ligando a nova declaração à lista de declarações.
            current_decl->next = new_decl;
            current_decl = new_decl;
            
            current_id = next_id_temp; 
        }
        
        // Ligando a lista de novos nós de declaração (head) ao restante da lista ($5)
        current_decl->next = $5;
        $$ = head; 
    }

Tipo: INT
    { $$ = new_ast_node(AST_TIPO_INT, @1.first_line); }
    | CAR
    { $$ = new_ast_node(AST_TIPO_CAR, @1.first_line); }

ListaComando: Comando
    { $$ = $1; }
    | Comando ListaComando
    { 
        $1->next = $2; 
        $$ = $1; 
    }

Comando: PONTO_VIRGULA
    { $$ = new_ast_node(AST_COMANDO_VAZIO, @1.first_line); }
    | Expr PONTO_VIRGULA
    { $$ = $1; } // Se a Expr for uma atribuição (ID=Expr), ela já é um comando.
    | RETORNE Expr PONTO_VIRGULA
    {
        $$ = new_ast_node(AST_COMANDO_RETORNE, @1.first_line);
        $$->child1 = $2;
    }
    | LEIA ID PONTO_VIRGULA
    {
        $$ = new_ast_node(AST_COMANDO_LEIA, @1.first_line);
        $$->child1 = new_ast_leaf(AST_EXPR_ID, $2, @2.first_line);
    }
    | ESCREVA Expr PONTO_VIRGULA
    {
        $$ = new_ast_node(AST_COMANDO_ESCREVA, @1.first_line);
        $$->child1 = $2;
    }
    | ESCREVA CADEIA_CARACTERES PONTO_VIRGULA
    {
        $$ = new_ast_node(AST_COMANDO_ESCREVA, @1.first_line);
        $$->child1 = new_ast_leaf(AST_CONST_CADEIA, $2, @2.first_line);
    }
    | NOVALINHA PONTO_VIRGULA
    { $$ = new_ast_node(AST_COMANDO_NOVALINHA, @1.first_line); }
    | SE ABRE_PAR Expr FECHA_PAR ENTAO Comando
    {
        // SE sem SENAO
        $$ = new_ast_node(AST_COMANDO_SE, @1.first_line);
        $$->child1 = $3; // Condição (Expr)
        $$->child2 = $6; // Bloco ENTAO (Comando)
    }
    | SE ABRE_PAR Expr FECHA_PAR ENTAO Comando SENAO Comando
    {
        // SE com SENAO
        $$ = new_ast_node(AST_COMANDO_SE_SENAO, @1.first_line);
        $$->child1 = $3; // Condição (Expr)
        $$->child2 = $6; // Bloco ENTAO (Comando)
        $$->child3 = $8; // Bloco SENAO (Comando)
    }
    | ENQUANTO ABRE_PAR Expr FECHA_PAR EXECUTE Comando
    {
        $$ = new_ast_node(AST_COMANDO_ENQUANTO, @1.first_line);
        $$->child1 = $3; // Condição (Expr)
        $$->child2 = $6; // Corpo do loop (Comando)
    }
    | Bloco
    { $$ = $1; } // Passa o nó Bloco diretamente

Expr: OrExpr
    { $$ = $1; } // Passa o nó da sub-expressão (OrExpr) para o nó pai
    | ID ATRIBUICAO Expr
    {
        // Cria um nó de atribuição. Filhos: ID e a Expressão à direita.
        $$ = new_ast_node(AST_COMANDO_ATRIB, @1.first_line);       // @1 é a linha do primeiro token (ID)
        $$->child1 = new_ast_leaf(AST_EXPR_ID, $1, @1.first_line); // $1 é o lexema do ID
        $$->child2 = $3;                                           // $3 é o nó da Expr
    }
    ;

// --- Expressões Lógicas ---
OrExpr: OrExpr OU AndExpr
    { $$ = new_ast_binary_op("OU", $1, $3); }
    | AndExpr
    { $$ = $1; }

AndExpr: AndExpr E EqExpr
    { $$ = new_ast_binary_op("E", $1, $3); }
    | EqExpr
    { $$ = $1; }

EqExpr: EqExpr IGUAL DesigExpr
    { $$ = new_ast_binary_op("==", $1, $3); }
    | EqExpr DIFERENTE DesigExpr
    { $$ = new_ast_binary_op("!=", $1, $3); }
    | DesigExpr
    { $$ = $1; }

DesigExpr: DesigExpr MENOR AddExpr
    { $$ = new_ast_binary_op("<", $1, $3); }
    | DesigExpr MAIOR AddExpr
    { $$ = new_ast_binary_op(">", $1, $3); }
    | DesigExpr MENOR_IGUAL AddExpr
    { $$ = new_ast_binary_op("<=", $1, $3); }
    | DesigExpr MAIOR_IGUAL AddExpr
    { $$ = new_ast_binary_op(">=", $1, $3); }
    | AddExpr
    { $$ = $1; }

AddExpr: AddExpr MAIS MulExpr
    { $$ = new_ast_binary_op("+", $1, $3); }
    | AddExpr MENOS MulExpr
    { $$ = new_ast_binary_op("-", $1, $3); }
    | MulExpr
    { $$ = $1; }

MulExpr: MulExpr MULT UnExpr
    { $$ = new_ast_binary_op("*", $1, $3); }
    | MulExpr DIV UnExpr
    { $$ = new_ast_binary_op("/", $1, $3); }
    | UnExpr
    { $$ = $1; }

UnExpr: MENOS PrimExpr
    { $$ = new_ast_unary_op("-", $2); }
    | NOT PrimExpr
    { $$ = new_ast_unary_op("!", $2); }
    | PrimExpr
    { $$ = $1; }

// --- Expressões Primárias (Folhas e Chamadas) ---
PrimExpr: ID ABRE_PAR ListExpr FECHA_PAR
    {
        // Chamada de função com argumentos
        $$ = new_ast_node(AST_EXPR_CHAMADA_FUNC, @1.first_line);
        $$->child1 = new_ast_leaf(AST_EXPR_ID, $1, @1.first_line);
        $$->child2 = $3; // Lista de Expressões (Argumentos)
    }
    | ID ABRE_PAR FECHA_PAR
    {
        // Chamada de função sem argumentos
        $$ = new_ast_node(AST_EXPR_CHAMADA_FUNC, @1.first_line);
        $$->child1 = new_ast_leaf(AST_EXPR_ID, $1, @1.first_line);
        $$->child2 = NULL;
    }
    | ID
    { $$ = new_ast_leaf(AST_EXPR_ID, $1, @1.first_line); }
    | CARCONST
    { $$ = new_ast_leaf(AST_CONST_CAR, $1, @1.first_line); }
    | INTCONST
    { $$ = new_ast_leaf(AST_CONST_INT, $1, @1.first_line); }
    | ABRE_PAR Expr FECHA_PAR
    { $$ = $2; } // Ignora parênteses

// --- Listas de Expressões (Argumentos) ---
ListExpr: Expr
    { $$ = $1; } 
    | ListExpr VIRGULA Expr
    { 
        $$ = create_list_item(AST_LISTA_EXPRESSOES, $3, $1);
    }

%%

void yyerror(const char *s) {
    fprintf(stderr, "ERRO: %s na linha %d\n\n", s, yylineno);
    exit(1);
}
