#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/types.h>
#include <capstone/capstone.h>
#include <elfutils/libdwfl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include "debug_helpers.h"

static pid_t child_pid = -1;
static char *child_exec = NULL;
static char **child_args = NULL;

static int tty_fd = -1;
static pid_t original_fg_pgrp = -1;
static pid_t child_pgid = -1;
static pid_t tracer_pgid = -1;

/* Restore the original foreground process group on program exit */
static void cleanup_terminal(void) {
    if (tty_fd >= 0 && original_fg_pgrp > 0) {
        tcsetpgrp(tty_fd, original_fg_pgrp);
        close(tty_fd);
        tty_fd = -1;
    }
}

void print_globals() {
    GlobalVar *globals = NULL;
    int nglobals = 0;

    if (get_globals(child_pid, &globals, &nglobals) == 0) {
        printf("got %d globals\n",nglobals);
        for (int i = 0; i < nglobals; i++) {
            /* Later you can read the value with PTRACE_PEEKDATA */
            printf("%-10s %-30s 0x%016lx  (%zu bytes)\n",
                globals[i].module_name, globals[i].name, globals[i].address, globals[i].size);
        }
        free_globals(globals, nglobals);
    }
}

static void run_tracer(void) {
    int status;
    struct user_regs_struct regs;

    /* Force initial stop after SEIZE */
    if (ptrace(PTRACE_INTERRUPT, child_pid, 0, 0) < 0)
        perror("PTRACE_INTERRUPT (initial)");

    if (waitpid(child_pid, &status, 0) < 0) {
        perror("waitpid initial");
        return;
    }

    /* Give the terminal to the child's process group before the first resume */
    if (tty_fd >= 0) {
        if (tcsetpgrp(tty_fd, child_pgid) < 0)
            perror("tcsetpgrp to child (initial)");
    }

    if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0) < 0) {
        perror("PTRACE_CONT initial");
        return;
    }

    printf("Spy attached (PID %d). Child is running in the foreground.\n", child_pid);
    printf("Press Ctrl-C to inspect the current execution state.\n\n");

    while (1) {
        if (waitpid(child_pid, &status, 0) < 0) {
            if (errno == EINTR)
                continue;
            perror("waitpid");
            break;
        }

        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
            break;
        }
        if (WIFSIGNALED(status)) {
            printf("Child terminated by signal %d\n", WTERMSIG(status));
            break;
        }

        if (WIFSTOPPED(status)) {
            int stopsig = WSTOPSIG(status);

            /* Regain terminal control immediately (tracer is allowed to do this
               even while backgrounded because we ignore SIGTTIN/SIGTTOU) */
            if (tty_fd >= 0) {
                tcsetpgrp(tty_fd, getpgrp());
            }

            if (stopsig == SIGTRAP) {
                /* This is the GDB-style Ctrl-C stop: SIGINT was delivered to the child
                   via the terminal driver and intercepted by ptrace */
//                printf("\n=== Child stopped by SIGINT (Ctrl-C from user) ===\n");

                
                if (ptrace(PTRACE_GETREGS, child_pid, 0, &regs) == 0) {
                    printf("syscall %x\n",regs.orig_rax);
                } else {
                    perror("PTRACE_GETREGS");
                }

///                printf("\n--- Resuming child (SIGINT will be suppressed) ---\n\n");

                /* Return terminal to child before continuing */
                if (tty_fd >= 0) {
                    tcsetpgrp(tty_fd, child_pgid);
                }

                /* Continue without delivering SIGINT (mirrors GDB's default SIGINT handling) */
                if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0) < 0) {
                    perror("PTRACE_CONT resume");
                    break;
                }
                continue;
            }
            // if the user hits Ctrl-Z, interpret this to mean exit 
            if (stopsig == SIGTSTP) {
                break;
            }

            /* Other stop signals are passed through */
            printf("Child stopped by signal %d\n", stopsig);
            if (ptrace(PTRACE_SYSCALL, child_pid, 0, stopsig) < 0) {
                perror("PTRACE_CONT with signal");
                break;
            }
        }
    }

    cleanup_terminal();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-h] <executable> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int opt;
    while ((opt = getopt(argc, argv, "+h")) != -1) {
        if (opt == 'h') {
            printf("Usage: %s [-h] <executable> [args...]\n", argv[0]);
            return EXIT_SUCCESS;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: no executable specified\n");
        return EXIT_FAILURE;
    }

    child_exec = argv[optind];
    child_args = &argv[optind];

    child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        /* Child becomes its own process-group leader */
        setpgid(0, 0);
        execvp(child_exec, child_args);
        perror("execvp");
        _exit(1);
    }

    /* === Tracer (debugger) side only === */

    /* Tracer must be in its own process group (separate from shell and child) */
    setpgid(0, 0);
    tracer_pgid = getpgrp();

    /* Ignore terminal job-control signals so the tracer is never stopped when
       it is backgrounded and needs to call tcsetpgrp() to regain the terminal */
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    /* Ignore SIGINT so Ctrl-C is never delivered to the tracer */
    signal(SIGINT, SIG_IGN);

    child_pgid = child_pid;

    /* Attach using SEIZE (modern, non-stopping attach) */
    if (ptrace(PTRACE_SEIZE, child_pid, 0, 0) < 0) {
        perror("PTRACE_SEIZE");
        return EXIT_FAILURE;
    }

    /* Open the controlling terminal for foreground-process-group switching */
    tty_fd = open("/dev/tty", O_RDWR | O_NOCTTY);
    if (tty_fd >= 0) {
        original_fg_pgrp = tcgetpgrp(tty_fd);
        if (original_fg_pgrp < 0) {
            close(tty_fd);
            tty_fd = -1;
        }
    }

    /* Ensure the terminal state is restored even if the program is terminated abnormally */
    atexit(cleanup_terminal);

    run_tracer();
    return EXIT_SUCCESS;
}