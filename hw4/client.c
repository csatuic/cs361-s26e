#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>

int main(int argc, char** argv) {
    if(argc<2) { printf("Usage: %s <port>\n",argv[0]); exit(1);}
    int port = atoi(argv[1]);
    if(port<1024) { 
        printf("Error: you specified port '%s'. Must be a number > 1024.\n",argv[1]);
        exit(1);
    }
    printf("Connecting to port %d.\n", port);

    // add your code here
}