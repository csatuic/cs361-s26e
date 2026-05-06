#include <stdio.h>
int call_count=0;
void target_func(void) {       
    printf("Inside target_func()\n");
}

int main(void) {
    target_func();
    target_func();
    target_func();

    printf("\n>>> target_func() was called %d times\n", call_count);
    return 0;
}

