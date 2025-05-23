#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>

Token tokens[MAX_TOKENS];
int tokenCount = 0;
int currentToken = 0;
char* source = NULL;
int sourcePos = 0;

void skipWhitespace() {
    while (source[sourcePos] == ' ' || source[sourcePos] == '\t')
        sourcePos++;
}

int isLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int isDigit(char c) {
    return (c >= '0' && c <= '9');
}

void addToken(TokenType type, const char *lexeme) {
    if (tokenCount < MAX_TOKENS) {
        tokens[tokenCount].type = type;
        strncpy(tokens[tokenCount].lexeme, lexeme, sizeof(tokens[tokenCount].lexeme)-1);
        tokens[tokenCount].lexeme[sizeof(tokens[tokenCount].lexeme)-1] = '\0';
        tokenCount++;
    }
}

void tokenize() {
    tokenCount = 0;
    currentToken = 0;
    sourcePos = 0;
    while (source[sourcePos] != '\0') {
        if (source[sourcePos] == ' ' || source[sourcePos] == '\t') {
            skipWhitespace();
            continue;
        }
        if (source[sourcePos] == '\n' || source[sourcePos] == '\r') {
            sourcePos++;
            continue;
        }
        if (strncmp(&source[sourcePos], "PROGRAMA", 8) == 0 && !isLetter(source[sourcePos+8])) {
            addToken(TOKEN_PROGRAMA, "PROGRAMA");
            sourcePos += 8;
            continue;
        }
        if (strncmp(&source[sourcePos], "INICIO", 6) == 0 && !isLetter(source[sourcePos+6])) {
            addToken(TOKEN_INICIO, "INICIO");
            sourcePos += 6;
            continue;
        }
        if (strncmp(&source[sourcePos], "FIM", 3) == 0 && !isLetter(source[sourcePos+3])) {
            addToken(TOKEN_FIM, "FIM");
            sourcePos += 3;
            continue;
        }
        if (strncmp(&source[sourcePos], "RES", 3) == 0 && !isLetter(source[sourcePos+3])) {
            addToken(TOKEN_RES, "RES");
            sourcePos += 3;
            continue;
        }
        char c = source[sourcePos];
        switch (c) {
            case '=': addToken(TOKEN_EQ, "="); sourcePos++; continue;
            case '+': addToken(TOKEN_PLUS, "+"); sourcePos++; continue;
            case '-': addToken(TOKEN_MINUS, "-"); sourcePos++; continue;
            case '*': addToken(TOKEN_TIMES, "*"); sourcePos++; continue;
            case '/': addToken(TOKEN_DIVIDE, "/"); sourcePos++; continue;
            case '(': addToken(TOKEN_LPAREN, "("); sourcePos++; continue;
            case ')': addToken(TOKEN_RPAREN, ")"); sourcePos++; continue;
            case ':': addToken(TOKEN_COLON, ":"); sourcePos++; continue;
            case '\"': {
                sourcePos++;
                int start = sourcePos;
                while (source[sourcePos] != '\"' && source[sourcePos] != '\0') sourcePos++;
                int len = sourcePos - start;
                char ident[64];
                strncpy(ident, &source[start], len);
                ident[len] = '\0';
                addToken(TOKEN_IDENT, ident);
                if (source[sourcePos] == '\"') sourcePos++;
                continue;
            }
        }
        if (isLetter(source[sourcePos])) {
            int start = sourcePos;
            while (isLetter(source[sourcePos]) || isDigit(source[sourcePos]) || source[sourcePos]=='_')
                sourcePos++;
            int len = sourcePos - start;
            char ident[64];
            strncpy(ident, &source[start], len);
            ident[len] = '\0';
            addToken(TOKEN_IDENT, ident);
            continue;
        }
        if (isDigit(source[sourcePos])) {
            int start = sourcePos;
            while (isDigit(source[sourcePos])) sourcePos++;
            int len = sourcePos - start;
            char num[64];
            strncpy(num, &source[start], len);
            num[len] = '\0';
            addToken(TOKEN_NUM, num);
            continue;
        }
        sourcePos++;
    }
    addToken(TOKEN_EOF, "EOF");

    printf("---- Tokens Gerados (%d tokens) ----\\n", tokenCount);
    for (int i = 0; i < tokenCount; i++) {
        printf("[%d] %d - '%s'\\n", i, tokens[i].type, tokens[i].lexeme);
    }
    printf("---- Fim dos Tokens ----\\n");
}

Token* peekToken() {
    if (currentToken < tokenCount)
        return &tokens[currentToken];
    return NULL;
}

Token* getToken() {
    if (currentToken < tokenCount)
        return &tokens[currentToken++];
    return NULL;
}
