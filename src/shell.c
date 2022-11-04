#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs/cmd/exec.h"
#include "libs/cmd/parser.h"
#include "libs/signal/signal.h"

int main() {
    // Create process table
    p_table = calloc(STRING_SIZE, sizeof(process));

    while (1) {
        // Reset signal handlers
        install_handler();

        // Shell prompt
        printf("$$ csh ## ");

        fflush(stdin);
        char* cmdline = readline();

        // Handle non-empty command line
        if (cmdline[0] != '\n' && cmdline[1] != '\0') {
            exec(cmdline);
        }

        // Release memory
        free(cmdline);
    }

    return 0;
}