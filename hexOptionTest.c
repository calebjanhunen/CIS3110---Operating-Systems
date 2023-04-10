#include <ctype.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *decimalToHex(char *hexNum, int num);

int main(int argc, char **argv) {
    char *buf, *filename = "stuff2.txt";
    int fd;
    off_t fsize;
    char *newline = "\n";
    char *space = " ";

    fd = open(filename, O_RDONLY); // open file with read only

    fsize = lseek(fd, 0, SEEK_END);
    buf = (char *)malloc(fsize);
    lseek(fd, 0, SEEK_SET);
    char hexLineNum[8] = "0000000";
    char hexChar[3] = "00";

    read(fd, buf, fsize); // read file into buffer
    // write(STDOUT_FILENO, buf, fsize); // write file to stdout
    // write(STDOUT_FILENO, newline, strlen(newline));
    for (int i = 0; i < fsize; i++) {
        int decimalVal = buf[i];

        if (i != 0 && i % 16 == 0) {
            write(STDOUT_FILENO, newline, strlen(newline));
        }

        if (i % 16 == 0) {
            write(STDOUT_FILENO, decimalToHex(hexLineNum, i), strlen(hexLineNum));
            write(STDOUT_FILENO, space, strlen(space));
        }

        write(STDOUT_FILENO, space, strlen(space));
        write(STDOUT_FILENO, decimalToHex(hexChar, decimalVal), strlen(hexChar));

        if ((i + 1) % 8 == 0) {
            write(STDOUT_FILENO, space, strlen(space));
        }
    }
    write(STDOUT_FILENO, newline, strlen(newline));
    write(STDOUT_FILENO, decimalToHex(hexLineNum, fsize), strlen(hexLineNum));
    write(STDOUT_FILENO, newline, strlen(newline));

    return 0;
}

char *decimalToHex(char *hexNum, int num) {
    // 0 out hexNum
    for (int i = 0; i < strlen(hexNum); i++) {
        hexNum[i] = '0';
    }

    int j = strlen(hexNum) - 1;
    long quotient = num, rem;
    while (quotient != 0) {
        rem = quotient % 16;
        if (rem < 10) {
            hexNum[j] = 48 + rem;

        } else {
            hexNum[j] = 87 + rem;
        }
        j--;
        quotient = quotient / 16;
    }
    return hexNum;
}