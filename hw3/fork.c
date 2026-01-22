#include<unistd.h>
#include<stdio.h>

int main(int argc, char** argv) {
    printf("Before fork(): my pid is %d\n",getpid());
    int childpid = fork();
    printf("After fork(): my pid is %d, child pid is %d. %s\n",
            getpid(),
            childpid,
            (!childpid?"Huh, I'm the child! But I don't feel any different!":""));
}