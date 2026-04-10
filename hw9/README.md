## Homework 9: spy on your children with ptrace

In this assignment, we use the Linux ptrace API and a variety of helpful libraries, to build a program that peeks into the internals of a running child. 
Below, we'll use the ptrace nomenclature, calling the program doing the peeking the _tracer_, and the child the _tracee_. 

The template code includes `spy.c`, `debug_helpers.{c,h}` and `Makefile`, as well as a small test program `test.c`. To build the spy, run `make spy`, and to
use it, run `./spy <some program to run>`. Hit Ctrl-C while the program is running, to get some spy output. 

The current spy is capable, but a little restricted in its abilities. Below, you will unleash its powers. 

### Lab step 1. Print the values of the tracees global variables

Use the `PTRACE_PEEK` command to read the value of each global variable, and print it in hexadecimal after the symbol name. The code for finding the global variables and their addresses is already provided.

### Lab step 2. Add a small command interface

Instead of immediately printing data about the tracee on Ctrl-C, start with the tracee suspended, and wait for a single character input from the user. 
Accept the following commands:

| key | Action |
| `g` | print globals with values |
| `f` | print current function | 
| `d` | print current disassembly |
| `c` | resume the tracee |
| `x` | exit tracer and tracee | 

For `c`, use the `PTRACE_CONTINUE` command. For `x`, send `SIGKILL` to the tracee, then `exit()`. 

### Lab step 3. Function call stepping

| `n` | resume to the next time the current function changes name, then print the new name

Use the `PTRACE_SINGLESTEP` command to repeatedly advance the tracee. For each step, check the name of the current function. 
Any time the current function changes, print the name of the new function. A user who keeps pressing `n` would see
a series of functions roughly describing the execution of the program.  

### Remaining step 4. Global variable monitoring 

| `m` | resume the tracee and periodically print globals with values |

To support periodic sampling, use `setitimer()` before `exec` in the child. The timer survives across `exec`, and the tracee will receive a `SIGALRM`, which 
the tracer can catch similar to how we catch the `SIGINT` on `Ctrl-C`. Until the user enables monitoring, simply ignore the `SIGALARM` in the tracer. 

### Remaining step 5. Add system call monitoring and manual filtering

| `s` | resume the tracee, then every time the tracee makes a system call, print the syscall number (or better yet, the name!), and resume the tracee |
| `S` | resume the tracee, then have user approve/reject each system call |

To monitor system calls, use the `PTRACE_SYSCALL` command.
For `S` for each system call, stop, print what call it is as above, and let the user decide whether to let it proceed.
To reject a syscall, use `PTRACE_SETREGS` to set `orig_rax` to `-1`. There is no `-1` system call, so the syscall will fail.



