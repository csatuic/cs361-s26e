## Homework 10: hands-on performance diagnostics

In this assignment, you are provided the source code, and a couple of 
binary object files, for a log analysis tool. 

It works, but it's not as fast as we would like.

### Step 1: Slow start

Build the analyzer with `make`, then run `./log_analyzer 1k.txt`. You should get some output indicating the number of events per hour, and the severity distribution. 

Use the `time` tool, to see what's going on. What do the three lines mean? Make sure you understand the significance of each measurement here.

In this case, the numbers indicate a lot of time spent waiting. Use `strace -cw` to see what system calls are doing all the waiting. 
Then, use `strace` with the `-e` parameter to filter on a specific system call name, and add the `-k` parameter to get a backtrace for each such system call.

One confusing aspect of the backtrace is that when compiled with optimization, the resulting executable doesn't quite match the source code. In particular, function inlining can make the backtrace hard to interpret, as there will be intermediate function calls missing in the backtrace. To see the difference, try `strace -k` with and without optimization, by modifying the `CFLAGS` variable in the `Makefile` from 
`-O3` (optimize for both speed and space) to `-00` (no optimization). 

 Use the backtrace information to find and eliminate the problem that's taking several seconds. 

### Step 2: Frustratingly slow report

Even with the above bug eliminated, the program takes its sweet time generating that report, for a mere 100 lines of log. 
Run the program with `time` again to see what is up now. 

The measurements suggest we're spending a lot of time working on system calls, not just waiting. 
Find out which system calls we are making and how much time they are taking, using `strace -c`. You'll find that one particular system call is using up about 80% of the time. 

Use `strace -k -e` again, to track down and kill this bug. Again, temporarily disabling optimization can result in an easier-to-read backtrace, but of course, not using optimization will often result in substantially worse performance as well. 

At this point, you should be able to run the analyzer in a small fraction of a second, with the `100.txt` log file. 

### Step 3: Larger log, longer time

However, step up to the `1k.txt` log file, and it's already going slower again, not to mention `10k.txt`. 
Use `time` again to have another look. Now, it looks like we mostly burning CPU in user space, doing some kind of work.

To figure out what work, use the `perf record` and `perf report` profiling sampler to sample the execution. You'll find `strcmp` at the top. Somewhere, we are making a lot of string comparisons. But where? You can try setting a breakpoint in strcmp using `gdb`, and doing a `backtrace`. This will eventually get you to the right answer, but it may take all day - who knows how many legit strcmps there are? 

Using the `--call-graph=dwarf` option to `perf record`, we collect the call stack, not just the instruction pointer at each sample time. Expand the top entry a few times, to see which function doing most of the strcmp calling.

Have a close look at that function - can you speed it up?