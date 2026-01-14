#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

// Constantes para mapear os enum class para tipos C int
#define DT_INT  1
#define DT_CHAR 2
#define DT_FLOAT 3
#define DT_VOID 4

// --- Início da Interface C ---
#ifdef __cplusplus
#include <memory>
#include <string>
#include <vector>
extern "C" {
#endif

// TIPOS OPACOS PARA O C (void*)
typedef void* SymbolTableRef; 
typedef void* SymbolRef;

// Gerenciamento
SymbolTableRef symtab_create();
void symtab_destroy(SymbolTableRef table);

// Controle de Escopo
void symtab_enter_scope(SymbolTableRef table);
void symtab_exit_scope(SymbolTableRef table);

// Inserção
int symtab_insert_var(SymbolTableRef table, const char* name, int type, int pos);
int symtab_insert_func(SymbolTableRef table, const char* name, int num_params, int return_type);

// Busca e Getters
SymbolRef symtab_lookup(SymbolTableRef table, const char* name);
SymbolRef symtab_lookup_current_scope(SymbolTableRef table, const char* name);
int sym_get_data_type(SymbolRef symbol);
int sym_get_position(SymbolRef symbol);
int sym_get_num_params(SymbolRef symbol);
int sym_get_variable_depth(SymbolRef symbol);
int symtab_get_depth(SymbolTableRef table);

// Fim da Interface C
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// Declarações C++
enum class SymbolType { VARIABLE, FUNCTION, PARAMETER };
// Usando as macros para que os inteiros correspondam aos defines C
enum class DataType { INT=DT_INT, CHAR=DT_CHAR, FLOAT=DT_FLOAT, VOID=DT_VOID }; 


// Estruturas e Classes C++
struct Symbol {
    int declaration_depth;
    std::string name;
    SymbolType type;
    DataType data_type;
    int position;
    int num_params;
    std::vector<Symbol> parameters;
    
    Symbol(const std::string& n, SymbolType t, DataType dt, int pos = 0, int np = 0, int depth = 0);
};

struct AVLNode {
    Symbol symbol;
    std::shared_ptr<AVLNode> left;
    std::shared_ptr<AVLNode> right;
    int height;
    
    AVLNode(const Symbol& s);
};

class SymbolAVLTree {
private:
    std::shared_ptr<AVLNode> root;
    
    int height(std::shared_ptr<AVLNode> node) const;
    int balanceFactor(std::shared_ptr<AVLNode> node) const;
    void updateHeight(std::shared_ptr<AVLNode> node);
    std::shared_ptr<AVLNode> rotateRight(std::shared_ptr<AVLNode> y);
    std::shared_ptr<AVLNode> rotateLeft(std::shared_ptr<AVLNode> x);
    std::shared_ptr<AVLNode> balance(std::shared_ptr<AVLNode> node);
    std::shared_ptr<AVLNode> insert(std::shared_ptr<AVLNode> node, const Symbol& symbol);
    std::shared_ptr<AVLNode> find(std::shared_ptr<AVLNode> node, const std::string& name) const;

public:
    SymbolAVLTree();
    void insert(const Symbol& symbol);
    std::shared_ptr<Symbol> find(const std::string& name) const;
};

class SymbolTable {
private:
    std::vector<SymbolAVLTree> scopes;

public:
    void enterScope();
    void exitScope();
    bool insert(const Symbol& symbol);
    bool insertFunction(const std::string& name, int num_params, DataType return_type);
    bool insertVariable(const std::string& name, DataType type, int position);
    bool insertParameter(const std::string& name, DataType type, int position);
    std::shared_ptr<Symbol> lookup(const std::string& name) const;
    std::shared_ptr<Symbol> lookupCurrentScope(const std::string& name) const;
    size_t scopeCount() const;
};

#endif // __cplusplus

#endif // SYMBOL_TABLE_H