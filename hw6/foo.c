#include<stdio.h>
#include<unistd.h>
#include"foobar.h"

int main() {
    setbaz(17);
    while(1) {
        sleep(1);
        printf("baz at %p is %d\n",bazaddr(),getbaz());
    }
}