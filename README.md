# Goianinha Compiler

Este é um repositório do compilador para a linguagem **Goianinha**. Este projeto foi desenvolvido como trabalho final da disciplina de Compiladores.

O compilador lê o código fonte escrito em *Goianinha* (`.g`) e gera código assembly MIPS (`output.asm`).

## Pré-requisitos

Para compilar e executar este projeto, você precisará das seguintes ferramentas instaladas no seu ambiente Linux:

*   **GCC** (GNU Compiler Collection)
*   **G++** (GNU C++ Compiler)
*   **Flex** (Fast Lexical Analyzer Generator)
*   **Bison** (GNU Parser Generator)
*   **Make** (Build automation tool)

Para instalar essas dependências no Ubuntu/Debian, você pode usar:

```bash
sudo apt-get update
sudo apt-get install build-essential flex bison
```

## Compilação

O projeto conta com um `makefile` para facilitar o processo de compilação.

1.  Abra o terminal na raiz do projeto.
2.  Execute o comando `make` para compilar todo o projeto:

```bash
make
```

Se tudo ocorrer bem, um executável chamado `goianinha` será criado na pasta.

Para limpar os arquivos objetos e executáveis gerados, use:

```bash
make clean
```

## Como Executar

Para compilar um programa escrito em Goianinha, utilize o executável gerado passando o arquivo de código fonte como argumento:

```bash
./goianinha <arquivo_fonte.g>
```

**Exemplo:**

```bash
./goianinha teste.g
```

Após a execução bem-sucedida:
1.  A análise sintática e semântica será realizada.
2.  Se não houver erros, um arquivo `output.asm` será gerado contendo o código MIPS correspondente.
3.  Você pode rodar esse código MIPS em um simulador como o SPIM ou MARS.

## Rodando os Testes

O projeto inclui um script automatizado de testes (`teste.sh`) e uma suíte de casos de teste na pasta `TESTES`.

*   **TESTES/Corretos**: Arquivos `.g` que devem compilar com sucesso.
*   **TESTES/Errados**: Arquivos `.g` que contêm erros intencionais para validar o tratamento de erros do compilador.

Para rodar a bateria de testes, execute:

```bash
./teste.sh
```

O script irá iterar sobre os arquivos de teste, executando o compilador e verificando o código de retorno.

## Estrutura do Projeto

*   **Analise_Lexica/**: Contém o arquivo `goianinha.l` (Flex) para reconhecimento de tokens.
*   **Analise_Sintatica/**: Contém o arquivo `goianinha.y` (Bison) para a gramática e parser.
*   **Analise_Semantica/**: Verificações de tipos e escopo.
*   **Tabela_Simbulos/**: Implementação da tabela de símbolos (em C++).
*   **AST/**: Estruturas e funções para manipulação da Árvore Sintática Abstrata.
*   **Gera_Codigo/**: Lógica para geração de código MIPS.
*   **TESTES/**: Casos de teste.
*   **main.c**: Ponto de entrada do compilador.
*   **makefile**: Script de automação de build.

## Exemplo de Código (Goianinha)

Aqui está um exemplo de um programa simples na linguagem:

```c
programa {
    int x;
    escreva "Digite um numero: ";
    leia x;
    
    se (x > 10) entao {
        escreva "Maior que 10";
    } senao {
        escreva "Menor ou igual a 10";
    }
    novalinha;
}
```
