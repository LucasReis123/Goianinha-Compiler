#include "symbolTable.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

extern "C" {

#define GET_SYMTAB(ref) (reinterpret_cast<SymbolTable*>(ref))
#define GET_SYMBOL(ref) (reinterpret_cast<std::shared_ptr<Symbol>*>(ref))
#define SAFE_STRING(ptr) ((ptr) ? (ptr) : "")

// Criação e Destruição
SymbolTableRef symtab_create() {
    // Aloca a classe C++ na memória e retorna o endereço como void*
    return reinterpret_cast<SymbolTableRef>(new SymbolTable());
}

void symtab_destroy(SymbolTableRef table) {
    // Converte o void* de volta e chama o destrutor do objeto C++
    delete GET_SYMTAB(table);
}

// CONTROLE DE ESCOPO
void symtab_enter_scope(SymbolTableRef table) {
    SymbolTable* sym_table = GET_SYMTAB(table);
    sym_table->enterScope(); 
}

void symtab_exit_scope(SymbolTableRef table) {
    GET_SYMTAB(table)->exitScope();
}


// INSERÇÃO
int symtab_insert_var(SymbolTableRef table, const char* name, int data_type_int, int position) {
    SymbolTable* sym_table = GET_SYMTAB(table);
    
    // Converte o char* (C) para std::string (C++)
    std::string var_name(SAFE_STRING(name));

    // Converte o int (C) para o enum class DataType (C++)
    DataType dt = static_cast<DataType>(data_type_int);

    return sym_table->insertVariable(var_name, dt, position) ? 0 : 1; 
}

// --- INSERIR FUNÇÃO ---
int symtab_insert_func(SymbolTableRef table, const char* name, int num_params, int return_type_int) {
    SymbolTable* sym_table = GET_SYMTAB(table);
    std::string func_name(SAFE_STRING(name));
    DataType dt = static_cast<DataType>(return_type_int);

    return sym_table->insertFunction(func_name, num_params, dt) ? 0 : 1; 
}


// --- LOOKUP ---
SymbolRef symtab_lookup(SymbolTableRef table, const char* name) {
    SymbolTable* sym_table = GET_SYMTAB(table);
    std::string lookup_name(SAFE_STRING(name));

    auto symbol_ptr = sym_table->lookup(lookup_name);

    if (!symbol_ptr) {
        return nullptr;
    }
    
    std::shared_ptr<Symbol>* heap_ptr = new std::shared_ptr<Symbol>(symbol_ptr);
    return reinterpret_cast<SymbolRef>(heap_ptr);
}


// --- GETTERS  ---

// Função para obter o Symbol* do SymbolRef
Symbol* get_raw_symbol(SymbolRef symbol) {
    if (!symbol) return nullptr;
    std::shared_ptr<Symbol>* heap_ptr = reinterpret_cast<std::shared_ptr<Symbol>*>(symbol);
    
    return heap_ptr->get();
}

int sym_get_num_params(SymbolRef symbol) {
    Symbol* raw_symbol = get_raw_symbol(symbol);
    if (!raw_symbol) return 0;

    return raw_symbol->num_params;
}

int sym_get_data_type(SymbolRef symbol) {
    Symbol* raw_symbol = get_raw_symbol(symbol);
    if (!raw_symbol) return -1; 

    return static_cast<int>(raw_symbol->data_type);
}

// --- LOOKUP NO ESCOPO ATUAL ---
SymbolRef symtab_lookup_current_scope(SymbolTableRef table, const char* name) {
    SymbolTable* sym_table = GET_SYMTAB(table);
    std::string lookup_name(SAFE_STRING(name));

    auto symbol_ptr = sym_table->lookupCurrentScope(lookup_name);

    if (!symbol_ptr) {
        return nullptr;
    }

    std::shared_ptr<Symbol>* heap_ptr = new std::shared_ptr<Symbol>(symbol_ptr);
    
    return reinterpret_cast<SymbolRef>(heap_ptr);
}

// Liberação de memória do SymbolRef
void sym_free_ref(SymbolRef symbol) {
    if (symbol) {
        delete reinterpret_cast<std::shared_ptr<Symbol>*>(symbol);
    }
}

// Implementação do Getter para Posição
int sym_get_position(SymbolRef symbol) {
    Symbol* raw_symbol = get_raw_symbol(symbol);
    if (!raw_symbol) return 0;

    return raw_symbol->position;
}

// Implementação do Getter para Profundidade de Declaração da variável na tabela de símbolos
int sym_get_variable_depth(SymbolRef symbol) {
    Symbol* raw_symbol = get_raw_symbol(symbol);
    if (!raw_symbol) return -1; 
    return raw_symbol->declaration_depth;
}

// Implementação do Getter para Profundidade da Tabela de Símbolos
int symtab_get_depth(SymbolTableRef table) {
    SymbolTable* symtab = GET_SYMTAB(table);
    
    return (int)symtab->scopeCount(); 
}

} // Fim do bloco extern "C"

// Implementação da estrutura Symbol
Symbol::Symbol(const std::string& n, SymbolType t, DataType dt, int pos, int np, int depth) : name(n), type(t), data_type(dt), position(pos), num_params(np) {}

// Implementação do AVLNode
AVLNode::AVLNode(const Symbol& s) : symbol(s), left(nullptr), right(nullptr), height(1) {}



// -------------------------- IMPLEMENTAÇÃO DA SymbolAVLTree

SymbolAVLTree::SymbolAVLTree() : root(nullptr) {}

int SymbolAVLTree::height(std::shared_ptr<AVLNode> node) const {
    return node ? node->height : 0;
}

int SymbolAVLTree::balanceFactor(std::shared_ptr<AVLNode> node) const {
    return node ? height(node->left) - height(node->right) : 0;
}

void SymbolAVLTree::updateHeight(std::shared_ptr<AVLNode> node) {
    if (node) {
        node->height = 1 + std::max(height(node->left), height(node->right));
    }
}

std::shared_ptr<AVLNode> SymbolAVLTree::rotateRight(std::shared_ptr<AVLNode> y) {
    auto x = y->left;
    y->left = x->right;
    x->right = y;
    
    updateHeight(y);
    updateHeight(x);
    
    return x;
}

std::shared_ptr<AVLNode> SymbolAVLTree::rotateLeft(std::shared_ptr<AVLNode> x) {
    auto y = x->right;
    x->right = y->left;
    y->left = x;
    
    updateHeight(x);
    updateHeight(y);
    
    return y;
}

std::shared_ptr<AVLNode> SymbolAVLTree::balance(std::shared_ptr<AVLNode> node) {
    if (!node) return nullptr;
    
    updateHeight(node);
    
    int bf = balanceFactor(node);
    
    // Casos de desbalanceamento
    if (bf > 1) {
        if (balanceFactor(node->left) >= 0) {
            return rotateRight(node);
        } else {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
    }
    if (bf < -1) {
        if (balanceFactor(node->right) <= 0) {
            return rotateLeft(node);
        } else {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
    }
    
    return node;
}

std::shared_ptr<AVLNode> SymbolAVLTree::insert(std::shared_ptr<AVLNode> node, const Symbol& symbol) {
    if (!node) return std::make_shared<AVLNode>(symbol);
    
    if (symbol.name < node->symbol.name) {
        node->left = insert(node->left, symbol);
    } else if (symbol.name > node->symbol.name) {
        node->right = insert(node->right, symbol);
    } else {
        return node;
    }
    
    return balance(node);
}

void SymbolAVLTree::insert(const Symbol& symbol) {
    root = insert(root, symbol);
}

std::shared_ptr<AVLNode> SymbolAVLTree::find(std::shared_ptr<AVLNode> node, const std::string& name) const {
    if (!node) return nullptr;
    
    if (name < node->symbol.name) {
        return find(node->left, name);
    } else if (name > node->symbol.name) {
        return find(node->right, name);
    } else {
        return node;
    }
}

std::shared_ptr<Symbol> SymbolAVLTree::find(const std::string& name) const {
    auto node = find(root, name);
    return node ? std::make_shared<Symbol>(node->symbol) : nullptr;
}

// -------------------------- IMPLEMENTAÇÃO DA SymbolTable

bool SymbolTable::insertFunction(const std::string& name, int num_params, DataType return_type) {
    Symbol symbol(name, SymbolType::FUNCTION, return_type, 0, num_params);
    return insert(symbol);
}

bool SymbolTable::insertVariable(const std::string& name, DataType type, int position) {
    Symbol symbol(name, SymbolType::VARIABLE, type, position);
    return insert(symbol);
}

bool SymbolTable::insertParameter(const std::string& name, DataType type, int position) {
    Symbol symbol(name, SymbolType::PARAMETER, type, position);
    return insert(symbol);
}

void SymbolTable::enterScope() {
    scopes.push_back(SymbolAVLTree());
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

bool SymbolTable::insert(const Symbol& symbol) {
    if (scopes.empty()) {
        enterScope();
    }
    
    if (scopes.back().find(symbol.name)) {
        return false;
    }
    
    Symbol new_symbol = symbol;
    new_symbol.declaration_depth = scopes.size();

    scopes.back().insert(new_symbol);
    return true;
}

std::shared_ptr<Symbol> SymbolTable::lookup(const std::string& name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto symbol = it->find(name);
        if (symbol) {
            return symbol;
        }
    }
    return nullptr;
}

std::shared_ptr<Symbol> SymbolTable::lookupCurrentScope(const std::string& name) const {
    if (scopes.empty()) return nullptr;
    return scopes.back().find(name);
}

size_t SymbolTable::scopeCount() const {
    return scopes.size();
}