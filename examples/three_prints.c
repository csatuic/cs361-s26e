#include<stdio.h>
main() {
	for(int i=0;i<10000;i++) {
		printf("Hello");
		printf(", world");
		printf("!\n");
		usleep(1000);
	}
}
