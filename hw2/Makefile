CC=gcc
CFLAGS=-g
.PHONY: clean

writeasm: writeasm.s
	gcc -no-pie -o $@ $^

clean: 
	rm -rf writeasm write printf sleeper pester inline
