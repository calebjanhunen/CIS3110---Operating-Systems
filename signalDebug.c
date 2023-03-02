#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void toggle_debug(int signo);

static int debug = 0;  /* initialize here */
int main(){
    int i = 0;
	
    if (signal(SIGUSR2, toggle_debug)  == SIG_ERR)
        fprintf(stderr, "Can't catch SIGUSR2");

    while(1){
        i++;
        if (debug == 1){
            fprintf(stderr, "i = %d\n",i);
        }
    }
}

void toggle_debug(int signo){
	signal(SIGUSR2, toggle_debug); //reset handler
    //toggle debug
    if (debug == 1)
        debug = 0;
    else
        debug = 1;
}






