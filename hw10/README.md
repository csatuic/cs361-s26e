## Homework 10: hands-on performance diagnostics

In this assignment, you are provided the source code, and a couple of 
binary object files, for a log analysis tool. 

It works, but it's not as fast as we would like.

### Step 1: Slow start

Build the analyzer with `make`, then run `./log_analyzer 10k.txt`. You should get some output indicating the number of events per hour, and the severity distribution. 

Use the `time` tool, to see what's going on. What do the three lines mean? Make sure you understand the significance of each measurement here.

In this case, the numbers indicate a lot of time spent waiting. Use `strace -cw` to see what system calls are doing all the waiting. 
Then, use `strace` with the `-e` parameter to filter on a specific system call name, and add the `-k` parameter to get a backtrace for each such system call. Use this information to find nad, and eliminate the problem that's taking several seconds. 

### Step 2: Frustratingly slow report

Even with the above bug eliminated, the program takes its sweet time generating that report, for a mere 10,000 lines of log. 
Run the program with `time` again to see what is up now. 

The measurements suggest we're spending a lot of time working on system calls, not just waiting. 
Find out which system calls we are making and how much time they are taking, using `strace -c`. You'll find that one particular system call is using up about 80% of the time. 

Use `strace -k -e` again, to track down and kill this bug. 

### 