// Simon Yoon
// ECE365 Operating Systems PS04Q4

#define _XOPEN_SOURCE 700
#define SZ_MAX 10000
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

/*
    gcc -o signal signal.c -std=c99
    ./signal 10 10 34      example inputs
    ./signal 10 10 2
*/
int sigcount = 0; 
int sigGen(int signum, int ppid, int m);
void sigHandler(int sig); 


int main(int argc, char *argv[])
{   
    int n, m, signum;
    pid_t cpid[SZ_MAX];

    struct sigaction sa; 
    sa.sa_handler=sigHandler;
    sa.sa_flags=SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    
    if(argv[1] && argv[2] && argv[3])
    {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        signum = atoi(argv[3]);
    }
    else
    {
        n = 1; // # of child processes
        m = 1; // # of signals sent
        signum = 0; // signal #
    }

    (void)sigaction(signum, &sa, NULL);

    for (int i = 0; i < n; i++) 
    {   
        int ppid; 
        switch (cpid[i] = fork())
        {
            case -1:
                fprintf(stderr, "Failed to exec: %s", strerror(errno));
                break;
            case 0:
                ppid = getppid();
                sigGen(signum, ppid, m);
                return 0; 
            case 1: 
                break; 
        } 
    }

    int status;
    pid_t pid;

    int tmp = n; 
    while (tmp > 0) 
    {
        pid = wait(&status);
        --tmp; 
    }
    printf("Signal Number: %d\nSignals delivered: %d\nSignals handled: %d\n", signum, n*m, sigcount);

    return 0; 
}

int sigGen(int sig, int ppid, int sigc)
{
    for(int i = 0; i<sigc; i++)
    {
        kill(ppid, sig);
    }
    return 0; 
}

void sigHandler(int sig)
{
    sigcount++;
}



