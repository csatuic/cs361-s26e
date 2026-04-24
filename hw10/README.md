## Homework 10: hands-on performance diagnostics

In this assignment, you are provided the source code, and a couple of 
binary object files, for a log analysis tool. 

It works, but it's not as fast as we would like.

### Lab Step 1: Slow start

Build the analyzer with `make`, then run `./log_analyzer 1k.txt`. You should get some output indicating the number of events per hour, and the severity distribution. 

Use the `time` tool, to see what's going on. What do the three lines mean? Make sure you understand the significance of each measurement here.

In this case, the numbers indicate a lot of time spent waiting. Use `strace -cw` to see what system calls are doing all the waiting. 
Then, use `strace` with the `-e` parameter to filter on a specific system call name, and add the `-k` parameter to get a backtrace for each such system call.

One confusing aspect of the backtrace is that when compiled with optimization, the resulting executable doesn't quite match the source code. In particular, function inlining can make the backtrace hard to interpret, as there will be intermediate function calls missing in the backtrace. To see the difference, try `strace -k` with and without optimization, by modifying the `CFLAGS` variable in the `Makefile` from 
`-O3` (optimize for both speed and space) to `-00` (no optimization). 

 Use the backtrace information to find and eliminate the problem that's taking several seconds. 

 *Demonstrate* to your TA what the problem was, and how you found it.

### Lab Step 2: Frustratingly slow report

Even with the above bug eliminated, the program takes its sweet time generating that report, for a mere 100 lines of log. 
Run the program with `time` again to see what is up now. 

The measurements suggest we're spending a lot of time working on system calls, not just waiting. 
Find out which system calls we are making and how much time they are taking, using `strace -c`. You'll find that one particular system call is using up about 80% of the time. 

Use `strace -k -e` again, to track down and kill this bug. Again, temporarily disabling optimization can result in an easier-to-read backtrace, but of course, not using optimization will often result in substantially worse performance as well. 

At this point, you should be able to run the analyzer in a small fraction of a second, with the `100.txt` log file. 

*Demonstrate* to your TA what the problem was, and how you found it.

### Lab Step 3: Larger log, longer time

However, step up to the `1k.txt` log file, and it's already going slower again, not to mention `10k.txt`. 
Use `time` again to have another look. Now, it looks like we mostly burning CPU in user space, doing some kind of work.

To figure out what work, use the `perf record` and `perf report` profiling sampler to sample the execution. You'll find `strcmp` at the top. Somewhere, we are making a lot of string comparisons. But where? You can try setting a breakpoint in strcmp using `gdb`, and doing a `backtrace`. This will eventually get you to the right answer, but it may take all day - who knows how many legit strcmps there are? 

Using the `--call-graph=dwarf` option to `perf record`, we collect the call stack, not just the instruction pointer at each sample time. Expand the top entry a few times, to see which function doing most of the strcmp calling.

Have a close look at that function - can you speed it up?

*Demonstrate* to your TA what the problematic function was, how you found it, and yur proposed fix.

### Remaining Step 4: Comprehensive mode - a problematic binary.

Next, run a comprehensive log analysis with `./log_analyzer --comprehensive 100.txt`. You'll find that the comprehensive analysis takes considerably longer to run. Even with a 100 line log, it takes a good 10 seconds. 

Use the methods discussed in class and above, to find out what function is causing this performance issue. 
Unfortunately, the code for this library function is not available. However, it's a very small function.

Have a look at the disassembly to see what's wrong with it (should be obvious). Then, use one of several ways to work around this performance bug:

1. Manually reverse-engineer the code, and reimplement the function (without the inefficiency) in the source code under a new name. Use that function instead.
2. As above, but lean on an AI to reverse-engineer the function in question. 
3. Edit the binary, overwriting the redundant call with `nop` (https://c9x.me/x86/html/file_module_x86_id_217.html). The opcode for `nop` is `0x90`. You would not be the first person resorting to this level of desperation, and it's a neat thing to try. 
First, use `objdump -Fd` to find the offset in `log_analyzer` containing the offending instructions. 
Here, keep in mind that the offset shown in the left column is from the beginning of instructions, not the beginning of the file. 
The file offsets listed at the symbol labels show the offset into the file at the symbel, leaving room for a 64 byte header (0x40 in hex).

Better find the exact hex values of the instruction you want to overwrite too, just so you can double-check that you have the right spot. 
Then, use the `hexedit` program already on `baz`, or any other hex editor. Hit `F1` for a help menu for this obscure old program. 
Go to the bytes you want to change (or search for the exact instruction bytes using `/`, but double check that you got the right offset!), overwrite them with `0x90`, then save and exit. Re-run `make`. If successful, your program should now run much faster.

### Remaining Step 5: normalize_field is taking too much time

Having eliminated the step 4 inefficiency, you'll now find that a function called `normalize_field` is consuming most of the remaining time. Use the tools discussed in class and mentioned above to work out what function is calling `normalize_field` so much.
To get more samples, consider using the `1k.txt` or `10k.txt` file instead of the `100.txt`. 

Unfortunately, you'll find that the offending function is also in a binary, with the source code lost to time and poor revision control and backup hygiene. Here, trying to replace the function is a lot harder, since it's being called from many places. 

Instead, work out what the function does, and see if you can safely reduce the number of calls to it, without changing the actual behavior of the program. Eliminating redundant calls to this function should give you 90\% of the gains with 10\% of the effort.

