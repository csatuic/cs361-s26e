#include<stdio.h>
#include<stdlib.h>

int main(int argc, char** argv) {
    char * ptr = malloc(atoi(argv[1]));
//    char * ptr2 = malloc(2000);
    printf("Pointer is %p, tag is %lx\n",ptr,*((unsigned long*)(ptr-8)));
    // free(ptr);
    // char * ptr3 = malloc(100000);
    // printf("Pointer is %p, tag is %lx\n",ptr,*((unsigned long*)(ptr2-8)));
    // printf("main() is at %p\n",main);
}