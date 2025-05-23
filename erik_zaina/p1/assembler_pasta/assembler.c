 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <stdbool.h>
 #include <string.h>
 #include <ctype.h>
 
 #define HEADER_BYTES 4U
 #define VM_MEMORY_BYTES 512U
 #define DATA_BASE_ADDR 0x0100
 #define RESULT_OFFSET (DATA_BASE_ADDR + 4)
 
 #define OP_NOP 0x00
 #define OP_STA 0x10
 #define OP_LDA 0x20
 #define OP_ADD 0x30
 #define OP_SUB 0x31
 #define OP_OR  0x40
 #define OP_AND 0x50
 #define OP_NOT 0x60
 #define OP_JMP 0x80
 #define OP_JMN 0x90
 #define OP_JMZ 0xA0
 #define OP_HLT 0xF0
 
 #define MAX_SYMBOLS 256  
 
 typedef struct {
     char  name[32];
     int   addr;   
     int   value;   
     bool  defined;  
 } Symbol;
 
 static Symbol g_symbols[MAX_SYMBOLS];
 static int    g_symCount = 0;
 
 enum Section { SEC_NONE, SEC_DATA, SEC_CODE };
 
 static int str_to_int(const char *s)
 /* Converte string decimal ou hexadecimal ("0x...") em inteiro. */
 {
     if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
         return (int)strtol(s, NULL, 16);
     return atoi(s);
 }
 
 static void sanitize_line(char *line)
 /* Remove comentários iniciados por ';' e espaços/nova‑linha ao final. */
 {
     char *sc = strchr(line, ';');
     if (sc) *sc = '\0';
 
     size_t len = strlen(line);
     while (len && isspace((unsigned char)line[len - 1]))
         line[--len] = '\0';
 }
 
 static int sym_find(const char *name)
 {
     for (int i = 0; i < g_symCount; ++i)
         if (strcmp(g_symbols[i].name, name) == 0)
             return i;
     return -1;
 }
 
 static void sym_insert(const char *name, int addr, int val, bool def)
 {
     if (sym_find(name) >= 0) return;          
     if (g_symCount >= MAX_SYMBOLS) {
         fprintf(stderr, "Erro: tabela de símbolos cheia!\n");
         exit(EXIT_FAILURE);
     }
 
     strncpy(g_symbols[g_symCount].name, name, 31);
     g_symbols[g_symCount].name[31] = '\0';
     g_symbols[g_symCount].addr    = addr;
     g_symbols[g_symCount].value   = val;
     g_symbols[g_symCount].defined = def;
     ++g_symCount;
 }
 
 static void coletarSimbolos(FILE *src,
                               uint8_t *mem,
                               int *dataPtr,
                               int *codeStart,
                               int *nextCodeAddr)
 {
     enum Section sec = SEC_NONE;
     char line[256];
 
     while (fgets(line, sizeof(line), src)) {
         sanitize_line(line);
         char *p = line; while (isspace((unsigned char)*p)) ++p;
         if (!*p) continue;
 
         if (strchr(p, ':')) {
             char label[32] = {0};
             sscanf(p, "%31[^:]:", label);
             if (*label && sec == SEC_CODE)
                 sym_insert(label, *nextCodeAddr, 0, true);
             continue;
         }
 
         if (!strncasecmp(p, ".DATA", 5)) { sec = SEC_DATA; continue; }
         if (!strncasecmp(p, ".CODE", 5)) { sec = SEC_CODE; continue; }
 
         if (sec == SEC_DATA) {
             char lbl[32], dir[16], val[32];
             int n = sscanf(p, "%31s %15s %31s", lbl, dir, val);
             if (n >= 2 && !strcasecmp(dir, "DB")) {
                 if (*dataPtr & 1) ++(*dataPtr);           /* alinhamento word */
                 int value = (n == 3 && strcmp(val, "?")) ? str_to_int(val) : 0;
                 bool def  = (n == 3 && strcmp(val, "?"));
                 sym_insert(lbl, *dataPtr, value, def);
                 mem[*dataPtr]     = (uint8_t)value;
                 mem[*dataPtr + 1] = 0x00;
                 *dataPtr += 2;
             }
         }
         else if (sec == SEC_CODE) {
             if (!strncasecmp(p, ".ORG", 4)) {
                 int org; if (sscanf(p, ".ORG %d", &org) == 1) {
                     *codeStart    = HEADER_BYTES + org * 2;
                     *nextCodeAddr = *codeStart;
                 }
                 continue;
             }
             *nextCodeAddr += 4;
         }
     }
 }
 
 static void emitirCodigo(FILE *src,
                              uint8_t *mem,
                              int *dataPtr,
                              int codeStart)
 {
     enum Section sec = SEC_NONE;
     int codeAddr = codeStart;
     char line[256];
 
     rewind(src);
     while (fgets(line, sizeof(line), src)) {
         sanitize_line(line);
         char *p = line; while (isspace((unsigned char)*p)) ++p;
         if (!*p) continue;
 
         if (strchr(p, ':')) continue; 
         if (!strncasecmp(p, ".DATA", 5)) { sec = SEC_DATA; continue; }
         if (!strncasecmp(p, ".CODE", 5)) { sec = SEC_CODE; continue; }
 
         if (sec == SEC_CODE) {
             if (!strncasecmp(p, ".ORG", 4)) {
                 int org; if (sscanf(p, ".ORG %d", &org) == 1)
                     codeAddr = HEADER_BYTES + org * 2;
                 continue;
             }
 
             char mnem[16] = {0}, opnd[32] = {0};
             int tok = sscanf(p, "%15s %31s", mnem, opnd);
             if (tok < 1) continue;
 
             uint8_t opcode = 0x00, opdByte = 0x00;
             #define MAP(X, Y) else if (!strcasecmp(mnem, X)) opcode = Y
             if      (!strcasecmp(mnem, "LDA")) opcode = OP_LDA;
             MAP("ADD", OP_ADD);  MAP("SUB", OP_SUB);  MAP("STA", OP_STA);
             MAP("HLT", OP_HLT);  MAP("NOP", OP_NOP);  MAP("NOT", OP_NOT);
             else if (!strncasecmp(mnem, "JMP", 3)) opcode = OP_JMP;
             else if (!strncasecmp(mnem, "JMN", 3)) opcode = OP_JMN;
             else if (!strncasecmp(mnem, "JMZ", 3)) opcode = OP_JMZ;
             MAP("OR",  OP_OR);   MAP("AND", OP_AND);
             else {
                 fprintf(stderr, "Mnemônico desconhecido: %s\n", mnem);
                 continue;
             }
             #undef MAP
 
             /* Calcula operando se necessário */
             if (opcode != OP_HLT && opcode != OP_NOP && opcode != OP_NOT && tok == 2) {
                 int idx = sym_find(opnd);
                 if (idx < 0) { 
                     if (*dataPtr & 1) ++(*dataPtr);
                     sym_insert(opnd, *dataPtr, 0, false);
                     idx = sym_find(opnd);
                     *dataPtr += 2;
                 }
                 opdByte = (uint8_t)((g_symbols[idx].addr - HEADER_BYTES) / 2);
             }
 
             /* Grava instrução (4 bytes) */
             mem[codeAddr]     = opcode;
             mem[codeAddr + 1] = 0x00;
             mem[codeAddr + 2] = opdByte;
             mem[codeAddr + 3] = 0x00;
             codeAddr += 4;
         }
     }
 }
 
 bool na_assemble(const char *asmPath, const char *binPath)
 {
     FILE *src = fopen(asmPath, "r");
     if (!src) { perror("Erro abrindo .asm"); return false; }
 
     uint8_t memory[VM_MEMORY_BYTES] = {0};
     const uint8_t header[HEADER_BYTES] = {0x03, 0x4E, 0x44, 0x52};
     memcpy(memory, header, HEADER_BYTES);
 
     int dataPtr      = DATA_BASE_ADDR;
     int codeStart    = HEADER_BYTES;   
     int nextCodeAddr = codeStart;
 
     sym_insert("RES", RESULT_OFFSET, 0, false);
 

     coletarSimbolos(src, memory, &dataPtr, &codeStart, &nextCodeAddr);
 
     emitirCodigo(src, memory, &dataPtr, codeStart);
 
     fclose(src);
 

 
     FILE *dst = fopen(binPath, "wb");
     if (!dst) { perror("Erro criando .bin"); return false; }
     fwrite(memory, 1, VM_MEMORY_BYTES, dst);
     fclose(dst);
 
     printf("Montagem concluída: %s -> %s\n", asmPath, binPath);
     return true;
 }
 
 int main(int argc, char **argv)
 {
     const char *input  = (argc > 1) ? argv[1] : "programa.asm";
     const char *output = (argc > 2) ? argv[2] : "programa.bin";
     return na_assemble(input, output) ? EXIT_SUCCESS : EXIT_FAILURE;
 }
 