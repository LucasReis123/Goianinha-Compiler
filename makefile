# Variáveis de compilação
CC = gcc                        # Compilador C
CXX = g++                       # Compilador C++
LEX = flex
YACC = bison
CFLAGS = -Wall -Wextra
LEXFLAGS = 
YACCFLAGS = -d

TARGET = goianinha

# Objetos C (compilados com gcc)
OBJS_C = goianinha.tab.o lex.yy.o main.o ast.o semantic.o codigo.o
# Objetos C++ (compilados com g++)
OBJS_CPP = symbolTable.o
# Lista total para o link final
OBJS_ALL = $(OBJS_C) $(OBJS_CPP)
# ALVO PRINCIPAL: goianinha
all: $(TARGET)


# Regra para linkar todos os objetos e gerar o executável final
# Para gerar goianinha, é compilado tudo em OBJS_ALL.
$(TARGET): $(OBJS_ALL)
    # Usando $(CXX) (g++) para o link final devido ao symbolTable.o
	$(CXX) $(CFLAGS) -o $@ $(OBJS_ALL) -lfl 

# Regra para compilar o Gerador de Código
codigo.o: ./Gera_Codigo/codigo.c ./AST/ast.h ./Tabela_Simbulos/symbolTable.h
	$(CC) $(CFLAGS) -c ./Gera_Codigo/codigo.c

# Regra para compilar a Análise Semântica
semantic.o: ./Analise_Semantica/semantic.c ./AST/ast.h ./Tabela_Simbulos/symbolTable.h
	$(CC) $(CFLAGS) -c ./Analise_Semantica/semantic.c

# Regra para compilar a Tabela de Símbolos (C++)
symbolTable.o: ./Tabela_Simbulos/symbolTable.cpp ./Tabela_Simbulos/symbolTable.h
	$(CXX) -c $(CFLAGS) ./Tabela_Simbulos/symbolTable.cpp

# Regras para compilar os arquivos gerados pelo Flex e Bison
goianinha.tab.o: goianinha.tab.c
	$(CC) $(CFLAGS) -c goianinha.tab.c

# Regra para compilar o arquivo gerado pelo Flex
lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c lex.yy.c

# Regra para compilar a arvore de sintaxe abstrata
ast.o: ./AST/ast.c ./AST/ast.h
	$(CC) $(CFLAGS) -c ./AST/ast.c

# Regra para compilar o arquivo gerado pelo goianinha.y
goianinha.tab.c goianinha.tab.h: ./Analise_Sintatica/goianinha.y
	$(YACC) $(YACCFLAGS) ./Analise_Sintatica/goianinha.y -o goianinha.tab.c

# Regra para compilar o arquivo gerado pelo goianinha.l
lex.yy.c: ./Analise_Lexica/goianinha.l goianinha.tab.h
	$(LEX) $(LEXFLAGS) ./Analise_Lexica/goianinha.l

# Regra de limpeza dos arquivos gerados
clean:
	rm -f $(TARGET) lex.yy.c goianinha.tab.c goianinha.tab.h *.o output.asm

.PHONY: all clean