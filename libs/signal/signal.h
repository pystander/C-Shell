#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

// Signal flag
int sigusr1;

// Signal block set
sigset_t blkset;

// SIGINT handler
void sigint_handler(int signum);

// SIGUSR1 handler
void sigusr1_handler(int signum);

// SIGCHLD handler
void sigchld_handler(int signum);

// Install signal handlers
void install_handler();

#endif