#include<stdio.h>
#include<unistd.h>
#include<signal.h>

void hw() {
 printf("hw\n");
}
int main() {
 struct sigaction act = {hw,0,0,0,0};
 sigaction(SIGINT,&act,0);
 while(1) sleep(10);
}
