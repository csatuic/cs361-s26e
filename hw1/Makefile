CC=gcc
CFLAGS=-g

main.pre.c: main.c
	gcc -E -o $@ $<
main.s: main.pre.c
	gcc -S $< -o $@
main.o: main.s
	gcc -c -o $@ $<

static: main.o uppercase.o lowercase.o changecase.o
	gcc ${CFLAGS} -o $@ $^

liblower.so: lower.c
	gcc -fPIC --shared ${CFLAGS} -o $@ $^
libupper.so: upper.c
	gcc -fPIC --shared ${CFLAGS} -o $@ $^
libchangecase.so: changecase.c
	gcc -fPIC --shared ${CFLAGS} -o $@ $^

dynamic: main.o uppercase.o lowercase.o libchangecase.so libupper.so liblower.so
	gcc -fPIC ${CFLAGS} -L. -o $@ $^ -lchangecase

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.pre.c *.s *.o *.so static dynamic
