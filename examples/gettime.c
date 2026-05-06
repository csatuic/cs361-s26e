#include<stdio.h>
#include<unistd.h>
#include<time.h>
void main() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    printf("Time  is %ld.%ld\n",time.tv_sec,time.tv_nsec);
    struct timespec time2;
    clock_gettime(CLOCK_MONOTONIC, &time2);
    printf("Time2 is %ld.%ld\n",time2.tv_sec,time2.tv_nsec);
}
