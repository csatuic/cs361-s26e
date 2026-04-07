#include<stdio.h>
#include<stdlib.h>

void g() {
    printf("Hi from G\n");
}
void f() {
    long array[2]={1,2};
    array[3]=g;
    printf("Array %ld %ld\n",array[0],array[1]);
}
int main() {
    f();
}