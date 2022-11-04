#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "libs/cmd/exec.h"
#include "libs/cmd/parser.h"
#include "libs/signal/signal.h"

void sigint_handler(int signum) {
    // Ignore SIGINT
    printf("\n$$ csh ## ");
    fflush(stdout);
}

void sigusr1_handler(int signum) {
    // Set flag = 1
    sigusr1 = 1;
}

void sigchld_handler(int signum) {
    int status;

    // Wait process without waiting
    pid_t pid = waitpid(-1, &status, WNOHANG);

    // Background process
    if (pid > 0) {
        char* cmd;

        // Check if pid exists in process table
        for (int i = 0; i < STRING_SIZE; i++) {
            if (p_table[i].pid == pid) {
                cmd = p_table[i].cmd;

                // Update table
                p_table[i].pid = 0;
                break;
            }
        }

        // Receive signal
        if (WIFSIGNALED(status)) {
            printf("[%d] %s %s\n", pid, cmd, strsignal(status));
        }
        // Normal exit
        else {
            printf("[%d] %s Done\n", pid, cmd);
        }
    }
}

void install_handler() {
    signal(SIGINT, sigint_handler);
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGCHLD, sigchld_handler);
}