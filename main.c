#include <stdio.h>
#include <stdlib.h>
#include "./AST/ast.h"
#include "./Tabela_Simbulos/symbolTable.h"

// Declarações externas
extern FILE *yyin;                                           // Arquivo que o Flex lê
extern char* yytext;                                         // Lexema atual recebido do Flex
extern int yyparse();                                        // Função do Bison para iniciar o parsing
extern void analyze_ast(SymbolTableRef symtab);              // Função de análise semântica
extern void generate_mips_code(const char *output_filename); // Função de geração de código MIPS
extern AST_Node* root_ast;                                   // Declaração da raiz global da AST, preenchida pelo Bison

// Variável Global para a Tabela de Símbolos (Usada pelo codigo.c)
SymbolTableRef global_symtab = NULL; 

// Nomes dos tipos de nós da AST para impressão
const char *AST_NodeKind_Names[] = {
    "AST_PROGRAMA",
    "AST_DECL_VAR",
    "AST_DECL_FUNC",
    "AST_TIPO_INT",
    "AST_TIPO_CAR",
    "AST_BLOCO",
    "AST_COMANDO_ATRIB",
    "AST_COMANDO_SE",
    "AST_COMANDO_SE_SENAO",
    "AST_COMANDO_ENQUANTO",
    "AST_COMANDO_RETORNE",
    "AST_COMANDO_LEIA",
    "AST_COMANDO_ESCREVA",
    "AST_COMANDO_NOVALINHA",
    "AST_COMANDO_VAZIO",
    "AST_EXPR_BINARIA",
    "AST_EXPR_UNARIA",
    "AST_EXPR_ID",
    "AST_EXPR_CHAMADA_FUNC",
    "AST_CONST_INT",
    "AST_CONST_CAR",
    "AST_CONST_CADEIA",
    "AST_LISTA_DECL_FUNC_VAR",
    "AST_LISTA_DECL_VAR",
    "AST_LISTA_COMANDO",
    "AST_LISTA_PARAMETROS",
    "AST_LISTA_EXPRESSOES"
};

// Tokens
#define PROGRAMA 256
#define CAR 257
#define INT 258
#define RETORNE 259
#define LEIA 260
#define ESCREVA 261
#define NOVALINHA 262
#define SE 263
#define ENTAO 264
#define SENAO 265
#define ENQUANTO 266
#define EXECUTE 267
#define ID 268
#define INTCONST 269
#define CARCONST 270
#define CADEIA_CARACTERES 271
#define VIRGULA 272
#define PONTO_VIRGULA 273
#define ABRE_PAR 274
#define FECHA_PAR 275
#define ABRE_CHAVE 276
#define FECHA_CHAVE 277
#define ATRIBUICAO 278
#define IGUAL 279
#define DIFERENTE 280
#define MENOR 281
#define MAIOR 282
#define MENOR_IGUAL 283
#define MAIOR_IGUAL 284
#define MAIS 285
#define MENOS 286
#define MULT 287
#define DIV 288
#define NOT 289
#define OU 290
#define E 291
#define EOF_TOKEN -1

// --- Função de Impressão da AST ---
void print_ast_node(AST_Node* node, int depth) {
    if (node == NULL) return;
    
    // Imprime o nó com indentação
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    // Converte o número do 'kind' para o nome da string
    const char *kind_name = (node->kind >= 0 && node->kind < sizeof(AST_NodeKind_Names) / sizeof(AST_NodeKind_Names[0]))
                            ? AST_NodeKind_Names[node->kind]
                            : "AST_KIND_DESCONHECIDO";
    
    printf("- K: %s (Linha: %d", kind_name, node->lineno);
    if (node->value) {
        printf(", Valor: %s", node->value);
    }
    printf(")\n");

    // Imprime os filhos
    print_ast_node(node->child1, depth + 1);
    print_ast_node(node->child2, depth + 1);
    print_ast_node(node->child3, depth + 1);
    print_ast_node(node->child4, depth + 1);
    
    // Imprime o próximo (para listas)
    print_ast_node(node->next, depth);
}


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }

    // Abre o arquivo de entrada
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", argv[1]);
        return 1;
    }

    int print_tree = 0; // 1 para imprimir a árvore, 0 para não imprimir
    
    // Executa o parser
    if (yyparse() == 0) {
        printf("\nAnálise sintática concluída com sucesso!\n");
        
        if (root_ast != NULL) {
            // Cria a tabela de símbolos
            SymbolTableRef symtab = symtab_create();

            // Imprime a AST
            if(print_tree){
                printf("\n--- ÁRVORE SINTÁTICA ABSTRATA (AST) CONSTRUÍDA ---\n");
                print_ast_node(root_ast, 0); 
                printf("----------------------------------------------------\n\n");
            }
                        
            // Análise Semântica
            analyze_ast(symtab);
            printf("Análise semantica concluída com sucesso!\n");

            global_symtab = symtab;

            // Geração de Código MIPS
            generate_mips_code("output.asm");


            symtab_destroy(symtab);
        } else {
            printf("A AST foi aceita, mas root_ast está NULL (Verifique se a regra 'Programa' em goianinha.y está atribuindo $$ e root_ast).\n");
        }
    } else {
        printf("Erros encontrados durante a análise sintática. AST não foi construída.\n");
    }

    fclose(yyin);
    return 0;
}