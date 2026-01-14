#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <string.h>
#include "./../AST/ast.h"
#include "./../Tabela_Simbulos/symbolTable.h"

// Definição das constantes de tipo
#define INT_T 1
#define CHAR_T 2
#define VOID_T 4

// Declaração da raiz da AST e Tabela de Símbolos
extern AST_Node* root_ast;
extern SymbolTableRef global_symtab;

// Lista ligada para variáveis globais serem armazenadasno scopo do programa
typedef struct GlobalVarNode {
    AST_Node* node;
    struct GlobalVarNode* next;
} GlobalVarNode;

GlobalVarNode* global_var_list = NULL;
GlobalVarNode* global_var_list_tail = NULL;

// Buffers para acumular o código MIPS
char data_section_buffer[4096] = "";        // Armazena .data (strings)
char text_section_buffer[16384] = "";       // Armazena .text (instruções)
int label_count = 0;                        // Contador para geração de rótulos únicos
int is_global_scope_flag = 1;               // Flag para indicar se estamos no escopo global
int current_var_offset = 0;                 // Offset atual para variáveis locais
int within_function = 0;                    // Flag para indicar se estamos dentro de uma função
int blocos_func = 0;                        // Contador de blocos dentro de funções


// Funções
void generate_mips_code(const char *output_filename);
void generate_node_code(AST_Node *node);
void generate_list_code(AST_Node* head);
int generate_expression(AST_Node *node); 
char* new_label(); 
void append_text(const char* format, ...);
void append_data(const char* format, ...);
int ast_type_to_data_type(AST_Node* type_node);
void add_global_var_node(AST_Node* node);


/*
    * Função: load_variable_address
    * -------------------------------
    * Gera o código MIPS para atualizar o registrador para apontar para o frame
    * pointer a cima de omde a variável está. Além disso, retorna o offset da variável
    *
    * Retorna: offset da variável em caso de sucesso, -1 em caso de erro.
*/
int load_variable_address(AST_Node *node) {
    const char *var_name;

    // --- Determinando da Estrutura (Cada kind armazena a informação em lugares diferentes) ---
    if (node->kind == AST_EXPR_ID) {
        var_name = node->value;
    } else if (node->kind == AST_COMANDO_ATRIB || node->kind == AST_COMANDO_LEIA) {
        var_name = node->child1->value;
    } else {
        fprintf(stderr, "Erro de AST: load_variable_address chamada com tipo de nó invalido (%d).\n", node->kind);
        return -1;
    }
    
    // Buscando o Símbolo
    SymbolRef symbol = symtab_lookup(global_symtab, var_name);
    
    if (symbol == NULL) {
        fprintf(stderr, "Erro de acesso: Variavel '%s' nao declarada.\n", var_name);
        return -1;
    }

    int var_depth = sym_get_variable_depth(symbol);    // Obtendo a profundidade da variável
    int tree_depth = symtab_get_depth(global_symtab);  // Obtendo a profundidade atual da árvore de símbolos
    int offset_id = sym_get_position(symbol);          // Obtendo o offset da variável
    
    // Liberando a referência logo após obter os dados
    sym_free_ref(symbol); 

    // Calculando a diferença de profundidade
    int depth_difference = tree_depth - var_depth;

    append_text("\n  # Acesso a variavel aninhada/global (%s)\n", var_name);

    // Geração do Código MIPS para Achar o Frame Pointer (FP) Correto
    
    // O registrador $t1 será o nosso ponteiro de frame (Base FP)
    // Começa apontando para o FP do escopo atual.
    append_text("  move $t1, $fp\n");
    
    // Se a variável for aninhada (depth_difference > 0), saltamos para o FP correto.
    if (depth_difference > 0) {
        for (int i = 0; i < depth_difference; i++) {
            // Carrega o valor do antigo FP na pilha
            append_text("  lw $t1, 0($t1)\n"); 
        }
    } else if (depth_difference < 0) {
        // Tentativa de acessar um escopo que não existe
        fprintf(stderr, "Erro critico: Profundidade da variavel e maior que a profundidade atual.\n");
        return -1;
    }
    
    return offset_id;
}

/*
    * Função: process_global_vars
    * -------------------------------
    * Processa todas as variáveis globais armazenadas na lista ligada.
    * Chama generate_node_code para cada nó AST_DECL_VAR, que contem
    * a lógica para adicionar a variável no escopo.
*/
void process_global_vars() {
    GlobalVarNode* current = global_var_list;
    
    // Processando os nós armazenados um por um
    while (current != NULL) {
        generate_node_code(current->node); 

        // Avança e libera o nó da lista
        GlobalVarNode* temp = current;
        current = current->next;
        free(temp); 
    }

    // Reseta a lista após o processamento
    global_var_list = NULL;
    global_var_list_tail = NULL;
}

/*
    * Função: add_global_var_node
    * -------------------------------
    * Adiciona um nó AST_DECL_VAR à lista ligada de variáveis globais.
    * Esses nós serão processados posteriormente na função process_global_vars.
*/
void add_global_var_node(AST_Node* node) {
    GlobalVarNode* new_node = (GlobalVarNode*)malloc(sizeof(GlobalVarNode));
    if (new_node == NULL) {
        perror("Falha ao alocar memoria para variavel global");
        return;
    }
    new_node->node = node;
    new_node->next = NULL;

    if (global_var_list == NULL) {
        global_var_list = new_node;
        global_var_list_tail = new_node;
    } else {
        global_var_list_tail->next = new_node;
        global_var_list_tail = new_node;
    }
}


/*
    * Função: append_text
    * -------------------------------
    * Adiciona uma linha no arquivo do código MIPS no campo .text
*/
void append_text(const char* format, ...) {
    va_list args;
    char buffer[512];
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Concatena no buffer global
    if (strlen(text_section_buffer) + strlen(buffer) < sizeof(text_section_buffer)) {
        strcat(text_section_buffer, buffer);
    } else {
        fprintf(stderr, "Buffer da seção .text estourou!\n");
    }
}

/*
    * Função: generate_list_code
    * -------------------------------
    * Chama generate_node_code para cada nó de uma lista.
*/
void generate_list_code(AST_Node* head) {
    AST_Node* current = head;
    while (current != NULL) {
        generate_node_code(current);
        current = current->next;
    }
}

/*
    * Função: append_data
    * -------------------------------
    * Adiciona uma linha no arquivo da seção de dados MIPS no campo .data
*/
void append_data(const char* format, ...) {
    va_list args;
    char buffer[512];
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Concatena no buffer global
    if (strlen(data_section_buffer) + strlen(buffer) < sizeof(data_section_buffer)) {
        strcat(data_section_buffer, buffer);
    } else {
        fprintf(stderr, "Buffer da seção .data estourou!\n");
    }
}

/*
    * Função: new_label
    * -------------------------------
    * Gera um novo rótulo único para uso em saltos e branches.
*/
char* new_label() {
    char *label = (char*)malloc(16);
    snprintf(label, 16, "L%d", label_count++);
    return label;
}

/*
    * Função: generate_expression
    * -------------------------------
    * Gera código MIPS para expressões representadas por nós AST.
*/
int generate_expression(AST_Node *node) {
    if (!node) return -1;

    switch (node->kind) {
        
        case AST_CONST_INT:
            append_text("\n  # Expressao: Constante INT\n");
            append_text("  li $t0, %s\n", node->value);
            return 0;

        case AST_CONST_CAR:
            append_text("\n  # Expressao: Constante CHAR\n");
            append_text("  li $t0, %d\n", node->value[1]);     // Pega o valor ASCII do char como em 'A'
            return 0;

        case AST_EXPR_ID:
            {
                append_text("\n  # Expressao: Variavel ID (%s)\n", node->value);

                int offset_id = load_variable_address(node);
                if (offset_id == -1) {
                    return;
                }

                append_text("  lw $t0, %d($t1)\n", offset_id); 
                return 0;
            }

        case AST_EXPR_BINARIA:
            append_text("\n  # Expressao: Binaria %s\n", node->value);
            
            // Parte Esquerda da expressão
            generate_expression(node->child1);    // Chama AST_EXPR_ID
            
            // Armazenando temporariamente o resultado da esquerda na pilha
            append_text("  sw $t0, 0($sp)\n");
            append_text("  addi $sp, $sp, -4\n");

            // Parte Direita da expressão
            generate_expression(node->child2);

            // Pegando de volta o valor da esquerda
            append_text("  lw $t1, 4($sp)\n");

            // 4. Operação
            if (strcmp(node->value, "+") == 0) {
                append_text("  add $t0, $t1, $t0\n");
            } else if (strcmp(node->value, "-") == 0) {
                append_text("  sub $t0, $t1, $t0\n");
            } else if (strcmp(node->value, "*") == 0) {
                append_text("  mult $t1, $t0\n");
                append_text("  mflo $t0\n");
            } else if (strcmp(node->value, "/") == 0) {
                append_text("  div $t1, $t0\n");
                append_text("  mflo $t0\n");
            } else if (strcmp(node->value, "==") == 0) { 
                append_text("  sub $t0, $t1, $t0\n");       // $t0 = Esquerda - Direita. Se 0, são iguais.
                append_text("  sltiu $t0, $t0, 1\n");       // $t0 = ($t0 == 0) ? 1 : 0. Retorna 1 se a diferença for 0.
            } else if (strcmp(node->value, "!=") == 0) {
                append_text("  sub $t0, $t1, $t0\n");       // t0 = esquerda - direita
                append_text("  sltu $t0, $zero, $t0\n");    // t0 = (t0 != 0) ? 1 : 0
            } else if (strcmp(node->value, ">") == 0) {     
                append_text("  slt $t0, $t0, $t1\n");       // $t0 = (t0 < t1) ? 1 : 0
            } else if (strcmp(node->value, "<") == 0) {
                append_text("  slt $t0, $t1, $t0\n");
            }
            append_text("  addi $sp, $sp, 4\n");
            return 0;
        
        case AST_EXPR_UNARIA:
            append_text("\n  # Expressao: Unaria %s\n", node->value);
            
            // Gerando o código para o operando, que coloca o valor em $t0
            generate_expression(node->child1);
            
            // Aplicarndo o operador unário
            if (strcmp(node->value, "-") == 0) {
                // Negação unária
                append_text("  neg $t0, $t0\n"); 
            } else if (strcmp(node->value, "!") == 0) {
                // Operador Lógico NOT (Se 0, torna 1; se não 0, torna 0)
                // SLTIU $t0, $t0, 1 -> $t0 = ($t0 < 1) ? 1 : 0. Isso nega 0 e torna não-zeros em 0.
                append_text("  sltiu $t0, $t0, 1\n"); 
            } else {
                fprintf(stderr, "Erro de compilacao: Operador unario desconhecido '%s'.\n", node->value);
                return -1;
            }
            return 0;

        case AST_COMANDO_ATRIB:
            generate_expression(node->child2); 
            append_text("\n  # Comando: Atribuicao %s = \n", node->child1->value);
            
            int offset_atrib = load_variable_address(node);
            if (offset_atrib == -1) {
                return;
            }
            
            // Salva o valor no offset obtido
            append_text("  sw $t0, %d($t1)\n", offset_atrib);
            return 0;

        case AST_EXPR_CHAMADA_FUNC:
            int aux_blocos_func = blocos_func;
            blocos_func = 0;
            char* func_name_call = node->child1->value;
            AST_Node* actual_args = node->child2;
            
            append_text("\n  # Expressao: Chamada de Funcao %s\n", func_name_call);

            AST_Node* current_arg = actual_args;
            int arg_count = 0;
            int stack_args_pushed = 0;
            
            // Processando Argumentos
            while (current_arg != NULL) {
                
                // Gerando o valor da expressão no $t0
                generate_expression(current_arg); 

                if (arg_count < 4) {
                    // Argumentos 1-4 vão para $a0 a $a3
                    char arg_reg[4];
                    snprintf(arg_reg, sizeof(arg_reg), "$a%d", arg_count);
                    append_text("  move %s, $t0\n", arg_reg);              // Move o valor para o registrador $aN
                    
                } else {
                    // Argumentos 5+ vão para a pilha (empilhando da esquerda para a direita)
                    append_text("\n  # Argumento %d vai para a pilha\n", arg_count + 1);
                    append_text("  sw $t0, 0($sp)\n");                    // Salva o valor em $t0 na pilha
                    append_text("  addi $sp, $sp, -4\n");                 // Aloca 4 bytes (decrementa $sp)
                    stack_args_pushed += 4;                               // Acumula o espaço alocado
                }

                current_arg = current_arg->next;
                arg_count++;
            }

            // Chama a função
            append_text("  jal %s\n", func_name_call);
            
            // Limpa os argumentos da pilha (se houver)
            // O chamador é responsável por desalocar o espaço dos argumentos 5+
            if (stack_args_pushed > 0) {
                append_text("\n  # Limpa %d bytes de argumentos da pilha\n", stack_args_pushed);
                append_text("  addi $sp, $sp, %d\n", stack_args_pushed);
            }
            
            // O valor de retorno está em $v0. Move para $t0 para ser usado na expressão.
            append_text("  move $t0, $v0\n");
            blocos_func = aux_blocos_func;
            return 0;
            
        default:
            return -1;
    }
}


/*
    * Gera o código MIPS para um nó AST específico.
    * @param node O nó AST para o qual gerar o código.
*/
void generate_node_code(AST_Node *node) {
    if (node == NULL) return;

    // Verifica se a declaração de variável foi feita no escopo global e
    // guarda essa informação para colocar no primeiro escopo do programa
    if ((is_global_scope_flag) && (within_function == 0)) {
        if (node->kind == AST_DECL_VAR) {
            // Se for uma declaração de variável global, armazena para processamento posterior
            add_global_var_node(node);
            return;
        }
    }
    
    switch (node->kind) {
        
        case AST_PROGRAMA:
            // Chamando as declarações globais primeiro
            generate_list_code(node->child1);
            
            // Início do código principal
            append_text(".globl main\n");
            append_text("main:\n");

            // --- Prólogo de MAIN ---
            append_text("\n  # Prologo: Salva $ra e $fp, e configura $fp para main\n");
            append_text("  addi $sp, $sp, -4\n"); 
            append_text("  sw $ra, 4($sp)\n");                    // Salva o endereço de retorno
            append_text("  sw $fp, 0($sp)\n");                    // Salva o $fp antigo
            append_text("  move $fp, $sp\n");                     // Configura $fp para a base do frame
            append_text("  addi $sp, $sp, -4\n");

            is_global_scope_flag = 0;
            generate_list_code(node->child2);                     // Bloco Principal

            // Epílogo da Main e Código de saída
            append_text("\n  # Epilogo: Restaura $fp e $ra\n");
            append_text("  lw $ra, 4($fp)\n"); 
            append_text("  lw $fp, 0($fp)\n"); 
            append_text("  addi $sp, $sp, 4\n");
            
            append_text("\n  # Fim da execucao\n");
            append_text("  li $v0, 10\n");
            append_text("  syscall\n");
            return;
            
        case AST_BLOCO:
            blocos_func +=1;
            // Prologo do bloco
            append_text("\n  # Prologo: Salva $ra e $fp, e configura $fp\n");
            append_text("  sw $fp, 0($sp)\n"); 
            append_text("  move $fp, $sp\n");
            append_text("  addi $sp, $sp, -4\n");

            symtab_enter_scope(global_symtab);
    
            int previous_frame_offset = current_var_offset;     // Salva o offset do escopo pai
            current_var_offset = 0;                             // O novo offset para o escopo atual começa em 0
            

            // Processa variáveis globais que foram armazenadas anteriormente
            if ((global_var_list != NULL) && !is_global_scope_flag) {
                process_global_vars();
            }

            generate_list_code(node->child1); 
            generate_list_code(node->child2); 

            // Epilogo do bloco
            append_text("\n  # Epilogo: Restaura $fp e $ra\n");
            append_text("  move $sp, $fp\n");
            append_text("  lw $fp, 0($sp)\n");
            
            symtab_exit_scope(global_symtab);
            current_var_offset = previous_frame_offset;         // Restaura o offset do escopo pai
            
            append_text("\n  # Bloco de comandos (Saida)\n");
            blocos_func -=1;
            return;

        case AST_DECL_VAR:
            append_text("\n  # Declaracao de variavel: %s\n", node->child2->value);
            AST_Node* current_id_node = node->child2; 
            
            while (current_id_node != NULL) {
                // Decrementando o offset para a nova variável
                current_var_offset -= 4;                        // Aloca 4 bytes
                
                // Deixando o lugar da variável separado na pilha
                append_text("  addi $sp, $sp, -4\n");

                // Obtendo o tipo
                int data_type = ast_type_to_data_type(node->child1);

                // Inserindo na Tabela de Símbolos com o offset
                symtab_insert_var(global_symtab, current_id_node->value, data_type, current_var_offset); 
                
                current_id_node = current_id_node->next;
            }
            return;
            
        case AST_COMANDO_ATRIB:
            {
                generate_expression(node); 
                return;
            }
            
        case AST_COMANDO_ESCREVA:
            if (node->child1->kind == AST_CONST_CADEIA) {
                char *str_label = new_label();
                
                // Escrevendo a string na seção de dados
                append_data("%s: .asciiz %s\n", str_label, node->child1->value);
                
                append_text("\n  # Comando: Escreva String\n");
                append_text("  li $v0, 4\n");
                append_text("  la $a0, %s\n", str_label);
                append_text("  syscall\n");
                
            } else {
                append_text("\n  # Comando: Escreva Valor (Int ou Char)\n");
                generate_expression(node->child1);

                int output_syscall = 1;                     // Assume Inteiro (1) por padrão

                if (node->child1->kind == AST_EXPR_ID) {
                    // Se for um identificador (variável), consulta a Tabela de Símbolos
                    SymbolRef symbol = symtab_lookup(global_symtab, node->child1->value);
                    
                    if (symbol == NULL) {
                        fprintf(stderr, "Erro de compilacao: Variavel '%s' nao declarada para escrita.\n", node->child1->value);
                        return;
                    }

                    if (sym_get_data_type(symbol) == CHAR_T) {
                        // Se o tipo da variável for CHAR_T, ajusta o syscall para imprimir caractere
                        output_syscall = 11;                // Syscall 11: Print Char
                        append_text("\n  # Tipo detectado: Variavel CHAR\n");
                    }
                    // Liberando a referência
                    sym_free_ref(symbol);
                } 

                // Chamando o syscall apropriado
                append_text("  li $v0, %d\n", output_syscall); // 1 (Int) ou 11 (Char)
                append_text("  move $a0, $t0\n");              // Movemdp o valor para o registrador de argumento
                append_text("  syscall\n");
            }
            return;

        case AST_COMANDO_NOVALINHA:
            append_text("\n  # Comando: Novalinha\n");
            append_text("  li $v0, 4\n");
            append_text("  la $a0, __newline\n");
            append_text("  syscall\n");
            return;

        case AST_DECL_FUNC:
            blocos_func = 0;
            within_function += 1;
    
            char* func_name = node->child2->value;
            append_text("\n.globl %s\n", func_name);
            append_text("%s:\n", func_name);
            
            // --- Prólogo da Função ---
            append_text("\n  # Prologo: Salva $ra e $fp, e configura $fp\n");
            append_text("  addi $sp, $sp, -4\n");
            append_text("  sw $ra, 4($sp)\n"); 
            append_text("  sw $fp, 0($sp)\n"); 
            append_text("  move $fp, $sp\n");
            append_text("  addi $sp, $sp, -4\n");

            // --- Mapeamento e Alocação de Parâmetros ---
            symtab_enter_scope(global_symtab);

            int prev_frame_offset = current_var_offset;
            current_var_offset = 0;              // Novo offset para o frame atual
            int arg_reg_count = 0;

            AST_Node* param_node = node->child3; // Lista de parâmetros
            
            // Salvando Argumentos $a0-$a3 no novo Frame
            AST_Node* current_param = param_node;
            
            while (current_param != NULL && arg_reg_count < 4) {
                char* param_name = current_param->child2->value;
                
                // Offset para variáveis locais
                current_var_offset -= 4; 

                // Obtendo o tipo do parâmetro
                int data_type = ast_type_to_data_type(current_param->child1);
                
                // Inserindo na Tabela de Símbolos com o offset
                symtab_insert_var(global_symtab, param_name, data_type, current_var_offset);

                // Gerando código para salvar o registrador $aN na pilha
                char arg_reg[4];
                snprintf(arg_reg, sizeof(arg_reg), "$a%d", arg_reg_count);
                
                append_text("\n  # Salvando argumento %d (%s) de %s para %d($fp)\n", 
                            arg_reg_count + 1, param_name, arg_reg, current_var_offset);
                
                append_text("  sw %s, 0($sp)\n", arg_reg);             // Salva o $aN
                append_text("  addi $sp, $sp, -4\n");

                current_param = current_param->next;
                arg_reg_count++;
            }
            
            // Mapeando Argumentos 5+
            // Estes já estão na pilha do chamador, acima do $fp do callee.
            // Começa em +8($fp) (4 bytes para $fp_antigo, 4 para $ra)
            int stack_arg_offset = 8; 
            while (current_param != NULL) {
                char* param_name = current_param->child2->value;
                int data_type = ast_type_to_data_type(current_param->child1);
                
                // Insere na Tabela de Símbolos com o offset
                symtab_insert_var(global_symtab, param_name, data_type, stack_arg_offset);
                
                append_text("\n  # Mapeando argumento %d (%s) em %d($fp)\n", 
                            arg_reg_count + 1, param_name, stack_arg_offset);
                
                stack_arg_offset += 4;
                current_param = current_param->next;
                arg_reg_count++;
            }

            // Processando o corpo da função (AST_BLOCO)
            generate_node_code(node->child4); 
            
            symtab_exit_scope(global_symtab);

            current_var_offset = prev_frame_offset;
            within_function -= 1;
            return;

        case AST_COMANDO_SE_SENAO:
            {   
                char *label_senao = new_label(); 
                char *label_fim = new_label();   
                
                // Gerando a expressão de condição (n==0) em $t0.
                generate_expression(node->child1); 
                
                // Se $t0 for FALSO (0), pula para o SENAO
                append_text("\n  # Comando: SE (Expressao em $t0)\n");
                append_text("  beq $t0, $zero, %s\n", label_senao);
                
                // Bloco ENTAO
                generate_node_code(node->child2); 
                append_text("  j %s\n", label_fim); 
                
                // Bloco SENAO
                append_text("%s:\n", label_senao);
                generate_node_code(node->child3); 
                
                // FIM
                append_text("%s:\n", label_fim);
                return;
            }

        case AST_COMANDO_ENQUANTO:
            {
                char *label_inicio = new_label(); 
                char *label_fim = new_label();    
                
                append_text("\n\n  # Comando: ENQUANTO\n");
                append_text("%s:\n", label_inicio);              // Rótulo do início do laço
                
                // Gerando a expressão de condição (n>0) em $t0.
                generate_expression(node->child1); 
                
                // Se $t0 for FALSO (0), pula para o FIM do laço
                append_text("  beq $t0, $zero, %s\n", label_fim);
                
                // Bloco EXECUTE
                generate_node_code(node->child2); 
                
                // Salto de volta para o INÍCIO
                append_text("  j %s\n", label_inicio);
                
                // Rótulo do FIM
                append_text("%s:\n", label_fim);
                return;
            }

        case AST_COMANDO_RETORNE:
            {
                // Gerando o valor de retorno.
                generate_expression(node->child1); 
                
                // Movendo o valor de retorno para o registrador $v0 (convenção MIPS).
                append_text("\n  # Comando: Retorne\n");
                append_text("  move $v0, $t0\n");
                
                // --- Epílogo da Função ---
                append_text("\n  # Epilogo: Restaura $fp e $ra\n");
                for(int i = 0; i < blocos_func; i++){
                    append_text("  lw $fp, 0($fp)\n");
                }
                append_text("  move $sp, $fp\n");               // $sp aponta para o $fp salvo
                append_text("  lw $ra, 4($sp)\n");              // $ra estava em $fp + 4
                append_text("  lw $fp, 0($sp)\n");              // $fp estava em $fp
                append_text("  addi $sp, $sp, 4\n");            // Ajustando $sp para cima
                
                // Retorna
                append_text("  jr $ra\n"); 
                return;
            
            }

        case AST_COMANDO_LEIA:
            {
                char* var_name_read = node->child1->value;
                
                append_text("\n  # Comando: Leia Valor para %s\n", var_name_read);
                append_text("  li $v0, 5\n");                // Código 5 para Read Int
                append_text("  syscall\n");                  // O valor lido está em $v0
                
                int offset_read = load_variable_address(node);
                if (offset_read == -1) {
                    return;
                }
                
                // Salva o valor lido ($v0) na variável
                append_text("  sw $v0, %d($t1)\n", offset_read);
                
                return;
            }

        case AST_EXPR_CHAMADA_FUNC:
            generate_expression(node);
            return;
            
        default:
            break;
    }
    
}

/*
    * Função principal que é chamada pela main para gerar o código MIPS a partir da AST.
*/
void generate_mips_code(const char *output_filename) {
    if (root_ast == NULL) {
        fprintf(stderr, "Erro: AST nao construida. Nao e possivel gerar codigo.\n");
        return;
    }
    
    // Inicializando buffers e contadores
    data_section_buffer[0] = '\0';
    text_section_buffer[0] = '\0';
    label_count = 0;

    // Abrindo o arquivo
    FILE *mips_file = fopen(output_filename, "w");
    if (!mips_file) {
        perror("Erro ao abrir arquivo de saida MIPS");
        return;
    }

    // Gerando o código
    generate_node_code(root_ast);

    // Seção de Dados (.data) - Deve vir primeiro
    fprintf(mips_file, ".data\n");
    fprintf(mips_file, "__newline: .asciiz \"\\n\"\n"); 
    fprintf(mips_file, "%s", data_section_buffer); 

    // Seção de Código (.text)
    fprintf(mips_file, ".text\n");
    fprintf(mips_file, "%s", text_section_buffer);
    
    fclose(mips_file);
    printf("Codigo MIPS gerado com sucesso no arquivo: %s\n\n", output_filename);
}