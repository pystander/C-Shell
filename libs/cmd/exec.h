#ifndef EXEC_H
#define EXEC_H

// Process
struct process {
    pid_t pid;
    char* cmd;
} process;

// Process table
struct process* p_table;

// Handle command line
void exec(char* cmdline);

// Parse piped command line and execute
void exec_pipe(char** cmd, int is_timeX, int is_bg);

// Parse external command line and execute
void exec_normal(char** cmd, int is_timeX, int is_bg);

#endif