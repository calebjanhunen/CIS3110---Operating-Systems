#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// In this example, the child sends data to a parent through a pipe
int main(void) {
    int fd[2], nbytes;
    pid_t childpid;
    char string[] = "Hello, world!";
    char readbuffer[80];

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    printf("fd[0]: %d, fd[1]: %d\n", fd[0], fd[1]);

    if ((childpid = fork()) == -1) {
        perror("fork");
        exit(1);
    }

    if (childpid == 0) {
        /* Child process closes up read side of pipe */
        close(fd[0]);

        /* Send "string" through the write side of pipe */
        write(fd[1], string, (strlen(string) + 1));
        exit(0);
    } else {
        /* Parent process closes up write side of pipe */
        close(fd[1]);

        /* Read in a string from the pipe */
        nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
        printf("Received string of from child: %s (%d bytes)\n", readbuffer, nbytes);
    }

    return (0);
}