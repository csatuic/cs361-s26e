#include<stdio.h>
#include<stdlib.h>

int array[4];
int var;

int main() {
    int *ptr = malloc(64);
    *ptr = 0x0605;
    char *bytes = (char*)ptr;
    bytes[64]=0x77;
    printf("0x%04x\n",ptr[0]);
}