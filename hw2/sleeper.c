#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<stdio.h>

int main() {
    printf("Commencing sleep, with PID %d\n",getpid());
    sleep(3600);
    printf("Sleep was interrupted.\n");
}