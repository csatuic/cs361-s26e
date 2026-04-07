#include<stdio.h>
#include<stdlib.h>

void g() {
    printf("Hi from G\n");
}
void f() {
    long array[2];
//    array[3]=g;
    printf("Array %ld %ld\n",array[0],array[1000]);
}
void another() {    
    long a = 5; 
    long b = 6; 
    long c = 7; 
    long d = 8; 
}
int main() {
    another(); // RSP = x; x-8;x-48;
    f(); // RSP = x
}