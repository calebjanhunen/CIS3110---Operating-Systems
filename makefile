CC = gcc
CFLAGS = -Wall -g -std=c11

# file descriptor valgrind: valgrind --quiet --track-fds=yes

a2: A2.c
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -o A2 A2.c -lm

clean:
	rm A2 *.hist