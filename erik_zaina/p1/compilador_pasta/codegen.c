#include "codegen.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Var varTable[MAX_VARS];
int varCount = 0;
int tempCount = 0;
char tempBuffer[64];
FILE* asmOut;

int addVar(const char* name) {
    for (int i = 0; i < varCount; i++)
        if (strcmp(varTable[i].name, name) == 0)
            return i;
    strncpy(varTable[varCount].name, name, sizeof(varTable[varCount].name)-1);
    varTable[varCount].name[sizeof(varTable[varCount].name)-1] = '\0';
    varTable[varCount].value = 0;
    varTable[varCount].defined = false;
    varCount++;
    printf("Depuração: Símbolo adicionado -> %s\n", name);
    return varCount - 1;
}

void updateVarValue(const char* name, int value) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(varTable[i].name, name) == 0) {
            varTable[i].value = value;
            varTable[i].defined = true;
            printf("Depuração: Atualizado %s com valor %d\n", name, value);
            return;
        }
    }
    addVar(name);
    updateVarValue(name, value);
}

void ensureConstantExists(int value) {
    char constName[64];
    sprintf(constName, "CONST_%d", value);
    for (int i = 0; i < varCount; i++)
        if (strcmp(varTable[i].name, constName) == 0)
            return;
    addVar(constName);
    updateVarValue(constName, value);
}

void newTemp(char* buffer) {
    sprintf(buffer, "TEMP_%d", tempCount++);
    addVar(buffer);
}

static void coletarExpr(ASTNode *n);
static void coletarStmt(Statement *s);

/* percorre a árvore de expressão */
static void coletarExpr(ASTNode *n)
{
    if (!n) return;

    switch (n->type) {
        case AST_NUM:
            ensureConstantExists(n->num);
            break;

        case AST_VAR:
            addVar(n->var);
            break;

        case AST_BINOP: {
            coletarExpr(n->binop.left);
            coletarExpr(n->binop.right);

            if (n->binop.left->type == AST_NUM &&
                n->binop.right->type == AST_NUM) {

                int res = 0;
                switch (n->binop.op) {
                    case '+': res = n->binop.left->num + n->binop.right->num; break;
                    case '-': res = n->binop.left->num - n->binop.right->num; break;
                    case '*': res = n->binop.left->num * n->binop.right->num; break;
                    case '/':
                        if (n->binop.right->num != 0)
                            res = n->binop.left->num / n->binop.right->num;
                        else
                            res = 0;
                        break;
                }
                ensureConstantExists(res);
            }
            break;
        }
    }
}

static void coletarStmt(Statement *s)
{
    for (; s; s = s->next) {
        addVar(s->var);        
        coletarExpr(s->expr);
    }
}

void genExpr(ASTNode* node) {
    if (node->type == AST_NUM) {
        char constName[64];
        sprintf(constName, "CONST_%d", node->num);
        ensureConstantExists(node->num);
        fprintf(asmOut, "LDA %s\n", constName);
        printf("Depuração: Gerado código: LDA %s  [valor %d]\n", constName, node->num);
    }
    else if (node->type == AST_VAR) {
        addVar(node->var);
        fprintf(asmOut, "LDA %s\n", node->var);
        printf("Depuração: Gerado código: LDA %s\n", node->var);
    }
    else if (node->type == AST_BINOP) {
        char op = node->binop.op;
        // + 
        if (op == '+') {
            if (node->binop.left->type == AST_VAR && node->binop.right->type == AST_VAR) {
                fprintf(asmOut, "LDA %s\n", node->binop.left->var);
                fprintf(asmOut, "ADD %s\n", node->binop.right->var);
            } else if (node->binop.left->type == AST_VAR && node->binop.right->type == AST_NUM) {
                char constName[64];
                sprintf(constName, "CONST_%d", node->binop.right->num);
                ensureConstantExists(node->binop.right->num);
                fprintf(asmOut, "LDA %s\n", node->binop.left->var);
                fprintf(asmOut, "ADD %s\n", constName);
            } else if (node->binop.left->type == AST_NUM && node->binop.right->type == AST_VAR) {
                char constName[64];
                sprintf(constName, "CONST_%d", node->binop.left->num);
                ensureConstantExists(node->binop.left->num);
                fprintf(asmOut, "LDA %s\n", constName);
                fprintf(asmOut, "ADD %s\n", node->binop.right->var);
            } else if (node->binop.left->type == AST_NUM && node->binop.right->type == AST_NUM) {
                int result = node->binop.left->num + node->binop.right->num;
                char constName[64];
                sprintf(constName, "CONST_%d", result);
                ensureConstantExists(result);
                fprintf(asmOut, "LDA %s\n", constName);
            } else {
                genExpr(node->binop.left);
                if (node->binop.right->type == AST_VAR) {
                    fprintf(asmOut, "ADD %s\n", node->binop.right->var);
                } else if (node->binop.right->type == AST_NUM) {
                    char constName[64];
                    sprintf(constName, "CONST_%d", node->binop.right->num);
                    ensureConstantExists(node->binop.right->num);
                    fprintf(asmOut, "ADD %s\n", constName);
                } else {
                    newTemp(tempBuffer);
                    fprintf(asmOut, "STA %s\n", tempBuffer);
                    genExpr(node->binop.right);
                    fprintf(asmOut, "ADD %s\n", tempBuffer);
                }
            }
        }
        // -
        else if (op == '-') {
            if (node->binop.left->type == AST_VAR && node->binop.right->type == AST_VAR) {
                fprintf(asmOut, "LDA %s\n", node->binop.left->var);
                fprintf(asmOut, "SUB %s\n", node->binop.right->var);
            } else if (node->binop.left->type == AST_VAR && node->binop.right->type == AST_NUM) {
                char constName[64];
                sprintf(constName, "CONST_%d", node->binop.right->num);
                ensureConstantExists(node->binop.right->num);
                fprintf(asmOut, "LDA %s\n", node->binop.left->var);
                fprintf(asmOut, "SUB %s\n", constName);
            } else if (node->binop.left->type == AST_NUM && node->binop.right->type == AST_VAR) {
                char constName[64];
                sprintf(constName, "CONST_%d", node->binop.left->num);
                ensureConstantExists(node->binop.left->num);
                fprintf(asmOut, "LDA %s\n", constName);
                fprintf(asmOut, "SUB %s\n", node->binop.right->var);
            } else if (node->binop.left->type == AST_NUM && node->binop.right->type == AST_NUM) {
                int result = node->binop.left->num - node->binop.right->num;
                char constName[64];
                sprintf(constName, "CONST_%d", result);
                ensureConstantExists(result);
                fprintf(asmOut, "LDA %s\n", constName);
            } else {
                genExpr(node->binop.left);
                if (node->binop.right->type == AST_VAR) {
                    fprintf(asmOut, "SUB %s\n", node->binop.right->var);
                } else if (node->binop.right->type == AST_NUM) {
                    char constName[64];
                    sprintf(constName, "CONST_%d", node->binop.right->num);
                    ensureConstantExists(node->binop.right->num);
                    fprintf(asmOut, "SUB %s\n", constName);
                } else {
                    newTemp(tempBuffer);
                    fprintf(asmOut, "STA %s\n", tempBuffer);
                    genExpr(node->binop.right);
                    char rightTemp[64];
                    newTemp(rightTemp);
                    fprintf(asmOut, "STA %s\n", rightTemp);
                    fprintf(asmOut, "LDA %s\n", tempBuffer);
                    fprintf(asmOut, "SUB %s\n", rightTemp);
                }
            }
        }
        // *
        else if (op == '*') {
            char tempResult[64];
            newTemp(tempResult);
            fprintf(asmOut, "LDA CONST_0\nSTA %s\n", tempResult);
            if (node->binop.right->type == AST_NUM) {
                int multiplier = node->binop.right->num;
                for (int i = 0; i < multiplier; i++) {
                    genExpr(node->binop.left);
                    fprintf(asmOut, "ADD %s\nSTA %s\n", tempResult, tempResult);
                }
            } else if (node->binop.right->type == AST_VAR) {
                int multiplier = -1;
                for (int i = 0; i < varCount; i++) {
                    if (strcmp(varTable[i].name, node->binop.right->var)==0 && varTable[i].defined) {
                        multiplier = varTable[i].value; break;
                    }
                }
                if (multiplier >= 0) {
                    printf("Depuração: Variável %s tem valor conhecido: %d\n", node->binop.right->var, multiplier);
                    for (int i = 0; i < multiplier; i++) {
                        genExpr(node->binop.left);
                        fprintf(asmOut, "ADD %s\nSTA %s\n", tempResult, tempResult);
                    }
                } else {
                    fprintf(asmOut, "; Multiplicação com variável dinâmica %s não suportada\n", node->binop.right->var);
                    fprintf(asmOut, "LDA CONST_0\n");
                    printf("Aviso: Multiplicação com valor desconhecido de %s\n", node->binop.right->var);
                }
            }
            fprintf(asmOut, "LDA %s\n", tempResult);
        }
        // /
        else if (op == '/') {
            if (node->binop.left->type==AST_NUM && node->binop.right->type==AST_NUM) {
                if (node->binop.right->num==0) {
                    fprintf(stderr,"Erro: Divisão por zero\n");
                    fprintf(asmOut,"; Erro: Divisão por zero\nLDA CONST_0\n");
                    return;
                }
                int result = node->binop.left->num / node->binop.right->num;
                char constName[64];
                sprintf(constName, "CONST_%d", result);
                ensureConstantExists(result);
                fprintf(asmOut,"LDA %s\n",constName);
                printf("Depuração: Divisão %d / %d = %d\n",
                       node->binop.left->num,node->binop.right->num,result);
            } else {
                char quotient[64], dividend[64], divisor[64];
                newTemp(quotient);
                newTemp(dividend);
                newTemp(divisor);
                fprintf(asmOut,"LDA CONST_0\nSTA %s\n",quotient);
                // dividendo
                if (node->binop.left->type==AST_NUM) {
                    char constName[64];
                    sprintf(constName,"CONST_%d",node->binop.left->num);
                    ensureConstantExists(node->binop.left->num);
                    fprintf(asmOut,"LDA %s\n",constName);
                } else if (node->binop.left->type==AST_VAR) {
                    fprintf(asmOut,"LDA %s\n",node->binop.left->var);
                } else genExpr(node->binop.left);
                fprintf(asmOut,"STA %s\n",dividend);
                // divisor
                if (node->binop.right->type==AST_NUM) {
                    if (node->binop.right->num==0) {
                        fprintf(stderr,"Erro: Divisão por zero\n");
                        fprintf(asmOut,"; Erro: Divisão por zero\nLDA CONST_0\n");
                        return;
                    }
                    char constName[64];
                    sprintf(constName,"CONST_%d",node->binop.right->num);
                    ensureConstantExists(node->binop.right->num);
                    fprintf(asmOut,"LDA %s\n",constName);
                } else if (node->binop.right->type==AST_VAR) {
                    fprintf(asmOut,"LDA %s\n",node->binop.right->var);
                } else genExpr(node->binop.right);
                fprintf(asmOut,"STA %s\n",divisor);
                ensureConstantExists(1);
                static int divLabelCounter=0;
                char divLoopLabel[64], divDoneLabel[64];
                sprintf(divLoopLabel,"DIV_LOOP_%d",divLabelCounter);
                sprintf(divDoneLabel,"DIV_DONE_%d",divLabelCounter++);
                fprintf(asmOut,"%s:\nLDA %s\nSUB %s\nJMN %s\nSTA %s\nLDA %s\nADD CONST_1\nSTA %s\nJMP %s\n%s:\n",
                        divLoopLabel, dividend, divisor, divDoneLabel,
                        dividend,quotient,quotient,divLoopLabel,divDoneLabel);
                fprintf(asmOut,"LDA %s\n",quotient);
                printf("Depuração: Gerado código para divisão com quociente em %s\n",quotient);
            }
        }
    }
}

void genAssignment(Statement* stmt) {
    if (stmt->expr && stmt->expr->type==AST_NUM) {
        updateVarValue(stmt->var, stmt->expr->num);
        char constName[64];
        sprintf(constName,"CONST_%d",stmt->expr->num);
        ensureConstantExists(stmt->expr->num);
        fprintf(asmOut,"LDA %s\nSTA %s\n",constName,stmt->var);
        printf("Depuração: Gerado código para atribuição direta: %s = %d\n",
               stmt->var, stmt->expr->num);
    } else {
        genExpr(stmt->expr);
        addVar(stmt->var);
        fprintf(asmOut,"STA %s\n",stmt->var);
        printf("Depuração: Gerado código para atribuição: %s = <expressão>\n",stmt->var);
    }
}

void generateAssembly(void)
{
    coletarStmt(statements);
    coletarExpr(program.resultExpr);
    ensureConstantExists(0);
    ensureConstantExists(1);      /* garante CONST_1 */

    /* -------- Emissão da seção .DATA -------- */
    fprintf(asmOut, ".DATA\n");
    fprintf(asmOut,
        "ONE DB 1\n"
        "CONST_0 DB 0\n"
        "CONST_1 DB 1\n"
        "NEG_1 DB 255\n"
        "RES DB ?\n");

    for (int i = 0; i < varCount; ++i) {
        const char *name  = varTable[i].name;
        int         value = varTable[i].value;

        if (!strcmp(name, "ONE")    || !strcmp(name, "CONST_0") ||
            !strcmp(name, "CONST_1")|| !strcmp(name, "NEG_1")   ||
            !strcmp(name, "RES"))
            continue;   /* já emitidos acima */

        if (!strncmp(name, "TEMP_", 5))
            fprintf(asmOut, "%s DB ?\n", name);
        else if (!strncmp(name, "CONST_", 6))
            fprintf(asmOut, "%s DB %d\n", name, value);
        else if (!varTable[i].defined)
            fprintf(asmOut, "%s DB ?\n", name);        /* variável comum */
        else
            fprintf(asmOut, "%s DB %d\n", name, value);/* variável init */
    }

    /* -------- Emissão da seção .CODE -------- */
    fprintf(asmOut, "\n.CODE\n.ORG 0\n");

    for (Statement *stmt = statements; stmt; stmt = stmt->next) {
        fprintf(asmOut, "; Atribuição: %s = ...\n", stmt->var);
        genAssignment(stmt);
    }

    fprintf(asmOut, "; Expressão de resultado\n");
    genExpr(program.resultExpr);
    fprintf(asmOut, "STA RES\nHLT\n");

    puts("Depuração: Código assembly gerado com sucesso!");
}