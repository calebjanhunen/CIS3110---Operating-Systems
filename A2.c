#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void sigHandler(int sigNum);

int numDone = 0;

// int **fd;

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Error: No files were provided. \n");
        return -1;
    }
    // fd[2] = malloc(argc * sizeof(int *));
    // printf("%ld \n", sizeof(fd));
    // printf("%ld \n", argc * sizeof(int));

    // int fd[argc - 1][2];
    pid_t child_pid;

    signal(SIGCHLD, sigHandler);
    for (int i = 0; i < argc - 1; i++) {
        // pipe(fd[i]);
        child_pid = fork();
        char *filename = argv[i + 1];

        if (child_pid == 0) { // child process
            printf("In child process (pid = %d), filename: %s\n", getpid(), filename);
            // for (int j = 0; j < i; j++) {
            //     // close all pipes not being used
            //     close(fd[j][0]);
            //     close(fd[j][1]);
            // }
            // close(fd[i][0]); // close read side of pipe

            // write(fd[i][1], filename, strlen(filename) + 1);

            // sleep(10 + (2 * i));
            sleep(1 + (2 * i));
            // close(fd[i][1]);
            exit(1);
        }
    }
    // printf("parent with id: %d\n", getpid());
    while (numDone < argc - 1) {
        pause();
    }

    // for (int i = 0; i < argc - 1; i++) {
    //     close(fd[i][1]);
    //     char buffer[30];
    //     read(fd[i][0], buffer, sizeof(buffer));
    //     close(fd[i][0]);
    //     printf("String read from child %d: %s \n", child_pid[i], buffer);
    // }
    return 0;
}

void sigHandler(int sigNum) {
    signal(SIGCHLD, sigHandler);
    int child_status;
    pid_t pid;
    // while ((pid = waitpid(-1, &child_status, WNOHANG)) > 0) {
    pid = waitpid(-1, &child_status, WNOHANG);
    printf("Caught SIGCHLD. Child %d terminated\n", pid);
    if (child_status == 0) {
    }
    numDone++;
}
