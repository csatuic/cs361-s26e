# Homework 3: Launching and communicating with child processes

Sometimes, a program wants to run another program. Sometimes it just wants to run launch it and forget it. 
Sometimes it needs the output of the program once it exits. And sometimes it needs to interact with the program it just started.

## Lab step 1: Build and run the limerick program

If you haven't already, clone this repo to your Linux environment, and change to the `hw3` folder. 
Provided is a small program `limerick`, which prints a short, witty, and often rude (or even lewd) piece of rhyming poetry. To build the program, first install Rust following these instructions (if you haven't already): https://rust-lang.org/tools/install/

Then `cd` into the limerick folder, and `cargo build`. The executable will now be in `hw3/limerick/target/debug/limerick`. 

`limerick` takes command line arguments, including: `-f <file>` to specify an input file containing limericks, `-o <file>` to specify an optional output file, and  `-l` to request a live presentation. 

Try it out, and see if you get one you like. If you know of a good one that's not in the text file, do share. Everybody loves a good limerick.

## Lab step 2: Run `limerick` from within your own program

As discussed in lecture, Unix processes are typically created by first `fork()` in the current process, which results in two near-identical copies of the original program. Then `exec()`, to replace the program image with a new program, while keeping file descriptors and process ID unmodified. 

`fork()` is straight-forward, except for the fact that you call it once, and it returns twice! `exec()` is quite the opposite: if all goes well, `exec()` never returns! 
It is also a little more difficult to use: in addition to the (absolute) path of the program you want to run, you need to provide a list of arguments in the form of an array of char *: the first one is the program name, and the last one must be 0. Once you get past that, it's not too bad.

In this step, write a program `step2` that uses `fork()` and `exec()` to run `limerick`, so that one piece of poetry shows up on the terminal. 

*Demonstrate:* Show your program, then build and run it, demonstrating it printing a limerick on the terminal.

## Lab step 3: Use `limerick -o` instead

In this step, write a program `step3` that is updated slightly, so that it has `limerick` write its output to a file with the `-o` flag. Once `limerick` is done, read the file contents and print it to the screen. 

NOTE: To make things a little more interesting, `limerick -o` waits a moment before writing to the file. You'll want to `wait()` for the child before you try reading the file. 

*Demonstrate:* Show your program, then build and run it, demonstrating it printing a limerick on the terminal.

## Lab step 4: Pipe the limerick, then uppercase it

In this step, write a `step4` program, which uses `pipe()` and `dup2()` to redirect `limerick`s output into a file descriptor that `step4` (the parent) can read from. Have `step4` read all of the output, convert it to uppercase, and print it to the terminal. 

*Demonstrate:* Show your code, and demonstrate it printing UPPERCASED limericks. 

## Remaining step 5: interactive presentation 



## Remaining step 6: Pass through signals

During live presentation, `limerick` has a custom signal handler for SIGINT, so that the audience can interrupt when they see the start of a limerick they don't like, or that they've seen before. 

In the final step, capture signals as in hw2, and pass them through to `limerick`. However, if the user hits CTRL-C twice in quick succession, they're clearly tired of limericks. In this case, exit the parent process instead. 

TODO: exiting the child, backgrounding, handling bad file descriptors and suspension



