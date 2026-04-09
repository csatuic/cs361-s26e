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

static volatile sig_atomic_t interrupt_flag = 0;

/* Async-safe message for signal handler */
static void safe_print(const char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

static void sigint_handler(int sig) {
    (void)sig;
    interrupt_flag = 1;
    safe_print("[SIGINT] Handler fired - setting flag\n");
}


static void run_tracer(void) {
    int status;
    struct user_regs_struct regs;

    /* Force initial stop */
    if (ptrace(PTRACE_INTERRUPT, child_pid, 0, 0) < 0)
        perror("PTRACE_INTERRUPT (initial)");

    if (waitpid(child_pid, &status, 0) < 0) {
        perror("waitpid initial");
        return;
    }
    // if (ptrace(PTRACE_SETOPTIONS, child_pid, 0,
    //            PTRACE_O_TRACEEXEC) < 0)
    //     perror("PTRACE_SETOPTIONS");

    if (ptrace(PTRACE_CONT, child_pid, 0, 0) < 0) {
        perror("PTRACE_CONT initial");
        return;
    }

    struct sigaction sa = { .sa_handler = sigint_handler, .sa_flags = SA_RESTART };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    printf("Spy attached (PID %d). Child is running freely.\n", child_pid);
    printf("Press Ctrl-C to inspect.\n\n");

    while (1) {
        /* Heartbeat so we know the loop is alive */
        static time_t last_beat = 0;
        time_t now = time(NULL);
        if (now != last_beat) {
            last_beat = now;
        }

        if (interrupt_flag) {
            interrupt_flag = 0;

            printf("interrupting\n");
            if (ptrace(PTRACE_INTERRUPT, child_pid, 0, 0) < 0)
                perror("PTRACE_INTERRUPT");
            printf("interrupted\n");

            if (waitpid(child_pid, &status, 0) < 0) {
                perror("waitpid after INTERRUPT");
                break;
            }
            printf("wait finished\n");

            if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGINT)
                printf("(SIGINT suppressed for child)\n");

            if (ptrace(PTRACE_GETREGS, child_pid, 0, &regs) == 0) {
                uintptr_t rip = regs.rip;
                print_function_name(child_pid, rip);
                print_disassembly(child_pid, rip);
            } else {
                perror("PTRACE_GETREGS");
            }

            printf("\n--- Resuming child ---\n\n");
            ptrace(PTRACE_CONT, child_pid, 0, 0);
        }

        if (waitpid(child_pid, &status, WNOHANG) > 0) {
            if (WIFEXITED(status)) {
                printf("Child exited with status %d\n", WEXITSTATUS(status));
                break;
            }
            if (WIFSIGNALED(status)) {
                printf("Child terminated by signal %d\n", WTERMSIG(status));
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-h] <executable> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
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
        setpgid(0, 0);
        execvp(child_exec, child_args);
        perror("execvp");
        _exit(1);
    }

    if (ptrace(PTRACE_SEIZE, child_pid, 0, 0) < 0) {
        perror("PTRACE_SEIZE");
        return EXIT_FAILURE;
    }

    run_tracer();
    return EXIT_SUCCESS;
}