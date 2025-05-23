
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h" 
#include "codegen.h" 

extern char *source;

extern FILE *asmOut;

extern Statement *statements;
extern Statement *lastStmt;
extern int varCount;
extern int tempCount;
extern Program program;


static char *carregarFonte(const char *caminho);
static char *criarNomeSaida(const char *caminhoFonte);
static void finalizar(void);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.lpn>\n", argv[0]);
        return EXIT_FAILURE;
    }

    source = carregarFonte(argv[1]);
    if (!source) {
        return EXIT_FAILURE;
    }

    char *nomeSaida = criarNomeSaida(argv[1]);
    if (!nomeSaida) {
        finalizar();
        return EXIT_FAILURE;
    }

    asmOut = fopen(nomeSaida, "w");
    if (!asmOut) {
        perror("Falha ao criar o arquivo de saída .asm");
        free(nomeSaida);
        finalizar();
        return EXIT_FAILURE;
    }
    free(nomeSaida);

    statements = lastStmt = NULL;
    varCount   = 0;
    tempCount  = 0;

    tokenize();
    parseProgram(); 
    generateAssembly();

    freeStatements(statements);
    freeAST(program.resultExpr);

    finalizar();
    printf("Compilação concluída com sucesso.\n");
    return EXIT_SUCCESS;
}



static char *carregarFonte(const char *caminho) {
    FILE *fp = fopen(caminho, "r");
    if (!fp) {
        perror("Falha ao abrir o arquivo‑fonte");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long tamanho = ftell(fp);
    rewind(fp);

    char *buffer = (char *)malloc((size_t)tamanho + 1);
    if (!buffer) {
        perror("Falha ao alocar memória para o código‑fonte");
        fclose(fp);
        return NULL;
    }

    size_t lidos = fread(buffer, 1, (size_t)tamanho, fp);
    buffer[lidos] = '\0';
    fclose(fp);

    return buffer;
}


static char *criarNomeSaida(const char *caminhoFonte) {
    size_t len = strlen(caminhoFonte);
    char *nome = (char *)malloc(len + 5);
    if (!nome) {
        perror("Falha ao alocar memória para o nome de saída");
        return NULL;
    }

    strcpy(nome, caminhoFonte);
    char *ext = strrchr(nome, '.');
    if (ext) *ext = '\0';
    strcat(nome, ".asm");
    return nome;
}

static void finalizar(void) {
    if (source) {
        free(source);
        source = NULL;
    }
    if (asmOut) {
        fclose(asmOut);
        asmOut = NULL;
    }
}
