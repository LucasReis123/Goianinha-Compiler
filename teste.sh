#!/bin/bash

# Definindo o executável do compilador
EXECUTABLE="./goianinha"

# Definindo o diretório base dos testes
TEST_DIR="TESTES"

clear

# Lista todos os arquivos .g no subdiretório Corretos
for file in "$TEST_DIR/Corretos"/*.g; do
    if [ -f "$file" ]; then
        echo -e "\nTestando: $file"
        
        # Executando o compilador, passando o arquivo como argumento e captura a saída e o código de saída
        $EXECUTABLE "$file"
        EXIT_CODE=$?
        
        # Verifica se o código de saída indica sucesso (geralmente 0)
        if [ $EXIT_CODE -eq 0 ]; then
            echo "Sucesso"
        else
            echo "ERRO: O compilador retornou código de erro $EXIT_CODE para um arquivo que deveria ser correto."
        fi
        
        # Pedindo para o usuário apertar uma tecla para continuar
        read -p "Pressione [ENTER] para o continuar" -n 1 -r
        clear
        echo
    fi
done

# Lista todos os arquivos .g no subdiretório Errados
for file in "$TEST_DIR/Errados"/*.g; do
    if [ -f "$file" ]; then
        echo -e "\nTestando: $file\n"
        
        # Executando o compilador, passando o arquivo como argumento e captura a saída e o código de saída
        $EXECUTABLE "$file"
        EXIT_CODE=$?
        
        # Pede para o usuário apertar uma tecla para continuar
        read -p "Pressione [ENTER] para continuar" -n 1 -r
        clear
        echo
    fi
done

echo -e "\n## Fim dos Testes."