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

        /*
        TODO:
        The command line arguments passed to the parent process are treated as file names - with
        one exception. If a command line argument is the string SIG, the corresponding child will not
        receive the filename - instead, the parent will send SIGINT to it using the kill function. The
        child does not need to catch it - it should just terminate, using the default SIGINT disposition.

        For example, given the command:
        A2 file1.txt SIG file2.txt
        The child process 0 (the first fork) would be given file1.txt, child 1 would be sent SIGINT,
        and child 2 would be given file2.txt
        */
        if (strcmp(argv[i + 1], "SIG") == 0 && child_pid > 0) {
            printf("childpid: %d \n", child_pid);
            kill(child_pid, SIGINT);
        }

        char *filename = argv[i + 1];

        if (child_pid < 0) {
            printf("Could not create child process.\n");
        } else if (child_pid == 0) { // child process
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

                // free pipe array
                for (int i = 0; i < argc - 1; i++) {
                    free(fd[i]);
                }
                free(fd);
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

            // free pipe array
            for (int i = 0; i < argc - 1; i++) {
                free(fd[i]);
            }
            free(fd);
            free(fileStr);
            exit(0);
        }
    }
    /*********************PARENT PROCESS**************************/

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
    int fileDesc;

    pid = waitpid(-1, &child_status, WNOHANG);
    close(fd[numChildrenTerminated][1]); // close write end of pipe
    if (WIFSIGNALED(child_status)) {
        printf("%d: Child process %d was terminated by signal %s\n", numChildrenTerminated, pid, strsignal(WTERMSIG(child_status)));
    }
    if (WTERMSIG(child_status) == 0) {
        printf("%d: Caught SIGCHLD. Child %d terminated normally\n", numChildrenTerminated, pid);
        int histArr[ALPHABET_SIZE];
        read(fd[numChildrenTerminated][0], histArr, sizeof(histArr)); // read histogram array from pipe

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

        close(fileDesc);

    } else {
        /*TODO:
        - If the child terminated abnormally - either exited with a value other that 0, or was killed
          by a signal - the signal handler does not read the data, since there's nothing to read.
            - The examples discussed in class (and posted on the course website) showed you
              how to get the exit status of a process.
            - The macros WIFSIGNALED and WTERMSIG will help you figure out if the child was
              killed by a signal (and which one), and the function strsignal will provide the name
              of the signal given its code.
        */
        // printf("Caught SIGCHLD. Child %d terminated with signal %d and exit status %d\n", pid, sigNum, WEXITSTATUS(child_status));
    }

    close(fd[numChildrenTerminated][0]); // close read end of pipe
    numChildrenTerminated++;
}
