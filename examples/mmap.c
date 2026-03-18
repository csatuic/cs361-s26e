#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include <fcntl.h>

int main() {
    int fd = open("thefile.txt",O_RDWR);
    char *ptr = mmap((void*)0x600000000l,15,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
    printf("File is at %p\n",ptr);
    ptr[0]='X';
    write(1,ptr,12);
    printf("\n");
}