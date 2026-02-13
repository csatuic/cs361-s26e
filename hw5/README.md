## Homework 5: Learning about memory layout with gdb 

In this assignment, we investigate how things are stored in memory. That is, the size, absolute and relative position of various data structures and types, allocated in a variety of ways.

### Why are we doing this?

There are several good reasons to understand this sort of thing. 
First, it is good on its own, with out further application, to know how the world works. As computer scientists, much of our world revolves around computers, so let's understand how they work. 
Second, understanding memory layout helps you design programs that perform well, avoiding unnecessary and resource-intensive steps. 
This is different from algorithmic complexity: a constant number of steps can take a dramatically different amount of time, depending on the implementation. 
Third, it also helps you avoid and diagnose program bugs. Especially in older languages like C and C++, programmers frequently make mistakes that can result in very surprising behavior. Understanding what behaviors are possible under what circumstances can help a great deal here.

### Lab Step 1: integers, pointers, and strings

The program `tonum.c` contains some starter code for a program that converts a string to integers. 
Change only the functions `to_long(), to_ints(), to_shorts() and to_bytes()`, such that the program writes out the correct values. 

For a correct result, the `long` should look much like the string when printed. So should the ints, except with a space between the first 8 and the second 8 characters, and so on. Use no helper functions in implementing your conversion functions - in particular, you must not use the `atoi()` family of functions, or `scanf()`, but no other functions either, instead, rely on addition, subtraction and bit shifting. 

To get started, implement `to_bytes`. Note that the `bytes` argument is not a conventional input. It's a pointer where you are meant to store the output of the function. Thus, bytes[0] should contain the correct first byte `0x88` when the function returns, and so on. 

The function `hex_value` converts a hexadecimal character to a number. Although it is provided, make sure you understand exactly how this works. 
For starters, try adding `bytes[0]=hex_value(string[0]);` to `to_bytes`. Should make half of your first byte have the right value. 

You may find `gdb` very useful here. Start it with `gdb tonum`, then run the program with `run`. Try using the `print` and `x` commands as demonstrated in class to inspect the your values as they are constructed. 

To stop in a particular function, use `break <name>` where `<name>` is the name of the function. Or `break tonum.c:77` to break on line 77. Then, use `step` to run the program one line at a time. Use `set variable=value` to change values on the fly in `gdb`, such as `set bytes[0]=0x88`. 
Use `help <command>` to get the documentation for a `gdb` command. 

Try using `make tonum` inside of `gdb`. It works, and saves an extra step as you modify your code. 

*Demonstrate:* `tonum` printing the correct values of the string converted to numbers. Also, demonstrate using the `print` command to print the bytes array as a 8 hexadecimal numbers, and the `x` command to print the bytes array as a single 8-bytes hexadecimal value (should look like the string, but backwards).

### Lab Step 2: structs and numbers

In the second step, we experiment with casting and the layout of structs in memory. The program `destruct.c` defines a `struct leet` with four members,
as well as one initialized global of type `struct leet`. 

Use `gdb` to investigate the exact layout of this struct in memory. With
`print` you'll get the values of each field, but with `x &l` you can see the 
full contents of the memory starting at the address of `l`. You will need to pass flags to `x` to tell it how you want it printed, as demonstrated in class. Try `help x` too. 

You'll find the struct is 8 bytes. Now, add code to print this struct as if it was a single `unsigned long` instead, using pointer casting. To do this, cast the address of `l` to an `unsigned long` pointer, then dereference the pointer with `*`. If you use `%d` to print it, you'll hopefully discover a number that repeatedly gives you warm and fuzzy feelings. 

Now, let's use a different approach. Declare a new variable `struct foo baz;`  Then use `memcpy` to copy the 8 bytes from `l` to `baz`. 

Use `gdb` to print the value of `baz` after the memcpy: put a breakpoint at the line just after the `memcpy`, then use `print baz` and `print l` to see what's in them. Use `x/8bx &baz` to view the individual bytes in baz. 

*Demonstrate:* Now use what you've learned to reverse this process. 
Show a TA a new `struct leet`, which when printed as an 8-byte unsigned integer in decimal notation yields: `461461461461461461`. Explain how you arrived the values in your new `struct leet`. 

### Lab Step 3: union busting

As discussed in class, unions provide a convenient mechanism for interpreting a single range of bytes as several different types. 

The file `united.c` defines a (tongue-in-cheek) `union proud` with three members: `contracts` and `enforcement`, and `membership`. Here, `enforcement` is a struct and membership is an array of 16-bit unsigned integers. 

In contrast with past programs, we are using `stdint.h` here, which provides shorter and more explicit names for many C types. For example `uint16_t` means 16-bit unsigned integer, or `unsigned short` in standard C on most architectures.

Use `gdb` to print the contents of `activities`: first set a breakpoint after it is initialized, the run the program, then `print activities`. Look carefully
at the values print. It's hard to spot the relationship, but if you use `print/x activities` it may become much clear, as hexadecimal digits line up correctly with bytes. For example, the `pickets` number looks just like the low order digits of the `contracts` number.

Try doubling the number of contracts, using the `gdb` command `set activities.contracts = activities.contracts*2;`, and note how this affects the elements in the enforcement struct. 

Try `print sizeof(activities)` to see how big the activities enum is. Compare this to `print sizeof(activities.enforcement)` and `print sizeof(activities.contracts)`. One is 2 bytes bigger than the other. Use `x/8xb &activities` to see the value of each individual byte of the enum,
starting at the address `&activities`. Make sure you understand exactly where each one of those 8 bytes go in the three union members. 
Specifically, which element of the `membership` array contains those two bytes missing in the `enforcement` struct? 

Finally, how many `pickets`, `strikes`, `bat_beatings`, `kneecaps`, and `concrete_shoes` are required in order to reach 3324321662465 `contracts`?

*Demonstrate:* show your TA the number of each enforcement activity required to achieve 3324321662465 `proud.contracts` in this union. Use `print` in gdb to show it, and explain the method you used to find the solution. 

### Remaining Step 4: baby steps with malloc

### Remaining Step 5: revealing malloc internals

