#ifndef SEM_H
#define SEM_H

#include "spinlock.h"

#define N_PROC 64

int my_procnum;
int handlerCount;

struct node
{
    int pid; 
    int procnum; 
    struct node *next; 
};

struct sem {
    char lock;
    int count;
    int sleeps[6];
    int wakes[6];
    int handlerCount[6];
    
    struct node pidtable[N_PROC];
    struct node *start; 
    struct node *end; 
    int length; 
};

/* Initialize the semaphore *s with the initial count. Initialize
any underlying data structures. sem_init should only be called
once in the program (per semaphore). If called after the
semaphore has been used, results are unpredictable. */
void sem_init(struct sem *s, int count);

/* Attempt to perform the "P" operation (atomically decrement
the semaphore). If this operation would block, return 0,
otherwise return 1. */
int sem_try(struct sem *s);

/* Perform the P operation, blocking until successful. */
void sem_wait(struct sem *s);

/* Perform the V operation. If any other tasks were sleeping
on this semaphore, wake them by sending a SIGUSR1 to their
process id (which is not the same as the virtual processor number).
If there are multiple sleepers (this would happen if multiple
virtual processors attempt the P operation while the count is <1)
then all must be sent the wakeup signal. */
void sem_inc(struct sem *s);

// Linked list manipulation 

//insert
void insert(struct node *n, struct sem *s);
//remove
struct node* rmv(struct sem *s);

#endif