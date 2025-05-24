#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024
#define MAX_TERMS 100
#define MAX_LOOP 1000

typedef struct {
    int sign;
    int is_mul;
    int is_div;
    int left;
    int right;
} Term;

void trim_whitespace(char *str) {
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    if (start != str) memmove(str, start, strlen(start) + 1);
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) *end-- = '\0';
}

int parse_term(const char *term_str, Term *term, int in_sign) {
    char buf[MAX_LINE];
    strncpy(buf, term_str, MAX_LINE - 1);
    buf[MAX_LINE - 1] = '\0';

    char *s = buf;
    char *end;
    long long acc = strtoll(s, &end, 10); // lê primeiro número
    int op_cnt = 0;
    char last_op = 0;

    while (*end) {
        while (isspace((unsigned char)*end)) end++;
        if (*end != '*' && *end != '/') break; // ← para aqui quando achar + ou -
        last_op = *end;
        op_cnt++;  // garante * / antes de + -
        char op = *end++;
        while (isspace((unsigned char)*end)) end++;
        long long num = strtoll(end, &end, 10);
        if (op == '*')
            acc *= num;
        else {
            if (num == 0) num = 1;
            acc /= num;
        }
    }

    if (op_cnt == 1) {
        const char *mul_pos = strchr(term_str, '*');
        const char *div_pos = strchr(term_str, '/');
        const char *op_pos = mul_pos ? mul_pos : div_pos;
        char left[MAX_LINE], right[MAX_LINE];
        strncpy(left, term_str, op_pos - term_str);
        left[op_pos - term_str] = '\0';
        strcpy(right, op_pos + 1);
        trim_whitespace(left);
        trim_whitespace(right);
        term->left = atoi(left);
        term->right = atoi(right);
        term->is_mul = last_op == '*';
        term->is_div = last_op == '/';
        term->sign = in_sign;
    } else {
        long long signed_val = acc * in_sign;
        term->is_mul = term->is_div = 0;
        term->sign = signed_val < 0 ? -1 : 1;
        if (signed_val < 0) signed_val = -signed_val;
        if (signed_val > MAX_LOOP) signed_val = MAX_LOOP;
        term->left = (int)signed_val;
        term->right = 0;
    }
    return 1;
}

void generate_varname_bf(const char *varname) {
    size_t len = strlen(varname);
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)varname[i];
        printf(">");
        int loop_limit = (c > MAX_LOOP) ? MAX_LOOP : c;
        for (int j = 0; j < loop_limit; j++) printf("+");
        printf(".");
    }
    printf(">");
    for (int i = 0; i < 61; i++) printf("+");
    printf(".");
    for (size_t i = 0; i < len + 1; i++) printf("<");
    printf("[-]");
}

void generate_mul_bf(int a, int b, int sign) {
    if (a < 0) { a = -a; sign = -sign; }
    if (b < 0) { b = -b; sign = -sign; }
    if (a > MAX_LOOP) a = MAX_LOOP;
    if (b > MAX_LOOP) b = MAX_LOOP;

    printf(">[-]");
    for (int i = 0; i < b; i++) printf("+");
    printf("[<");
    for (int i = 0; i < a; i++) printf("%c", sign == 1 ? '+' : '-');
    printf("> -]");
    printf("<");
}

void generate_div_bf(int a, int b, int sign) {
    if (a < 0) { a = -a; sign = -sign; }
    if (b <= 0) b = 1;
    if (a > MAX_LOOP) a = MAX_LOOP;
    if (b > MAX_LOOP) b = MAX_LOOP;

    printf(">>[-]");
    for (int i = 0; i < a; i++) printf("+");
    printf(">[-]");
    printf(">[-]");
    printf("<<[");
    printf(">[-");
    for (int i = 0; i < b; i++) printf("-");
    printf(">>+<<]");
    printf(">>[-]");
    printf(">>]");
    printf(">>");
    if (sign == 1) printf("[>+<-]");
    else printf("[>-<-]");
    printf("<<");
}

void generate_val_bf(int val, int sign) {
    if (val < 0) { val = -val; sign = -sign; }
    if (val > MAX_LOOP) val = MAX_LOOP;
    for (int i = 0; i < val; i++) printf("%c", sign == 1 ? '+' : '-');
}

int main() {
    char line[MAX_LINE];
    if (!fgets(line, sizeof(line), stdin)) return 0;
    char *nl = strchr(line, '\n');
    if (nl) *nl = '\0';

    char varname[MAX_LINE];
    size_t varlen = 0;
    while (line[varlen] != '=' && line[varlen] != '\0') varlen++;
    if (varlen >= MAX_LINE) varlen = MAX_LINE - 1;
    strncpy(varname, line, varlen);
    varname[varlen] = '\0';
    trim_whitespace(varname);

    char *eq = strchr(line, '=');
    if (!eq) return 1;
    char expr[MAX_LINE];
    strcpy(expr, eq + 1);
    trim_whitespace(expr);

    Term terms[MAX_TERMS];
    int term_count = 0;
    char *p = expr;
    int current_sign = 1;

while (*p && term_count < MAX_TERMS) {
    while (*p == ' ') p++;
    if (*p == '+' || *p == '-') {           
        current_sign = (*p == '+') ? 1 : -1;
        p++;
        continue;
    }
     char *term_start = p;
    while (*p && *p != '+' && *p != '-') p++;
        char term_str[MAX_LINE];
        size_t len = p - term_start;
        if (len >= MAX_LINE) len = MAX_LINE - 1;
        strncpy(term_str, term_start, len);
        term_str[len] = '\0';
        trim_whitespace(term_str);
        if (strlen(term_str) == 0) continue;
        parse_term(term_str, &terms[term_count], current_sign);
        term_count++;
    }

    generate_varname_bf(varname);

    for (int i = 0; i < term_count; i++) {
        if (terms[i].is_mul)
            generate_mul_bf(terms[i].left, terms[i].right, terms[i].sign);
        else if (terms[i].is_div)
            generate_div_bf(terms[i].left, terms[i].right, terms[i].sign);
        else
            generate_val_bf(terms[i].left, terms[i].sign);
    }

    return 0;
}
