#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include <sys/wait.h>
#include<stdlib.h>


int main(int argc, char** argv) {
    printf("Before fork(): my pid is %d\n",getpid());

    int fds[2];
    pipe(fds);

    int childpid = fork();  
    if(childpid==0) {
        close(fds[0]);  
        write(fds[1],"Hel",3);
        sleep(1);
        write(fds[1],"lo!\n",4);
        printf("Child before exec\n");
        char *args[] = {"ls",0};
        dup2(fds[1],1);
        int ret = execv("/usr/bin/ls",args);
//        printf("Child after exec, retval %d, errno %d\n",ret,errno);
//        perror("Failed because");
    }
    else {
        close(fds[1]);  
        char buf[100];
        int got;
        while((got = read(fds[0],buf,100))>0) {
            printf("Got: %s\n",buf);
        }
        if(got < 0) {
            perror("couldn't read");
            exit(1);
        }
        printf("parent\n");
        wait(0);
    }
/*    printf("After fork(): my pid is %d, child pid is %d. %s\n",
            getpid(),
            childpid,
            (!childpid?"Huh, I'm the child! But I don't feel any different!":""));*/
}