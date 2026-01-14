#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definição dos Tipos de Nós da AST (Node Kind)
typedef enum {
    // Programa e Estrutura Principal
    AST_PROGRAMA,

    // Declarações
    AST_DECL_VAR,
    AST_DECL_FUNC,
    AST_TIPO_INT,
    AST_TIPO_CAR,
    
    // Comandos e Estruturas de Controle
    AST_BLOCO,
    AST_COMANDO_ATRIB, // Atribuição: ID = Expr
    AST_COMANDO_SE,
    AST_COMANDO_SE_SENAO,
    AST_COMANDO_ENQUANTO,
    AST_COMANDO_RETORNE,
    AST_COMANDO_LEIA,
    AST_COMANDO_ESCREVA,
    AST_COMANDO_NOVALINHA,
    AST_COMANDO_VAZIO,

    // Expressões
    AST_EXPR_BINARIA,      // Operações com dois operandos (+, -, <, ==, E, OU, etc.)
    AST_EXPR_UNARIA,       // Operações com um operando (-, !)
    AST_EXPR_ID,           // Uso de um identificador (variável)
    AST_EXPR_CHAMADA_FUNC, // Chamada de função
    AST_CONST_INT,         // Constante inteira
    AST_CONST_CAR,         // Constante caractere
    AST_CONST_CADEIA,      // Constante string

    // Listas (para encadeamento)
    AST_LISTA_DECL_FUNC_VAR,
    AST_LISTA_DECL_VAR,
    AST_LISTA_COMANDO,
    AST_LISTA_PARAMETROS,
    AST_LISTA_EXPRESSOES
} AST_NodeKind;

// 2. Estrutura do Nó da AST
typedef struct AST_Node {
    AST_NodeKind kind;
    int lineno;  // Linha no código fonte
    char* value; // Lexema para folhas (ID, INTCONST, operador para binárias, etc.)

    // Ponteiros para os filhos
    struct AST_Node *child1; 
    struct AST_Node *child2;
    struct AST_Node *child3;
    struct AST_Node *child4;

    // Ponteiro para o próximo nó na mesma lista (para listas encadeadas)
    struct AST_Node *next;

    int data_type;
} AST_Node;


// Variável Global para a Raiz da AST
extern AST_Node* root_ast; 

// Funções de Criação de Nós

/**
 * Cria e inicializa um nó da AST.
 * @param kind O tipo do nó (ex: AST_COMANDO_SE).
 * @param lineno A linha onde o nó se origina no código fonte.
 * @return Ponteiro para o novo nó alocado.
 */
AST_Node* new_ast_node(AST_NodeKind kind, int lineno);

/**
 * Cria um nó folha (para IDs e Constantes).
 * @param kind O tipo do nó folha (ex: AST_EXPR_ID, AST_CONST_INT).
 * @param value A string (lexema) associada ao nó. Será duplicada internamente.
 * @param lineno A linha onde o nó se origina.
 * @return Ponteiro para o novo nó folha.
 */
AST_Node* new_ast_leaf(AST_NodeKind kind, char* value, int lineno);

/**
 * Cria um nó para uma expressão unária (ex: -x, !b).
 * @param op O operador (ex: "-", "!").
 * @param expr O nó da sub-expressão (operando).
 * @return Ponteiro para o novo nó unário.
 */
AST_Node* new_ast_unary_op(char* op, AST_Node* expr);

/**
 * Cria um nó para uma expressão binária (ex: a + b, x == y).
 * @param op O operador (ex: "+", "<=", "E").
 * @param left O nó do operando esquerdo.
 * @param right O nó do operando direito.
 * @return Ponteiro para o novo nó binário.
 */
AST_Node* new_ast_binary_op(char* op, AST_Node* left, AST_Node* right);

/**
 * Cria um item encadeado para listas. Útil para Listas de Comandos ou Declarações.
 * @param list_kind O tipo da lista (ex: AST_LISTA_COMANDO).
 * @param current_item O item que acabou de ser reconhecido.
 * @param next_item A lista (ou nó) que já foi construída.
 * @return O novo cabeçalho da lista.
 */
AST_Node* create_list_item(AST_NodeKind list_kind, AST_Node* current_item, AST_Node* next_item);


#endif // AST_H