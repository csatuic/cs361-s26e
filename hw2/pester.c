#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<stdio.h>
#include<errno.h>

int main(int argc, char** argv) {
    if(argc<2) {
        printf("Usage: ./pester <sleeper PID>\n");
        exit(1);
    }
    int pid = atoi(argv[1]);
    while(1) {
        if(kill(pid, SIGINT)) {
            if(errno==ESRCH) {
                printf("My work is done here.\n");
                exit(0);
            }
            perror("Something went wrong sending signal\n");
            exit(1);
        }
        sleep(1);
        printf("Hey, process %d are you sleeping?\n",pid);
    }
}