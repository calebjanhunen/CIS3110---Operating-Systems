#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void sigHandler(int sigNum);

int numChildrenTerminated = 0;
int **fd;

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Error: No files were provided. \n");
        return -1;
    }

    // malloc pipe array
    fd = malloc((argc - 1) * sizeof(int *));
    for (int i = 0; i < argc - 1; i++) {
        fd[i] = malloc(2 * sizeof(int));
    }

    // register signal handler
    signal(SIGCHLD, sigHandler);
    pid_t child_pid;

    for (int i = 0; i < argc - 1; i++) {
        pipe(fd[i]);
        child_pid = fork();
        char *filename = argv[i + 1];

        if (child_pid == 0) { // child process
            printf("In child process (pid = %d), filename: %s\n", getpid(), filename);
            // close all pipes not being used
            for (int j = 0; j < argc - 1; j++) {
                if (j != i) {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
            }
            close(fd[i][0]); // close read side of pipe

            int fileDesc = open(filename, O_RDONLY); // open file

            // if file does not exist
            if (fileDesc < 0) {
                sleep(1 + (2 * i));
                printf("file, %s, could not be open \n", filename);
                close(fd[i][1]); // close write side of pipe
                exit(1);
            }

            // cleaning up terminating child
            sleep(1 + (2 * i));
            close(fd[i][1]); // close write end of pipe
            close(fileDesc); // close file descriptor
            for (int i = 0; i < argc - 1; i++) {
                free(fd[i]);
            }
            free(fd);
            exit(0);
        }
    }
    // PARENT
    while (numChildrenTerminated < argc - 1) {
        pause();
    }

    for (int i = 0; i < argc - 1; i++) {
        free(fd[i]);
    }
    free(fd);
    return 0;
}

void sigHandler(int sigNum) {
    signal(SIGCHLD, sigHandler);
    int child_status;
    pid_t pid;
    pid = waitpid(-1, &child_status, WNOHANG);
    close(fd[numChildrenTerminated][1]); // close write end of pipe
    if (child_status == 0) {
        printf("%d: Caught SIGCHLD. Child %d terminated normally\n", numChildrenTerminated, pid);
        // TODO: read from read end of pipe from child
        close(fd[numChildrenTerminated][0]); // close read end of pipe

    } else {
        printf("Caught SIGCHLD. Child %d terminated with signal %d and exit status %d\n", pid, sigNum, WEXITSTATUS(child_status));
    }
    numChildrenTerminated++;
}
