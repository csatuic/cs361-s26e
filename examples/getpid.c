#include<stdio.h>
int main() {
	unsigned long pid;
	asm volatile("mov $39,%%rax;syscall;mov %%rax,%[pidreg]":[pidreg]"=r"(pid)::);
	printf("pid is  %lu",pid);
}


