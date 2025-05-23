#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Statement *statements = NULL; 
Statement *lastStmt   = NULL; 
Program    program;  


static ASTNode *parseExpr(void);
static ASTNode *parseTermo(void);
static ASTNode *parseFator(void);
static void     registrarAtrib(void);

/* Construtores da árvore de sintaxe */

ASTNode *newNumNode(int valor) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = AST_NUM;
    n->num  = valor;
    return n;
}

ASTNode *newVarNode(const char *ident) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = AST_VAR;
    strncpy(n->var, ident, sizeof(n->var) - 1);
    n->var[sizeof(n->var) - 1] = '\0';
    return n;
}

ASTNode *newBinOpNode(char op, ASTNode *lhs, ASTNode *rhs) {
    ASTNode *n      = malloc(sizeof(ASTNode));
    n->type         = AST_BINOP;
    n->binop.op     = op;
    n->binop.left   = lhs;
    n->binop.right  = rhs;
    return n;
}

/* ------------------------------------------------------------------------- */
/* Implementação do analisador sintático ---------------------------------- */
/* ------------------------------------------------------------------------- */

static ASTNode *parseExpr(void) {
    ASTNode *esq = parseTermo();
    for (Token *tok = peekToken(); tok &&
        (tok->type == TOKEN_PLUS || tok->type == TOKEN_MINUS); tok = peekToken()) {
        getToken(); /* consome operador */
        ASTNode *dir = parseTermo();
        esq = newBinOpNode(tok->lexeme[0], esq, dir);
    }
    return esq;
}


static ASTNode *parseTermo(void) {
    ASTNode *esq = parseFator();
    for (Token *tok = peekToken(); tok &&
        (tok->type == TOKEN_TIMES || tok->type == TOKEN_DIVIDE); tok = peekToken()) {
        getToken();
        ASTNode *dir = parseFator();
        esq = newBinOpNode(tok->lexeme[0], esq, dir);
    }
    return esq;
}


static ASTNode *parseFator(void) {
    Token *tok = peekToken();
    if (!tok) return NULL;

    switch (tok->type) {
        case TOKEN_LPAREN:
            getToken();
            {
                ASTNode *n = parseExpr();
                getToken(); /* TOKEN_RPAREN */
                return n;
            }
        case TOKEN_NUM:
            getToken();
            return newNumNode(atoi(tok->lexeme));
        case TOKEN_IDENT:
            getToken();
            return newVarNode(tok->lexeme);
        default:
            return NULL;
    }
}


static void registrarAtrib(void) {
    Token *identTok = getToken();
    if (!identTok || identTok->type != TOKEN_IDENT) {
        fprintf(stderr, "Era esperado um IDENT na atribuição.\n");
        return;
    }

    char varNome[64];
    strncpy(varNome, identTok->lexeme, sizeof(varNome));

    Token *eqTok = getToken();
    if (!eqTok || eqTok->type != TOKEN_EQ) {
        fprintf(stderr, "Era esperado '=' após o identificador.\n");
        return;
    }

    ASTNode *expressao = parseExpr();

    /* Cria nó de declaração */
    Statement *decl = malloc(sizeof(Statement));
    strncpy(decl->var, varNome, sizeof(decl->var));
    decl->expr = expressao;
    decl->next = NULL;

    if (!statements) {
        statements = lastStmt = decl;
    } else {
        lastStmt->next = decl;
        lastStmt       = decl;
    }

    printf("Depuração: Atribuição capturada -> %s = ...\n", varNome);
}

/* ------------------------------------------------------------------------- */
/* Entrada principal do parser -------------------------------------------- */
/* ------------------------------------------------------------------------- */
void parseProgram(void) {

    Token *tok = getToken();
    if (!tok || tok->type != TOKEN_PROGRAMA) {
        fprintf(stderr, "Erro: palavra‑chave PROGRAMA não encontrada.\n");
        exit(EXIT_FAILURE);
    }

    tok = getToken();
    if (!tok || tok->type != TOKEN_IDENT) {
        fprintf(stderr, "Erro: nome do programa ausente.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(program.name, tok->lexeme, sizeof(program.name));
    printf("Depuração: Nome do programa -> %s\n", program.name);

    tok = getToken();
    if (!tok || tok->type != TOKEN_COLON) {
        fprintf(stderr, "Erro: ':' esperado após o nome do programa.\n");
        exit(EXIT_FAILURE);
    }

    tok = getToken();
    if (!tok || tok->type != TOKEN_INICIO) {
        fprintf(stderr, "Erro: INICIO não encontrado.\n");
        exit(EXIT_FAILURE);
    }
    printf("Depuração: Seção INICIO iniciada\n");

    while ((tok = peekToken()) && tok->type != TOKEN_RES) {
        registrarAtrib();
    }

    tok = getToken();
    if (!tok || tok->type != TOKEN_RES) {
        fprintf(stderr, "Erro: palavra‑chave RES não encontrada.\n");
        exit(EXIT_FAILURE);
    }
    printf("Depuração: Encontrado RES\n");

    tok = getToken();
    if (!tok || tok->type != TOKEN_EQ) {
        fprintf(stderr, "Erro: '=' esperado após RES.\n");
        exit(EXIT_FAILURE);
    }

    program.resultExpr = parseExpr();
    printf("Depuração: Expressão de resultado capturada\n");

    tok = getToken();
    if (!tok || tok->type != TOKEN_FIM) {
        fprintf(stderr, "Erro: FIM esperado.\n");
        exit(EXIT_FAILURE);
    }
    printf("Depuração: Encontrado FIM\n");
}

void freeAST(ASTNode *n) {
    if (!n) return;
    if (n->type == AST_BINOP) {
        freeAST(n->binop.left);
        freeAST(n->binop.right);
    }
    free(n);
}

void freeStatements(Statement *s) {
    while (s) {
        Statement *prox = s->next;
        freeAST(s->expr);
        free(s);
        s = prox;
    }
}
