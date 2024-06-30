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

// Variables globales
Token currentToken;
FILE *file;
FILE *outputFile;

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
            } else if (strcmp(buffer, "null") == 0) {
                token->type = PR_NULL;
            } else {
                printf("Error lexico: Token no reconocido: %s\n", buffer);
                fprintf(outputFile, "<error>Error lexico: Token no reconocido: %s</error>\n", buffer);
                continue;
            }
            return 1;
        } else {
            printf("Error lexico: Caracter desconocido: %c\n", c);
            fprintf(outputFile, "<error>Error lexico: Caracter desconocido: %c</error>\n", c);
            continue; // Continuar con la siguiente línea
        }
        return 1;
    }
    return 0; // Fin del archivo
}

void obtenerSiguienteToken() {
    if (!obtenerSiguiente(file, &currentToken)) {
        currentToken.type = EOF_TOKEN;
    }
}

void error(char *mensaje) {
    printf("Error sintactico: %s en el token %s\n", mensaje, tokenNames[currentToken.type]);
    fprintf(outputFile, "<error>Error sintactico: %s en el token %s</error>\n", mensaje, tokenNames[currentToken.type]);
}

void sincronizar() {
    while (currentToken.type != COMA && currentToken.type != CIERRA_CORCHETE && currentToken.type != DER_LLAVE && currentToken.type != EOF_TOKEN) {
        obtenerSiguienteToken();
    }
    if (currentToken.type == COMA || currentToken.type == CIERRA_CORCHETE || currentToken.type == DER_LLAVE) {
        obtenerSiguienteToken();
    }
}

void verificar_valor();

void verificar_objeto() {
    if (currentToken.type == IZQ_LLAVE) {
        obtenerSiguienteToken();
        if (currentToken.type == DER_LLAVE) {
            obtenerSiguienteToken();
            return;
        }
        while (currentToken.type == CADENA) {
            char key[MAX_LENGTH];
            strcpy(key, currentToken.lexeme);
            obtenerSiguienteToken();
            if (currentToken.type != DOS_PUNTOS) {
                error("Se esperaba ':'");
                // Intentar sincronizar con el siguiente valor válido
                sincronizar();
                return;
            }
            obtenerSiguienteToken();
            fprintf(outputFile, "<%s>", key);
            verificar_valor();
            fprintf(outputFile, "</%s>\n", key);
            if (currentToken.type == COMA) {
                obtenerSiguienteToken();
                continue;
            } else if (currentToken.type == DER_LLAVE) {
                obtenerSiguienteToken();
                return;
            } else {
                error("Se esperaba ',' o '}'");
                sincronizar();
                return;
            }
        }
        if (currentToken.type != DER_LLAVE) {
            error("Se esperaba '}'");
            sincronizar();
        }else {
            obtenerSiguienteToken();
        }
    } else {
        error("Se esperaba '{'");
        sincronizar();
    }
}


void verificar_array() {
    if (currentToken.type == ABRE_CORCHETE) {
        obtenerSiguienteToken();
        if (currentToken.type == CIERRA_CORCHETE) {
            obtenerSiguienteToken();
            return;
        }
        while (1) {
            fprintf(outputFile, "<item>\n");
            verificar_valor();
            fprintf(outputFile, "</item>\n");

            if (currentToken.type == COMA) {
                obtenerSiguienteToken();
                continue;
            } else if (currentToken.type == CIERRA_CORCHETE) {
                obtenerSiguienteToken();
                return;
            } else {
                error("Se esperaba ',' o ']'");
                sincronizar();
                return;
            }
        }
    } else {
        error("Se esperaba '['");
        sincronizar();
    }
}


void verificar_valor() {
    switch (currentToken.type) {
        case CADENA:
            fprintf(outputFile, "%s", currentToken.lexeme);
            obtenerSiguienteToken();
            break;
        case NUMERO:
            fprintf(outputFile, "%s", currentToken.lexeme);
            obtenerSiguienteToken();
            break;
        case BOOL_TRUE:
            fprintf(outputFile, "true");
            obtenerSiguienteToken();
            break;
        case BOOL_FALSE:
            fprintf(outputFile, "false");
            obtenerSiguienteToken();
            break;
        case PR_NULL:
            obtenerSiguienteToken();
            break;
        case IZQ_LLAVE:
            verificar_objeto();
            break;
        case ABRE_CORCHETE:
            verificar_array();
            break;
        default:
            error("Valor no valido");
            sincronizar();
    }
}


void verificar_json() {
    obtenerSiguienteToken();
    verificar_valor();
    if (currentToken.type != EOF_TOKEN) {
        error("Final inesperado del archivo");
    } else {
        printf("El archivo JSON es sintacticamente correcto.\n");
    }
}

int main() {
    char filename[MAX_LENGTH];
    char outputFilename[MAX_LENGTH];
    printf("Ingrese el nombre del archivo a analizar: \n");
    printf("Obs: el archivo debe encontrarse en la misma carpeta. \n");
    scanf("%s", filename);

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    printf("Ingrese el nombre del archivo XML de salida: \n");
    scanf("%s", outputFilename);
    outputFile = fopen(outputFilename, "w");
    if (outputFile == NULL) {
        perror("Error al abrir el archivo XML de salida");
        fclose(file);
        return 1;
    }

    verificar_json();

    fclose(file);
    fclose(outputFile);

    return 0;
}
