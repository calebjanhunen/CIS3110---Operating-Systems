#include <ctype.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int displayMenu(int outputMode);
int displayOptionMenu();
int secondaryDisplay();
char *decimalToHex(char *hexNum, int num);
void displayHex(char *buf, off_t fsize);
void displayAscii(char *buf, off_t fsize);

int main(int argc, char **argv) {
    int fd, outputMode = 0;
    char *buf, filename[PATH_MAX];
    off_t fsize; // file size

    int x = displayMenu(1); // 3 = exit;

    while (x != 3) {
        if (x == -2 || x == -1 || x == 0) { //-2 = invalid filename, -1 = invalid input entered, 0 = display menu again,
            if (fd != 0) {
                close(fd);
            }
            if (x == -2) {
                char errMess[] = "Cannot open file ";
                strcat(errMess, filename);

                write(STDOUT_FILENO, errMess, strlen(errMess));
                char *newline = " \n";
                write(STDOUT_FILENO, newline, strlen(newline));
            }
            x = displayMenu(outputMode);
        } else if (x == 2) { // display option menu;
            outputMode = 0;
            while (outputMode != 1 && outputMode != 2) { // 1 for ascii, 2 for hex
                outputMode = displayOptionMenu();
            }
            x = displayMenu(outputMode);
        } else if (x == 1) { // display file
            char *disp = "Enter file name: ";
            write(STDOUT_FILENO, disp, strlen(disp));
            ssize_t bytesRead = read(STDIN_FILENO, filename, PATH_MAX);
            filename[bytesRead - 1] = '\0'; // setting last index of string entered to null terminator

            /******************************Display File***********************/
            fd = open(filename, O_RDONLY); // open file with read only
            if (fd < 0) {                  // error when opening file
                x = -2;
            } else { // File successfully opened

                // malloc size of file
                fsize = lseek(fd, 0, SEEK_END);
                buf = (char *)malloc(fsize);
                lseek(fd, 0, SEEK_SET);

                read(fd, buf, fsize); // read file into buffer
                disp = "***************FILE CONTENT*************** \n\n";
                write(STDOUT_FILENO, disp, strlen(disp));

                if (outputMode == 1 || outputMode == 0) { // ascii
                    displayAscii(buf, fsize);
                } else if (outputMode == 2) { // hex
                    displayHex(buf, fsize);
                }
                disp = "\n\n***************END OF FILE CONTENT*************** \n\n";
                write(STDOUT_FILENO, disp, strlen(disp));
                free(buf);
                x = -1;
                while (x == -1) {
                    x = secondaryDisplay();
                }
            }
        }
    }

    close(fd);
    return 0;
}

int displayMenu(int outputMode) { // outputMode = 1 for ascii, 2 for hex
    char *menuDisplay, menuInputStr[400], menuInput;

    if (outputMode == 1 || outputMode == 0) {
        menuDisplay = "Current mode is ASCII... Press: \n'o' - to enter a file name \n'd' - to select display mode \n'x' - exit \n";
        write(STDOUT_FILENO, menuDisplay, strlen(menuDisplay));
        scanf("%s", menuInputStr);

    } else if (outputMode == 2) {
        menuDisplay = "Current mode is hex... Press: \n'o' - to enter a file name \n'd' - to select display mode \n'x' - exit \n";
        write(STDOUT_FILENO, menuDisplay, strlen(menuDisplay));
        scanf("%s", menuInputStr);
    }

    if (strlen(menuInputStr) != 1) {
        menuDisplay = "Not a valid input \n";
        write(STDOUT_FILENO, menuDisplay, strlen(menuDisplay));
        return -1;
    } else {
        menuInput = menuInputStr[0];
        menuInput = tolower(menuInput);
    }

    if (menuInput == 'o') { // display file
        return 1;

    } else if (menuInput == 'd') { // select display mode
        return 2;
    } else if (menuInput == 'x') { // exit
        return 3;
    } else { // invalid input entered
        menuDisplay = "Not a valid input \n";
        write(STDOUT_FILENO, menuDisplay, strlen(menuDisplay));
        return -1;
    }
    return 0;
}

int displayOptionMenu() {
    char *menuDisplay, dispOptInputStr[400], dispOptionInput;
    menuDisplay = "Enter: \n'a' for ASCII display \n'h' for hex display \n";
    write(STDOUT_FILENO, menuDisplay, strlen(menuDisplay));
    scanf("%s", dispOptInputStr);

    if (strlen(dispOptInputStr) != 1) {
        menuDisplay = "Not a valid input \n";
        write(STDOUT_FILENO, menuDisplay, strlen(menuDisplay));
        return -1;
    } else {
        dispOptionInput = dispOptInputStr[0];
        dispOptionInput = tolower(dispOptionInput);
    }
    if (dispOptionInput == 'a') {
        return 1;
    } else if (dispOptionInput == 'h') {
        return 2;
    }
    menuDisplay = "Not a valid input \n";
    write(STDOUT_FILENO, menuDisplay, strlen(menuDisplay));

    return -1;
}

int secondaryDisplay() {
    char inputStr[400], input;
    char *disp = "Input... \n'm' to return to the main menu \n'x' to exit \n";
    write(STDOUT_FILENO, disp, strlen(disp));
    scanf("%s", inputStr);

    if (strlen(inputStr) != 1) {
        return -1;
    } else {
        input = inputStr[0];
    }

    if (input == 'm') {
        return 0;
    } else if (input == 'x') {
        return 3;
    } else {
        return -1;
    }
    return 0;
}

void displayAscii(char *buf, off_t fsize) {
    for (int i = 0; i < fsize; i++) {
        if ((buf[i] >= 0 && buf[i] <= 9) || (buf[i] >= 11 && buf[i] <= 15)) {
            buf[i] = 32;
        }
        if (buf[i] >= 127 || buf[i] < 0) {
            buf[i] = 63;
        }
    }
    write(STDOUT_FILENO, buf, fsize); // write file to stdout
}

void displayHex(char *buf, off_t fsize) {
    char *newline = "\n";
    char *space = " ";
    char hexLineNum[8] = "0000000";
    char hexChar[3] = "00";

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
