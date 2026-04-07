#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include <fcntl.h>

int main() {
    char *ptr = malloc(1024*4096); //mmap(0x600000000,8192,PROT_WRITE|PROT_READ,MAP_SHARED|MAP_ANONYMOUS,0,0);
    printf("Pointer is %p\n",ptr);
sleep(3000);
    ptr[10000]='X';
    printf("This is fine\n");
    ptr[16000]='Y';
    printf("This is not\n");
}
