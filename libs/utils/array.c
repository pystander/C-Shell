#include <stdio.h>
#include <string.h>

#include "libs/utils/array.h"

int length(char** array) {
    int i = 0;

    // Count string
    while (array[i] != NULL) {
        i++;
    }

    return i;
}

void print_array(char** array) {
    int i = 0;

    // Print string
    while (array[i] != NULL) {
        printf("Argument %d: %s\n", i, array[i]);
        i++;
    }
}