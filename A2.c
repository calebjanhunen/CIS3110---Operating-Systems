#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// function definitions
void sigHandler(int sigNum);
int getIndex(int *arr, int element);

// constants
#define ALPHABET_SIZE 26

// global variables
int numChildrenTerminated = 0;
int child_pid[100];
int child_pid_length;
int fd[100][2];

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Error: No files were provided. \n");
        return -1;
    }
    child_pid_length = argc - 1;

    // register signal handler
    signal(SIGCHLD, sigHandler);

    for (int i = 0; i < child_pid_length; i++) {
        pipe(fd[i]);
        child_pid[i] = fork();
        char *filename;
        filename = argv[i + 1];

        if (strcmp(argv[i + 1], "SIG") == 0 && child_pid[i] > 0) {
            kill(child_pid[i], SIGINT);
        }

        if (child_pid[i] < 0) {
            printf("Could not create child process.\n");
        } else if (child_pid[i] == 0) { // child process
            printf("In child process (pid = %d), filename: %s\n", getpid(), filename);
            sleep(1);
            off_t fsize;
            char *fileStr;
            int histogramArr[ALPHABET_SIZE] = {0};

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
            free(fileStr);
            exit(0);
        }
    }
    /*********************PARENT PROCESS**************************/

    // wait for all children to terminate before exiting
    while (numChildrenTerminated < child_pid_length) {
        pause();
    }
    return 0;
}

void sigHandler(int sigNum) {
    signal(SIGCHLD, sigHandler); // re-register signal handler
    int child_status;
    pid_t pid;
    int fileDesc;

    pid = waitpid(-1, &child_status, WNOHANG);
    int pidIndex = getIndex(child_pid, pid);
    close(fd[pidIndex][1]); // close write end of pipe

    printf("Caught SIGCHLD from child %d: ", pid);
    if (child_status == 0) {
        int histArr[ALPHABET_SIZE];
        read(fd[pidIndex][0], histArr, sizeof(histArr)); // read histogram array from pipe

        int pidDigitCount = (int)((log10(pid)) + 1);              // calculate number of digits in each pid value
        char filename[4 + pidDigitCount + 6];                     // create filename string large enough for each pid value ("file" + pid_num + ".hist\0")
        snprintf(filename, sizeof(filename), "file%d.hist", pid); // create file name
        fileDesc = open(filename, O_RDWR | O_CREAT, 0640);        // open/create file to write histogram to

        // write histogram array to file
        int charCount;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            charCount = histArr[i] == 0 ? 1 : (int)((log10(histArr[i])) + 1); // calculate number of digits in each number for creating a string of the right size
            char fileStr[2 + charCount + 2];                                  // create string for each line in the file (ex: "a 1234" + \n + \0)
            snprintf(fileStr, sizeof(fileStr), "%c %d\n", i + 97, histArr[i]);
            write(fileDesc, fileStr, strlen(fileStr)); // write file string (histogram array) to file
        }
        printf("Child Terminated normally.\n");
        close(fileDesc);
    } else {
        printf("Child terminated with signal: %s \n", strsignal(WTERMSIG(child_status)));
    }

    close(fd[pidIndex][0]); // close read end of pipe
    numChildrenTerminated++;
}

int getIndex(int *arr, int element) {
    for (int i = 0; i < child_pid_length; i++) {
        if (arr[i] == element) {
            return i;
        }
    }

    return -1;
}