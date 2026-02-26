#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include <fcntl.h>

int main() {
    int fd = open("thefile.txt",O_RDWR);
    char *ptr = mmap(0x600000000,15,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
    printf("File is at %p",ptr);
    ptr[0]='X';
    write(1,ptr,10);
}