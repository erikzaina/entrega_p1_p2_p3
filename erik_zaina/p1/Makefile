CC = gcc
CFLAGS = -Wall -Wextra -O2 -m64

# Diretórios
ASSEMBLER_DIR = assembler_pasta
COMPILADOR_DIR = compilador_pasta
EXECUTOR_DIR = executor_pasta
BIN_DIR = bin

# Arquivos fonte
COMPILADOR_SRCS = $(COMPILADOR_DIR)/main.c $(COMPILADOR_DIR)/lexer.c $(COMPILADOR_DIR)/parser.c $(COMPILADOR_DIR)/codegen.c
ASSEMBLER_SRCS = $(ASSEMBLER_DIR)/assembler.c
EXECUTOR_SRCS = $(EXECUTOR_DIR)/executor.c

# Executáveis (na pasta bin)
COMPILADOR = $(BIN_DIR)/compilador
ASSEMBLER = $(BIN_DIR)/assembler
EXECUTOR = $(BIN_DIR)/executor

# Targets padrão
all: $(BIN_DIR) $(COMPILADOR) $(ASSEMBLER) $(EXECUTOR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(COMPILADOR): $(COMPILADOR_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

$(ASSEMBLER): $(ASSEMBLER_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

$(EXECUTOR): $(EXECUTOR_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# Limpeza
clean:
	rm -f $(COMPILADOR) $(ASSEMBLER) $(EXECUTOR)

.PHONY: all clean
