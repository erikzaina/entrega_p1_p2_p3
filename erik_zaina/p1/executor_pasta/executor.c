
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <stdbool.h>
 #include <string.h>
 
 #define TAM_MEMORIA 516
 #define TAM_LINHA 16
 #define TAM_CABECALHO 4
 

 static void mostrar_memoria(const uint8_t *memoria, size_t bytes)
 {
     for (size_t i = 0; i < bytes; i += TAM_LINHA) {
         printf("%08zx:", i);
         for (size_t j = 0; j < TAM_LINHA && i + j < bytes; ++j) {
             printf(" %02x", memoria[i + j]);
         }
         putchar('\n');
     }
 }
 

 static bool carregar_programa(const char *caminho, uint8_t *memoria)
 {
     static const uint8_t CABECALHO_ESPERADO[TAM_CABECALHO] = {0x03, 0x4E, 0x44, 0x52};
 
     FILE *arq = fopen(caminho, "rb");
     if (!arq) {
         perror("Falha ao abrir o arquivo");
         return false;
     }
 
     uint8_t cabecalho[TAM_CABECALHO];
     if (fread(cabecalho, 1, TAM_CABECALHO, arq) != TAM_CABECALHO ||
         memcmp(cabecalho, CABECALHO_ESPERADO, TAM_CABECALHO) != 0) {
         fprintf(stderr, "Cabeçalho inválido!\n");
         fclose(arq);
         return false;
     }
 
     fread(memoria + TAM_CABECALHO, 1, TAM_MEMORIA - TAM_CABECALHO, arq);
     fclose(arq);
     return true;
 }
  
 int main(void)
 {
     uint8_t acumulador = 0;
     uint8_t contadorPrograma = 0;
     bool flagZero = false, flagNegativo = false;
 
     uint8_t memoria[TAM_MEMORIA] = {0};
 
     if (!carregar_programa("programa.bin", memoria)) {
         return EXIT_FAILURE;
     }
 
     puts("=== Memória inicial ===");
     mostrar_memoria(memoria, TAM_MEMORIA);
 
     for (;;) {
         uint8_t opcode = memoria[contadorPrograma];
 
         if (opcode == 0xF0) break;
 
         flagZero     = (acumulador == 0);
         flagNegativo = (acumulador & 0x80) != 0;
 
         uint16_t endereco = (memoria[contadorPrograma + 2] * 2u) + TAM_CABECALHO;
 
         switch (opcode) {
             case 0x00: /* NOP */
                 break;
 
             case 0x10: /* STA */
                 memoria[endereco] = acumulador;
                 break;
 
             case 0x20: /* LDA */
                 acumulador = memoria[endereco];
                 break;
 
             case 0x30: { /* ADD */
                 int16_t tmp = (int8_t)acumulador + (int8_t)memoria[endereco];
                 acumulador = (uint8_t)tmp;
                 break;
             }
 
             case 0x31: { /* SUB */
                 int16_t tmp = (int8_t)acumulador - (int8_t)memoria[endereco];
                 acumulador = (uint8_t)tmp;
                 break;
             }
 
             case 0x40: /* OR  */
                 acumulador |= memoria[endereco];
                 break;
 
             case 0x50: /* AND */
                 acumulador &= memoria[endereco];
                 break;
 
             case 0x60: /* NOT */
                 acumulador = ~acumulador;
                 contadorPrograma += 4;
                 continue;               /* evita +4 extra ao final do loop */
 
             case 0x80: /* JMP */
                 contadorPrograma = endereco;
                 continue;
 
             case 0x90: /* JMN */
                 if (flagNegativo) {
                     contadorPrograma = endereco;
                     continue;
                 }
                 break;
 
             case 0xA0: /* JMZ */
                 if (flagZero) {
                     contadorPrograma = endereco;
                     continue;
                 }
                 break;
 
             default:
                 fprintf(stderr, "Opcode desconhecido: 0x%02X\n", opcode);
                 return EXIT_FAILURE;
         }
 
         contadorPrograma += 4;
     }
 
     puts("\n=== Memória final ===");
     mostrar_memoria(memoria, TAM_MEMORIA);
 
     printf("\nAC final: 0x%02X\n", acumulador);
     printf("PC final: 0x%02X\n", contadorPrograma);
 
     /* Procura pela primeira ocorrência do valor do acumulador na memória */
     int resultadoEndereco = -1;
     for (int i = TAM_CABECALHO; i < TAM_MEMORIA; i += 2) {
         if (memoria[i] == acumulador) {
             resultadoEndereco = i;
             break;
         }
     }
 
     if (resultadoEndereco != -1) {
         int8_t resultado = (int8_t)memoria[resultadoEndereco];
         printf("Resultado localizado em 0x%04X: 0x%02X = %d\n",
                resultadoEndereco, memoria[resultadoEndereco], resultado);
     } else {
         puts("Não foi possível encontrar RES na memória.");
     }
 
     return EXIT_SUCCESS;
 }
 