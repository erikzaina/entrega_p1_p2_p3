#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

typedef enum {
    // Palavras-chave
    LEX_PROGRAMA, LEX_INICIO, LEX_FIM,
    LEX_SE, LEX_SENAO, LEX_ENQUANTO,
    LEX_FUNCAO, LEX_RES,
    LEX_INT, LEX_FLOAT, LEX_CHAR, LEX_VOID,

    // Operadores relacionais
    LEX_EQ, LEX_NEQ, LEX_LT, LEX_GT, LEX_LE, LEX_GE,

    // Operadores aritméticos
    LEX_PLUS, LEX_MINUS, LEX_MULT, LEX_DIV,

    // Símbolos
    LEX_LPAREN, LEX_RPAREN, LEX_LBRACE, LEX_RBRACE,
    LEX_COMMA, LEX_COLON, LEX_ASSIGN, LEX_QUOTE, LEX_NEWLINE,

    // Outros
    LEX_ID, LEX_INT_NUM, LEX_FLOAT_NUM,
    LEX_EOF, LEX_UNKNOWN
} LexType;

typedef struct {
    LexType type;
    char value[128];
} LexToken;

LexToken get_next_token(const uint8_t *source, int *position);
void skip_whitespace(const uint8_t *source, int *position);

#endif