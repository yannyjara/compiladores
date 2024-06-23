#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_LENGTH 100

typedef enum {
    IZQ_LLAVE,    // {
    DER_LLAVE,    // }
    ABRE_CORCHETE, // [
    CIERRA_CORCHETE, // ]
    COMA,       // ,
    DOS_PUNTOS, // :
    CADENA,
    NUMERO,
    BOOL_TRUE,
    BOOL_FALSE,
    PR_NULL,
    EOF_TOKEN
} TipoToken;

char *tokenNames[] = {
    "IZQ_LLAVE",
    "DER_LLAVE",
    "ABRE_CORCHETE",
    "CIERRA_CORCHETE",
    "COMA",
    "DOS_PUNTOS",
    "CADENA",
    "NUMERO",
    "BOOL_TRUE",
    "BOOL_FALSE",
    "PR_NULL",
    "EOF"
};

typedef struct {
    TipoToken type;
    char lexeme[MAX_LENGTH];
} Token;

int obtenerSiguiente(FILE *file, Token *token) {
    int c;

    while ((c = fgetc(file)) != EOF) {
        if (isspace(c)) {
            continue; // Para ignorar espacios en blanco
        } else if (c == '{') {
            token->type = IZQ_LLAVE;
        } else if (c == '}') {
            token->type = DER_LLAVE;
        } else if (c == '[') {
            token->type = ABRE_CORCHETE;
        } else if (c == ']') {
            token->type = CIERRA_CORCHETE;
        } else if (c == ',') {
            token->type = COMA;
        } else if (c == ':') {
            token->type = DOS_PUNTOS;
        } else if (c == '"') {
            token->type = CADENA;
            int i = 0;
            while ((c = fgetc(file)) != '"' && c != EOF) {
                token->lexeme[i++] = c;
            }
            token->lexeme[i] = '\0';
            return 1;
        } else if (isdigit(c) || c == '-' || c == '.') {
            token->type = NUMERO;
            int i = 0;
            token->lexeme[i++] = c;
            while ((isdigit(c = fgetc(file)) || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-') && i < MAX_LENGTH) {
                token->lexeme[i++] = c;
            }
            token->lexeme[i] = '\0';
            ungetc(c, file); // Devolver último caracter no numérico al buffer
            return 1;
        } else if (isalpha(c)) {
            char buffer[MAX_LENGTH];
            int i = 0;
            buffer[i++] = c;
            while (isalpha(c = fgetc(file)) && i < MAX_LENGTH) {
                buffer[i++] = c;
            }
            buffer[i] = '\0';
            ungetc(c, file); // Devolver último caracter no alfabético al buffer

            if (strcmp(buffer, "true") == 0) {
                token->type = BOOL_TRUE;
            } else if (strcmp(buffer, "false") == 0) {
                token->type = BOOL_FALSE;
            } else {
                printf("Error léxico: Token no reconocido: %s\n", buffer);
                continue;
            }
            return 1;
        } else {
            printf("Error léxico: Caracter desconocido: %c\n", c);
            continue; // Continuar con la siguiente línea
        }
        return 1;
    }
    return 0; // Fin del archivo
}

int main() {
    char filename[MAX_LENGTH];
    printf("Ingrese el nombre del archivo a analizar: \n");
    printf("Obs: el archivo debe encontrarse en la misma carpeta. \n");
    scanf("%s", filename);

    FILE *file = fopen(filename, "r");
    Token token;

    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    while (obtenerSiguiente(file, &token)) {
        printf("%s", tokenNames[token.type]);
        if (token.type == CADENA || token.type == NUMERO) {
            printf(" %s", token.lexeme);
        }
        printf("\n");
    }

    fclose(file);

    return 0;
}
