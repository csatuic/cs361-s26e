## Homework : multi-threaded socket server

In this assignment we revisit the limerick server from `hw4` using a multi-threaded approach: for each connected client, the server spawns a new thread, responsible for handling communication with that one client. 

In contrast with the epoll-based server, managing individual per-client state is considerably easier.
There's no need to juggle many connections with epoll. Instead, every I/O system call simply blocks if the client isn't ready. On the other hand, the multi-threaded approach makes working with shared state across clients 
considerably more fraught, as the interleaving of operations between threads is fully unknown in the absence of explicit synchronization. 

### Lab Step 1: Independent clients

Make sure you `git pull` the latest changes to the class repository. The file `server.c` contains a simple Limerick server that supports a single client at a time. Modify `server.c` to support multiple concurrent clients. 

As in the past, the test program in the `client` folder is written in Rust. Use `cargo run` in the `client` folder
to run this test harness. It offers several command line arguments to vary the test setup. 

The goal for step 1 is that the server supports communicating concurrently with multiple `client` instances or other socket programs. 

*Demonstrate:* The server communicating appropriately with one or more clients, even when an "idle" client is introduced using `nc`, as in the previous socket assignment. 

### Lab Step 2: Batch-wise coordination among clients

Update the server's `AWAIT` command, to no longer simply return the previously delivered limerick. Instead, make `AWAIT` wait for a different client to `PRESENT` a limerick. That is, the server has no memory for limericks, instead, it's simply clients exchanging poetry, in real time, via the server. 

In this step, you only need to support the standard `client` functionality, which delivers the entire limerick in a single batch. 

However, it is critical that the server never deliver a partial or garbled limerick to a client. That is, limerick delivery must be atomic. Use a `pthread_mutex_t` around any code that updates or reads the limerick. 

*Demonstrate*: the server running with three clients. Two clients issue the `AWAIT` command, the third issues the `PRESENT` command. The `AMAIT`ing clients then receive the delivered limerick. For this demonstration, use an `nc` client, rather than the provided `client` program.

### Lab Step 3: Live cross-client delivery

Further improve the server to support live client-to-client delivery of limericks. Use the `limerick` client with the `-l` option. Here, `AWAIT`ing clients should have an experience mirroring the live delivery of the limerick client: any time delivering client presents another syllable, forward it immediately to the `AWAIT`ing clients. 

Do not have the thread handling `PRESENT` commands directly write to the clients. Instead, use a global variable to handle cross-thread communication, and `pthread_mutex_t` locks avoid race conditions. 
Make sure you support at least two `AWAIT`ing clients, and that each client sees a nicely presented limerick. 

Critically, ensure that every `AWAIT`ing client receives every word, and that no `AWAIT`ing client receives a given word more than once unless
the limerick actually contains a word that repeats. 

There are several ways you could ensure that every client receives every word, but here's one that may be among the easier: 
Use a global variable with a number indicating how many `AWAIT`ing clients still need to read the word. 
Before accepting a new word from a `PRESENT`ing client, check that the number is zero. 
After accepting the word, but before releasing the lock on the word, set the number to the number of `AWAIT`ing clients.
Finally, decrement the count whenever a client reads the current word. 

To ensure that clients only receive each word once, consider using a word sequence number. 
For the word itself, the count mentioned above, and the sequence number, make sure you protect this shared state with a lock. 

*Demonstrate:* Live presentation with a single `PRESENT`ing client, and two `nc` clients `AWAIT`ing. The `nc` clients should both see a correct live Limerick presentation, considering both timing and content. 

### Remaining step 4 : stop wasting CPU on the server

To achieve the required funcionality of step 3, you have been using a `pthread_mutex_t` lock. However, this forces the server to continuously poll the global variable to see if there is something new to deliver. 

In this step, use a `pthread_condvar_t` to have the server more efficiently wait for the clients. As discussed in class, conditon variables must be used *only* to achieve efficiency improvements: the program should continue working correctly even if the condition wait is commented out. Thus, leave your original functionality in place, but introduce condition variable operations to achieve a more efficient wait. 

### Remaining step : Exclusive delivery

In this final step, introduce a new `privatelisten` command. When a client issues a `privatelisten` command, it waits for the next incoming limerick delivery. The 

// finish this
