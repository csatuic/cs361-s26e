#include<stdio.h>
static int globalcount=0;
void counter() {
    int localcount = 0;
    for(long i=0;i<1000000000;i++) {
    globalcount++;
    dummy();
    }
}

int main() {
    static int count=0;
    counter();
    printf("global count is %d\n",globalcount);
}