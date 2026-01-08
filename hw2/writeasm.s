.section .text
.globl main
main: 
    // put some greeting assembly here

	mov $60, %rax;
	syscall;

.section .rodata
hello:
	.string "Hello world!\n";
