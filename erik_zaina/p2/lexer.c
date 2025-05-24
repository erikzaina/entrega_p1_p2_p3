#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void skip_whitespace(const uint8_t *source, int *position) {
    while (source[*position] == ' ' || source[*position] == '\t') {
        (*position)++;
    }
}

LexToken get_next_token(const uint8_t *source, int *position) {
    skip_whitespace(source, position);
    LexToken token = {0};
    char current = source[*position];

    if (current == '\0') {
        token.type = LEX_EOF;
        strcpy(token.value, "<EOF>");
        return token;
    }

    if (current == '\n') {
        token.type = LEX_NEWLINE;
        strcpy(token.value, "\\n");
        (*position)++;
        return token;
    }

    if ((current >= 'A' && current <= 'Z') || (current >= 'a' && current <= 'z')) {
        int start = *position;
        while ((source[*position] >= 'A' && source[*position] <= 'Z') ||
               (source[*position] >= 'a' && source[*position] <= 'z') ||
               (source[*position] >= '0' && source[*position] <= '9') ||
               source[*position] == '_') {
            (*position)++;
        }
        int length = *position - start;
        strncpy(token.value, (const char*)&source[start], length);
        token.value[length] = '\0';

        if (strcmp(token.value, "PROGRAMA") == 0) token.type = LEX_PROGRAMA;
        else if (strcmp(token.value, "INICIO") == 0) token.type = LEX_INICIO;
        else if (strcmp(token.value, "FIM") == 0) token.type = LEX_FIM;
        else if (strcmp(token.value, "SE") == 0) token.type = LEX_SE;
        else if (strcmp(token.value, "SENAO") == 0) token.type = LEX_SENAO;
        else if (strcmp(token.value, "ENQUANTO") == 0) token.type = LEX_ENQUANTO;
        else if (strcmp(token.value, "FUNCAO") == 0) token.type = LEX_FUNCAO;
        else if (strcmp(token.value, "RES") == 0) token.type = LEX_RES;
        else if (strcmp(token.value, "int") == 0) token.type = LEX_INT;
        else if (strcmp(token.value, "float") == 0) token.type = LEX_FLOAT;
        else if (strcmp(token.value, "char") == 0) token.type = LEX_CHAR;
        else if (strcmp(token.value, "void") == 0) token.type = LEX_VOID;
        else token.type = LEX_ID;

        return token;
    }

    if ((current >= '0' && current <= '9') || (current == '-' && (source[*position + 1] >= '0' && source[*position + 1] <= '9'))) {
        int start = *position;
        int has_dot = 0;
        int has_digit_before_dot = 0;

        if (current == '-') {
            (*position)++;
            if (!(source[*position] >= '0' && source[*position] <= '9')) {
                token.type = LEX_UNKNOWN;
                token.value[0] = '-';
                token.value[1] = '\0';
                (*position)++;
                return token;
            }
        }

        while (source[*position] >= '0' && source[*position] <= '9') {
            has_digit_before_dot = 1;
            (*position)++;
        }

        if (source[*position] == '.') {
            if (!has_digit_before_dot && current != '0') {
                token.type = LEX_UNKNOWN;
                token.value[0] = '.';
                token.value[1] = '\0';
                (*position)++;
                return token;
            }
            has_dot = 1;
            (*position)++;
            while (source[*position] >= '0' && source[*position] <= '9') {
                (*position)++;
            }
        }

        if (source[*position] == 'e' || source[*position] == 'E') {
            has_dot = 1;
            (*position)++;
            if (source[*position] == '-' || source[*position] == '+') {
                (*position)++;
            }
            while (source[*position] >= '0' && source[*position] <= '9') {
                (*position)++;
            }
        }

        int length = *position - start;
        strncpy(token.value, (const char*)&source[start], length);
        token.value[length] = '\0';
        token.type = has_dot ? LEX_FLOAT_NUM : LEX_INT_NUM;
        return token;
    }

    char next = source[*position + 1];
    if (current == '=' && next == '=') {
        token.type = LEX_EQ;
        strcpy(token.value, "==");
        *position += 2;
    } else if (current == '!' && next == '=') {
        token.type = LEX_NEQ;
        strcpy(token.value, "!=");
        *position += 2;
    } else if (current == '<' && next == '=') {
        token.type = LEX_LE;
        strcpy(token.value, "<=");
        *position += 2;
    } else if (current == '>' && next == '=') {
        token.type = LEX_GE;
        strcpy(token.value, ">=");
        *position += 2;
    } else {
        switch (current) {
            case '<': token.type = LEX_LT; strcpy(token.value, "<"); break;
            case '>': token.type = LEX_GT; strcpy(token.value, ">"); break;
            case '+': token.type = LEX_PLUS; strcpy(token.value, "+"); break;
            case '-': token.type = LEX_MINUS; strcpy(token.value, "-"); break;
            case '*': token.type = LEX_MULT; strcpy(token.value, "*"); break;
            case '/': token.type = LEX_DIV; strcpy(token.value, "/"); break;
            case '(': token.type = LEX_LPAREN; strcpy(token.value, "("); break;
            case ')': token.type = LEX_RPAREN; strcpy(token.value, ")"); break;
            case '{': token.type = LEX_LBRACE; strcpy(token.value, "{"); break;
            case '}': token.type = LEX_RBRACE; strcpy(token.value, "}"); break;
            case ',': token.type = LEX_COMMA; strcpy(token.value, ","); break;
            case ':': token.type = LEX_COLON; strcpy(token.value, ":"); break;
            case '=': token.type = LEX_ASSIGN; strcpy(token.value, "="); break;
            case '"': token.type = LEX_QUOTE; strcpy(token.value, "\""); break;
            default:
                token.type = LEX_UNKNOWN;
                token.value[0] = current;
                token.value[1] = '\0';
        }
        (*position)++;
    }

    return token;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo.txt>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *source = malloc(file_size + 1);
    if (!source) {
        fclose(file);
        fprintf(stderr, "Erro de alocação de memória\n");
        return 1;
    }

    fread(source, 1, file_size, file);
    source[file_size] = '\0';
    fclose(file);

    int pos = 0;
    LexToken token;

    do {
        token = get_next_token(source, &pos);
        printf("Tipo: %d, Valor: %s\n", token.type, token.value);
    } while (token.type != LEX_EOF);

    free(source);
    return 0;
}