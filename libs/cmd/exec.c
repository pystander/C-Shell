#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "libs/cmd/exec.h"
#include "libs/cmd/parser.h"
#include "libs/signal/signal.h"
#include "libs/utils/array.h"

// Built-in command list
char* builtin[] = {"exit", "timeX", NULL};

void exec(char* cmdline) {
    char** cmd;

    // Command flags
    int is_builtin = 0, is_timeX = 0, is_pipe = 0, is_bg = 0;

    // Check for built-in command
    for (int i = 0; i < length(builtin); i++) {
        if (strncmp(cmdline, builtin[i], strlen(builtin[i])) == 0) {
            is_builtin = 1;
            break;
        }
    }

    // Check for piped command
    if (strchr(cmdline, '|') != NULL) {
        // Error - Two consecutive |
        if (strstr(cmdline, "||") != NULL || strstr(cmdline, "| |") != NULL) {
            printf("csh: should not have two consecutive | without in-between command\n");
            return;
        }

        is_pipe = 1;
    }

    // Check for background command
    if (strchr(cmdline, '&') != NULL) {
        // Error - & appear in the middle
        if (strcmp(strchr(cmdline, '&'), "&\n") != 0) {
            printf("csh: '&' should not appear in the middle of the command line");
            return;
        }

        // Ignore '&' character and set flag
        cmdline[strlen(cmdline)-2] = '\0';
        is_bg = 1;
    }

    // Built-in
    if (is_builtin) {
        // Built-in command "exit"
        if (strncmp(cmdline, "exit", 4) == 0) {
            if (strlen(cmdline) > 5) {
                // Error - With other arguments
                printf("csh: \"exit\" with other arguments!!!\n");
                return;
            }

            // Exit shell
            printf("csh: Terminated\n");
            exit(0);
        }
        // Built-in command "timeX"
        else if (strncmp(cmdline, "timeX", 5) == 0) {
            // Error - Standalone command
            if (strlen(cmdline) < 7) {
                printf("csh: \"timeX\" cannot be a standalone command\n");
                return;
            }

            // Ignore string "timeX" and set flag
            cmdline = cmdline + 6;
            is_timeX = 1;
        }
    }

    // Error - timeX in background
    if (is_timeX && is_bg) {
        printf("csh: \"timeX\" cannot be run in background mode\n");
        return;
    }

    // Piped command
    if (is_pipe) {
        cmd = tokenize(cmdline, "|");
        exec_pipe(cmd, is_timeX, is_bg);
    }
    // External command
    else {
        cmd = tokenize(cmdline, " ");
        exec_normal(cmd, is_timeX, is_bg);
    }

    free(cmd);
}

void exec_pipe(char** cmd, int is_timeX, int is_bg) {
    // Dynamic pipe file descriptor
    int n = length(cmd);
    int pfd[n][2];

    for (int i = 0; i < n; i++) {
        // Tokenize pipe-separated command line
        char** argv = tokenize(cmd[i], " ");

        // Built pipe
        pipe(pfd[i]);

        // Assign process
        pid_t pid = fork();

        if (pid < 0) {
            exit(-1);
        }
        // Child process
		else if (pid == 0) {
            // First pipe: current write end -> stdout
            // Middle pipes: previous read end -> stdin; current write end -> stdout
            // Last pipe: previous read end -> stdin
			if (i != 0) {
                // Set previous pipe read end to stdin
				dup2(pfd[i-1][0], 0);

                // Close previous pipe
				close(pfd[i-1][0]);
				close(pfd[i-1][1]);
			}

			if (i != n-1) {
                // Set current pipe write end to stdout
				dup2(pfd[i][1], 1);

                // Close current pipe
				close(pfd[i][0]);
				close(pfd[i][1]);
			}

            // Wait for SIGUSR1 signal from parent
            signal(SIGUSR1, sigusr1_handler);
            while (sigusr1 != 1);

            // Execute command
            if (execvp(argv[0], argv) == -1) {
                printf("csh: '%s': No such file or directory\n", argv[0]);
                exit(1);
            }
        }
        // Parent process
        else {
            int status;
            struct rusage used;

            // Close all pipes
            if (i != 0) {
                close(pfd[i-1][0]);
                close(pfd[i-1][1]);
            }

            // Send SIGUSR1 signal to child
            kill(pid, SIGUSR1);

            // Background mode
            if (is_bg) {
                // Block SIGCHLD signal until prompt
                sigemptyset(&blkset);
                sigaddset(&blkset, SIGCHLD);
                sigprocmask(SIG_BLOCK, &blkset, NULL);

                // Store pid and cmd in process table
                for (int i = 0; i < STRING_SIZE; i++) {
                    if (p_table[i].pid == 0) {
                        p_table[i].pid = pid;
                        p_table[i].cmd = strdup(cmd[0]);
                        break;
                    }
                }
            }
            // Foreground mode
            else {
                // Wait child to terminate
                wait4(pid, &status, 0, &used);

                // Print signal message if signaled
                if (WIFSIGNALED(status)) {
                    printf("%s\n", strsignal(status));
                }

                // Print rusage if running timeX
                if (is_timeX) {
                    float user_time = used.ru_utime.tv_sec + used.ru_utime.tv_usec / 1000000.0;
                    float sys_time = used.ru_stime.tv_sec + used.ru_stime.tv_usec / 1000000.0;
                    printf("(PID)%d  (CMD)%s    (user)%.3f s  (sys)%.3f s\n", pid, argv[0], user_time, sys_time);
                }

                // Unblock SIGCHLD signal
                sigprocmask(SIG_UNBLOCK, &blkset, NULL);

            }
        }
    }
}

void exec_normal(char** cmd, int is_timeX, int is_bg) {
    // Assign process
    pid_t pid = fork();

    if (pid < 0) {
        exit(-1);
    }
    // Child process
    else if (pid == 0) {
        // Wait for SIGUSR1 signal from parent
        signal(SIGUSR1, sigusr1_handler);
        while (sigusr1 != 1);

        if (execvp(cmd[0], cmd) == -1) {
            printf("csh: '%s': No such file or directory\n", cmd[0]);
            exit(1);
        }
    }
    // Parent process
    else {
        int status;
        struct rusage used;

        // Send SIGUSR1 signal to child
        kill(pid, SIGUSR1);

        // Background mode
        if (is_bg) {
            // Block SIGCHLD until prompt
            sigemptyset(&blkset);
            sigaddset(&blkset, SIGCHLD);
            sigprocmask(SIG_BLOCK, &blkset, NULL);

            // Store pid and cmd in process table
            for (int i = 0; i < STRING_SIZE; i++) {
                if (p_table[i].pid == 0) {
                    p_table[i].pid = pid;
                    p_table[i].cmd = strdup(cmd[0]);
                    break;
                }
            }
        }
        // Foreground mode
        else {
            // Wait child to terminate
            wait4(pid, &status, 0, &used);

            // Print signal message if signaled
            if (WIFSIGNALED(status)) {
                printf("%s\n", strsignal(status));
            }

            // Print rusage if running timeX
            if (is_timeX) {
                float user_time = used.ru_utime.tv_sec + used.ru_utime.tv_usec / 1000000.0;
                float sys_time = used.ru_stime.tv_sec + used.ru_stime.tv_usec / 1000000.0;
                printf("(PID)%d  (CMD)%s    (user)%.3f s  (sys)%.3f s\n", pid, cmd[0], user_time, sys_time);
            }

            // Unblock SIGCHLD signal
            sigprocmask(SIG_UNBLOCK, &blkset, NULL);
        }
    }
}