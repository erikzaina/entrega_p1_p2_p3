#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <stdlib.h>

typedef enum { AST_NUM, AST_VAR, AST_BINOP } ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int num;
        char var[64];
        struct {
            char op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;
    };
} ASTNode;

typedef struct Statement {
    char var[64];
    ASTNode* expr;
    struct Statement* next;
} Statement;

typedef struct {
    char name[64];
    Statement* stmts;
    ASTNode* resultExpr;
} Program;

extern Statement* statements;
extern Statement* lastStmt;
extern Program program;

ASTNode* newNumNode(int value);
ASTNode* newVarNode(const char* name);
ASTNode* newBinOpNode(char op, ASTNode* left, ASTNode* right);

ASTNode* parseExpression();
ASTNode* parseTerm();
ASTNode* parseFactor();
void parseAssignment();
void parseProgram();

void freeAST(ASTNode* node);
void freeStatements(Statement* stmt);

#endif
