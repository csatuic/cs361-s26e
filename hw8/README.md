## Homework 8: concurrency bug hunt

The program `flagsquare.c` is a small job queue system, where each job applies an operation to an array of flags. 
The operations are ADD, REMOVE, and SWAP, where ADD enables a randomly selected flag in the array, if it isn't already enabled. 
REMOVE disables a randomly selected flag, if it isn't already disabled, and SWAP swaps the state of two randomly selected flags,
if they aren't both enabled or disabled. 

The program consists of a main thread, which issues jobs into a shared queue, and a number of worker threads which fetch one job at a time from the queue and executes the job. At 300 lines, this program may already be too long to simply read and catch the bugs by hand. Better to get some practice using commonly available tools! 

### Step 0: try it out with one worker

Use `make flagsquare` to build the program, then run it with `./flagsquare -w 1` for one worker thread, and `./flagsquare -w 1 -s` for one worker thread with SWAP jobs enabled. You should see a small illustration of the flags in your terminal, together with two counts: one based on a running total maintained by the program, and one count that is computed at the end, by counting how many flags were enabled. The two will match up nicely with one worker thread. 

You can use the `-j` parameter to run fewer jobs, or the `-d` parameter to slow down the execution in case you want to see what the program is doing. 

### Lab Step 1: two workers - the program tends to not finish

If you run the program with `./flagsquare -w 2` however, it usually doesn't finish. Let's use `gdb` to find out why.
Start gdb with `gdb --args ./flagsquare -w 2`, then `run`, and you should see the program hang again. If you hit `ctrl-C`,
and run `info thread` you can see what threads were running. Use `thread` with the appropriate thread id (from `info thread`), 
to switch to debugging one particular thread. From here, you can use `bt` and other commands as usual. 

To make things a little easier to keep track of, you can use `thread name` to give the current thread a nice label, like `main`, or `display`. 

Use `info thread`, `thread`, `bt`, `frame` and `print`, to see what the worker thread is hung up on.
You might also notice that one of the worker thread always manages to finish, while the other hangs.

Find and fix the bug that causes this hang. 

*Demonstrate:* the program running to completion with two workers. 

### Lab Step 2: two or more workers - something is wrong with the count

Often, when using two or more workers we find that the two counts at the top of the display don't match up. 

Find and fix the bug(s) that causes this inconsistency. Because the problem is fairly localized, you could consider reading the code. 
Or, use the threadsanitizer utility to get some ideas. Compile the binary with `gcc -fsanitize=thread -g flagsquare.c -o flagsquare`. 
Using the default settings, the thread sanitizer output will be mixed up with the flag display, and include a lot of 
hits from the display thread, which is in fact racy but harmless at this point. You can disable the interactive display output with the `-q` flag. 
 You'll get a fair number of hits, but one of them is probably right.

 *Demonstrate:* the program producing an accurate final count with two workers.  

### Lab Step 3: two or more workers, with swaps - occasional hangs

If you run with `./threadsquare -w 4 -s -j 10000000`, you'll find that the program occasionally hangs. Not every time - you may have to try several times for it to happen. For this bug, make sure you use `gdb` to track down the problem. 
There will be several threads involved, so make sure you use the commands above to navigate between them, select the right frame, and print
the relevant values and perhaps addresses to see what's going on. 
An obscure hint: A closer look at index variables may be warranted. 

*Demonstrate:* the program running to completion with four workers, 10000000 jobs, and swaps enabled. 

### Remaining Step 4: The program tends to hang with a small queue - a more subtle race

To simulate an overloaded system, reduce QUEUE_SIZE from 100 to 5. Now, the counts are once again inconsistent, even by just a little bit, and occasionally, the program hangs again. Use the thread sanitizer to track this down, and fix the bug. 

Hint: for thread safety, it's sometimes necessary to work on a temporary copy of shared data. 

A correct solution runs to completion and produces accurate counts with a QUEUE_SIZE of 5, swaps enabled and 4 threads. 

### Remaining Step 5: The program tends to hang with many worker threads - a different synchronization problem

With 2--4 workers, your program should now be working fine the vast majority of the time. But if you increase to `-w 16`, you'll find the program still hangs relatively often. Use `gdb` to figure out what threads are hanging and where, and figure out why they aren't continuing. 

Hint: https://linux.die.net/man/3/pthread_cond_signal

A correct solution runs to completion and produces accurate final counts with QUEUE_SIZE 5 or 100, with 16 or even 32 threads. 

### Remaining Step 6: Occasional count mismatches still occur

Even with all the fixes above, you'll find that the display shows "Alert: count mismatch" from time to time. 

First, add `exit(1)` to the block where this print happens (after the print) to make the remaining mismatches easier to spot. Then, identify and fix the cause of this count mismatch. Here, `gdb` is not the right tool. Consider reviewing the thread sanitizer output (running with flagsquare without `-q`), and carefully study the code that leads up to the alert. 

A correct solution never produces the count mismatch alert, even during the run, and even with many threads. 