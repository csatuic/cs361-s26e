.globl main
time: 
	.quad 1,000000000
main:
	mov $35, %rax;
	lea time(%rip),%rdi;
	syscall
	ret
