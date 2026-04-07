#include<stdio.h>
static int globalcount;
int hello_count=0;
void hello() {
    static int count=0;
    int localcount=0;
    printf("hello %p %p %p %d\n",&globalcount,&count,&localcount,count++);
}

int main() {
    static int count=0;
    pthread_mutex_lock(&hello_lock);
    hello();
    pthread_mutex_unlock(&hello_lock);
    hello();
}