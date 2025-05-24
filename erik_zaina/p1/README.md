## Erik Pires Zaina

# **Projeto completo (P1)**

## **Tutorial de Uso**:
```bash
# Compilar todos os componentes
make

# Compilar um programa
bin/compilador programa.lpn

# Gerar o binário
bin/assembler programa.asm programa.bin

# Executar
bin/executor programa.bin
```

## 📋 **Exemplo de programa**:
```
PROGRAMA "Teste":
INICIO
a = 10 / 2
b = a + 2 
c = a + b - (2 * 2)
RES = c
FIM
```

**Limitações pertinentes** ⚠️
- Cada parte do sistema deverá ser executada um a um (seguindo a ordem conforme o Tutorial de Uso).
- O nome de cada arquivo (.lpn , .asm , .bin) deve obrigatoriamente ser "programa".
- Algumas expressões tem resultado diferente no executor e ao importar o .bin no Neander.
