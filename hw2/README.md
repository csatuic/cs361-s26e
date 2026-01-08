# Homework 2: Discovering increasingly low-level ways to greet the planet

In this assignment, we dig deeper into the rich world of, you guessed it, `Hello World!`. 
Starting with a familiar `printf` implementation, we use tools to discover how `printf` works.
Then we build a series of increasingly low-level implementations of this ever-popular piece of software,
learning a few things along the way. 

## Lab step 1: Build and run the printf program

If you haven't already, clone this repo to your Linux environment, and change to the `hw2` folder. 
`make printf` builds the program for you. To run it, use `./printf` to specify which printf you're
looking for (the one in the current folder: `.`). (If you use just `printf`, you'll
get this helpful message from the built-in `printf` command in bash: 
`printf: usage: printf [-v var] format [arguments]`).

Have a look at the `Makefile`. It has no target for printf. Figure out why this works anyway.

*Demonstrate:* build and run the program, and explain to the TA why make knows how to build printf.

## Lab step 2: How does printf actually do it?

As we've learned in class, processes must use system calls to communicate with the outside world.
This includes putting characters on a screen! So which system call does printf use to do this? Let's find out.

Run the `printf` program under `gdb` with `gdb printf`, then type `run`. 
To stop execution at the first system call, use `catch syscall`, then `run`. 
This works almost exactly the same as a breakpoint (which you add with `break`). 
The only difference is that it stops execution at every place where a system call is made. 

Use `bt` to see the backtrace of the program at this point. Use `continue` (or just `c`) to continue execution. If you hit Enter, it runs the same command again. So try `c`, then hold in Enter until you see the `Hello world` print. Now you should be able to see which system call was used. 

Delete the catch point with `del 1`, and introduce a new one for just the system call you're interested in:
`catch syscall SYSCALLNAME`, replacing the syscall you want, then `run` again. Execution will stop right before the print. 

Try a `bt` to see the stack of function calls that resulted in eventually making this syscall. At the top, you'll see `main`, but no `printf`. This is because the compiler decided to replace `printf` with `puts` due to it being just a simple string. 

Use `disass` to see the assembly of the function at the system call location. 

*Demonstrate: show the TA the disassembly at the system call location, and point out the instruction that makes the actual system call. 

## Lab step 3: Do it with `libc` `write()` instead

In the backtrace (`bt` in gdb), you may have noticed a function called write. That's a helper function for the raw system call (we'll call this `sys_write`). Write is used to write data to file descriptors. 

We could of course resort to using documentation to figure out how to use `write()`, which takes quite different arguments than `printf()`. However, in this case we'll use `strace` instead, to see how `puts/printf` did it! Run the command `strace ./printf`, to get a list of every system call that your `printf` program ran in the course of its execution. Near the end, you'll find `write` listed.

It looks a little garbled though, as the output of `strace` is mixed with the output of `./printf`. Fortunately, `strace` prints to the `stderr` file descriptor (number 2), while `./printf` prints to `stdout` (number 1), so we can tell `bash` to redirect the output of either one to a file using:

`strace ./printf 2> trace.txt` or

`strace ./printf > hw.txt` 

The `>` is a redirection operator. If you went with the first one, the trace is now in `trace.txt`. To see its contents, try `cat trace.txt`, or just open it in your editor. If you just want the end of the file, `tail -10 trace.txt` gets the last 10 lines, or `head -10 trace.txt` gets the first 10. `grep write trace.txt` gets every line that contains the string `write`. 

The output of `strace` shows the system call, its arguments, and its return value. Based on this information only, create a new program `write.c`. The new program should have the same output, but without using `printf()` or `puts()`. Instead, use `write()`. 

You'll likely get a compiler warning like this `warning: implicit declaration of function ‘write’;`. That's because we didn't tell the compiler that such a function exists. In this case, it'll work anyway, as 
write.o will simply have a relocation entry for `write`, and the linker will find a matching symbol in `libc`. 
However, it's both bad practice, and a common source of errors to try to link with undeclared functions.
To both get rid of the warning, and avoid any potential problems: `#include<unistd.h>`.

*Demonstrate:* Show your TA your program using `write()`, and demonstrate it greeting the world.

## Lab step 4: Do it with assembly instead

To take this a step further, let's just write the whole thing in assembly instead. You're given a template program in `writeasm.s`, and a nice Makefile target. However, while the program already contains the the string with a handy `hello:` label that you can reference, the program simply exits: it doesn't write anything to stdout.

Naturally, we could use AI or documentation to try to figure out what parameters to pass the syscall, but
let's use tools instead. Run your program above inside gdb `gdb ./write`, and `catch syscall write`, `run` to 
stop at the write system call. 

Use `bt` to see the current call stack: should be your `main()` function, followed by `write()`.
Use `disass` to get a listing of the assembly for the current function (`write()`). 
This shows you exactly what `write()` does. The vast majority of it is error handling, which we can ignore,
but there are two key lines: one that specifies which system call to use, and another that actually initiates the system call. You'll need those in your program too. Maybe compare to `writeasm.s` if you have a hard time
finding them. 

A system call needs arguments too though! On an x86 64-bit Linux machine, simple integer arguments to a function are always passed in these registers, in this order: %rdi, %rsi, %rdx, ... (https://docs.oracle.com/cd/E19253-01/816-5138/fcowb/index.html). It so happens, that system calls accept arguments in the same order, so this function simply reuses the values already in the registers. We still need to know what they are though.

You can actually see the register values using `info reg`. Look for the registers listed above. But we can do better. In `gdb`, use `up` then `disass` again to see the assembly of the calling function `main()`. 

Now you can see how your `main()` puts your arguments in the appropriate registers. You'll want to do the same thing in writeasm.s. Note that the `lea 0x...(%rip),%rax` probably isn't what you want to actually write in writeasm.s, unless you want to manually work out the relative offset between the symbol `hello` and the current instruction! Instead, just use `hello(%rip)` and let the assembler worry about it. 

You might be tempted just have an AI do this for you. I'd suggest resisting the temptation, and trying to struggle through it instead. Maybe ask the AI (or better, your TA!) for help if you get stuck. 
There will be many occasions in this class to read assembly, and some occasions to write a tiny bit of it.
Look at this as a light warm-up. 

*Demonstrate:* show the TA your assembly code, and demonstrate building it with `make writeasm` and running it with `./writeasm`. 

## Lab step 5: Do it with inline assembly instead

Finally, very few people write entire programs in assembly these days. But systems programmers will often
find occasion to use a little bit of assembly in a larger program written in a higher level language. 

As discussed in lecture, C provides _inline assembly_ a convenient facility for baking a little bit of assembly into an otherwise C program. 

Modify `inline.c` by adding an inline assembly block that prints the pre-existing greeting. 
Keep in mind that in inline assembly, you need double percent to refer to registers, thus: `%%rsi`, not `%rsi`. Use a single percent and an integer to refer to input constraints by number, thus `%0` for the first input constraint. 

*Demonstrate:* show your assembly code to your TA. Let them change the greeting a bit, then run `make inline; ./inline` to demonstrate your program working.

## Remaining step 6: Stop them from interrupting your sleep

Signals offer a very limited, but quite useful, means of out-of-band communication with applications. 
They're used to kill them, to interrupt them, suspend them, wake them up from sleeping, and inform them of a
number of events regarding their children and open file descriptors. 

If you hit CTRL-C while running a program on the terminal, the signal SIGINT is delivered to the application.
The default signal handler will exit the application. 

In this step, modify the existing program `sleeper.c` to ignore such interruptions. 
Use the `sigaction()` function from libc to change the signal disposition of `sleeper.c`, so that 
it prints a little message, then goes right back to sleep. 

Here, try to rely purely on `man sigaction` when developing your solution. That is, try to not look for examples online, or use an AI to write anything for you. If there are things you don't understand in the `man` page, don't be shy to get clarifications from an AI or TA, but try your best to work out how to write your program based on the `man` page, rather than other examples. `man sigaction` contains a lot of interesting information about how you may handle signals - take some time to peruse it. 

The provided program `pester` takes a PID as argument, and keeps pestering the indicated process with SIGINT signals. Try it out. 

## Remaining step 7: Really stop them from interrupting your sleep

Being constantly pestered like that can get frustrating. Instead of simply tolerating the interruptions, let's retaliate. 

Modify `sleeper.c` to determine the PID of the process that interrupted your sleep, then `SIGINT` it right back, using `kill()`: this is backed by `sys_kill`, which is what's used to send signals to other processes. Here, rely on the documentation in `man 2 kill`. The `2` here is to distinguish `kill()` from the command line tool `kill`, which you can read about under `man kill`. 

To learn how to find the PID of the process that sent a signal, have another look at `man sigaction`. You will need to pass the `SA_SIGINFO` flag, and a different handler function. 

If you get it to work, you should be able to start sleeper, then if you run pester with the sleeper's PID, sleeper will interrupt pester right back after the first SIGINT, and enjoy an uninterrupted rest after that.

*Hacker challenge:* in principle, someone could launch pester like this: `while true; do ./pester THEPID; done`. That is, keep launching a new `pester` whenever one dies. To counter this, we would have to kill 
not only the pester, but their parent, and maybe their grandparent, just to be safe? If you're up for it,
try building such a truly vengeful sleeper!

## Advanced remaining step 8: Handle SIGINT in Rust. 

Now write a Rust program instead. Start with `cargo new` to create a new project. Then 
add a signal handler. 

For this, we will need to add a library dependency, a `crate` in Rust speak to support signal handling.
There are several alternatives, including `libc`, `signal-hook` and `tokio::signal`, but we'll use `ctrlc` this time around: it's nice, and all we need. 

To add ctrlc as a dependency to your project, use `cargo add ctrlc`. Have a look at `Cargo.toml` to see what it did. There is documentation for this crate at https://crates.io/crates/ctrlc. Also, if you are unfamiliar with Rust, this website https://rust-lang.org/learn/ has excellent tutorials. 

To start, move the `println` line into a new function, called `myhandler`. Try calling it from `main` to make sure you got it right. Run the program with `cargo run`. 

Once that works, use `ctrlc::set_handler(myhandler);` to start listening for Ctrl-C, similar to how `sigaction` works in C. Naturally, `ctrlc` uses the same system calls that `sigaction` uses. 

You'll find that the program finishes immediately. To fix this, add a sleep interval with `std::thread::sleep`. https://doc.rust-lang.org/std/thread/fn.sleep.html

## Advanced remaining step 9: Count signals in Rust

Instead of the `myhandler` function, try passing an anonymous `move` closure that does the same thing. 
Once you get that to work, declare a mutable `count` variable in main, then have the closure increment the
count every time it is executed. To print the number, just add `{count}` to the `Hello World` string. 



