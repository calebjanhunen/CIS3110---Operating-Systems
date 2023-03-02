    #include <signal.h>
    #include <stdio.h>
    #include <unistd.h>

    void sigUsr(int sigNum);

    int main (void)
    {
    	if (signal(SIGUSR1, sigUsr) == SIG_ERR)
            fprintf(stderr, "Can't catch SIGUSR1");
    	if (signal(SIGUSR2, sigUsr) == SIG_ERR)
            fprintf(stderr, "Can't catch SIGUSR2");
    	if (signal(SIGINT, sigUsr) == SIG_ERR)
            fprintf(stderr, "Can't catch SIGINT");
    
        for (;;)
            //Wait until the signal handler has returned
            pause();
    }

    void sigUsr(int sigNum)
    {      
        /* re-install signal handling */
    	if (signal(SIGUSR1, sigUsr) == SIG_ERR)
            fprintf(stderr, "Can't catch SIGUSR1");
    	if (signal(SIGUSR2, sigUsr) == SIG_ERR)
            fprintf(stderr, "Can't catch SIGUSR2");
    	if (signal(SIGINT, sigUsr) == SIG_ERR)
            fprintf(stderr, "Can't catch SIGUSR2");
    
        /* Process the signal */
        if (sigNum == SIGUSR1)
            printf("received SIGUSR1\n");
        else if (sigNum == SIGUSR2)
            printf("received SIGUSR2\n");
        else if (sigNum == SIGINT)
            printf("Tried to kill me, eh? Not so fast, buddy!\n");
        else
            fprintf(stderr, "Received signal %d\n", sigNum);
    }



