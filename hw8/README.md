## Homework 8: concurrency bug hunt

The program `flagsquare.c` is a small job queue system, where each job applies an operation to an array of flags. 
The operations are ADD, REMOVE, and SWAP, where ADD enables a randomly selected flag in the array, if it isn't already enabled. 
REMOVE disables a randomly selected flag, if it isn't already disabled, and SWAP swaps the state of two randomly selected flags,
if they aren't both enabled or disabled. 

The program consists of a main thread, which issues jobs into a shared queue, and a number of worker threads which fetch one job at a time from the queue and executes the job. 

### Step 0: try it out with one worker

Use `make flagsquare` to build the program, then run it with `./flagsquare -w 1` for one worker thread, and `./flagsquare -w 1 -s` for one worker thread with SWAP jobs enabled. You should see a small illustration of the flags in your terminal, together with two counts: one based on a running total maintained by the program, and one count that is computed at the end, by counting how many flags were enabled. The two will match up nicely with one worker thread. 

You can use the `-j` parameter to run fewer jobs, or the `-d` parameter to slow down the execution in case you want to see what the program is doing. 

### Lab Step 1: two workers - the program tends to not finish

If you run the program with `./flagsquare -w 2` however, it usually doesn't finish. Let's use `gdb` to find out why.
Start gdb with `gdb --args ./flagsquare -w 2`, then `run`, and you should see the program hang again. If you hit `ctrl-C`,
and run `info thread` you can see what threads were running. Use `thread` with the appropriate thread id (from `info thread`), 
to switch to debugging one particular thread. From here, you can use `bt` and other commands as usual. 

To make things a little easier to keep track of, you can use `thread name` to give the current thread a nice label, like `main`, or `display`. 

Use `thread info`, `thread`, `bt`, `frame` and `print`, to see what the worker thread is hung up on, and which job it was working on when it happened. 
How did it get this far, only to hang on this job?

Find and fix the bug that causes this hang. 

### Lab Step 2: two or more workers - something is wrong with the count

Often, when using two or more workers we find that the two counts at the top of the display don't match up. 

Find and fix the bug(s) that causes this inconsistency. Because the problem is fairly localized, you could consider reading the code. 
Or, use the `clang` threadsanitizer utility to get some ideas. Compile the binary with `clang` instead of `gcc` using `clang -fsanitize=thread -g flagsquare.c -o flagsquare`. You'll get a fair number of hits, but one of them is probably right.

### Lab Step 3: two or more workers, with swaps - occasional hangs

If you run with `./threadsquare -w 4 -s -j 10000000`, you'll find that the program occasionally hangs. Not every time - you may have to try several times for it to happen. For this bug, make sure you use `gdb` to track down the problem. 
There will be several threads involved, so make sure you use the commands above to navigate between them, select the right frame, and print
the relevant values and perhaps addresses to see what's going on. 
An obscure hint: A closer look at index variables may be warranted. 

### Remaining Step 4: Things breaking under heavy load - a more subtle bug

To simulate an overloaded system, reduce QUEUE_SIZE from 100 to 5. Now, the counts are once again inconsistent, even by just a little bit, and occasionally, the program hangs again. Use the `clang` thread sanitizer to track this down, and fix the bug. 

Hint: for thread safety, it's sometimes necessary to work on a temporary copy of shared data. 

### Remaining Step 5: TBD