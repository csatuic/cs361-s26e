#include <stdalign.h>
#include <stdio.h>

struct innerstruct {
    int a; 
    char b;    
};

struct innerstruct i;

struct outerstruct {
    char c;
    // pad 3 bytes
    struct innerstruct inner;    
};
struct outerstruct *op;

union combo {
    int a;
    char b;
    char* c;
    struct innerstruct i;
};

int main(int argc, char **argv) {
    struct outerstruct o;
    
    printf("Size of outerstruct: %ld, size of innerstruct %ld\n", sizeof(o), sizeof(o.inner));
    printf("Align of outerstruct: %ld, align of innerstruct %ld\n", alignof(o), alignof(o.inner));
    
    union combo c;
    c.c=(char*)0xCAFEBABE12345678;
    printf("%p full pointer\n",c.c);
    printf("c.a=%x c.i.b %hhx\n",c.a,c.i.b);

    c.i.b = 0x00;
    printf("%p updated pointer\n",c.c);

    printf("c.b = %hhx \n",c.b);

}