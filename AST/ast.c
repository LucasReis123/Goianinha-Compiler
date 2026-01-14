#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaração e Inicialização da Raiz Global
AST_Node* root_ast = NULL;


// IMPLEMENTAÇÃO DAS FUNÇÕES DE CRIAÇÃO DE NÓS DA AST

// Função genérica para criar um novo nó da AST.
AST_Node* new_ast_node(AST_NodeKind kind, int lineno) {
    // Aloca a memória para o novo nó
    AST_Node* node = (AST_Node*)malloc(sizeof(AST_Node));
    
    if (node == NULL) {
        perror("Erro de alocação de memória para AST_Node");
        exit(EXIT_FAILURE);
    }
    
    // Zera todos os campos (child1, child2, next, etc., garantindo que sejam NULL)
    memset(node, 0, sizeof(AST_Node));
    
    // Inicializa os campos básicos
    node->kind = kind;
    node->lineno = lineno;
    
    return node;
}

// Cria um nó folha (para IDs e Constantes).
AST_Node* new_ast_leaf(AST_NodeKind kind, char* value, int lineno) {
    AST_Node* node = new_ast_node(kind, lineno);
    
    if (value != NULL) {
        // Usa strdup para criar uma CÓPIA do lexema (yytext ou $n),
        // garantindo que a string persista após a regra do bison ser concluída.
        node->value = strdup(value);
        if (node->value == NULL) {
            perror("Erro ao duplicar string (strdup)");
            free(node);
            exit(EXIT_FAILURE);
        }
    }
    
    return node;
}

// Cria um nó para uma expressão unária (ex: -x, !b).
AST_Node* new_ast_unary_op(char* op, AST_Node* expr) {
    // Usando a linha do operando como a linha do nó
    int lineno = expr ? expr->lineno : 0; 
    
    AST_Node* node = new_ast_leaf(AST_EXPR_UNARIA, op, lineno);
    
    // O único operando é o child1
    node->child1 = expr;
    
    return node;
}

// Cria um nó para uma expressão binária (ex: a + b, x == y).
AST_Node* new_ast_binary_op(char* op, AST_Node* left, AST_Node* right) {
    // Usando a linha do operando esquerdo como a linha do nó
    int lineno = left ? left->lineno : 0; 

    // Cria o nó e armazena o operador no campo value
    AST_Node* node = new_ast_leaf(AST_EXPR_BINARIA, op, lineno);
    
    node->child1 = left;  // Operando Esquerdo
    node->child2 = right; // Operando Direito
    
    return node;
}

// Cria um item encadeado para listas (ListaComando, ListaDeclVar, etc.)
AST_Node* create_list_item(AST_NodeKind list_kind, AST_Node* current_item, AST_Node* next_item) {
    if (current_item == NULL) {
        return next_item;
    }
    if (next_item == NULL) {
        return current_item;
    }

    // Função de encadeamento simples:
    AST_Node* head = next_item;
    AST_Node* current = next_item;

    if (current == NULL) {
        return current_item; // O novo item é a cabeça
    }

    // Percorre até o final da lista (cabeça é 'next_item')
    // A lista usa o ponteiro 'next' para encadear
    while (current->next != NULL) {
        current = current->next;
    }

    // Anexa o novo item ao final
    current->next = current_item;

    return head;
}
