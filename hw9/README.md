## Homework 9: spy on your children with ptrace

In this assignment, we use the Linux ptrace API and a variety of helpful libraries, to build a program that peeks into the internals of a running child. 
Below, we'll use the ptrace nomenclature, calling the program doing the peeking the _tracer_, and the child the _tracee_. 

The template code includes `spy.c`, `debug_helpers.{c,h}` and `Makefile`, as well as a small test program `test.c`. To build the spy, run `make spy`, and to
use it, run `./spy <some program to run>`. Hit Ctrl-C while the program is running, to get some spy output. If you are building this on your own machine, try installing the packages `elfutils` and `libdw-dev` to get it to build.

The current spy is capable, but a little restricted in its abilities. Below, you will unleash its powers. 

### Lab step 1. Print the values of the tracees global variables

The template code prints out the values of *all* the global variables, including what's in any shared libraries the tracee uses. 
Limit the print to globals in the executable. You can use the `realpath()` function to turn a relative path name (like `./test`) to an absolute one (like `/home/jakob/test`).

Then, use the `PTRACE_PEEKDATA` command to read the value of each global variable, and print it in hexadecimal after the symbol name. 
Keep in mind that different variables may have different size. 
However, for simplicity, you may assume the size is a multiple of 8 bytes, so you can use the `%016llx` printf format string. 

*Demonstrate:* the program running on the test tracee as before, but prints only the globals of the tracee ()

### Lab step 2. Add a small command interface

Instead of immediately printing data about the tracee on Ctrl-C wait for a single character input from the user. 
You can implement this any way you want, but one easy way is to `read()`
2 bytes: one for the command, and one for `<newline>`.

Accept the following commands:

| key | Action |
| ---- | ------ |
| `g` | print globals with values |
| `f` | print current function |
| `d` | print current disassembly |
| `c` | resume the tracee |
| `x` | exit tracer and tracee | 

The `switch` statement in C may be handy for this step. 
For `x`, send `SIGKILL` to the tracee, then `exit()`. 

*Demonstrate* running each of the commands above, after hitting CTRL-C. 

### Lab step 3. Function call stepping

| key | Action |
| ---- | ------ |
| `n` | resume to the next time the current function changes name, then print the new name |

Use the `PTRACE_SINGLESTEP` command to repeatedly advance the tracee. For each step, check the name of the current function. 
Any time the current function changes, print the name of the new function. A user who keeps pressing `n` would see
a series of functions roughly describing the execution of the program.  

Note that after `PTRACE_SINGLESTEP` the child will get a `SIGTRAP` signal after one instruction. You'll want to handle that (decide whether to print, etc)
rather than pass it through to the child. 

*Demonstrate:* When the user hits CTRL-C then types 'n', the program single steps until the function name changes, then prints the function name.
The user may type 'n' again, to get the next function, or 'c' to continue running normally. 

### Remaining step 4. Global variable monitoring 

| key | Action |
| ---- | ------ |
| `m` | resume the tracee and periodically print globals with values |

To support periodic sampling, use `setitimer()` before `exec` in the child. The timer survives across `exec`, and the tracee will receive a `SIGALRM`, which 
the tracer can catch similar to how we catch the `SIGINT` on `Ctrl-C`. Until the user enables monitoring, simply ignore the `SIGALARM` in the tracer. 

### Remaining step 5. Add system call monitoring and manual filtering

| key | Action |
| ---- | ------ |
| `s` | resume the tracee, then every time the tracee makes a system call, print the syscall number (or better yet, the name!), and resume the tracee |
| `S` | resume the tracee, then have user approve/reject each system call |

To monitor system calls, use the `PTRACE_SYSCALL` command.
For `S` for each system call, stop, print what call it is as above, and let the user decide whether to let it proceed.
To reject a syscall, use `PTRACE_SETREGS` to set `orig_rax` to `-1`. There is no `-1` system call, so the syscall will fail.



