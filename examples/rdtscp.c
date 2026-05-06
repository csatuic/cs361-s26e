#include<stdio.h>
#include <x86intrin.h>
#include <stdint.h>

void main() {
    uint32_t aux;
    uint64_t tsc = _rdtscp(&aux);
    printf("Current timestamp is %lu\n", tsc);


    uint64_t newtsc = _rdtscp(&aux);
    printf("And now timestamp is %lu, %lu more\n", newtsc-tsc);
}
