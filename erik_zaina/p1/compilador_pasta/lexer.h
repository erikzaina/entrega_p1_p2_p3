#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    TOKEN_PROGRAMA,
    TOKEN_INICIO,
    TOKEN_FIM,
    TOKEN_RES,
    TOKEN_IDENT,
    TOKEN_NUM,
    TOKEN_EQ,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_DIVIDE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COLON,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
} Token;

#define MAX_TOKENS 1024

extern Token tokens[MAX_TOKENS];
extern int tokenCount;
extern int currentToken;

extern char* source;
extern int sourcePos;

void skipWhitespace();
int isLetter(char c);
int isDigit(char c);
void addToken(TokenType type, const char *lexeme);
void tokenize();
Token* peekToken();
Token* getToken();

#endif // LEXER_H
