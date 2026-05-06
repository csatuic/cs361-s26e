#include<stdio.h>
#include<signal.h>
void handler(int) {
    printf("hi there\n");
}

union { int a; short b; };

int main() {
    struct sigaction sa = {
                handler,
               0,
               0,
               0,
               0,
           };
    sigaction(SIGINT,&sa,0);
    while(1);
}