#include<stdio.h>
#include<unistd.h>
#include"foobar.h"
int main() {
    for(int i=0;i<1000000;i++) {
        setbaz(i);
        printf("bar set baz at %p to %d\n",bazaddr(),getbaz(i));
        sleep(1);
    }
}