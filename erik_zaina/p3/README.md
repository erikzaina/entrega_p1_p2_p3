## Erik Pires Zaina

# **Brainfuck Compilador & Executor**

## 🛠️ Pré-requisitos

- GCC (ou qualquer compilador C compatível com `-std=c99`)  
- `make` em ambiente Unix-like (Linux, macOS, WSL…)

## **Tutorial de Uso**:
```bash

# compila tudo
make

# roda com input de teste
cat test_input.txt | ./compilador | ./executor

# Saída esperada: 
# AÇÃO = 5*3 + 1*4 + 5 - 20


# limpa os arquivos compilados gerados
make clean

```

## 📋 **Exemplo de programa**:
```
AÇÃO = 5*3 + 1*4 + 5 - 20 

```
## 📋 **Limitações Pertinentes**:
```
Divisão não funciona como deveria.
```