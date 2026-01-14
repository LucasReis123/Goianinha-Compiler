#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./../AST/ast.h"
#include "./../Tabela_Simbulos/symbolTable.h"

// --- Constantes para Tipos ---
#define INT_T   1
#define CHAR_T  2
#define VOID_T  4
#define TYPE_ERROR 99


// Declaração externa da raiz da AST
extern AST_Node* root_ast;

// Variável global para controle do deslocamento atual do frame
int current_frame_offset = 0;

// Função principal de análise
void analyze_ast(SymbolTableRef symtab);

void analyze_list(SymbolTableRef symtab, AST_Node* node);

int count_list_items(AST_Node* head);

AST_Node* find_function_declaration(AST_Node* root, const char* name);

// Função recursiva de travessia
void analyze_node(SymbolTableRef symtab, AST_Node* node, int is_function_body);

// Protótipo para reportar erros semânticos
void semantic_error(int line, const char* message);

// Protótipo para obter o tipo de um nó após checagem (análise de expressões)
int check_and_get_type(SymbolTableRef symtab, AST_Node* node);

int current_func_type = VOID_T;

// ====================================================================

// Função para retornar o erro semântico e terminar a compilação
void semantic_error(int line, const char* message) {
    fprintf(stderr, "\nERRO SEMÂNTICO (Linha %d): %s\n\n", line, message);
    exit(EXIT_FAILURE); // Termina a compilação após o primeiro erro grave
}

// Mapeia o nó do Tipo da AST para o inteiro de DataType
int ast_type_to_data_type(AST_Node* type_node) {
    if (!type_node) return VOID_T;
    
    if (type_node->kind == AST_TIPO_INT) return INT_T;
    if (type_node->kind == AST_TIPO_CAR) return CHAR_T;
    
    return VOID_T;
}

/**
 * @brief Checa recursivamente o tipo de uma expressão e o anexa ao nó da AST.
 * @return O tipo inteiro (INT_T, CHAR_T, etc.).
 */
int check_and_get_type(SymbolTableRef symtab, AST_Node* node) {
    if (node == NULL) return VOID_T;
    
    int type_l, type_r;
    SymbolRef symbol;
    
    switch (node->kind) {
        
        case AST_CONST_INT:
            node->data_type = INT_T;
            return INT_T;
            
        case AST_CONST_CAR:
            node->data_type = CHAR_T;
            return CHAR_T;

        case AST_CONST_CADEIA:
            node->data_type = CHAR_T;
            return CHAR_T;

        case AST_EXPR_ID:
            // Checando declaração e obtendo o tipo
            symbol = symtab_lookup(symtab, node->value);
            
            if (symbol == NULL) {
                semantic_error(node->lineno, "Uso de identificador não declarado.");
                return TYPE_ERROR;
            }

            // Anexando o tipo encontrado
            node->data_type = sym_get_data_type(symbol);
            
            return node->data_type;
            
        case AST_EXPR_BINARIA:
            // Checando os tipos dos operandos
            type_l = check_and_get_type(symtab, node->child1);
            type_r = check_and_get_type(symtab, node->child2);

            // Operadores Aritméticos (+, -, *, /)
            if (strcmp(node->value, "+") == 0 || strcmp(node->value, "-") == 0 ||
                strcmp(node->value, "*") == 0 || strcmp(node->value, "/") == 0) 
            {
                // Regra: Aritméticos devem ser aplicados em tipos int
                if (type_l != INT_T || type_r != INT_T) {
                    semantic_error(node->lineno, "Operadores aritméticos requerem operandos do tipo 'int'.");
                    return TYPE_ERROR;
                }
                node->data_type = INT_T;
                return INT_T;
            }

            // Operadores Relacionais (>, <, ==, !=, etc.)
            if (strcmp(node->value, ">") == 0 || strcmp(node->value, "<") == 0 ||
                strcmp(node->value, ">=") == 0 || strcmp(node->value, "<=") == 0 ||
                strcmp(node->value, "==") == 0 || strcmp(node->value, "!=") == 0) 
            {
                // Regra: Operadores relacionais requerem operandos de mesmo tipo.
                if (type_l != type_r || type_l == VOID_T) {
                    semantic_error(node->lineno, "Operadores relacionais requerem operandos do mesmo tipo (int ou char).");
                    return TYPE_ERROR;
                }

                // Regra: Expressão relacional tem tipo int.
                node->data_type = INT_T;
                return INT_T;
            }
            
            // Operadores Lógicos (OU, E)
            if (strcmp(node->value, "OU") == 0 || strcmp(node->value, "E") == 0)
            {
                // Regra: Lógicos devem ser aplicados em tipos int
                if (type_l != INT_T || type_r != INT_T) {
                    semantic_error(node->lineno, "Operadores lógicos requerem operandos do tipo 'int'.");
                    return TYPE_ERROR;
                }
                node->data_type = INT_T;
                return INT_T;
            }

            break;
            
        case AST_EXPR_UNARIA:
             type_l = check_and_get_type(symtab, node->child1);
             // Operador '!' (NOT)
             if (strcmp(node->value, "!") == 0) {
                 if (type_l != INT_T) {
                     semantic_error(node->lineno, "Operador de negação (!) requer operando do tipo 'int'.");
                     return TYPE_ERROR;
                 }
                 node->data_type = INT_T;
                 return INT_T;
             }
             // Operador '-' (Menos unário)
             if (strcmp(node->value, "-") == 0) {
                 if (type_l != INT_T) {
                     semantic_error(node->lineno, "Operador unário (-) requer operando do tipo 'int'.");
                     return TYPE_ERROR;
                 }
                 node->data_type = INT_T;
                 return INT_T;
             }
             break;

        case AST_COMANDO_ATRIB:
            {
                // ** Checando a Variável**
                SymbolRef id_symbol = symtab_lookup(symtab, node->child1->value);
                if (id_symbol == NULL) {
                    semantic_error(node->lineno, "Variável de atribuição não declarada.");
                    return TYPE_ERROR; // Retorna erro
                }

                int id_type = sym_get_data_type(id_symbol);
                
                // ** Checando a Expressão ou outra Atribuição **
                int expr_type = check_and_get_type(symtab, node->child2);
                
                // ** Checagem de Tipos **
                if (id_type != expr_type) {
                    semantic_error(node->lineno, "Incompatibilidade de tipos na atribuição aninhada/expressão.");
                    return TYPE_ERROR;
                }
                
                // ** A Atribuição é uma Expressão (Tipo do retorno é o tipo da variável)**
                node->data_type = id_type;
                return id_type;
            }

        case AST_EXPR_CHAMADA_FUNC:
            {
                char* func_name = node->child1->value;
                AST_Node* actual_args_head = node->child2;

                // Buscando o símbolo da função na Tabela de Símbolos (para tipo de retorno)
                SymbolRef func_symbol = symtab_lookup(symtab, func_name);
                if (func_symbol == NULL) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Função '%s' não foi declarada.", func_name);
                    semantic_error(node->lineno, msg);
                    return TYPE_ERROR;
                }

                // Obtendo a declaração completa na AST (Para pegar a lista de Parâmetros Formais)
                AST_Node* func_decl_node = find_function_declaration(root_ast, func_name);
                if (func_decl_node == NULL) {
                    semantic_error(node->lineno, "Erro interno: Declaração de função não encontrada na AST.");
                    return TYPE_ERROR;
                }
                
                // A lista de parâmetros formais
                AST_Node* formal_params = func_decl_node->child3; 

                // Travessia paralela e checagem de tipo/contagem
                AST_Node* current_formal = formal_params;
                AST_Node* current_actual = actual_args_head;
                int index = 1;

                while (current_formal != NULL && current_actual != NULL) {
                    // TIPO FORMAL (Tipo do parâmetro)
                    int formal_type = ast_type_to_data_type(current_formal->child1);
                    
                    // TIPO REAL (Tipo do argumento)
                    int actual_type = check_and_get_type(symtab, current_actual);
                    
                    // COMPARAÇÃO
                    if (actual_type != formal_type) {
                        char msg[256];
                        snprintf(msg, sizeof(msg), 
                                "Parâmetro %d possui tipo diferente do esperado.", index);
                        semantic_error(node->lineno, msg);
                        return TYPE_ERROR;
                    }

                    // Movendo para o próximo
                    current_formal = current_formal->next; // Próximo formal
                    current_actual = current_actual->next; // Próximo argumento
                    index++;
                }

                // Contando os parâmetros formais
                int formal_count = sym_get_num_params(func_symbol); 
                
                // Contando os argumentos passados
                int actual_count = count_list_items(actual_args_head);
                
                if (formal_count != actual_count) {
                    char msg[100];
                    snprintf(msg, sizeof(msg), 
                            "Número incorreto de argumentos na chamada de '%s'. Esperado: %d, Recebido: %d.", 
                            func_name, formal_count, actual_count);
                    semantic_error(node->lineno, msg);
                    return TYPE_ERROR;
                }

                // Retorna o tipo de retorno da função
                int return_type = sym_get_data_type(func_symbol);
                node->data_type = return_type;
                return return_type;
            }
        default:

            return VOID_T; 
    }
    
    return VOID_T;
}

/**
 * @brief Função recursiva que percorre a AST e realiza a análise semântica.
 */
void analyze_node(SymbolTableRef symtab, AST_Node* node, int is_function_body) {
    if (node == NULL) return;

    // --- AÇÕES DE PRÉ-ORDEM ---
    switch (node->kind) {
        case AST_PROGRAMA:
    
        case AST_BLOCO:
            if(!is_function_body) {
                symtab_enter_scope(symtab);
            }
            
            analyze_list(symtab, node->child1); 
            analyze_list(symtab, node->child2); 

            if(!is_function_body) {
                symtab_exit_scope(symtab);
            }

            return;
            
        case AST_DECL_VAR:
            {
                // Pega o TIPO da declaração
                int data_type = ast_type_to_data_type(node->child1);
                
                // Ponteiro para o nó ID principal
                AST_Node* current_id_node = node->child2; 
                
                while (current_id_node != NULL) {
                    
                    // Pega o nome do ID
                    char* var_name = current_id_node->value;
                    
                    // CHECAGEM DE REDECLARAÇÃO
                    if (symtab_lookup_current_scope(symtab, var_name) != NULL) {
                        semantic_error(node->lineno, "Redeclaração de variável no escopo atual.");
                    }

                    current_frame_offset -= 4;
                    
                    // INSERÇÃO NA TABELA
                    symtab_insert_var(symtab, var_name, data_type, current_frame_offset);

                    // AVANÇA PARA O PRÓXIMO ID NA LISTA
                    current_id_node = current_id_node->next; 
                }
                
                return;
            }
        
        case AST_DECL_FUNC:
            {
                char* func_name = node->child2->value;
                int return_type = ast_type_to_data_type(node->child1);
                current_func_type = return_type;

                // Inserindo o nome da função no escopo GLOBAL.
                AST_Node *param_list_head = node->child3;
                int num_params = count_list_items(param_list_head);
                symtab_insert_func(symtab, func_name, num_params, return_type); 

                // Entrando no escopo da função.
                symtab_enter_scope(symtab);

                current_frame_offset = 0;

                // Iterando sobre a lista de parâmetros e inserindo.
                AST_Node* param_list_node = node->child3;
                while (param_list_node != NULL) {
                    // O nó de lista de parâmetros (AST_LISTA_PARAMETROS) tem:
                    // child1: Tipo (ex: AST_TIPO_INT)
                    // child2: ID (ex: AST_EXPR_ID 'n')
                    
                    char* param_name = param_list_node->child2->value;
                    int param_type = ast_type_to_data_type(param_list_node->child1);
                    
                    // OFFSET DO PARÂMETRO
                    current_frame_offset -= 4;
                    
                    // Regra: Parâmetros formais têm escopo local e devem ser inseridos.
                    symtab_insert_var(symtab, param_name, param_type, current_frame_offset); 
                    
                    param_list_node = param_list_node->next;
                }

                analyze_node(symtab, node->child4, 1);
        
                // Saindo do escopo da função.        
                symtab_exit_scope(symtab);
                
                return;

            }
        
        default: 
            break;
    }
    
    // --- TRAVESSIA LATERAL (Filhos) ---
    analyze_node(symtab, node->child1, 0);
    analyze_node(symtab, node->child2, 0);
    analyze_node(symtab, node->child3, 0);
    analyze_node(symtab, node->child4, 0);

    switch (node->kind) {
        case AST_PROGRAMA:
            break;
        
        default:
            if (node->next != NULL) {
                analyze_node(symtab, node->next, 0);
            }
            break;
    }


    // --- AÇÕES DE PÓS-ORDEM ---
    switch (node->kind) {
        
        case AST_COMANDO_ATRIB:
            {
                // Procura o símbolo da variável à esquerda
                SymbolRef id_symbol = symtab_lookup(symtab, node->child1->value);
                if (id_symbol == NULL) {
                    semantic_error(node->lineno, "Variável de atribuição não declarada.");
                    break;
                }
                int id_type = sym_get_data_type(id_symbol);
                
                // Expressão
                int expr_type = check_and_get_type(symtab, node->child2);
                
                // Tipos devem ser iguais
                if (id_type != expr_type) {
                    semantic_error(node->lineno, "Incompatibilidade de tipos na atribuição.");
                }

                sym_free_ref(id_symbol);
            }
            break;
            
        case AST_COMANDO_SE:

        case AST_COMANDO_SE_SENAO:
        
        case AST_COMANDO_ENQUANTO:
            {
                // Expressão de condição deve ser do tipo int (valor lógico)
                int condition_type = check_and_get_type(symtab, node->child1);
                if (condition_type != INT_T) {
                    semantic_error(node->lineno, "Expressão de condição em 'se/enquanto' deve ser do tipo 'int'.");
                }
            }
            break;
        
        case AST_COMANDO_RETORNE:
        {
            int returned_type = check_and_get_type(symtab, node->child1); 
            
            // O tipo de retorno esperado é a variável global
            int expected_type = current_func_type; 

            // Checagem de Tipos
            if (returned_type != expected_type) {
                semantic_error(node->lineno, "Tipo da expressão retornada difere do tipo da função.");
            }
            
            break;
        }

        default:
            break;
    }
}

void analyze_list(SymbolTableRef symtab, AST_Node* node) {
    while (node != NULL) {
        analyze_node(symtab, node, 0);
        node = node->next; 
    }
}

int count_list_items(AST_Node* head) {
    int count = 0;
    AST_Node* current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Função para buscar o AST_DECL_FUNC pelo nome na AST global
AST_Node* find_function_declaration(AST_Node* root, const char* name) {
    if (root == NULL) return NULL;
    
    // O nó raiz (AST_PROGRAMA) tem a lista de globais em child1
    AST_Node* current_node = root->child1;

    while (current_node != NULL) {
        if (current_node->kind == AST_DECL_FUNC) {
            // Verifica o nome da função
            if (strcmp(current_node->child2->value, name) == 0) {
                return current_node;
            }
        }
        // Move para o próximo na lista global
        current_node = current_node->next; 
    }
    return NULL;
}

/**
 * @brief Ponto de entrada para a análise semântica.
 */
void analyze_ast(SymbolTableRef symtab) {
    if (root_ast != NULL) {
        analyze_node(symtab, root_ast, 0);
    }
}