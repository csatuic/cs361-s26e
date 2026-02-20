#define _GNU_SOURCE
#include<unistd.h>
#include<stdio.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include"disktable.h"
#include<stdbool.h>
#include<stdlib.h>

struct RaceCourse { 
    int participants; 
    long total; 
    long winnings[10]; 
};

int main(int argc, char** argv) {
    if(argc<2) {
        fprintf(stderr,"Usage: racer <participant count>\n");
        exit(1);
    }
    int fd;
    bool leader=false;
    struct stat stats;
    if(stat("racecourse.bin",&stats)==-1) {
        leader=true;
        fd = open("racecourse.bin",O_CREAT|O_RDWR,0755);
        int ret = ftruncate(fd,sizeof(struct RaceCourse));
    }
    else {
        fd = open("racecourse.bin",O_RDWR);
    }

    volatile struct RaceCourse *c = mmap(0,sizeof(struct RaceCourse),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(c<=0) {
        perror("Couldn't map file\n");
        exit(1);
    }
    if(leader) {
        c->participants=atoi(argv[1]);
    }
    int my_participant_id = c->participants;
    c->participants -= 1;

    while(c->participants>0);    
    printf("Everybody is here, let's go!\n");

   
    /* add your code here */

    // clean up the racecourse for the next race
    if(leader)
        unlink("racecourse.bin");
}