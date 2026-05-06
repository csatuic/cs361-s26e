#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
int main(int argc, char** argv) {
    long x = atoi(argv[1]);
    long y = atoi(argv[2]);
    struct timespec time, time2;
    clock_gettime(CLOCK_MONOTONIC, &time);
    long z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    z = x / y;
    clock_gettime(CLOCK_MONOTONIC, &time2);
    printf("%ld/%ld=%ld. Division took %ld nanoseconds\n",x,y,z,(time2.tv_sec-time.tv_sec)*1000000000+(time2.tv_nsec-time.tv_nsec));
}
