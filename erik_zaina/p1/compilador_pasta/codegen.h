#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdbool.h>
#include "parser.h"

typedef struct {
char name[64];
int value;
bool defined;
} Var;

#define MAX_VARS 256

extern Var varTable[MAX_VARS];
extern int tempCount;
extern int varCount;
int addVar(const char* name);
void updateVarValue(const char* name, int value);
void ensureConstantExists(int value);
void newTemp(char* buffer);

extern FILE* asmOut;
void genExpr(ASTNode* node);
void genAssignment(Statement* stmt);
void generateAssembly();

#endif 
