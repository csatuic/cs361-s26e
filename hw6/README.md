## Homework 6: mmap and shared memory

In this assignment, we use the versatile and powerful `mmap` system call in several ways, culminating in two processes communicating via shared memory, rather than through kernel-mediated pipes or sockets. 

In a typical program, `mmap` is used for extremely common things, and one less common use. 

First, `malloc` internally uses `mmap` to allocate *large* chunks of memory, where the definition of *large* varies and is tunable, but is often somewhere in the 128kb to 1MB range. If the application requests a chunk of this size or greater, `malloc` will make an `mmap` system call. If the application later frees the chunk, `free` will make an `munmap` system call. That is, large chunks are directly handled by system calls, rather than the reuse system that `malloc` maintains for smaller chunks. For these large allocations, `mmap` passes the `MAP_ANONYMOUS` flag to mmap, indicating that there is no file involved. 

Second, shared libraries, also known as shared objects, or `.so` files, are mapped into an application's virtual address space using `mmap`.
Here, the dynamic loader passes `mmap` an open file descriptor, and requests to have the contents of the associated file mapped into the application's virtual address space. If the library is not already in memory (perhaps this is the first application using it), then the library will need to be loaded into RAM. 
However, if the library is already in use by another application, then the same RAM is used by both programs, reducing resource requirements. 

Finally, some applications read large files from disk. This is often accomplished using `mmap` instead of `read`, as it can be more efficient. This is especially true for files big enough that they do not fit in memory. 

Rarely, it is also used for inter-process communication. This is error prone, and not usually the best design choice. We'll do it here because it exhibits some interesting behaviors that we'll study in more detail in the second half of the semester. 

### Lab Step 1: find the cut-off for a *large* memory allocation

In this step, we use experimentation to figure out what size `malloc` uses as the threshold for a *large* memory allocation. 

Write a program from scratch, that allocates an increasingly large amount of memory. 
For each allocation, print the address and the boundary tag of the returned chunk. The boundary tag is the 8 bytes before the byte that the returned pointer points at. Use the '%p' printf format specifier to print the chunk address, and the '%x' specifier for the boundary tag. You will notice that the boundary tag contains 
more than just the size: it also includes one low-order bit to indicate if the previous chunk is free or in-use, and one bit to indicate if it was allocated from the heap, or with `mmap`. 

- Work out which bit (and value of the bit) corresponds to free/in_use. By allocating two chunks, then printing the boundary tag of the first chunk, freeing it, and printing the tag again, you should be able to work it out. 
- Work out which bit corresponds to heap/mmap. By allocating increasingly large chunks, and looking for a change in a low order bit as you pass a certain size, you should be able to work that out.
- Also work out the exact size where `malloc` switches from heap allocations (reusing memory, or growing the heap with `brk`) to `mmap` system calls. 
- Finally, observe the actual system calls of your program with `strace` as we have done before. For a request of `x` bytes, does `malloc` ask `mmap` for `x` bytes or something different? How is it different?

*Demonstrate:* show your TA a program that prints the boundary tags of various allocations, in such a way that you can answer the questions above.
Show your `strace` results, and explain your reasoning. 

### Lab Step 2: read a persistent datastructure from a file with mmap

Malloc supports anonymous and file-backed operation. Above, `malloc` used `mmap` with the `MAP_ANONYMOUS` flag, to simply request a region of allocated memory. 
With file-backed `mmap`, the returned pointer points a region of memory containing the contents of the specified file.
Moreover, this is a bi-directional mapping: if you write something to the memory, it ends up in the file, without you having explicitly write anything out to disk. But that's for step 3. 



### Lab Step 3: write to a persistent datastructure with mmap

- 

### Remaining Step 4: Observe the operation of a shared library

With a shared library, multiple processes share the RAM that holds the library code. That is, while they may see the code at different virtual addresses, it is stored in the same physical RAM. 

The programs `foo.c` and `bar.c` both use the `foobar.so` shared library. `foo` sets the the `foobar` global variable `baz` to 17, then continuously observes the contents of the variable and prints the results. `bar` meanwhile, sets the same variable `baz` to 1, then periodically increments it, and prints the value. 

Observe the addresses of the access functions `setbaz()` and `getbaz()`, as well as the address of `baz` itself. Now, using `strace` work out the following:

- what `mmap` calls during the start-up of `foo` and `bar` correspond to loading the shared library?
- is there more than one per program? How do the allocated regions map to the addresses of the functions and the `baz` global variable?
- are there any differences in the flags used? What motivates the difference in flags used?

### Remaining Step 5: inter-process communication (IPC) using shared memory

Finally, consider the struct below:

`struct RaceCourse {
    int participants;
    long total;
    long winnings[10];
}`

The plan here is to run a little friendly competition between multiple processes. Write a program that takes an integer argument on the command line, indicating the intended number of participants. You will start multiple instances of this program, in separate ssh windows. 

When the first instance starts (finding no file), it creates a file big enough to hold a `struct RaceCourse` using `open()` with the `O_CREATE` flag and `lseek`, then `mmaps()` it
into memory, and casts the pointer as a `struct RaceCourse*`. Finally, it sets the `participants` field to the number from the command line argument,
then waits (using a while loop), until all the participants have arrived.

The other processes mmap the existing file, and increment the number of participants by one. 
Finally, when all the participants have arrived, all of the processes immediately proceed to repeatedly increment first the `total` by one, then the `winnings` by one, until the total reaches 1 billion. 

Then, each process prints the total and its winnings. Finally, the first instance deletes the file. 

- What do you observe about the sum of winnings vs. the reported total?
- How does this vary with the number of participants?

