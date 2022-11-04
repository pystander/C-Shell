#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libs/cmd/parser.h>
#include <libs/utils/array.h>

char* readline() {
    // Allocate memory and read stdin
    char* cmdline = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    fgets(cmdline, BUFFER_SIZE, stdin);

    return cmdline;
}

char** tokenize(char* cmdline, char* delimiter) {
    int i = 0;

    // Tokenize command line with delimiter
    char** tokens = (char**) malloc(sizeof(char*) * STRING_SIZE);
    char* token = strtok(cmdline, delimiter);

    // Parse tokenized string
    while (token != NULL) {
        int n = strlen(token);

        // Set end of line to NULL if space or newline
        if (token[n-1] == ' ' || token[n-1] == '\n') {
            token[n-1] = '\0';
        }

        // Shift left if started with space or newline
        if (token[0] == ' ' || token[0] == '\n') {
            memmove(token, token+1, n);
        }

        // Store tokenized string to array
        tokens[i++] = token;
        token = strtok(NULL, delimiter);
    }

    // End array with NULL
    tokens[i] = NULL;
    return tokens;
}