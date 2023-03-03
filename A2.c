#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void sigHandler(int sigNum);

// constants
#define ALPHABET_SIZE 26

// global variables
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
            off_t fsize;
            char *fileStr;
            int histogramArr[ALPHABET_SIZE] = {0};
            printf("In child process (pid = %d), filename: %s\n", getpid(), filename);
            // close all pipes not being used
            for (int j = 0; j < i; j++) {
                close(fd[j][0]);
                close(fd[j][1]);
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

            // malloc size of file
            fsize = lseek(fileDesc, 0, SEEK_END);
            fileStr = (char *)malloc(fsize);
            lseek(fileDesc, 0, SEEK_SET);

            read(fileDesc, fileStr, fsize); // read file

            // compute histogram array
            for (int fileIndex = 0; fileIndex < fsize; fileIndex++) {
                char lowerCaseChar = tolower(fileStr[fileIndex]);
                if (97 <= lowerCaseChar && lowerCaseChar <= 122) {
                    int histIndex = lowerCaseChar - 97;
                    histogramArr[histIndex]++;
                }
            }

            write(fd[i][1], histogramArr, sizeof(histogramArr)); // write histogram array to pipe

            // cleaning up terminating child
            sleep(1 + (2 * i));
            close(fd[i][1]); // close write end of pipe
            close(fileDesc); // close file descriptor
            for (int i = 0; i < argc - 1; i++) {
                free(fd[i]);
            }
            free(fd);
            free(fileStr);
            exit(0);
        }
    }
    // PARENT

    // wait for all children to terminate before exiting
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
    signal(SIGCHLD, sigHandler); // re-register signal handler
    int child_status;
    pid_t pid;
    char filename[20], fileStr[300]; // hardcode size?
    int fileDesc;

    pid = waitpid(-1, &child_status, WNOHANG);
    close(fd[numChildrenTerminated][1]); // close write end of pipe
    if (child_status == 0) {
        printf("%d: Caught SIGCHLD. Child %d terminated normally\n", numChildrenTerminated, pid);
        int histArr[ALPHABET_SIZE];
        read(fd[numChildrenTerminated][0], histArr, sizeof(histArr)); // read histogram array from pipe

        snprintf(filename, 20, "file%d.hist", pid);        // create file name
        fileDesc = open(filename, O_RDWR | O_CREAT, 0640); // open/create file to write histogram to

        // fileStr = malloc(sizeof(histArr) + ALPHABET_SIZE); // malloc string with sufficient space
        // printf("strlen: %ld\n", strlen(fileStr));

        // store histogram array (int) into string (char *) to write to file
        int index = 0;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            // char temp[10];
            index += snprintf(&fileStr[index], 300 - index, "%c %d\n", i + 97, histArr[i]);
            // strcat(fileStr, temp);
        }
        printf("%s \n", fileStr);
        write(fileDesc, fileStr, strlen(fileStr)); // write file string (histogram array) to file

        close(fileDesc);

        // printf("FILE DESCRIPTOR %s, %d CLOSED \n", filename, fileDesc);
    } else {
        printf("Caught SIGCHLD. Child %d terminated with signal %d and exit status %d\n", pid, sigNum, WEXITSTATUS(child_status));
    }

    close(fd[numChildrenTerminated][0]); // close read end of pipe
    numChildrenTerminated++;
    // free(fileStr);
}
