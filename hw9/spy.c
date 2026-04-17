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
#include <sys/time.h>
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

void print_globals(char* binary) {
    GlobalVar *globals = NULL;
    int nglobals = 0;

    if (get_globals(child_pid, &globals, &nglobals) == 0) {
        for (int i = 0; i < nglobals; i++) {            
            /* Later you can read the value with PTRACE_PEEKDATA */
            if(strcmp(binary,globals[i].module_name)==0) {

                printf("%-10s %-30s 0x%016lx  (%zu bytes): ",
                    globals[i].module_name, globals[i].name, globals[i].address, globals[i].size);

                errno=0;
                int peekresult=ptrace(PTRACE_PEEKDATA, child_pid, globals[i].address, 0);
                if(peekresult==-1 && errno) {
                    perror("ERROR PEEKING\n");
                    exit(1);
                }
                printf("%x\n",peekresult);
            }

        }
        free_globals(globals, nglobals);
    }
}


static void run_tracer(char* binary) {
    int status;
    struct user_regs_struct regs;
    int function_stepping = 0;

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

    if (ptrace(PTRACE_CONT, child_pid, 0, 0) < 0) {
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

            if (stopsig == SIGALRM) {
//                printf("Got alarm from child!\n");

                  /* Return terminal to child before continuing */
                if (tty_fd >= 0) {
                    tcsetpgrp(tty_fd, child_pgid);
                }

                /* Continue without delivering SIGINT (mirrors GDB's default SIGINT handling) */
                if (ptrace((function_stepping?PTRACE_SINGLESTEP:PTRACE_CONT), child_pid, 0, 0) < 0) {
                    perror("PTRACE_CONT resume");
                    break;
                }
                continue;
            }

            if (stopsig == SIGINT || stopsig == SIGTRAP) {
                /* This is the GDB-style Ctrl-C stop: SIGINT was delivered to the child
                   via the terminal driver and intercepted by ptrace */
                if(!function_stepping) {
                    printf("\n$: ");
                    fflush(stdout);
                    char cmd[10];
                    if(read(0,cmd,10)!=2) {
                        printf("One-character commands please.");
                        exit(1);                    
                    }                

                    if (ptrace(PTRACE_GETREGS, child_pid, 0, &regs) == 0) {
                        uintptr_t rip = regs.rip;

                        switch(cmd[0]) {
                        case 'f': 
                        print_function_name(child_pid, rip);
                            break;
                        case 'd': 
                        print_disassembly(child_pid, rip);
                        break;
                        case 'g':
                        print_globals(binary);
                        break;
                        case 'x':
                        kill(SIGKILL,child_pid);
                        exit(0);
                        break;
                        case 'c':
                        break;
                        case 'n':
                            char buf[100];
                            printf("Function is %s\n", get_function_name(child_pid, rip, buf, 100));
                            function_stepping=1;
                            break;
                        default: printf("Unknown command\n");
                        }
                    } else {
                        perror("PTRACE_GETREGS");
                    }
                }
                else {
                    if (ptrace(PTRACE_GETREGS, child_pid, 0, &regs) == 0) {
                        uintptr_t rip = regs.rip;
                        char buf[100];
                        memset(buf,0,100);
                        printf("Function is %s\n", get_function_name(child_pid, rip, buf, 100));
                    }
                }

                /* Return terminal to child before continuing */
                if (tty_fd >= 0) {
                    tcsetpgrp(tty_fd, child_pgid);
                }
                /* Continue without delivering SIGINT (mirrors GDB's default SIGINT handling) */
                if (ptrace((function_stepping?PTRACE_SINGLESTEP:PTRACE_CONT), child_pid, 0, 0) < 0) {
                    perror("PTRACE_SINGLETRACE/CONT resume");
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
            if (ptrace(PTRACE_CONT, child_pid, 0, stopsig) < 0) {
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
        struct itimerval timer;
        getitimer(ITIMER_REAL, &timer);
        timer.it_value.tv_sec = 1;
        timer.it_interval.tv_sec = 1; // every second
        if(setitimer(ITIMER_REAL, &timer, 0)) {
            perror("timer problem");
        }

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

    run_tracer(realpath(child_exec,0));
    return EXIT_SUCCESS;
}