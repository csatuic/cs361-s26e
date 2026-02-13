#include<string.h>
char hw[]="hello world\n";
void main() {
    int len = strlen(hw);
    asm volatile("mov $1,%%rax; \
                  mov $1,%%rdi;\
                  lea %0,%%rsi; syscall;"::"m"(hw),"d"(len):"rax","rdi","rsi");
}