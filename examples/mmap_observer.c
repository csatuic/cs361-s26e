#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include <fcntl.h>

int main() {
    int fd = open("thefile.txt",O_RDWR);
    char *ptr = mmap((void*)0x500000000l,15,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
    printf("File is at %p\n",ptr);
    while(1) {
        sleep(1);
        printf("I'm seeing: ");
        fflush(stdout);
        
        write(1,ptr,12);
        printf("\r");
    }
}