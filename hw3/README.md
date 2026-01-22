# Homework 3: Launching and communicating with child processes

Sometimes, a program wants to run another program. Sometimes it just wants to run launch it and forget it. 
Sometimes it needs the output of the program once it exits. And sometimes it needs to interact with the program it just started.

## Lab step 1: Build and run the limerick program

If you haven't already, clone this repo to your Linux environment, and change to the `hw3` folder. 
Provided is a small program `limerick`, which prints a short, witty, and often rude (or even lewd) piece of rhyming poetry. To build the program, first install Rust following these instructions (if you haven't already): https://rust-lang.org/tools/install/

Then `cd` into the limerick folder, and `cargo build`. The executable will now be in `hw3/limerick/target/debug/limerick`. 

`limerick` takes command line arguments, including: `-f <file>` to specify an input file containing limericks, `-o <file>` to specify an optional output file, and  `-l` to request a live presentation. 

Try it out, and see if you get one you like. If you know of a good one that's not in the text file, do share on Piazza. Everybody loves a good limerick.

## Lab step 2: Run `limerick` from within your own program

As discussed in lecture, Unix processes are typically created by first `fork()` in the current process, which results in two near-identical copies of the original program. Then `exec()`, to replace the program image with a new program, while keeping file descriptors and process ID unmodified. 

`fork()` is straight-forward, except for the fact that you call it once, and it returns twice! Try building and running the `fork_example` program: `make fork_example`, `./fork_example` to get a feel for it.  

`exec()` is quite the opposite: if all goes well, `exec()` never returns! It is also a little more difficult to use: in addition to the (absolute) path of the program you want to run, you need to provide a list of arguments in the form of an array of char *: the first one is the program name, and the last one must be 0. Once you get past that, it's not too bad. See `man exec` for the documentation. There are several flavors of this function - Consider useing `execv` for this assignment, but this choice is not very important. 

In this step, write a program `step2` that uses `fork()` and `exec()` to run `limerick`, so that one piece of poetry shows up on the terminal. 

*Demonstrate:* Show your program, then build and run it, demonstrating it printing a limerick on the terminal.
*NOTE:* for those pursuing Advanced Step 6, you may demonstrate steps 2-4 using a Rust program instead. However,
be reminded that quiz and exam questions will cover C only. 

## Lab step 3: Use `limerick -o` instead

In this step, write a program `step3` that is updated slightly, so that it has `limerick` write its output to a file with the `-o` flag. Once `limerick` is done, read the file contents and print it to the screen. 

NOTE: To make things a little more interesting, `limerick -o` waits a moment before writing to the file. You'll want to `wait()` for the child before you try reading the file. 

*Demonstrate:* Show your program, then build and run it, demonstrating it printing a limerick on the terminal.

## Remaining Step 4: Pipe the limerick, then uppercase it

In this step, write a `step4` program, which uses `pipe()` and `dup2()` to redirect `limerick`s output into a file descriptor that `step4` (the parent) can read from. 

Here, `pipe()` creates two new file descriptors: one that you can write into, and one that lets you read back
what was written. What's neat about a `pipe` is that when you `fork()`, the child gets copies of those same end points. If you have the parent `close()` one end, and the child `close()` the other end of the pipe, then you
have established a connection between your two processes. 

Next, `dup2()` enables you to redirect output. `limerick` writes to `stdout`: file descriptor 1 (or to a file if you pass a file name `-o`). However, with `dup2()` we can change what `file descriptor 1` means. In this case, you want `limerick` to the *write* end of the pipe as file descriptor 1, so that anything `limerick` writes ends up in the pipe, instead of on the screen. 

Finally, have `step4` read all of the output, convert it to uppercase, and print it to the terminal. Homework 1 has some handy code for uppercasing strings!

## Remaining step 5: Pass through signals

`limerick` also supports a *live presentation* mode. During live presentation, `limerick` has a custom signal handler for SIGINT, so that the audience can interrupt when they see the start of a limerick they don't like, or one that they've seen before, by hitting CTRL-C. If you try it, and have a hard time exiting out of the program, try CTRL-Z to suspend the process, and then `kill %1` to kill the most recently spawned process in the shell. Alternatively, find the process ID, and use `kill -9 <PID>` to put an abrupt end to the limerick presentation.  

In this step, capture signals as in hw2, and pass them through to `limerick`. However, if the user hits CTRL-C twice in quick succession, they're clearly tired of limericks. In this case, exit the parent process instead. Use `clock_gettime()` to check if the ctrl-c's were in *quick succession*. Perhaps 250 milliseconds is a good definition?

## Advanced step 6: Run a child process with std::process::Command

Rust aims to be far more platform independent than C, which is primarily architecture independent, but heavily dependent on the operating system. As a result, it offers platform independent wrappers over OS specific functionality. All `libc` functions are directly available through the `libc` crate, but it's smoother to use a Rust-native library. 

Write the equivalent of steps 2-4 in Rust, using std::process::Command. 
https://doc.rust-lang.org/std/process/struct.Command.html

Start by creating a new program with `cargo new step2`. Then edit step2/src/main.rs to add the 
necessary code for step2. 

Give it a shot. Although the language barrier will present a challenge to those new to Rust, the programs themselves are considerably easier to write in Rust. Do refer to the Learn Rust resources https://rust-lang.org/learn/, and feel free to reach out to your instructor or TAs for help with rust. (In 2026, Jakob and Jacob are your go-to rust helpers).

## Advanced step 7: Study the limerick program, and the cargo build system

Have a close look at the program in `limerick/src/main.rs`. In the `-l` interactive presentation mode, the limerick is presented syllable-by-syllable, a fairly impressive feat for a 130 line program in a low-level language. First, read the program to track down where each syllable is being printed, and where a word 
is broken up into syllables. What is the function that does the work of breaking up the word?

The Rust standard library does not come with a word-to-syllable extractor. So how did we come to have this wonderful function available? After all, `main.rs` is all the code there is for `limerick`. 

The answer is that the code comes from the crate `hypher`. A software component made freely available online via `crates.io`. Use `cargo tree` to see what other crates `limerick` includes, directly and indirectly. Try `cargo clean; cargo build` to see the `limerick` program be automatically assembled from these various pieces. 


## Advanced step 8: Dynamic typing in Rust

This bit of code is an interesting example of dynamic typing in Rust. The `if`-branch, returns a FileWriter, 
but the `else`-branch returns a Stdout. These types don't have anything to do with each other, yet `writer` can take either value. 

 `` let mut outfile;
    let writer : &mut dyn Write = 
    if let Some(name) = &opt.outfile {
        sleep(Duration::from_millis(1000));
        outfile = File::create(name).unwrap();
        &mut outfile
    }
    else {
        &mut io::stdout()
    };
``

This uses dynamically typed pointers, and allows to think of writer as *either* stdout or the output file, depending on what the user specifies on the command line. 

In contrast with C++, Rust doesn't have a class hierarchy. However, the trait system allows you to express polymorphism in a similar way. Traits are closer to Java's interfaces than superclasses. 

This is not the only way to implement polymorphic behavior, however! For the adventurous Rust student, try refactoring the &dyn Write pointer to instead use an enum with two variants: Stdout and File. 