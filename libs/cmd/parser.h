#ifndef PARSER_H
#define PARSER_H

#define BUFFER_SIZE 1024
#define STRING_SIZE 30
#define DELIMITER " \n\t"

// Prompt user input and return string
char* readline();

// Split command line into array with delimiter
char** tokenize(char* cmdline, char* delimiter);

# endif