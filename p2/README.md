## Erik Pires Zaina

# **Projeto (P2) Apenas o Lexer**

## **Tutorial de Uso**:
```bash

Pré-requisitos: **GCC** e **make** em ambiente Unix-like.

# Compilar todos os componentes
make

# Para remover objetos e o binário:
```bash
make clean
```

## Executar

```bash
./lexer teste.txt
```

## 📋 **Exemplo de saida**:
```
Tipo: 1, Valor: PROGRAMA
Tipo: 28, Valor: \n
Tipo: 9, Valor: int
Tipo: 15, Valor: x
Tipo: 18, Valor: =
Tipo: 16, Valor: 42
Tipo: 0, Valor: <EOF>
```


**Limitações pertinentes** ⚠️
- Se nenhum arquivo for informado ou o arquivo não puder ser aberto, o programa exibe: Uso: ./lexer <arquivo.txt>
- Cada lexema é limitado a 128 caracteres.
- Algumas expressões tem resultado diferente no executor e ao importar o .bin no Neander.

## Notas de Uso

Crie um arquivo `exemplo.txt` com algo como:

```cpp
PROGRAMA "testando":
INICIO
int x = 10
SE (x > 5) {
float y = x * 2.5
}
RES = x
FIM
```
