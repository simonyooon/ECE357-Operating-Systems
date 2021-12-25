#include "spinlock.h"
#include "sem.h"

int play(struct sem *from, struct sem *to, int moves)
{
    for(int i = 0; i<moves; i++)
    {
        sem_wait(from);
        sem_inc(to);
    }
    return 0; 
}

void usage() { 
    printf("Usage: ./shellgame.exe <pebbles> <moves>\n");
}

//Function to display the info at the end, after each child terminates
void disp(struct sem *tmp, int num)
{
    printf("%d\t\t%d\n", num, tmp->count);
    for(int i = 0; i<6; i++)
        printf("VCPU: %d\t\t\t\t%d\t\t%d\n", i, tmp->sleeps[i], tmp->wakes[i]);
    printf("\n");
}

//Function to display the info regarding the child, ie pid and 'task' number
//and number of times handler is invoked
void dispChild(int ret)
{
    printf("Child %d (pid %d) done - Signal handler was invoked %d times\n", my_procnum, getpid(), handlerCount);
    printf("VCPU: %d done\n", my_procnum);
    printf("Child pid %d exited w/ %d\n", getpid(), ret);
} 

int main(int argc, char *argv[])
{
    if(argc <= 1)
    {
        usage();
        return 1;
    }
    int pebbles, moves; 

    pebbles = atoi(argv[1]);
    moves = atoi(argv[2]);

    // initialize 3 semaphores in shared memory region
    struct sem *A = (struct sem*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    struct sem *B = (struct sem*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    struct sem *C = (struct sem*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    sem_init(A, pebbles);
    sem_init(B, pebbles);
    sem_init(C, pebbles);


    //Fork 6 processes to simulate different threads
    int n = 6; 
    pid_t childPids[n];

    for (int i = 0; i < n; i++) 
    {   
        int ppid; 
        switch (childPids[i] = fork())
        {
            case -1:
                fprintf(stderr, "Error processing fork: %s", strerror(errno));
                break;

            case 0:
                printf("VCPU %d starting, pid %d\n", i, getpid());
                my_procnum = i; 
                int ret; 
                switch (i)
                {
                    case 0: 
                        ret = play(A, B, moves);
                        dispChild(ret);
                        break;
                    case 1: 
                        ret = play(A, C, moves);
                        dispChild(ret);
                        break;
                    case 2:
                        ret = play(B, A, moves);
                        dispChild(ret);
                        break;
                    case 3: 
                        ret = play(B, C, moves);
                        dispChild(ret);
                        break;
                    case 4: 
                        ret = play(C, A, moves);
                        dispChild(ret);
                        break;
                    case 5:
                        ret = play(C, B, moves);
                        dispChild(ret);
                        break;
                }
                return 0; 

            case 1: 
                break; 
        } 
    }

    printf("Main process waiting for spawned children...\n");

    int status;
    pid_t pid;

    //Wait for each task to terminate
    int tmp = n; 
    while (tmp > 0) 
    {
        pid = wait(&status);
        --tmp;
    }
    
    //Print final info
    printf("Sem#\t\tval\t\tSleeps\t\tWakes\n");
    disp(A, 0);
    disp(B, 1);
    disp(C, 2);

}