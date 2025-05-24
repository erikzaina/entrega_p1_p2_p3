#include <stdio.h>
#include <stdlib.h>

#define TAPE_SIZE 30000
#define MAX_PROGRAM 100000
#define MAX_LOOP_DEPTH 100

int run_bf(const char *program) {
    unsigned char tape[TAPE_SIZE] = {0};
    int pointer = 0;
    int ip = 0;
    int loop_stack[MAX_LOOP_DEPTH];
    int stack_ptr = 0;

    while (program[ip]) {
        char instr = program[ip];

        if (instr == '>') {
            pointer++;
            if (pointer >= TAPE_SIZE) {
                fprintf(stderr, "Pointer overflow\n");
                return 1;
            }
        } else if (instr == '<') {
            pointer--;
            if (pointer < 0) {
                fprintf(stderr, "Pointer underflow\n");
                return 1;
            }
        } else if (instr == '+') {
            tape[pointer]++;
        } else if (instr == '-') {
            tape[pointer]--;
        } else if (instr == '.') {
            putchar(tape[pointer]);
        } else if (instr == ',') {
            int input = getchar();
            tape[pointer] = (input == EOF) ? 0 : (unsigned char)input;
        } else if (instr == '[') {
            if (tape[pointer] == 0) {
                int level = 1;
                while (level > 0) {
                    ip++;
                    if (program[ip] == '\0') {
                        fprintf(stderr, "Unmatched '['\n");
                        return 1;
                    }
                    if (program[ip] == '[') level++;
                    if (program[ip] == ']') level--;
                }
            } else {
                if (stack_ptr >= MAX_LOOP_DEPTH) {
                    fprintf(stderr, "Loop stack overflow\n");
                    return 1;
                }
                loop_stack[stack_ptr++] = ip;
            }
        } else if (instr == ']') {
            if (stack_ptr == 0) {
                fprintf(stderr, "Unmatched ']'\n");
                return 1;
            }
            if (tape[pointer] != 0) {
                ip = loop_stack[stack_ptr - 1];
            } else {
                stack_ptr--;
            }
        }
        ip++;
    }

    printf("%d\n", tape[0]);
    return 0;
}

int main() { 
    char program[MAX_PROGRAM];
    int idx = 0;
    int ch;

    while ((ch = getchar()) != EOF && idx < MAX_PROGRAM - 1) {
        if (ch == '>' || ch == '<' || ch == '+' || ch == '-' ||
            ch == '.' || ch == ',' || ch == '[' || ch == ']') {
            program[idx++] = (char)ch;
        }
    }
    program[idx] = '\0';

    return run_bf(program);
}