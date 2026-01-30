## Homework 4: Network Programming with Sockets and Epoll 

Networking is critical to most modern software. In this assignment, we use the standard UNIX Socket API:`socket()`, `bind()`, `listen()`, `accept()` and `connect()`, as well as the Linux-specific `epoll()` system call to create a limerick exchange server, where clients may connect to `PRESENT` and `ENJOY` limericks together. 

### Lab step 1. Download and build the template

If you haven't already, clone the class repository and change to the `hw4` folder within it. 
There are two shell programs, `test_client.sh` and `test_server.sh`. Try running `./test_server.sh NNNN` where `NNNN` is a number you chose, greater than 1024.
Then, in a separate terminal, run `./test_client.sh NNNN` with the same number. You should see a message sent from the client to the server.

### Lab step 2. Write your own socket client in `client.c`

A minimal template is provided in `client.c`. Using the `socket()` and `connect()` functions, add functionality to `client.c` to connect to the test server and send it a message. 

You will need to create an `AF_INET` socket of type `SOCK_STREAM` with protocol `0` using `socket()`. 
Then, put together a `struct sockaddr_in` with correct address `127.0.0.1` and the port number from the
command line, which the template already parses for you. The 16-bit port number is specified in big-endian in internet protocols, so use `htons()` to reverse the order of the bytes in your port number.

To test, follow the procedure above: first make sure the test server is running (you may need to pick a different number, in case the one you chose is in use), then start your client with the same number. 

*Demonstrate:* Your client program connecting successfully to `./test_server.sh <PORT>`. 

### Lab step 3. Implement a basic socket server in `server.c`

Starting with `server.c`, write a program that matches the `./test_server.sh` functionality. 
Similar to the client in step 2, first `socket()` to establish a file descriptor, then put together a `struct sockaddr_in` which you pass to `bind()` to register this socket the particular port you want to listen to.
For the address, use `INADDR_ANY` instead of a specific address, to say that address on the local machine is fair game. For the port, however, you have to be specific. Use the port number from the command line. 
 
Check the return value from `bind` and handle errors diligently! Use `perror()` for easy error reporting. 
Once successfully bound, tell the socket to start accepting incoming connections with `listen()`. 
Then, use `accept()` to receive the first incoming connection. The return value is a file descriptor,
which you may read from and write to. 

*Demonstrate:* Your client program successfully connecting to your server program, and the server
printing out a message sent by the client.*

### Lab step 4. A single-client Limerick server

The `limerick` program from `hw3` now includes a networking feature. Given a `-s localhost:NNNN` argument, it will connect to a server on the same  machine that it is running on, using port `NNNN`. It will then do the following present a limerick to the server, and then expect to receive a limerick from the server. Build the `limerick` program in the `hw3/limerick` folder as in homework 3. 

Create a new server program, which is able to serve the `limerick` program over a socket. 
If the `limerick` program says `PRESENT`, the server must respond with `GO AHEAD`. The client will then present a Limerick over the socket. If the client says `AWAIT`, the server must respond with a previously received Limerick in turn. You may want to use `getline()` to receive individual commands and lines of poetry, 
and `strncmp()` to compare strings. 

All limericks consist of 5 lines, with an A-A-B-B-A structure, where A is 8-10 syllables, and B is 5-6 syllables. The `limerick` program will expect that any Limericks it receives conform to this specification.
Simply reproducing the most recently received Limerick correctly is enough to keep it from complaining.

*Demonstrate:* Your limerick server successfully engaging with first one `limerick` program, then another.

### Lab step 5: Handle live presentation, without using `fscanf` or `getline`

By default, the `limerick` program writes an entire limerick to the socket in one step. However, when passed the `-l` flag, it will present its Limericks more poetically. 

In preparation for the remaining steps, write your server such that it accumulates a Limerick over time, rather
than expect the entire poem to arrive in one `read`. 

*Demonstrate:* Your server working with a `./limerick -l` client. 

### Remaining step 6: Concurrent clients with `epoll`

Extend your server to support multiple concurrent clients, using the `epoll` system call API. First, use `epoll_create1()` to create a new epoll file descriptor. Then, for each file descriptor you want to listen
to concurrently, use `epoll_ctl` to add it to the epoll file descriptor. Finally, use `epoll_wait` on the epoll file descriptor to wait for all the registered file descriptors at the same time. 

For this to work, you'll want to register both the server socket and every incoming connection in the epoll. 
Then, loop over `epoll_wait`. For every `epoll_wait` call, handle any file descriptor that is ready. 
Watch out for exited clients - you will want to use `epoll_ctl` to deregister these from the epoll file descriptor, or you will keep hearing from the closed connection.

In this step, supporting limerick clients without `-l` is sufficient.

To test your solution, start the server, then start multiple limerick clients that all connect to the server
and talk to it at the same time. 

### Remaining step 7: Concurrent live clients

Finally, make sure that you can also handle live limerick clients. You will find that with multiple concurrent limerick clients using `-l`, their messages *interleave*. To keep things straight, use a separate buffer for each client. 

To test your solution, start the server, then start multiple limerick clients with the `-l` flag, that all connect to the server and talk to it at the same time. 

### Advanced step 8: Live cross-client presentation

In the above steps, AWAITing clients simply receive the a bulk copy of a Limerick the server previously received. Make the server substantially more engaging by supporting cross-client presentation. 

Upon AWAIT, have the server pause the client until a different client decides to PRESENT. Then, forward
every syllable from the presenter to the awaiter as they arrive. 

For this, you will need a data structure to match up awaiters with presenters: keep in mind that there
may be multiple awaiters available when a presenter appears. Let them all enjoy the show together.



