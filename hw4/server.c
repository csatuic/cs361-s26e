#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<sys/types.h>
#include<netinet/in.h>

int main(int argc, char** argv) {
    if(argc<2) { printf("Usage: %s <port>\n",argv[0]); exit(1);}
    int port = atoi(argv[1]);
    if(port<1024) { 
        printf("Error: you specified port '%s'. Must be a number > 1024.\n",argv[1]);
        exit(1);
    }
    printf("Listening to port %d.\n", port);

    // add your code here
    
    // if you're having issues with unavailable ports, you may want to add the following,
    // which says that programs may aggressively reuse ports across program restarts
    // NOTE: this doesn't help if you're trying to use the same port as somebody else!
    /* 
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            perror("setsockopt SO_REUSEADDR");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    */
}