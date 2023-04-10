CC = gcc
CFLAGS = -Wall -g -std=c11

# file descriptor valgrind: valgrind --quiet --track-fds=yes

a1: A1.c
	$(CC) $(CFLAGS) -o A1 A1.c

hextest: hexOptionTest.c
	$(CC) $(CFLAGS) -o hextest hexOptionTest.c

clean:
	rm A1 hextest